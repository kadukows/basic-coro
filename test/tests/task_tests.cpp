#include <gtest/gtest.h>

#include <basiccoro/task.hpp>
#include <basiccoro/awaitables.hpp>

namespace basiccoro
{

class AwaitableTaskShould : public ::testing::Test
{
protected:
    AwaitableTask<void> setBoolThenHalt(bool& b)
    {
        b = true;
        co_await e_;
    }

    AwaitableTask<void> haltThenSetBool(bool& b)
    {
        co_await e_;
        b = true;
    }

    SingleEvent<void> e_;
};

TEST_F(AwaitableTaskShould, StartItselfEagearly)
{
    bool tested = false;
    setBoolThenHalt(tested);

    EXPECT_TRUE(tested);
}

TEST_F(AwaitableTaskShould, resumeItself)
{
    bool tested = false;
    haltThenSetBool(tested);
    e_.set();

    EXPECT_TRUE(tested);
}

}  // namespace basiccoro
