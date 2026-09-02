#pragma once

#include "justaman/containers/vector.hpp"
#include <array>
#include <bit>
#include <concepts>
#include <limits>

namespace jman 
{
    /**
        @brief
            A class to wrap keys (indices) to values keeping cache friendliness and O(1) insertion / removal / search
            also uses the pagenation technique to avoid a lot of empty gaps between values and uses std::array
            \n
            the pages size must be a power of two so isntead of using regular math for calculating page id and stuff
            we just need to do some bits operations
            \n
            trick found on a yt comment from an ECS tutorial:
            \n
            page_index = key >> L; page_offset = key - (key << L) r id & ( PAGE_SIZE - 1). If pagesize is 256 then L=8 etc
    */
    template <typename V, std::unsigned_integral K = std::size_t, std::size_t PageSize = 128>
    class sparse_set 
    {
        public:
            using value_type = V;
            using size_type = K;
            using difference_type = std::ptrdiff_t;
            using reference = V&;
            using const_reference = const V&;
            using pointer = V*;
            using const_pointer = const V*;

            static constexpr size_type INVALID_INDEX { std::numeric_limits<size_type>::max() }; // invalid sparse & dense index
            static constexpr size_type PAGE_SIZE { PageSize }; 
            static constexpr size_type PAGE_MASK { PAGE_SIZE - 1 };
            static constexpr size_type L = std::bit_width(PAGE_SIZE) - 1; // similar to std::log2 but its a valid constexpr

            static_assert(
                std::has_single_bit(PAGE_SIZE), 
                "PageSize must be a power of two"
            );  // make sure  PAGE_SIZE is single bit (true if power of 2)
            static_assert(
                PAGE_SIZE <= std::numeric_limits<size_type>::max(), 
                "PageSize cannot be larger than the maximum value of Key type K"
            ); 
            // fix issue for when sparse indices could overflow cuz of size_type capacities ex. size_type uint8_t and PageSize is 512

        private:
            jman::vector<std::array<size_type, PAGE_SIZE>> m_sparse_data {}; // holds the actual sparse data for pages
            jman::vector<size_type> m_page_map {}; //translates page id -> real position in m_sparse_data
            jman::vector<V> m_dense {}; // real values stored contigiously without any gaps
            jman::vector<K> m_dense_keys {}; // m_dense but change values for keys, simple

            struct sparse_location 
            {
                size_type real_page; // real page index from m_sparse_data
                size_type offset; // offset of the value in the page
            };

            static JMAN_FORCE_INLINE size_type __page_index(K key) noexcept { return key >> L; }
            static JMAN_FORCE_INLINE size_type __page_offset(K key) noexcept { return key & PAGE_MASK; }

            JMAN_FORCE_INLINE bool __page_exists(K key) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE sparse_location __locate(K key) const noexcept;

            JMAN_FORCE_INLINE size_type __index_unchecked(K key) const noexcept;
            void __ensure_sparse_size(K key) ;

        public:
            sparse_set() = default;
            ~sparse_set() noexcept = default;

            sparse_set(const sparse_set&) = default;
            sparse_set& operator=(const sparse_set&) = default;
            
            sparse_set(sparse_set&&) = default;
            sparse_set& operator=(sparse_set&&) = default;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool empty() const noexcept { return m_dense.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type size() const noexcept { return m_dense.size(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type pages() const noexcept { return m_sparse_data.size(); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE jman::vector<K>& keys() noexcept { return m_dense_keys; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE jman::vector<V>& values() noexcept { return m_dense; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const jman::vector<K>& keys() const noexcept { return m_dense_keys; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const jman::vector<V>& values() const noexcept { return m_dense; }

            // modifiers
            void JMAN_FORCE_INLINE reserve_dense(K count) { m_dense_keys.reserve(count); m_dense.reserve(count); }
            void JMAN_FORCE_INLINE reserve_pages(K count) { m_sparse_data.reserve(count); m_page_map.reserve(count); }

            void shrink_to_fit();
            void clear() noexcept;

            // elements
            template <typename T>
            requires std::convertible_to<T, V>
            reference insert(K key, T&& value) noexcept;
            template <typename ...Args>
            reference emplace(K key, Args&&... args) noexcept; 
            template <typename ...Args>
            reference emplace_or_replace(K key, Args&&... args) noexcept;
            template <typename T>
            requires std::convertible_to<T, V>
            reference replace(K key, T&& value) noexcept;

            void erase(K key) noexcept;

            // searching
            JMAN_NO_DISCARD reference get(K key);
            JMAN_NO_DISCARD pointer find(K key) noexcept;
            JMAN_NO_DISCARD const_reference get(K key) const;
            JMAN_NO_DISCARD const_pointer find(K key) const noexcept;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool contains(K key) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type dense_index(K entity) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE K key_from_index(size_type index) const; // index reffers to the dense index>
    };
}

// #####################################################################################
// -------------------------------- PRIVATE METHODS ----------------------------------//
// #####################################################################################

template <typename V, std::unsigned_integral K, std::size_t PageSize>
void jman::sparse_set<V, K, PageSize>::__ensure_sparse_size(K key) 
{
    size_type page_index = __page_index(key);
    
    // make sure the m_page_map has enough size for the new index
    if (page_index >= m_page_map.size())
        m_page_map.resize(page_index + 1, INVALID_INDEX);

    // if there is no page asignated, asignate one ofc
    if (m_page_map[page_index] == INVALID_INDEX) {
        m_page_map[page_index] = m_sparse_data.size();
        
        auto& page = m_sparse_data.emplace_back();
        page.fill(INVALID_INDEX);
    }
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_FORCE_INLINE bool jman::sparse_set<V, K, PageSize>::__page_exists(K key) const noexcept 
{
    size_type page = __page_index(key);

    return page < m_page_map.size() && m_page_map[page] != INVALID_INDEX;
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_FORCE_INLINE jman::sparse_set<V, K, PageSize>::sparse_location jman::sparse_set<V, K, PageSize>::__locate(K key) const noexcept 
{
    return {
        m_page_map[__page_index(key)],
        __page_offset(key)
    };
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_FORCE_INLINE jman::sparse_set<V, K, PageSize>::size_type jman::sparse_set<V, K, PageSize>::__index_unchecked(K key) const noexcept
{
    auto loc = locate(key);
    return m_sparse_data[loc.real_page][loc.offset];
}

// #####################################################################################
// ------------------------------------ MODIFIERS ------------------------------------//
// #####################################################################################

template <typename V, std::unsigned_integral K, std::size_t PageSize>
void jman::sparse_set<V, K, PageSize>::shrink_to_fit()
{
    m_dense.shrink_to_fit();
    m_dense_keys.shrink_to_fit();
    m_page_map.shrink_to_fit();
    
    // remove only the last trailing pages if they r empty, avoid removing from middle cuz it could destroy performance for the extra O(n) iterations
    // + this avoid the invalidating iterators problem in mid search so yea
    while (!m_sparse_data.empty()) {
            const auto& last_page = m_sparse_data.back();
            
            bool is_empty = true;
            for (size_type item: last_page) {
                if (item != INVALID_INDEX) {is_empty = false; break;}
            }

            if (!is_empty) break;

            size_type last_page_index = m_sparse_data.size() - 1;
            for (size_type j = 0; j < m_page_map.size(); ++j) {
                if (m_page_map[j] == last_page_index) {
                    m_page_map[j] = INVALID_INDEX;
                    break;
                }
            }

            m_sparse_data.pop_back();
    }

    m_sparse_data.shrink_to_fit();
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
void jman::sparse_set<V, K, PageSize>::clear() noexcept
{
    m_dense.clear();
    m_dense_keys.clear();
    m_page_map.clear();
    m_sparse_data.clear();
}

// #####################################################################################
// ------------------------------------- ELEMENTS ------------------------------------//
// #####################################################################################

template <typename V, std::unsigned_integral K, std::size_t PageSize>
template <typename T>
requires std::convertible_to<T, V>
V& jman::sparse_set<V, K, PageSize>::insert(K key, T&& value) noexcept
{
    __ensure_sparse_size(key);

    sparse_location loc = __locate(key);
    size_type denseIndex = m_sparse_data[loc.real_page][loc.offset];

    if (denseIndex == INVALID_INDEX)
    {
        denseIndex = m_dense.size();
        m_sparse_data[loc.real_page][loc.offset] = denseIndex;
        m_dense.push_back( static_cast<V>(std::forward<T>(value)) );
        m_dense_keys.push_back( key );
    } else
        m_dense[denseIndex] = static_cast<V>(std::forward<T>(value));

    return m_dense[denseIndex];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
template <typename ...Args>
V& jman::sparse_set<V, K, PageSize>::emplace(K key, Args&&... args) noexcept
{
    __ensure_sparse_size(key);

    sparse_location loc = __locate(key);
    size_type denseIndex = m_sparse_data[loc.real_page][loc.offset];

    if (denseIndex == INVALID_INDEX)
    {
        denseIndex = m_dense.size();
        m_sparse_data[loc.real_page][loc.offset] = denseIndex;
        m_dense.emplace_back( std::forward<Args>(args)... );
        m_dense_keys.push_back( key );
    } else
        throw std::out_of_range("jman::sparse_set<V, K>::emplace() failed: called emplace with an active value.");

    return m_dense[denseIndex];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
template <typename ...Args>
V& jman::sparse_set<V, K, PageSize>::emplace_or_replace(K key, Args&&... args) noexcept
{
    __ensure_sparse_size(key);

    sparse_location loc = __locate(key);
    size_type denseIndex = m_sparse_data[loc.real_page][loc.offset];

    if (denseIndex == INVALID_INDEX)
    {
        denseIndex = m_dense.size();
        m_sparse_data[loc.real_page][loc.offset] = denseIndex;
        m_dense.emplace_back( std::forward<Args>(args)... );
        m_dense_keys.push_back( key );
    } else
        m_dense[denseIndex] = V { std::forward<Args>(args)... };

    return m_dense[denseIndex];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
template <typename T>
requires std::convertible_to<T, V>
V& jman::sparse_set<V, K, PageSize>::replace(K key, T&& value) noexcept
{
    __ensure_sparse_size(key);

    sparse_location loc = __locate(key);
    size_type denseIndex = m_sparse_data[loc.real_page][loc.offset];

    if (denseIndex != INVALID_INDEX)
        m_dense[denseIndex] = static_cast<V>(std::forward<T>(value));
    else
        throw std::out_of_range("jman::sparse_set<V, K>::replace() failed: tried to replace a non existent value");

    return m_dense[denseIndex];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
void jman::sparse_set<V, K, PageSize>::erase(K key) noexcept
{
    const size_type denseIndex = dense_index(key);
    if (denseIndex == INVALID_INDEX) return;

    sparse_location id_loc = __locate(key);
                
    if (m_dense_keys.back() != key)
    {
        size_type index_to_remove = denseIndex;
        K last_key = m_dense_keys.back();

        sparse_location last_loc = __locate(last_key);
        
        // set the info of the erased element to the last
        m_dense_keys[index_to_remove] = last_key;
        m_dense[index_to_remove] = std::move_if_noexcept(m_dense.back());
        m_sparse_data[last_loc.real_page][last_loc.offset] = index_to_remove;
    }

    // remove the last item that its duplicated
    m_sparse_data[id_loc.real_page][id_loc.offset] = INVALID_INDEX;
    m_dense.pop_back();
    m_dense_keys.pop_back();
}

// #######################################################################################
// ------------------------------------- SEARCHING ------------------------------------//
// #######################################################################################

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD V& jman::sparse_set<V, K, PageSize>::get(K key)
{
    const size_type ind = dense_index(key);
    if (ind == INVALID_INDEX) throw std::out_of_range("jman::sparse_set<V, K>::get() failed: invalied key");

    return m_dense[ind];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD V* jman::sparse_set<V, K, PageSize>::find(K key) noexcept
{
    const size_type ind = dense_index(key);
    if (ind == INVALID_INDEX) return nullptr;

    return &m_dense[ind];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD const V& jman::sparse_set<V, K, PageSize>::get(K key) const
{
    const size_type ind = dense_index(key);
    if (ind == INVALID_INDEX) throw std::out_of_range("jman::sparse_set<V, K>::get() failed: invalied key");

    return m_dense[ind];
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD const V* jman::sparse_set<V, K, PageSize>::find(K key) const noexcept
{
    const size_type ind = dense_index(key);
    if (ind == INVALID_INDEX) return nullptr;

    return &m_dense[ind];
}


template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD JMAN_FORCE_INLINE bool jman::sparse_set<V, K, PageSize>::contains(K key) const noexcept
{
    if (!__page_exists(key)) return false;
    return __index_unchecked(key) != INVALID_INDEX;
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD JMAN_FORCE_INLINE jman::sparse_set<V, K, PageSize>::size_type jman::sparse_set<V, K, PageSize>::dense_index(K key) const noexcept
{
    if (!__page_exists(key)) return INVALID_INDEX;
    return __index_unchecked(key);
}

template <typename V, std::unsigned_integral K, std::size_t PageSize>
JMAN_NO_DISCARD JMAN_FORCE_INLINE K jman::sparse_set<V, K, PageSize>::key_from_index(size_type index) const // index reffers to the dense index
{
    if (index >= m_dense_keys.size()) throw std::out_of_range("jman::sparse_set<V, K>::key_from_index() failed: Index provided out of range for dense");
    return m_dense_keys[index];
}
