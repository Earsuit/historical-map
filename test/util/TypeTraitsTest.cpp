#include "src/util/TypeTraits.h"

#include <gtest/gtest.h>

#include <vector>
#include <iostream>

using namespace util;

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

TEST(TypeTraitsTest, is_alll_same)
{
    static_assert(is_all_same_v<param_pack<int>, param_pack<int>>);    
    static_assert(is_all_same_v<param_pack<int, float, int>, param_pack<int, float, int>>);
    static_assert(is_all_same_v<param_pack<int, float, int&>, param_pack<int, float, int>>);
    static_assert(is_all_same_v<param_pack<int, float, int&&>, param_pack<int, float, int>>);
    static_assert(!is_all_same_v<param_pack<int, float, int>, param_pack<int, float>>);
    static_assert(!is_all_same_v<param_pack<int, float, int>, param_pack<int, float, float>>);
}