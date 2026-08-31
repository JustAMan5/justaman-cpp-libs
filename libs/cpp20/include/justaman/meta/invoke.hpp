#include <utility>

// idk what this shit does i just know its for calling any type of func depending on what is it
namespace jman 
{   
    namespace __detail 
    {
        // pointers to member methods of a class
        template <typename Base, typename T, typename Derived, typename... Args>
        auto __invoke_impl(T Base::*pmf, Derived&& obj, Args&&... args)
            -> decltype((std::forward<Derived>(obj).*pmf)(std::forward<Args>(args)...)) 
        {
            return (std::forward<Derived>(obj).*pmf)(std::forward<Args>(args)...);
        }

        // invocable object like lambdas functions etc etc
        template <typename F, typename... Args>
        auto __invoke_impl(F&& f, Args&&... args)
            -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) 
        {
            return std::forward<F>(f)(std::forward<Args>(args)...);
        }
    } // namespace __detail

    template <typename F, typename... Args>
    auto invoke(F&& f, Args&&... args) 
        -> decltype(__detail::__invoke_impl(std::forward<F>(f), std::forward<Args>(args)...)) 
    {
        return __detail::__invoke_impl(std::forward<F>(f), std::forward<Args>(args)...);
    }
} // namespace jman