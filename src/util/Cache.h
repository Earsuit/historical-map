#ifndef SRC_UTIL_CACHE
#define SRC_UTIL_CACHE

#include <map>
#include <list>

namespace util {
template<typename T, typename Y>
class Cache {
public:
    Cache(int size):
        size{size}
    {
    }

    void reset()
    {
        cache.clear();
        lifetime.clear();
        lifetimeIndex.clear();
    }

    void clear(T key)
    {
        cache.erase(key);
        lifetime.erase(lifetimeIndex[key]);
        lifetimeIndex.erase(key);
    }

    Y& operator[](const T& key)
    {
        if (!cache.contains(key)) {
            // there is no such year in cache before,
            // so this time must be an insert
            if (cache.size() == size) {
                cache.erase(lifetime.back());
                lifetimeIndex.erase(lifetime.back());
                lifetime.pop_back();
            }
        } else {
            // this key is accessed again, update it's lifetime
            lifetime.erase(lifetimeIndex[key]);
        }

        lifetime.push_front(key);
        lifetimeIndex[key] = lifetime.cbegin();

        return cache[key];
    }

    bool contains(const T& key) const
    {
        return cache.contains(key);
    }

private:
    int size;
    std::map<T, Y> cache;
    std::list<T> lifetime;
    std::map<T, typename std::list<T>::const_iterator> lifetimeIndex;
};
}

#endif /* SRC_UTIL_CACHE */
