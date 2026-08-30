#pragma once

#include <chrono>
#include <justaman/utils/attributes.h>

namespace jman 
{
    class stopwatch 
    {
        using Duration =                        std::chrono::duration<double>;
        using Clock    =                        std::chrono::steady_clock;
        using TimePoint =                       Clock::time_point;

        Clock::time_point                       m_start = {};
        Duration                                m_accumulated = Duration::zero();
        bool                                    m_running = false;

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
            JMAN_NO_DISCARD JMAN_FORCE_INLINE Duration elapsed_duration() const noexcept;
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
        m_start = Clock::now();
        m_running = true;
}
JMAN_FORCE_INLINE void jman::stopwatch::pause() noexcept 
{
    if (!m_running) return;
    m_accumulated += Clock::now() - m_start;
    m_running = false;
}
JMAN_FORCE_INLINE void jman::stopwatch::resume() noexcept 
{
    if (m_running) return;
    m_start = Clock::now();
    m_running = true;
}
JMAN_FORCE_INLINE void jman::stopwatch::reset() noexcept 
{
    m_accumulated = Duration::zero();
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

JMAN_FORCE_INLINE jman::stopwatch::Duration jman::stopwatch::elapsed_duration() const noexcept 
{
    Duration total = m_accumulated;
    if (m_running)
        total += Clock::now() - m_start;
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