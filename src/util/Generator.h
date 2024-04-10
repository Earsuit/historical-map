#ifndef SRC_UTIL_COROUTINE_H
#define SRC_UTIL_COROUTINE_H

#include <coroutine>
#include <concepts>
#include <exception>

namespace util {
template<typename T>
class Generator {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        promise_type()  = default;                              
        ~promise_type() = default;
        
        auto initial_suspend() {                                 
            return std::suspend_always{};
        }

        auto final_suspend() noexcept {
            return std::suspend_always{};
        }

        auto get_return_object() {                              
            return Generator{handle_type::from_promise(*this)};
        }

        auto return_void() {
            return std::suspend_never{};
        }
      
        template<std::convertible_to<T> From>
        auto yield_value(From&& from) {                          
            current_value = std::forward<From>(from);;
            return std::suspend_always{};
        }

        void unhandled_exception() {
            exception_ = std::current_exception();
        }

        T current_value;
        std::exception_ptr exception_;
    };

    Generator() = default;
    Generator(handle_type h): coro(h) {}                         
    ~Generator() {
        if (coro) {
            coro.destroy();
        }
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other): coro(std::move(other.coro)) 
    {
    }

    Generator& operator=(Generator&& other) {
        coro = other.coro;
        other.coro = nullptr;
        return *this;
    }

    T getValue() {
        return std::move(coro.promise().current_value);
    }

    bool next() {                                             
        coro.resume();
        return !coro.done();
    }
private:
    handle_type coro;
};
}

#endif
