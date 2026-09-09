#pragma once

#include <cstdint>
#include <memory>
#include <justaman/containers/vector.hpp>
#include "justaman/utils/attributes.h"

namespace jman 
{
    /** 
        @brief
            fsignal (FastSignal) is very fast when firing since its a vector, the connections live one after the other in memory
            so the CPU can easiliy optimize it and make the access fast, tho its not safe,
            removing or adding connections during iteration can cause some callbacks get called twice or more
            and others get skipped, and i dont think this implementation is thread safe 
            also has the problem that if an object A fires and object B listents, when B is destroyed
            A doesnt know that and still tries to wake up that listener and causes a crash

        @note
            uses func pointers so some regular methods with std::function dont work in here like capture lambdas

        @note  
            for my benchmakrs it connecting and disconnecting is fast but a bit slow cuz of heap allocations but still a good number 
            (1000 connects in 0.084 ms and 1000 disconnects (with disconnect_all()) in 0.005 ms) and fire runs blanzingly fast with 1000 connections in 0.007 ms
    */
    template <typename ...Args>
    class fsignal 
    {
        static constexpr std::uint64_t ALIVE_FLAG_BIT = 1ULL << 63;
        static constexpr std::uint64_t ONCE_FLAG_BIT = 1ULL << 62;

        using callback = void(*)(Args...);
        
        struct slot // 16 bytes
        {
            callback        callback;
            std::uint64_t   packed; // bottom 32 bits r the generation, the 64th bit is for alive, the 63th for the once flag
        };

        jman::vector<slot>                  m_slots;
        jman::vector<std::size_t>           m_freeList;

        struct signal_lifetime {};
        std::shared_ptr<signal_lifetime>    m_lifetime; // cool fix for when the connection lives longer than the signal

        private: 
            JMAN_FORCE_INLINE static bool __alive_flag(slot slot) noexcept 
                { return (slot.packed & ALIVE_FLAG_BIT) != 0; }
            JMAN_FORCE_INLINE static bool __once_flag(slot slot) noexcept 
                { return (slot.packed & ONCE_FLAG_BIT) != 0; }
            JMAN_FORCE_INLINE static std::uint32_t __gen_flag(slot slot) noexcept 
                { return static_cast<std::uint32_t>(slot.packed & 0xFFFFFFFF); }

            JMAN_FORCE_INLINE static void __set_alive_flag(slot& slot, bool value) noexcept 
                { slot.packed = value ? (slot.packed | ALIVE_FLAG_BIT) : (slot.packed & ~ALIVE_FLAG_BIT);; }
            JMAN_FORCE_INLINE static void __set_once_flag(slot& slot, bool value) noexcept 
                { slot.packed = value ? (slot.packed | ONCE_FLAG_BIT) : (slot.packed & ~ONCE_FLAG_BIT);; }
            JMAN_FORCE_INLINE static void __set_gen_flag(slot& slot, std::uint32_t value) noexcept 
                { slot.packed = (slot.packed & 0xFFFFFFFF00000000ULL) | static_cast<std::uint64_t>(value); }
            void unchecked_disconnect_slot(std::size_t index);
            
        public:

            /**
                @note
                    doesnt automatically disconnect the function when going out of scope
                    use scoped_connection instead 
            */
            struct connection 
            {
                connection(const connection& other) = delete;
                connection& operator=(const connection& other) = delete;

                connection(connection&& other) = default;
                connection& operator=(connection&& other) = default;
                
                JMAN_NO_DISCARD bool    connected() const noexcept;
                JMAN_NO_DISCARD bool    once() const noexcept;
                void                    disconnect();
                
                private:
                    friend class fsignal<Args...>;

                    connection(std::shared_ptr<signal_lifetime>& l, fsignal* s, std::size_t i, std::uint32_t g)
                        :m_signal_lifetime(l), m_signal(s), m_index(i), m_generation(g) {}

                    std::weak_ptr<signal_lifetime>      m_signal_lifetime;
                    fsignal*                            m_signal;
                    std::size_t                         m_index;
                    std::uint32_t                       m_generation;
            };

            class scoped_connection {
                private:
                    connection m_conn;
                public:
                    scoped_connection(connection&& c): m_conn(std::move(c)) {}
                    ~scoped_connection() { m_conn.disconnect(); }

                    scoped_connection(const scoped_connection&) = delete;
                    scoped_connection& operator=(const scoped_connection&) = delete;
                    
                    scoped_connection(scoped_connection&&) noexcept = default;
                    scoped_connection& operator=(scoped_connection&&) noexcept = default;

                    JMAN_FORCE_INLINE void disconnect() { m_conn.disconnect(); }

            };

            fsignal(): m_lifetime(std::make_shared<signal_lifetime>()) {};
            ~fsignal() noexcept;

            fsignal(const fsignal& other) = delete;
            fsignal& operator=(const fsignal& other) = delete;

            fsignal(fsignal&& other) noexcept;
            fsignal& operator=(fsignal&& other) noexcept;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t   connections() const noexcept { return m_slots.size() - m_freeList.size(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t   internal_size() const noexcept { return m_slots.size(); }
                                        // returns the internal size of the m_slots vector
                                        // to get the free list size just do internal_sizs() - connections()

            connection                                              connect(callback callback);
            connection                                              once(callback callback);
            JMAN_NO_DISCARD JMAN_FORCE_INLINE scoped_connection     scoped_connect(callback callback) { return {connect(callback)}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE scoped_connection     scoped_once(callback callback) { return {once(callback)}; }

            template <typename ...T>
            requires (std::convertible_to<T, Args> && ...)
            void                                            fire(T&&...);
            void                                            disconnect_all();
            JMAN_FORCE_INLINE void                          reserve_internal(std::size_t count);
    };

}

// ######################################################################################################
// ------------------------------------- FSIGNAL::CONNECTION METHODS ----------------------------------//
// #####################################################################################################

template <typename ...Args>
void jman::fsignal<Args...>::connection::disconnect()
{
    auto lock = m_signal_lifetime.lock();
    if (!lock || !connected()) return;

    m_signal->unchecked_disconnect_slot(m_index);
    m_signal == nullptr;
}

template <typename ...Args>
JMAN_NO_DISCARD bool jman::fsignal<Args...>::connection::connected() const noexcept
{
    if (m_signal_lifetime.expired()) return false; // fallback to not connected when the signal is dead;
    slot sl = m_signal->m_slots[m_index];
            
    return signal && 
            m_index < m_signal->m_slots.size() && 
            __alive_flag(sl) && 
            __gen_flag(sl) == m_generation;
}

template <typename ...Args>
JMAN_NO_DISCARD bool jman::fsignal<Args...>::connection::once() const noexcept
{
    if (m_signal_lifetime.expired()) return false; // fallback to not connected when the signal is dead;
    slot sl = m_signal->m_slots[m_index];
            
    return signal && 
            m_index < m_signal->m_slots.size() && 
            __alive_flag(sl) && 
            __gen_flag(sl) == m_generation;
            __once_flag(sl);
}


// ##########################################################################################
// ------------------------------------- PRIVATE METHODS ----------------------------------//
// ##########################################################################################

template <typename ...Args>
void jman::fsignal<Args...>::unchecked_disconnect_slot(std::size_t index)
{
    slot& sl = m_slots[index];

    __set_alive_flag(sl, false);
    __set_once_flag(sl, false);
    sl.callback = nullptr; 

    m_freeList.push_back(index);
}


// ##########################################################################################
// ------------------------------- CONSTRUCTORS / DESTRUCTORS ------------------------------//
// ##########################################################################################

template <typename ...Args>
jman::fsignal<Args...>::~fsignal() noexcept {
    for (slot& sl: m_slots)
        __set_alive_flag(sl, false);
}

template <typename ...Args>
jman::fsignal<Args...>::fsignal(fsignal&& other) noexcept
{
    m_lifetime = std::move(other.m_lifetime);
    m_freeList = std::move(other.m_freeList);
    m_slots = std::move(other.m_slots);
}
template <typename ...Args>
jman::fsignal<Args...>& jman::fsignal<Args...>::operator=(fsignal&& other) noexcept
{
    if (this == &other)
    {
        m_lifetime = std::move(other.m_lifetime);
        m_freeList = std::move(other.m_freeList);
        m_slots = std::move(other.m_slots);
    }

    return *this;
}


// #################################################################################
// ------------------------------------- METHODS --------------------------------//
// #################################################################################

template <typename ...Args>
jman::fsignal<Args...>::connection 
    jman::fsignal<Args...>::connect(jman::fsignal<Args...>::callback callback) {

    std::size_t index;
    if (!m_freeList.empty()) {
        index = m_freeList.back();
        m_freeList.pop_back();
        __set_gen_flag(m_slots[index], gen_flag(m_slots[index]) + 1);
    } else {
        index = m_slots.size();
        m_slots.emplace_back();
    }
    
    m_slots[index].callback = std::move(callback);
    __set_alive_flag(m_slots[index], true);
    __set_once_flag(m_slots[index], false); // just in case

    return { 
        m_lifetime, 
        this, 
        index, 
        __gen_flag(m_slots[index])
    };
}

template <typename ...Args>
jman::fsignal<Args...>::connection 
    jman::fsignal<Args...>::once(jman::fsignal<Args...>::callback callback) {

    std::size_t index;
    if (!m_freeList.empty()) {
        index = m_freeList.back();
        m_freeList.pop_back();
        __set_gen_flag(m_slots[index], gen_flag(m_slots[index]) + 1);
    } else {
        index = m_slots.size();
        m_slots.emplace_back();
    }
    
    m_slots[index].callback = std::move(callback);
    __set_alive_flag(m_slots[index], true);
    __set_once_flag(m_slots[index], true);

    return { 
        m_lifetime, 
        this, 
        index, 
        __gen_flag(m_slots[index])
    };
}

template <typename ...Args>
template <typename ...T>
requires (std::convertible_to<T, Args> && ...)
void jman::fsignal<Args...>::fire(T&&... args) 
{ 
    for (std::size_t i = 0; i < m_slots.size(); ++i) {
        slot sl = m_slots[i];

        if (alive_flag(sl)) {
            sl.callback(static_cast<Args>(args)...); // sadly we cant forward them, we must copy
            if (__once_flag(sl)) unchecked_disconnect_slot(i);
        }
    }
}

template <typename ...Args>
void jman::fsignal<Args...>::disconnect_all() 
{
    for (std::size_t i = 0; i < m_slots.size(); ++i) {
        if (__alive_flag(m_slots[i]))
            unchecked_disconnect_slot(i);
    }
}

template <typename ...Args>
JMAN_FORCE_INLINE void jman::fsignal<Args...>::reserve_internal(std::size_t count)
{
    m_slots.reserve(count);
}