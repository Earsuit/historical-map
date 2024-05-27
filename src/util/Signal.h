#ifndef SRC_UTIL_SIGNAL_H
#define SRC_UTIL_SIGNAL_H

#include "src/util/TypeTraits.h"

#include <list>
#include <functional>
#include <mutex>
#include <type_traits>

namespace util {
template<typename T, typename R, typename... Args>
class Connection;

template<typename F>
class Signal {
};

template<typename T, typename Rs, typename... Parms>
Connection<Rs(Parms...), Rs, Parms...> connect(Signal<Rs(Parms...)>& signal, T* receiver, Rs(T::*f)(Parms...))
{
    return signal.connect(receiver, f);
}

template<typename R, typename... Args>
class Signal<R(Args...)> {
public:
    template<typename... Ts>
    requires (util::is_all_same_v<util::param_pack<Args...>, util::param_pack<Ts...>>)
    void operator()(Ts&&... args)
    {
        std::scoped_lock lk{lock};
        for (auto& slot : slots) {
            slot(std::forward<Ts>(args)...);
        }
    }

    friend class Connection<R(Args...), R, Args...>;

    template<typename T, typename Rs, typename... Parms>
    friend Connection<Rs(Parms...), Rs, Parms...> connect(Signal<Rs(Parms...)>& signal, T* receiver, Rs(T::*f)(Parms...));

private:
    std::recursive_mutex lock;
    std::list<std::function<R(Args...)>> slots;

    void disconnect(const Connection<R(Args...), R, Args...>& connection)
    {
        std::scoped_lock lk{lock};
        slots.erase(connection.it);
    }

    template<typename T>
    auto connect(T* receiver, R(T::*f)(Args...))
    {   
        std::scoped_lock lk{lock};
        slots.emplace_front([receiver, f](Args&&... args){
            (receiver->*f)(std::forward<Args>(args)...);
        });

        return Connection<R(Args...), R, Args...>{this, slots.begin()};
    }
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
