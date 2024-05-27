#include "src/util/Signal.h"

#include <gtest/gtest.h>

namespace {
using namespace util::signal;

struct Foo {
    int result = 0;
    void foo() { result++; }
    void set(int a) { result = a; }
};

struct Sender {
    Signal<void()> sig0;
    Signal<void(int)> sig1;
};

class SignalTest : public ::testing::Test {
public:
    SignalTest() = default;

    Sender sender;
    Foo foo;
};

TEST_F(SignalTest, connectNoParameter)
{
    connect(&sender, &Sender::sig0, &foo, &Foo::foo);
    sender.sig0();

    EXPECT_EQ(foo.result, 1);
}

TEST_F(SignalTest, connectWithParameter)
{
    Signal<void(int)> sig;
    Foo foo;
    connect(&sender, &Sender::sig1, &foo, &Foo::set);

    sender.sig1(10);
    EXPECT_EQ(foo.result, 10);

    sender.sig1(5);
    EXPECT_EQ(foo.result, 5);
}

TEST_F(SignalTest, disconnect)
{
    auto connection = connect(&sender, &Sender::sig0, &foo, &Foo::foo);
    sender.sig0();

    EXPECT_EQ(foo.result, 1);

    connection.disconnect();
    
    sender.sig0();

    EXPECT_EQ(foo.result, 1);
}

}