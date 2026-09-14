#pragma once

#include "attributes.h"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <random>

namespace jman 
{
    class random32 
    {
        std::mt19937 m_engine;

        public:
            random32();
            explicit random32(std::uint32_t seed);

            void                                            reseed(std::uint32_t seed = std::random_device()());
            void                                            discard(unsigned long long z);

            JMAN_NO_DISCARD std::uint32_t                   min() const noexcept;
            JMAN_NO_DISCARD std::uint32_t                   max() const noexcept;
            JMAN_NO_DISCARD std::uint32_t                   raw_next();

            bool                                            chance(double probability = 0.5);

            template<std::integral T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::integral T = int>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             sign();

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             variation(T mean, T stddev);

            template<typename T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T&      choice(const std::vector<T>& vec);

            template<typename T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T&      choice(std::initializer_list<T> list);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::floating_point T = float>
            JMAN_NO_DISCARD std::size_t JMAN_FORCE_INLINE   weighted_index(const std::vector<T>& weights);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::floating_point T = float>
            JMAN_NO_DISCARD std::size_t JMAN_FORCE_INLINE   weighted_index(std::initializer_list<T> weights);
    };  

    class random64
    {
        std::mt19937_64 m_engine;

        public:
            random64();
            explicit random64(std::uint64_t seed);

            void reseed(std::uint64_t seed = std::random_device()());
            void discard(unsigned long long z);

            JMAN_NO_DISCARD std::uint64_t min() const noexcept;
            JMAN_NO_DISCARD std::uint64_t max() const noexcept;
            JMAN_NO_DISCARD std::uint64_t raw_next();

            bool chance(double probability = 0.5);

            template<std::integral T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T range(T min, T max);

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T range(T min, T max);

            template<std::integral T = int>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T sign();

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T variation(T mean, T stddev);

            template<typename T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& choice(const std::vector<T>& vec);

            template<typename T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& choice(std::initializer_list<T> list);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::floating_point T = float>
            JMAN_NO_DISCARD std::size_t JMAN_FORCE_INLINE weighted_index(const std::vector<T>& weights);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::floating_point T = float>
            JMAN_NO_DISCARD std::size_t JMAN_FORCE_INLINE weighted_index(std::initializer_list<T> weights);
    };  
} // namespace jman

namespace jman 
{

// ###################################################################################################################
// ---------------------------------------------- RANDOM 32 CLASS --------------------------------------------------//
// ###################################################################################################################

// #################################################################################
// --------------------------------- CONSTRUCTORS --------------------------------//
// #################################################################################

inline random32::random32() 
{
    m_engine = std::mt19937 { std::random_device()() };
}

inline random32::random32(std::uint32_t seed) 
{
    m_engine = std::mt19937 { seed };
}

// #################################################################################
// --------------------------------- SEED HANDLING --------------------------------//
// #################################################################################


JMAN_FORCE_INLINE void random32::reseed(std::uint32_t seed)
{   
    m_engine = std::mt19937 { seed };
}
JMAN_FORCE_INLINE void random32::discard(unsigned long long z)
{
    m_engine.discard(z);
}

// ############################################################################
// --------------------------------- GETTERS --------------------------------//
// ############################################################################


JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint32_t random32::min() const noexcept
{
    return m_engine.min();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint32_t random32::max() const noexcept
{
    return m_engine.max();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint32_t random32::raw_next()
{
    return m_engine();
}

// ####################################################################################
// --------------------------------- DISTRIBUTATIONS --------------------------------//
// ####################################################################################


JMAN_NO_DISCARD JMAN_FORCE_INLINE bool random32::chance(double probability)
{
    std::bernoulli_distribution d{ probability };
    return d(m_engine);
}

template<std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random32::range(T min, T max) 
{
    std::uniform_int_distribution<T> d{ min, max };
    return d(m_engine);
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random32::range(T min, T max)
{
    std::uniform_real_distribution<T> d{ min, max };
    return d(m_engine);
}

template<std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random32::sign()
{
    return ((m_engine() & 2UL) == 0) ? -1 : 1;
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random32::variation(T mean, T stddev)
{
    std::normal_distribution<T> d{ mean, stddev };
    return d(m_engine);
}

template<typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& random32::choice(const std::vector<T>& vec)
{
    std::uniform_int_distribution<> d{ 0, vec.size() - 1 };
    return vec[d(m_engine)];
}

template<typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& random32::choice(std::initializer_list<T> list)
{
    std::uniform_int_distribution<> d{ 0, list.size() - 1 };
    return list[d(m_engine)];
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t random32::weighted_index(const std::vector<T>& weights)
{
    std::discrete_distribution<> d{weights.begin(), weights.end()};
    return d(m_engine);
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t random32::weighted_index(std::initializer_list<T> weights)
{
    std::discrete_distribution<> d{weights.begin(), weights.end()};
    return d(m_engine);
}

// ###################################################################################################################
// ---------------------------------------------- RANDOM 64 CLASS --------------------------------------------------//
// ###################################################################################################################

// #################################################################################
// --------------------------------- CONSTRUCTORS --------------------------------//
// #################################################################################

inline random64::random64() 
{
    m_engine = std::mt19937_64 { std::random_device()() };
}

inline random64::random64(std::uint64_t seed) 
{
    m_engine = std::mt19937_64 { seed };
}

// #################################################################################
// --------------------------------- SEED HANDLING --------------------------------//
// #################################################################################


JMAN_FORCE_INLINE void random64::reseed(std::uint64_t seed)
{   
    m_engine = std::mt19937_64 { seed };
}
JMAN_FORCE_INLINE void random64::discard(unsigned long long z)
{
    m_engine.discard(z);
}

// ############################################################################
// --------------------------------- GETTERS --------------------------------//
// ############################################################################


JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint64_t random64::min() const noexcept
{
    return m_engine.min();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint64_t random64::max() const noexcept
{
    return m_engine.max();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint64_t random64::raw_next()
{
    return m_engine();
}

// ####################################################################################
// --------------------------------- DISTRIBUTATIONS --------------------------------//
// ####################################################################################


JMAN_NO_DISCARD JMAN_FORCE_INLINE bool random64::chance(double probability)
{
    std::bernoulli_distribution d{ probability };
    return d(m_engine);
}

template<std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random64::range(T min, T max) 
{
    std::uniform_int_distribution<T> d{ min, max };
    return d(m_engine);
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random64::range(T min, T max)
{
    std::uniform_real_distribution<T> d{ min, max };
    return d(m_engine);
}

template<std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random64::sign()
{
    return ((m_engine() & 2UL) == 0) ? -1 : 1;
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T random64::variation(T mean, T stddev)
{
    std::normal_distribution<T> d{ mean, stddev };
    return d(m_engine);
}

template<typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& random64::choice(const std::vector<T>& vec)
{
    std::uniform_int_distribution<> d{ 0, vec.size() - 1 };
    return vec[d(m_engine)];
}

template<typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& random64::choice(std::initializer_list<T> list)
{
    std::uniform_int_distribution<> d{ 0, list.size() - 1 };
    return list[d(m_engine)];
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t random64::weighted_index(const std::vector<T>& weights)
{
    std::discrete_distribution<> d{weights.begin(), weights.end()};
    return d(m_engine);
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t random64::weighted_index(std::initializer_list<T> weights)
{
    std::discrete_distribution<> d{weights.begin(), weights.end()};
    return d(m_engine);
}

} // namespace jman