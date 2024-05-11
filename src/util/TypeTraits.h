#ifndef SRC_UTIL_TYPE_TRAITS_H
#define SRC_UTIL_TYPE_TRAITS_H

#include <type_traits>
#include <iterator>

namespace util {
template<typename T>
struct is_point_to_const_value : std::false_type
{
};

template<typename T>
struct is_point_to_const_value<const T*> : std::true_type
{
};

template<typename T>
struct is_const_iterator
{
    static constexpr bool value = is_point_to_const_value<typename std::iterator_traits<T>::pointer>::value;
};

template<typename T>
inline constexpr bool is_const_iterator_v = is_const_iterator<T>::value;

template<typename F, typename R, typename... Args>
concept Callable = std::same_as<std::invoke_result_t<F, Args...>, R>;

template<typename T, typename U>
using ForwardType = std::conditional_t<std::is_lvalue_reference_v<T>,
                                       std::remove_reference_t<U>&,
                                       std::remove_reference_t<U>&&>;

template<typename T, typename U>
constexpr ForwardType<T, U> forward_if(U&& item)
{
    return static_cast<ForwardType<T, U>>(item);
}
}

#endif
