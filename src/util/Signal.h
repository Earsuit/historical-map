#ifndef SRC_UTIL_SIGNAL_H
#define SRC_UTIL_SIGNAL_H

#include "src/util/TypeTraits.h"

#include <list>
#include <functional>
#include <mutex>
#include <type_traits>
#include <map>
#include <set>

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

template<typename T, typename Y, typename Rs, typename... Parms>
void disconnectAll(T* sender, Signal<Rs(Parms...)> T::* signal, Y* receiver)
{
    (sender->*signal).disconnect(reinterpret_cast<void*>(receiver));
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

    template<typename T, typename Y, typename Rs, typename... Parms>
    friend void disconnectAll(T* sender, Signal<Rs(Parms...)> T::* signal, Y* receiver);

private:
    using ItType = std::list<std::function<R(Args...)>>::iterator;
    struct IteratorCmp
    {
        bool operator()(const ItType& lhs, const ItType& rhs) const
        {
            // the original compare logic is wrong,
            // we have to follow strict weak ordering
            return &(*lhs) < &(*rhs);
        }
    };

    std::recursive_mutex lock;
    std::list<std::function<R(Args...)>> slots;
    std::map<void*, std::set<ItType, IteratorCmp>> its;

    void disconnect(const Connection<R(Args...)>& connection)
    {
        std::scoped_lock lk{lock};
        its[connection.receiver].erase(connection.it);
        slots.erase(connection.it);
    }

    void disconnect(void* receiver)
    {
        std::scoped_lock lk{lock};
        if (its.contains(receiver)) {
            for (const auto it : its[receiver]) {
                slots.erase(it);
            }

            its.erase(receiver);
        }
    }

    template<typename T>
    auto connect(T* receiver, R(T::*f)(Args...))
    {   
        std::scoped_lock lk{lock};
        slots.emplace_front([receiver, f](Args&&... args){
            (receiver->*f)(std::forward<Args>(args)...);
        });

        const auto it = slots.begin();
        void* ptr = reinterpret_cast<void*>(receiver);
        if (its.contains(ptr)) {
            its[ptr].emplace(it);
        } else {
            its.emplace(std::make_pair(ptr, std::set<ItType, IteratorCmp>{it}));
        }

        return Connection<R(Args...)>{reinterpret_cast<void*>(receiver), this, it};
    }
};

template<typename R, typename... Args>
class Connection<R(Args...)> {
public:
    Connection():
        signal{nullptr}
    {}

    Connection(void* receiver, Signal<R(Args...)>* signal, std::list<std::function<R(Args...)>>::iterator it):
        receiver{receiver},
        signal{signal},
        it{it}
    {}

    void disconnect()
    {
        if (signal != nullptr) {
            signal->disconnect(*this);
            signal = nullptr;
        }
    }

    friend class Signal<R(Args...)>;

private:
    void* receiver;
    Signal<R(Args...)>* signal;
    std::list<std::function<R(Args...)>>::iterator it;
};
}

#endif
