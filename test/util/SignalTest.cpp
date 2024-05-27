#include "src/util/Signal.h"

#include <gtest/gtest.h>

namespace {
using namespace util::signal;

struct Foo {
    int item1 = 0;
    int item2 = 0;
    int item3 = 0;

    void plus() { item1++; }
    void minus() { item2--;}
    void set(int a) { item3 = a; }
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
    connect(&sender, &Sender::sig0, &foo, &Foo::plus);
    sender.sig0();

    EXPECT_EQ(foo.item1, 1);
}

TEST_F(SignalTest, connectWithParameter)
{
    Signal<void(int)> sig;
    Foo foo;
    connect(&sender, &Sender::sig1, &foo, &Foo::set);

    sender.sig1(10);
    EXPECT_EQ(foo.item3, 10);

    sender.sig1(5);
    EXPECT_EQ(foo.item3, 5);
}

TEST_F(SignalTest, disconnectWithConnection)
{
    Foo foo2;
    auto connection = connect(&sender, &Sender::sig0, &foo, &Foo::plus);
    connect(&sender, &Sender::sig0, &foo2, &Foo::plus);
    sender.sig0();

    EXPECT_EQ(foo.item1, 1);
    EXPECT_EQ(foo2.item1, 1);

    connection.disconnect();
    
    sender.sig0();

    EXPECT_EQ(foo.item1, 1);
    EXPECT_EQ(foo2.item1, 2);
}

TEST_F(SignalTest, disconnectWithObjPtr)
{
    Foo foo2;
    connect(&sender, &Sender::sig0, &foo, &Foo::plus);
    connect(&sender, &Sender::sig0, &foo, &Foo::minus);
    connect(&sender, &Sender::sig1, &foo, &Foo::set);
    connect(&sender, &Sender::sig0, &foo2, &Foo::plus);
    connect(&sender, &Sender::sig0, &foo2, &Foo::minus);

    sender.sig0();
    sender.sig1(10);

    EXPECT_EQ(foo.item1, 1);
    EXPECT_EQ(foo.item2, -1);
    EXPECT_EQ(foo2.item1, 1);
    EXPECT_EQ(foo2.item2, -1);
    EXPECT_EQ(foo.item3, 10);

    disconnectAll(&sender, &Sender::sig0, reinterpret_cast<void*>(&foo));
    
    sender.sig0();
    sender.sig1(20);

    EXPECT_EQ(foo.item1, 1);
    EXPECT_EQ(foo.item2, -1);
    EXPECT_EQ(foo2.item1, 2);
    EXPECT_EQ(foo2.item2, -2);
    EXPECT_EQ(foo.item3, 20);
}

}