#ifndef SRC_UTIL_SIGNAL_H
#define SRC_UTIL_SIGNAL_H

#include <list>
#include <functional>
#include <mutex>

namespace util {
template<typename T, typename R, typename... Args>
class Connection;

template<typename F>
class Signal {
};

template<typename R, typename... Args>
class Signal<R(Args...)> {
public:
    template<typename T, typename F>
    auto connect(T* receiver, R(F::*f)(Args...))
    {   
        std::scoped_lock lk{lock};
        slots.emplace_front([receiver, f](Args&&... args){
            (receiver->*f)(std::forward<Args>(args)...);
        });

        return Connection<R(Args...), R, Args...>{this, slots.begin()};
    }

    void emit(Args&&... args)
    {
        std::scoped_lock lk{lock};
        for (auto& slot : slots) {
            slot(std::forward<Args>(args)...);
        }
    }

    void disconnect(const Connection<R(Args...), R, Args...>& connection)
    {
        std::scoped_lock lk{lock};
        slots.erase(connection.it);
    }

private:
    std::recursive_mutex lock;
    std::list<std::function<R(Args...)>> slots;
};

template<typename T, typename R, typename... Args>
class Connection {
public:
    Connection(Signal<T>* signal, std::list<std::function<R(Args...)>>::iterator it):
        signal{signal},
        it{it}
    {}

    void disconnect()
    {
        signal->disconnect(*this);
    }

    friend class Signal<R(Args...)>;

private:
    Signal<T>* signal;
    std::list<std::function<R(Args...)>>::iterator it;
};
}

#endif
