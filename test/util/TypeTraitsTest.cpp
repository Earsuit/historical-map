#include "src/util/TypeTraits.h"

#include <gtest/gtest.h>

#include <vector>
#include <iostream>

struct Foo {
    int foo;
};

template<typename T>
void forwardFromRvalue(T&& v)
{
    for (auto&& i : v) {
        static_assert(std::is_rvalue_reference_v<decltype(util::forward_if<T>(i))>);
    }
}

template<typename T>
void forwardFromLvalue(T&& v)
{
    for (auto&& i : v) {
        static_assert(std::is_lvalue_reference_v<decltype(util::forward_if<T>(i))>);
    }
}

template<typename T>
void forwardFromConstLvalue(T&& v)
{
    for (auto&& i : v) {
        static_assert(std::is_lvalue_reference_v<decltype(util::forward_if<T>(i))>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(util::forward_if<T>(i))>>);
    }
}

TEST(TypeTraitsTest, forward_ifOnRvalue)
{
    forwardFromRvalue(std::vector<Foo>{});
}

TEST(TypeTraitsTest, forward_ifOnLvalue)
{
    std::vector<Foo> vec;
    forwardFromLvalue(vec);
}

TEST(TypeTraitsTest, forward_ifOnConstLvalue)
{
    const std::vector<Foo> vec;
    forwardFromConstLvalue(vec);
}