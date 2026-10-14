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
#   define JMAN_THROW(exception) std::terminate()
#   define JMAN_TRY if constexpr (true)
// create a valid exception decl so the catch block compiles without any syntax errors but never reaches to the final binary code
#   define JMAN_CATCH(exception_decl) if constexpr (false) for (exception_decl {""}; false;)
#   define JMAN_RETHROW std::terminate() 

#endif