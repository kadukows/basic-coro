#include <gtest/gtest.h>

#include <basiccoro/task.hpp>
#include <basiccoro/awaitables.hpp>

namespace basiccoro
{

class AwaitableTaskShould : public ::testing::Test
{
protected:
    AwaitableTask<void> increaseValueThanHalt(int& i)
    {
        i += 1;
        co_await e_;
        i += 1;
    }

    AwaitableTask<void> haltThenIncreaseValue(int& i)
    {
        co_await e_;
        i += 1;
    }

    SingleEvent<void> e_;
};

TEST_F(AwaitableTaskShould, StartItselfEagearly)
{
    int tested = 0;
    increaseValueThanHalt(tested);

    EXPECT_EQ(tested, 1);
}

TEST_F(AwaitableTaskShould, ResumeItself)
{
    int tested = 0;
    haltThenIncreaseValue(tested);

    EXPECT_EQ(tested, 0);
    e_.set();
    EXPECT_EQ(tested, 1);
}

}  // namespace basiccoro
