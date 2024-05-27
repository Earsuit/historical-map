#include "src/util/Signal.h"

#include <gtest/gtest.h>

namespace {
using namespace util;

struct Foo {
    int result = 0;
    void foo() { result++; }
    void set(int a) { result = a; }
};

TEST(SignalTest, connectNoParameter)
{
    Signal<void()> sig;
    Foo foo;
    connect(sig, &foo, &Foo::foo);
    sig();

    EXPECT_EQ(foo.result, 1);
}

TEST(SignalTest, connectWithParameter)
{
    Signal<void(int)> sig;
    Foo foo;
    connect(sig, &foo, &Foo::set);

    sig(10);
    EXPECT_EQ(foo.result, 10);

    sig(5);
    EXPECT_EQ(foo.result, 5);
}

TEST(SignalTest, disconnect)
{
    Signal<void()> sig;
    Foo foo;
    auto connection = connect(sig, &foo, &Foo::foo);
    sig();

    EXPECT_EQ(foo.result, 1);

    connection.disconnect();
    
    sig();

    EXPECT_EQ(foo.result, 1);
}

}