#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <justaman/containers/vector.hpp>
#include <justaman/cryptography/hashing.hpp>
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>

namespace jman 
{
    /**
        @deprecated:
            this class is not very safe and its pending for renovation in the future

        @brief: 
            simple flat hash map with robin hood hasing and without SIMD instructions, use Google's impl for more complex and fast results
        Iterators:
            iterators will invalidate when the rehashing.
            iterators could invalidate when inserting (cuz of the robin hood swaps) or removing (if the iterator point to the removed element or any of the items after that)
        Exception handling:
            Rollback when an exception is thrown for any constructor is only guarenteed for the copy operations of this class.
            Operations like inserting or removing don't have this protections because it would need to track every swap or create a temp buffer
            for the elements, so instead these operations are marked as noexcept, if an exception is thrown by any constructor, copy or move while inserting or removing,
            it will call std::terminate() to finish the program.
        @warning: 
            since this hash map doesnt use internally pairs to store keys and values, when dereferencing an iterator it returns a new constructed pair, 
            also -> operator doesnt work
        @brief:
            Robing hood hasing tells that elements near their ideal hash position r anomenated "rich" elements, elements far away from their hasing r called "poor"
            \n
            this is just "let the poor steal from the rich to stay equal", so when an inserting point has a psl (probe sequence length / ideal hash pos distance from current)
            higher than the current index, we swap them, and keep going until we actually reach the a dead spot


        @todo: ADD MORE DYNAMIC, CLEAN CODE +  ADD EXTRACT() METHOD LIKE std::unordered_map, ADD BETTER TYPES FOR RANGES 
                AND ACTUALLY USE STD::EQUAL_TO<K> AND ADD IT ON THE TEMPLATE AND MAKE CUSTOM HASH TO AVOID USING THE STANDARD VERSION
    */
    template <typename K, typename V, class Hash = std::hash<K>>
    class JMAN_DEPRECATED flat_umap 
    {
        static_assert(!std::is_reference_v<K> && !std::is_reference_v<V>);
        static_assert(!std::is_void_v<K> && !std::is_void_v<V>);

        public:
            using key_type          = K;
            using mapped_type       = V;
            using value_type        = V;
            using size_type         = std::size_t;
            using difference_type   = std::ptrdiff_t;
            using hasher            = Hash;
            using key_equal         = std::equal_to<K>;
            using reference         = V&;
            using const_reference   = const V&;
            using pointer           = V*;
            using const_pointer     = const V*;

        private:
            using psl_t     = std::uint8_t;
            using h2_t      = std::int8_t;

            static constexpr h2_t EMPTY = -1;
            static constexpr size_type MINIMUM_CAPACITY = 16;

            struct slot 
            {
                alignas(K) std::byte key[sizeof(K)];
                alignas(V) std::byte val[sizeof(V)];
            }; // unitialized aligned memory

            struct ctrl_t 
            {
                psl_t psl = 0;
                h2_t signature = EMPTY; // h2 fingerprint
            };
            
            jman::vector<ctrl_t> m_ctrl; // slots info
            jman::vector<slot> m_slots; // actual data

            size_type m_size = 0;
            size_type m_capacity = 0; 

        private:
            JMAN_FORCE_INLINE K& __key(slot& sl) noexcept
            {
                return *std::launder(reinterpret_cast<K*>(sl.key));
            }

            JMAN_FORCE_INLINE V& __value(slot& sl) noexcept
            {
                return *std::launder(reinterpret_cast<V*>(sl.val));
            }

            JMAN_FORCE_INLINE const K& __key(const slot& sl) const noexcept
            {
                return *std::launder(reinterpret_cast<const K*>(sl.key));
            }

            JMAN_FORCE_INLINE const V& __value(const slot& sl) const noexcept
            {
                return *std::launder(reinterpret_cast<const V*>(sl.val));
            }

            JMAN_FORCE_INLINE bool __needs_rehash() const noexcept
            {
                return (m_size + 1) * 5 > m_capacity * 4; // 0.8 = 4 / 5 for avoiding floats operations
            }

            JMAN_FORCE_INLINE void __grow()
            {
                if (!__needs_rehash()) return;
                this->rehash(jman::max(m_capacity * 2, MINIMUM_CAPACITY));
            }

            /**
                @brief
                    kinda like a filter to avoid comparing complex keys all the time
                    some hashes r very unique and uniform at the top bits, but much more sqatered and predictable a the bottom bits
                    since the fast-range reduction grabs the top bits from the hash aswell we can simply 
            */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE static h2_t get_h2_signature(std::size_t hash) noexcept
            {
                return static_cast<h2_t>((hash >> 30) & 0x7F);
            }

            JMAN_FORCE_INLINE void destroy_slot(slot& sl) noexcept
            {
                jman::destruct_at<K>(jman::addressof(__key(sl)));
                jman::destruct_at<V>(jman::addressof(__value(sl)));
            }

            void move_slot(slot& src, slot& dst) noexcept
            {
                K& srcKey = __key(src);
                K& dstKey = __key(dst);
                jman::destruct_at(jman::addressof(dstKey));
                jman::construct_at<K>(jman::addressof(dstKey), std::move_if_noexcept(srcKey));
                jman::destruct_at(jman::addressof(srcKey));

                V& srcVal = __value(src);
                V& dstVal = __value(dst);
                jman::destruct_at(jman::addressof(dstVal));
                jman::construct_at<V>(jman::addressof(dstVal), std::move_if_noexcept(srcVal));
                jman::destruct_at(jman::addressof(srcVal));
            }
            
            template <typename KK, typename VV>
            requires std::convertible_to<KK, K> && std::convertible_to<VV, V>
            void unchecked_assign(KK&& key, VV&& value) noexcept;

            template <typename KK>
            requires std::convertible_to<KK, K>
            slot* find_or_insert_default(KK&& key) noexcept;
        
        public:
            struct const_iterator 
            {
                private:
                    const flat_umap* m_map;
                    mutable size_type m_index;

                    JMAN_FORCE_INLINE void __skip_empty_inc() noexcept {
                        while (m_index < m_map->m_capacity && m_map->m_ctrl[m_index].signature == flat_umap::EMPTY)
                            ++m_index;
                    }

                    JMAN_FORCE_INLINE void __skip_empty_dec() noexcept {
                        while (m_index > 0 && m_map->m_ctrl[m_index].signature == flat_umap::EMPTY)
                            --m_index;
                    }

                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using value_type        = std::pair<const K&, const V&>;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = const value_type*;
                    using reference         = const value_type&;

                    const_iterator() { __skip_empty_inc(); }
                    explicit const_iterator(const flat_umap* ptr, size_type index): m_map(ptr), m_index(index) { __skip_empty_inc(); }

                    JMAN_FORCE_INLINE const_iterator& operator++() const noexcept { ++m_index; __skip_empty_inc(); return *this; }
                    JMAN_FORCE_INLINE const_iterator operator++(int) const noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE const_iterator& operator--() const noexcept { --m_index; __skip_empty_dec(); return *this; }
                    JMAN_FORCE_INLINE const_iterator operator--(int) const noexcept { iterator retval = *this; --(*this); return retval; }
                    
                    JMAN_FORCE_INLINE const_iterator operator+(difference_type index) const noexcept { return const_iterator(m_index + index); }
                    JMAN_FORCE_INLINE const_iterator operator-(difference_type index) const noexcept { return const_iterator(m_index - index); }
                    
                    JMAN_FORCE_INLINE difference_type operator-(const const_iterator& other) const noexcept { return m_index - other.m_index; }
                    
                    JMAN_FORCE_INLINE const_iterator& operator+=(difference_type diff) const noexcept { m_index += diff; return *this; }
                    JMAN_FORCE_INLINE const_iterator& operator-=(difference_type diff) const noexcept { m_index -= diff; return *this; }
                    friend JMAN_FORCE_INLINE const_iterator operator+(difference_type diff, const_iterator it) noexcept { return {diff + it.m_index}; }

                    JMAN_FORCE_INLINE value_type operator*() const { 
                        auto& slot = m_map->m_slots[m_index]; 
                        return {m_map->__key(slot), m_map->__value(slot)};
                    }

                    JMAN_FORCE_INLINE bool operator==(const const_iterator& other) const noexcept { return m_index == other.m_index && m_map == other.m_map; }
                    JMAN_FORCE_INLINE bool operator!=(const const_iterator& other) const noexcept { return m_index != other.m_index || m_map != other.m_map; }

                    JMAN_FORCE_INLINE bool operator>=(const const_iterator& other) const noexcept { return m_index >= other.m_index; }
                    JMAN_FORCE_INLINE bool operator>(const const_iterator& other) const noexcept { return m_index > other.m_index; }
                    JMAN_FORCE_INLINE bool operator<=(const const_iterator& other) const noexcept { return m_index <= other.m_index; }
                    JMAN_FORCE_INLINE bool operator<(const const_iterator& other) const noexcept { return m_index < other.m_index; }
            };

            struct iterator 
            {
                private:
                    const flat_umap* m_map;
                    size_type m_index;

                    JMAN_FORCE_INLINE void __skip_empty_inc() noexcept {
                        while (m_index < m_map->m_capacity && m_map->m_ctrl[m_index].signature == flat_umap::EMPTY)
                            ++m_index;
                    }

                    JMAN_FORCE_INLINE void __skip_empty_dec() noexcept {
                        while (m_index > 0 && m_map->m_ctrl[m_index].signature == flat_umap::EMPTY)
                            --m_index;
                    }

                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using value_type        = std::pair<const K&, V&>;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = value_type*;
                    using reference         = value_type&;

                    iterator() { __skip_empty_inc(); }
                    explicit iterator(flat_umap* ptr, size_type index): m_map(ptr), m_index(index) { __skip_empty_inc(); }

                    JMAN_FORCE_INLINE iterator& operator++() noexcept { ++m_index; __skip_empty_inc(); return *this; }
                    JMAN_FORCE_INLINE iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE iterator& operator--() noexcept { --m_index; __skip_empty_dec(); return *this; }
                    JMAN_FORCE_INLINE iterator operator--(int) noexcept { iterator retval = *this; --(*this); return retval; }
                    
                    JMAN_FORCE_INLINE iterator operator+(difference_type index) const noexcept { return iterator(m_index + index); }
                    JMAN_FORCE_INLINE iterator operator-(difference_type index) const noexcept { return iterator(m_index - index); }
                    
                    JMAN_FORCE_INLINE difference_type operator-(const iterator& other) const noexcept { return m_index - other.m_index; }
                    
                    JMAN_FORCE_INLINE iterator& operator+=(difference_type diff) noexcept { m_index += diff; return *this; }
                    JMAN_FORCE_INLINE iterator& operator-=(difference_type diff) noexcept { m_index -= diff; return *this; }
                    friend JMAN_FORCE_INLINE iterator operator+(difference_type diff, iterator it) noexcept { return {diff + it.m_index}; }

                    JMAN_FORCE_INLINE value_type operator*() const { 
                        auto& slot = m_map->m_slots[m_index]; 
                        return {m_map->__key(slot), const_cast<V&>(m_map->__value(slot))};
                    }

                    JMAN_FORCE_INLINE bool operator==(const iterator& other) const noexcept { return m_index == other.m_index && m_map == other.m_map; }
                    JMAN_FORCE_INLINE bool operator!=(const iterator& other) const noexcept { return m_index != other.m_index || m_map != other.m_map; }

                    JMAN_FORCE_INLINE bool operator>=(const iterator& other) const noexcept { return m_index >= other.m_index; }
                    JMAN_FORCE_INLINE bool operator>(const iterator& other) const noexcept { return m_index > other.m_index; }
                    JMAN_FORCE_INLINE bool operator<=(const iterator& other) const noexcept { return m_index <= other.m_index; }
                    JMAN_FORCE_INLINE bool operator<(const iterator& other) const noexcept { return m_index < other.m_index; }

                    operator const_iterator() const noexcept { return {m_map, m_index}; };
            };

            struct node_type 
            {

            };

            flat_umap() = default;
            flat_umap(std::initializer_list<std::pair<K, V>> list);
            explicit flat_umap(std::size_t capacity);
            
            flat_umap(const flat_umap& other);
            flat_umap& operator=(const flat_umap& other);
            
            flat_umap(flat_umap&& other) noexcept;
            flat_umap& operator=(flat_umap&& other) noexcept;
            
            ~flat_umap() noexcept;

            // ----------------- GETTERS ------------- //
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type      capacity() const noexcept { return m_capacity; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type      size() const noexcept { return m_size; }
            /** @warning: this method is just an aproximation */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type      max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(slot); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool           empty() const noexcept { return m_size == 0; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator       begin() noexcept { return iterator{this, 0}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator       end() noexcept { return iterator{this, m_capacity}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator begin() const noexcept { return const_iterator{this, 0}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator end() const noexcept { return const_iterator{this, m_capacity}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator cbegin() const noexcept { return const_iterator {this, 0}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator cend() const noexcept { return const_iterator {this, m_capacity}; }

            // ----------------- LOOKUP ------------- //

            JMAN_NO_DISCARD reference                      at(const key_type& key);  
            JMAN_NO_DISCARD const_reference                at(const key_type& key) const;  
            template<typename KK>
            requires std::convertible_to<KK, K>
            JMAN_NO_DISCARD reference                      operator[](KK&& key);
            JMAN_NO_DISCARD bool                           contains(const K& key) const noexcept;
            JMAN_NO_DISCARD pointer                        find(const key_type& key) noexcept;
            JMAN_NO_DISCARD const_pointer                  find(const key_type& key) const noexcept;

            // -------------- HASHING ----------------//

            JMAN_NO_DISCARD JMAN_FORCE_INLINE float        load_factor() const noexcept { return m_size / static_cast<float>(m_capacity); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE float        max_load_factor() const noexcept { return 0.8f; }
            void                                           rehash(size_type count);
            void                                           reserve(size_type count);

            // -------------- MODIFIERS ----------------//

            template<typename KK, typename VV>
            requires std::convertible_to<KK, K> && std::convertible_to<VV, V>
            void                    assign(KK&& key, VV&& value);
            template <typename KK, typename ...Args>
            requires std::convertible_to<KK, K>
            void                    emplace(KK&& key, Args&&... args);
            void                    erase(const key_type& key) noexcept; // much safer and faster if we make the method directly noexcept
            void                    clear() noexcept;
    };


};

// ##################################################################################################################
// -------------------------------------------- PRIVATE / HIDDEN METHODS ------------------------------------------//
// ##################################################################################################################

template <typename K, typename V, class Hash>
template <typename KK, typename VV>
requires std::convertible_to<KK, K> && std::convertible_to<VV, V>
void jman::flat_umap<K, V, Hash>::unchecked_assign(KK&& key, VV&& value) noexcept
{
    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    ctrl_t incoming_ctrl {0, get_h2_signature(hash)};
    slot incoming_slot {};
    bool slot_already_in = false;

    while (true) {
        slot& current_slot = m_slots[index];
        ctrl_t& current_ctrl = m_ctrl[index];

        // emtpy slot
        if (current_ctrl.signature == EMPTY) { 
            if (!slot_already_in) {
                jman::construct_at<K>(reinterpret_cast<K*>(current_slot.key), std::forward<KK>(key));
                jman::construct_at<V>(reinterpret_cast<V*>(current_slot.val), std::forward<VV>(value));
            } else {
                jman::construct_at<K>(reinterpret_cast<K*>(current_slot.key), std::move_if_noexcept(__key(incoming_slot)));
                jman::construct_at<V>(reinterpret_cast<V*>(current_slot.val), std::move_if_noexcept(__value(incoming_slot)));

                destroy_slot(incoming_slot);
            }
            current_ctrl = incoming_ctrl;
            ++m_size;
            return;
        }

        // already exists so just update the value
        bool signature_match = current_ctrl.signature == incoming_ctrl.signature;
        if (signature_match && __key(current_slot) == (slot_already_in ? __key(incoming_slot) : key)) {
            if (!slot_already_in) {
                __value(current_slot) = std::forward<VV>(value);
            } else {
                __value(current_slot) = std::move_if_noexcept(__value(incoming_slot));
                destroy_slot(incoming_slot);
            }
            return;
        }

        // robin hood swap  
        if (incoming_ctrl.psl > current_ctrl.psl) {
            if (!slot_already_in) {
                jman::construct_at<K>(reinterpret_cast<K*>(incoming_slot.key), std::forward<KK>(key));
                jman::construct_at<V>(reinterpret_cast<V*>(incoming_slot.val), std::forward<VV>(value));
                slot_already_in = true;
            }
            std::swap(__value(incoming_slot), __value(current_slot));
            std::swap(__key(incoming_slot), __key(current_slot));
            std::swap(incoming_ctrl, current_ctrl);
        }

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++incoming_ctrl.psl;
    }
}

template <typename K, typename V, class Hash>
template <typename KK>
requires std::convertible_to<KK, K>
jman::flat_umap<K, V, Hash>::slot* jman::flat_umap<K, V, Hash>::find_or_insert_default(KK&& key) noexcept
{
    __grow();
    
    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    ctrl_t incoming_ctrl {0, get_h2_signature(hash)};
    slot incoming_slot {};
    bool slot_already_in = false;

    while (true) {
        slot& current_slot = m_slots[index];
        ctrl_t& current_ctrl = m_ctrl[index];

        // emtpy slot
        if (current_ctrl.signature == EMPTY) { 
            if (!slot_already_in) {
                jman::construct_at<K>(reinterpret_cast<K*>(current_slot.key), std::forward<KK>(key));
                jman::construct_at<V>(reinterpret_cast<V*>(current_slot.val));
            } else {
                jman::construct_at<K>(reinterpret_cast<K*>(current_slot.key), std::move_if_noexcept(__key(incoming_slot)));
                jman::construct_at<V>(reinterpret_cast<V*>(current_slot.val), std::move_if_noexcept(__value(incoming_slot)));

                destroy_slot(incoming_slot);
            }
            current_ctrl = incoming_ctrl;
            ++m_size;
            return &current_slot;
        }

        // already exists so just return the value
        bool signature_match = current_ctrl.signature == incoming_ctrl.signature;
        if (signature_match && __key(current_slot) == (slot_already_in ? __key(incoming_slot) : key)) {
            return &current_slot;
        }

        // robin hood swap  
        if (incoming_ctrl.psl > current_ctrl.psl) {
            if (!slot_already_in) {
                jman::construct_at<K>(reinterpret_cast<K*>(incoming_slot.key), std::forward<KK>(key));
                jman::construct_at<V>(reinterpret_cast<V*>(incoming_slot.val));
                slot_already_in = true;
            }
            std::swap(__value(incoming_slot), __value(current_slot));
            std::swap(__key(incoming_slot), __key(current_slot));
            std::swap(incoming_ctrl, current_ctrl);
        }

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++incoming_ctrl.psl;
    }
}

// #####################################################################################################################
// -------------------------------------------- CONSTRUCTORS / DESTRUCTORS ------------------------------------------//
// #####################################################################################################################

template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>::flat_umap(std::initializer_list<std::pair<K, V>> list)
{
    m_slots.resize(std::max(list.size(), MINIMUM_CAPACITY));
    m_ctrl.resize(std::max(list.size(), MINIMUM_CAPACITY));

    for (auto [key, value]: list)
        unchecked_assign(key, value);
}

template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>::flat_umap(size_type capacity)
{
    m_slots.resize(std::max(capacity, MINIMUM_CAPACITY));
    m_ctrl.resize(std::max(capacity, MINIMUM_CAPACITY));
}


template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>::flat_umap(const flat_umap& other)
{   
    m_slots.resize(other.m_capacity);

    if constexpr (std::is_nothrow_copy_constructible_v<K> && std::is_nothrow_copy_constructible_v<V>) {
        for (std::size_t i = 0; i < other.m_capacity; ++i)
        {
            ctrl_t ctrl = other.m_ctrl[i];
            slot& slot = other.m_slots[i];

            if (ctrl.signature >= 0) {
                jman::construct_at<K>(m_slots[i].key, __key(slot));
                jman::construct_at<V>(m_slots[i].val, __value(slot));
            }
        }
        
        m_ctrl = other.m_ctrl;
    } else {
        m_ctrl.resize(other.m_capacity);

        try {
            for (std::size_t i = 0; i < other.m_capacity; ++i)
            {
                ctrl_t ctrl = other.m_ctrl[i];
                slot& slot = other.m_slots[i];

                if (ctrl.signature >= 0) {
                    jman::construct_at<K>(m_slots[i].key, __key(slot));
                    jman::construct_at<V>(m_slots[i].val, __value(slot));
                    m_ctrl[i] = other.m_ctrl[i];
                }
            }
        } catch(...) {
            for (std::size_t i = 0; i < other.m_capacity; ++i)
            {
                ctrl_t ctrl = m_ctrl[i];
                slot& slot = other.m_slots[i];

                if (ctrl.signature >= 0) {
                    jman::destruct_at<K>(m_slots[i].key);
                    jman::destruct_at<V>(m_slots[i].val);
                }
            }
            m_slots.resize(0);
            m_ctrl.resize(0);

            throw;
        }
    }

    m_size = other.m_size;
    m_capacity = other.m_capacity;
}

template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>& jman::flat_umap<K, V, Hash>::operator=(const flat_umap& other)
{
    if (this != &other)
    {
        clear();

        m_slots.resize(other.m_capacity);
        if constexpr (std::is_nothrow_copy_constructible_v<K> && std::is_nothrow_copy_constructible_v<V>) {
            for (std::size_t i = 0; i < other.m_capacity; ++i)
            {
                ctrl_t ctrl = other.m_ctrl[i];
                slot& slot = other.m_slots[i];

                if (ctrl.signature >= 0) {
                    jman::construct_at<K>(m_slots[i].key, __key(slot));
                    jman::construct_at<V>(m_slots[i].val, __value(slot));
                }
            }
            
            m_ctrl = other.m_ctrl;
        } else {
            m_ctrl.resize(other.m_capacity);

            try {
                for (std::size_t i = 0; i < other.m_capacity; ++i)
                {
                    ctrl_t ctrl = other.m_ctrl[i];
                    slot& slot = other.m_slots[i];

                    if (ctrl.signature >= 0) {
                        jman::construct_at<K>(m_slots[i].key, __key(slot));
                        jman::construct_at<V>(m_slots[i].val, __value(slot));
                        m_ctrl[i] = other.m_ctrl[i];
                    }
                }
            } catch(...) {
                for (std::size_t i = 0; i < other.m_capacity; ++i)
                {
                    ctrl_t ctrl = m_ctrl[i];
                    slot& slot = other.m_slots[i];

                    if (ctrl.signature >= 0) {
                        jman::destruct_at<K>(m_slots[i].key);
                        jman::destruct_at<V>(m_slots[i].val);
                    }
                }
                m_slots.resize(0);
                m_ctrl.resize(0);

                m_size = 0;
                m_capacity = 0;

                throw;
            }
        }

        m_size = other.m_size;
        m_capacity = other.m_capacity;
    }

    return *this;
}



template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>::flat_umap(flat_umap&& other) noexcept
{
    m_slots.resize(other.m_capacity);

    for (std::size_t i = 0; i < other.m_capacity; ++i)
    {
        ctrl_t ctrl = other.m_ctrl[i];
        slot& slot = other.m_slots[i];

        if (ctrl.signature >= 0) {
            __key(m_slots[i]) = std::move_if_noexcept(__key(slot));
            __value(m_slots[i]) = std::move_if_noexcept(__value(slot));

            destroy_slot(slot);
        }
    }

    m_ctrl = std::move(other.m_ctrl);

    m_size = other.m_size;
    m_capacity = other.m_capacity;

    other.m_size = 0;
    other.m_capacity = 0;
    other.m_slots.resize(0);
}

template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>& jman::flat_umap<K, V, Hash>::operator=(flat_umap&& other) noexcept
{
    if (this != &other)
    {
        clear();

        m_slots.resize(other.m_capacity);
        for (std::size_t i = 0; i < other.m_capacity; ++i)
        {
            ctrl_t ctrl = other.m_ctrl[i];
            slot& slot = other.m_slots[i];

            if (ctrl.signature >= 0) {
                __key(m_slots[i]) = std::move_if_noexcept(__key(slot));
                __value(m_slots[i]) = std::move_if_noexcept(__value(slot));

                destroy_slot(slot);
            }
        }

        m_ctrl = std::move(other.m_ctrl);
        m_size = other.m_size;
        m_capacity = other.m_capacity;
    }

    return *this;
}




template <typename K, typename V, class Hash>
jman::flat_umap<K, V, Hash>::~flat_umap() noexcept
{
    clear();
}

// ##################################################################################################
// -------------------------------------------- LOOK UPS ------------------------------------------//
// ##################################################################################################

template <typename K, typename V, class Hash>
V& jman::flat_umap<K, V, Hash>::at(const key_type& key)
{
    if JMAN_UNLIKELY(empty()) 
        throw std::out_of_range("jman::flat_umap<K, V>::at() failed: tried to grab from an empty map");

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || ctrl.psl < psl) 
            throw std::out_of_range("jman::flat_umap<K, V>::at() failed: no value found for the provided key.");

        if (ctrl.signature == h2 && __key(m_slots[index]) == key)
            return __value(m_slots[index]);

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

template <typename K, typename V, class Hash>
const V& jman::flat_umap<K, V, Hash>::at(const K& key) const
{
    if JMAN_UNLIKELY(empty()) 
        throw std::out_of_range("jman::flat_umap<K, V>::at() failed: tried to grab from an empty map");

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || ctrl.psl < psl) 
            throw std::out_of_range("jman::flat_umap<K, V>::at() failed: no value found for the provided key.");

        if (ctrl.signature == h2 && __key(m_slots[index]) == key)
            return __value(m_slots[index]);

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

template <typename K, typename V, class Hash>
template<typename KK>
requires std::convertible_to<KK, K>
V& jman::flat_umap<K, V, Hash>::operator[](KK&& key)
{
    return __value(*find_or_insert_default(static_cast<K>(std::forward<KK>(key))));
}

template <typename K, typename V, class Hash>
bool jman::flat_umap<K, V, Hash>::contains(const K& key) const noexcept
{
    if JMAN_UNLIKELY(empty()) 
        return false;

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || ctrl.psl < psl) 
            return false;

        if (ctrl.signature == h2 && __key(m_slots[index]) == key)
            return true;

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

template <typename K, typename V, class Hash>
V* jman::flat_umap<K, V, Hash>::find(const key_type& key) noexcept
{
    if JMAN_UNLIKELY(empty()) 
        return nullptr;

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || ctrl.psl < psl) 
            return nullptr;

        if (ctrl.signature == h2 && __key(m_slots[index]) == key)
            return &__value(m_slots[index]);

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

template <typename K, typename V, class Hash>
const V* jman::flat_umap<K, V, Hash>::find(const K& key) const noexcept
{   
    if JMAN_UNLIKELY(empty()) 
        return nullptr;

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || ctrl.psl < psl) 
            return nullptr;

        if (ctrl.signature == h2 && __key(m_slots[index]) == key)
            return &__value(m_slots[index]);

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

// ##################################################################################################
// -------------------------------------------- HASHING ------------------------------------------//
// ##################################################################################################

template <typename K, typename V, class Hash>
void jman::flat_umap<K, V, Hash>::rehash(std::size_t count)
{
    if JMAN_UNLIKELY(count == m_capacity) return;

    jman::vector<slot> old_slots = std::move(m_slots);
    jman::vector<ctrl_t> old_ctrl = std::move(m_ctrl);

    m_capacity = count;
    m_slots.assign(m_capacity, {});
    m_ctrl.assign(m_capacity, {});
    
    if (m_size == 0) return;
    m_size = 0;

    for (std::size_t i = 0; i < old_slots.size(); ++i)
    {
        ctrl_t ctrl = old_ctrl[i];
        slot& slot = old_slots[i];

        if (ctrl.signature >= 0 && m_size < m_capacity) {
            unchecked_assign(std::move_if_noexcept(__key(slot)), std::move_if_noexcept(__value(slot)));
            destroy_slot(slot);
        } else if (ctrl.signature >= 0) {
            destroy_slot(slot); // truncate and destroy old object
        }
    }
}

template <typename K, typename V, class Hash>
void jman::flat_umap<K, V, Hash>::reserve(std::size_t count)
{
    if (count <= m_capacity) return;
    rehash(count);
}

// ##################################################################################################
// -------------------------------------------- MODIFIERS -----------------------------------------//
// ##################################################################################################

template <typename K, typename V, class Hash>
template<typename KK, typename VV>
requires std::convertible_to<KK, K> && std::convertible_to<VV, V>
void jman::flat_umap<K, V, Hash>::assign(KK&& key, VV&& value)
{
    __grow();
    unchecked_assign(std::forward<KK>(key), static_cast<V>(std::forward<VV>(value)));
}

template <typename K, typename V, class Hash>
template <typename KK, typename ...Args>
requires std::convertible_to<KK, K>
void jman::flat_umap<K, V, Hash>::emplace(KK&& key, Args&&... args)
{
    __grow();
    unchecked_assign(static_cast<K>(std::forward<KK>(key)), V {std::forward<Args>(args)...});
}

template <typename K, typename V, class Hash>
void jman::flat_umap<K, V, Hash>::erase(const K& key) noexcept
{   
    if (empty()) return;

    std::size_t hash = jman::hash64_murmur3_mix(Hash{}(key));
    h2_t h2 = get_h2_signature(hash);
    psl_t psl = 0;
    std::size_t index = jman::hash64_fast_range(hash, m_capacity);

    /*
        Erasing an element is just shifting back the elements on the same bucket
    */

    while (true) {
        ctrl_t ctrl = m_ctrl[index];

        if (ctrl.signature == EMPTY || psl > ctrl.psl) 
            return;

        if (ctrl.signature == h2 && __key(m_slots[index]) == key) {
            std::size_t next = index + 1;
            if JMAN_UNLIKELY(next >= m_capacity) next = 0;

            while (m_ctrl[next].signature != EMPTY && m_ctrl[next].psl > 0) {
                move_slot(m_slots[next], m_slots[index]);

                m_ctrl[index] = m_ctrl[next];
                --m_ctrl[index].psl;
                index = next;
                ++next;
                if JMAN_UNLIKELY(next >= m_capacity) next = 0;
            }

            destroy_slot(m_slots[index]);
            m_ctrl[index] = {};

            --m_size;
            return;
        }

        ++index;
        if JMAN_UNLIKELY(index >= m_capacity) index = 0;
        ++psl;
    }
}

template <typename K, typename V, class Hash>
void jman::flat_umap<K, V, Hash>::clear() noexcept
{
    for (std::size_t i = 0; i < m_capacity; ++i)
    {
        slot& slot = m_slots[i];
        ctrl_t ctrl = m_ctrl[i];

        if (ctrl.signature == EMPTY) continue;
        destroy_slot(slot);

        m_ctrl[i] = {};
    }

    m_size = 0;
}