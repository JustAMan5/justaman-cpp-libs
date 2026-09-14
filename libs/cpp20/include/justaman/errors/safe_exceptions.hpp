#pragma once

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || (defined(_HAS_EXCEPTIONS) && _HAS_EXCEPTIONS != 0) || defined(_CPPUNWIND)
#   define JMAN_HAS_EXCEPTIONS_ENABLED 1
#else
#   define JMAN_HAS_EXCEPTIONS_ENABLED 0
#endif


#if JMAN_HAS_EXCEPTIONS_ENABLED

#   define JMAN_THROW(exception) throw exception
#   define JMAN_TRY try
#   define JMAN_CATCH(x) catch (x)
#   define JMAN_RETHROW throw

#else

#   include <exception>
    // force direct program termination when any exception is thrown
#   define JMAN_THROW(exception) std::terminate()
#   define JMAN_TRY if (true)
#   define JMAN_CATCH(x) if (false)
#   define JMAN_RETHROW

#endif