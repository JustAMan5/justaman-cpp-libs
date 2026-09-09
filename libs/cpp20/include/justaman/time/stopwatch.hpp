#pragma once

#include <chrono>
#include <justaman/utils/attributes.h>

namespace jman 
{
    class stopwatch 
    {
        using duration =                        std::chrono::duration<double>;
        using clock    =                        std::chrono::steady_clock;
        using time_point =                      clock::time_point;

        clock::time_point                       m_start =       {};
        duration                                m_accumulated = duration::zero();
        bool                                    m_running =     false;

        public:
            // CONSTRUCTOR
            stopwatch() = default;

            // STATE CONTROLS
            JMAN_FORCE_INLINE void                     start() noexcept;
            JMAN_FORCE_INLINE void                     reset() noexcept;
            JMAN_FORCE_INLINE void                     pause() noexcept;
            JMAN_FORCE_INLINE void                     resume() noexcept;


            // STATE CONSULT
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool     is_running() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool     is_paused() const noexcept;


            // TIME PASSED
            JMAN_NO_DISCARD JMAN_FORCE_INLINE duration elapsed_duration() const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE double   elapsed_seconds() const;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE double   elapsed_milliseconds() const;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE double   elapsed_microseconds() const;


            // LIMITS
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool     expired(double seconds) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE float    get_progress(double targetSeconds) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE double   get_remaining_time(double targetSeconds) const noexcept;
    };
}

// #######################################################################################
// ------------------------------------- STATE CONTROL --------------------------------//
// ######################################################################################

JMAN_FORCE_INLINE void jman::stopwatch::start() noexcept 
{
    if (m_running) return;
    m_start = clock::now();
    m_running = true;
}
JMAN_FORCE_INLINE void jman::stopwatch::pause() noexcept 
{
    if (!m_running) return;
    m_accumulated += clock::now() - m_start;
    m_running = false;
}
JMAN_FORCE_INLINE void jman::stopwatch::resume() noexcept 
{
    if (m_running) return;
    m_start = clock::now();
    m_running = true;
}
JMAN_FORCE_INLINE void jman::stopwatch::reset() noexcept 
{
    m_accumulated = duration::zero();
    m_running = false;
}

// #######################################################################################
// ------------------------------------- STATE CONSULT --------------------------------//
// ######################################################################################

JMAN_FORCE_INLINE bool jman::stopwatch::is_running() const noexcept { return m_running; }
JMAN_FORCE_INLINE bool jman::stopwatch::is_paused() const noexcept { return !m_running; }

// ####################################################################################
// ------------------------------------- TIME PASSED --------------------------------//
// ###################################################################################

JMAN_FORCE_INLINE jman::stopwatch::duration jman::stopwatch::elapsed_duration() const noexcept 
{
    duration total = m_accumulated;
    if (m_running)
        total += clock::now() - m_start;
    return total;
}
JMAN_FORCE_INLINE double jman::stopwatch::elapsed_seconds() const 
{
    return elapsed_duration().count();
}
JMAN_FORCE_INLINE double jman::stopwatch::elapsed_milliseconds() const 
{
    using Ms = std::chrono::duration<double, std::milli>;
    return std::chrono::duration_cast<Ms>(elapsed_duration()).count();
}
JMAN_FORCE_INLINE double jman::stopwatch::elapsed_microseconds() const 
{
    using Mc = std::chrono::duration<double, std::micro>;
    return std::chrono::duration_cast<Mc>(elapsed_duration()).count();
}

// ################################################################################
// ------------------------------------- LIMITS --------------------------------//
// ###############################################################################

JMAN_FORCE_INLINE bool jman::stopwatch::expired(double seconds) const noexcept 
{
    return elapsed_seconds() > seconds;
}
JMAN_FORCE_INLINE float jman::stopwatch::get_progress(double targetSeconds) const noexcept 
{
    return elapsed_seconds() / targetSeconds;
}
JMAN_FORCE_INLINE double jman::stopwatch::get_remaining_time(double targetSeconds) const noexcept
{
    return targetSeconds - elapsed_seconds();
}