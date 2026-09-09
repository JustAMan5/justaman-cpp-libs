#pragma once

#include "/rbxsignal/fsignal.hpp"

namespace jman 
{
    template <typename T>
    class observable
    {
        fsignal<T>  m_signal {};
        T           m_value  {};

        public:
            using connection            = fsignal<T>::connection;
            using scoped_connection     = fsignal<T>::scoped_connection;
            using callback              = void(*)(const T&);

            observable(T&& val): m_value(val) {};

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t           connections() const noexcept { return m_signal.connections(); }

            JMAN_FORCE_INLINE connection                            connect(callback callback) { return m_signal.connect(callback); }
            JMAN_FORCE_INLINE connection                            once(callback callback) { return m_signal.connect(callback); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE scoped_connection     scoped_connect(callback callback) { return m_signal.scoped_connect(callback); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE scoped_connection     scoped_once(callback callback) { return m_signal.scoped_once(callback); }

            void disconnect_all() { m_signal.disconnect_all(); }

            observable& operator=(const T& val) {
                m_value = val;
                m_signal.fire(m_value);
            }

            observable& operator=(T&& val) {
                m_value = val;
                m_signal.fire(m_value);
            }

            const T& get() const noexcept { return m_value; }
            const T& operator*() const noexcept { return m_value; }
    };

}