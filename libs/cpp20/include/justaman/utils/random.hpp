#pragma once

#include "attributes.h"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <random>

namespace jman 
{
    class random32 
    {
        std::mt19937 m_engine;

        public:
            using result_type = std::mt19937::result_type;

            inline random32();
            inline explicit random32(result_type seed);

            JMAN_FORCE_INLINE void                          reseed(result_type seed = std::random_device()());
            JMAN_FORCE_INLINE void                          discard(unsigned long long z);

            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   min() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   max() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   raw_next();

            bool                                            chance(double probability = 0.5);

            template<std::integral T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::integral T = int>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             sign();
 
            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             variation(T mean, T stddev);



            
            template<std::input_iterator Iter>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const typename std::iterator_traits<Iter>::value_type&      
                                                            choice(Iter begin, Iter end);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::input_iterator Iter>
            requires std::is_same_v<typename std::iterator_traits<Iter>::value_type, double>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE Iter          weighted_iter(Iter begin, Iter end);
    };  

    class random64
    {
        std::mt19937_64 m_engine;

        public:
            using result_type = std::mt19937_64::result_type;

            inline random64();
            inline explicit random64(result_type seed);

            JMAN_FORCE_INLINE void                          reseed(result_type seed);
            JMAN_FORCE_INLINE void                          reseed();
            JMAN_FORCE_INLINE void                          discard(unsigned long long z);

            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   min() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   max() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE result_type   raw_next();

            bool                                            chance(double probability = 0.5);

            template<std::integral T>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             range(T min, T max);

            template<std::integral T = int>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             sign();

            template<std::floating_point T = float>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T             variation(T mean, T stddev);

            template<std::input_iterator Iter>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const typename std::iterator_traits<Iter>::value_type&      
                                                            choice(Iter begin, Iter end);

            /** 
            * @note: internally it uses std::discrete_distribution, so there is a heap allocation internally for cache or smt idk, so watch out 
            */
            template<std::input_iterator Iter>
            requires std::is_same_v<typename std::iterator_traits<Iter>::value_type, double>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE Iter          weighted_iter(Iter begin, Iter end);
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

inline random32::random32(result_type seed) 
{
    m_engine = std::mt19937 { seed };
}

// #################################################################################
// --------------------------------- SEED HANDLING --------------------------------//
// #################################################################################


JMAN_FORCE_INLINE void random32::reseed(result_type seed)
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


JMAN_NO_DISCARD JMAN_FORCE_INLINE random32::result_type random32::min() const noexcept
{
    return m_engine.min();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE random32::result_type random32::max() const noexcept
{
    return m_engine.max();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE random32::result_type random32::raw_next()
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

template<std::input_iterator Iter>
JMAN_NO_DISCARD JMAN_FORCE_INLINE const typename std::iterator_traits<Iter>::value_type& random32::choice(Iter begin, Iter end)
{
    std::uniform_int_distribution<> d{0, std::distance(begin, end)};
    return *std::advance(begin, d(m_engine));
}

template<std::input_iterator Iter>
requires std::is_same_v<typename std::iterator_traits<Iter>::value_type, double>
JMAN_NO_DISCARD JMAN_FORCE_INLINE Iter random32::weighted_iter(Iter begin, Iter end)
{
    std::discrete_distribution<> d{begin, end};
    return std::advance(begin, d(m_engine));
}

// ###################################################################################################################
// ---------------------------------------------- RANDOM 64 CLASS --------------------------------------------------//
// ###################################################################################################################

// #################################################################################
// --------------------------------- CONSTRUCTORS --------------------------------//
// #################################################################################

inline random64::random64() 
{
    std::random_device rd;
    std::uint_fast64_t seed = (static_cast<std::uint_fast64_t>(rd()) << 32) | rd();
    m_engine = std::mt19937_64 { seed };
}

inline random64::random64(result_type seed) 
{
    m_engine = std::mt19937_64 { seed };
}

// #################################################################################
// --------------------------------- SEED HANDLING --------------------------------//
// #################################################################################


JMAN_FORCE_INLINE void random64::reseed(result_type seed)
{   
    m_engine = std::mt19937_64 { seed };
}
JMAN_FORCE_INLINE void random64::reseed()
{   
    std::random_device rd;
    std::uint_fast64_t seed = (static_cast<std::uint_fast64_t>(rd()) << 32) | rd();
    m_engine = std::mt19937_64 { seed };
}
JMAN_FORCE_INLINE void random64::discard(unsigned long long z)
{
    m_engine.discard(z);
}

// ############################################################################
// --------------------------------- GETTERS --------------------------------//
// ############################################################################


JMAN_NO_DISCARD JMAN_FORCE_INLINE random64::result_type random64::min() const noexcept
{
    return m_engine.min();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE random64::result_type random64::max() const noexcept
{
    return m_engine.max();
}
JMAN_NO_DISCARD JMAN_FORCE_INLINE random64::result_type random64::raw_next()
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
    std::discrete_distribution<std::size_t> d{weights.begin(), weights.end()};
    return d(m_engine);
}

template<std::floating_point T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t random64::weighted_index(std::initializer_list<T> weights)
{
    std::discrete_distribution<std::size_t> d{weights.begin(), weights.end()};
    return d(m_engine);
}

} // namespace jman