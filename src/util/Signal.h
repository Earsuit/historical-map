#ifndef SRC_UTIL_SIGNAL_H
#define SRC_UTIL_SIGNAL_H

#include "src/util/TypeTraits.h"

#include <list>
#include <functional>
#include <mutex>
#include <type_traits>

namespace util::signal {
template<typename T>
class Connection
{};

template<typename R, typename... Args>
class Connection<R(Args...)>;

template<typename F>
class Signal {
};

template<typename T, typename Y, typename Rs, typename... Parms>
Connection<Rs(Parms...)> connect(T* sender, Signal<Rs(Parms...)> T::* signal, Y* receiver, Rs(Y::*f)(Parms...))
{
    return (sender->*signal).connect(receiver, f);
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

    friend class Connection<R(Args...)>;

    template<typename T, typename Y, typename Rs, typename... Parms>
    friend Connection<Rs(Parms...)> connect(T* sender, Signal<Rs(Parms...)> T::* signal, Y* receiver, Rs(Y::*f)(Parms...));

private:
    std::recursive_mutex lock;
    std::list<std::function<R(Args...)>> slots;

    void disconnect(const Connection<R(Args...)>& connection)
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

        return Connection<R(Args...)>{this, slots.begin()};
    }
};

template<typename R, typename... Args>
class Connection<R(Args...)> {
public:
    Connection() = default;

    Connection(Signal<R(Args...)>* signal, std::list<std::function<R(Args...)>>::iterator it):
        signal{signal},
        it{it}
    {}

    void disconnect()
    {
        signal->disconnect(*this);
    }

    friend class Signal<R(Args...)>;

private:
    Signal<R(Args...)>* signal;
    std::list<std::function<R(Args...)>>::iterator it;
};
}

#endif
