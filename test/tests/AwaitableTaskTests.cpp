#include <memory>

#include <gmock/gmock.h>

#include <basiccoro/AwaitableTask.hpp>
#include <basiccoro/SingleEvent.hpp>

namespace basiccoro
{

using namespace ::testing;

class AwaitableTaskShould : public Test
{
protected:

    SingleEvent<void> e_;
};

TEST_F(AwaitableTaskShould, StartItselfEagearlyThenResumeItself)
{
    const auto coro = [this](int &i) -> AwaitableTask<void>
        {
            i += 1;
            co_await e_;
            i += 1;
            co_await e_;
            i += 1;
        };

    int tested = 0;
    coro(tested);
    EXPECT_EQ(tested, 1);
    e_.set();
    EXPECT_EQ(tested, 2);
    e_.set();
    EXPECT_EQ(tested, 3);
}

TEST_F(AwaitableTaskShould, SwitchBetweenAwaitingTargets)
{
    const auto coro_add = [this](int i, int j) -> AwaitableTask<int>
        {
            co_await e_;
            co_return i + j;
        };

    const auto coro = [this, &coro_add](int &i) -> AwaitableTask<void>
        {
            i += co_await coro_add(40, 2);
            co_await e_;
            i += 1;
        };

    int i = 0;
    coro(i);

    EXPECT_EQ(i, 0);
    e_.set();
    EXPECT_EQ(i, 42);
    e_.set();
    EXPECT_EQ(i, 43);
}


TEST_F(AwaitableTaskShould, WorkProperlyWithMoveOnlyObjects)
{
    const auto coro_adder = [this](int i, int j) -> AwaitableTask<std::unique_ptr<int>>
        {
            co_await e_;
            co_return std::make_unique<int>(i + j);
        };
    const auto assigner = [&](auto& result, int i, int j) -> AwaitableTask<void>
        {
            result = co_await coro_adder(i, j);
        };

   std::unique_ptr<int> result;
   assigner(result, 40, 2);

   EXPECT_FALSE(result);
   e_.set();
   EXPECT_THAT(result, Pointee(Eq(42)));
}

TEST_F(AwaitableTaskShould, ProperlyDestroyAwaitingChildsAfterDestroyingParentEvent)
{
    struct SetBoolWhenDestroyed
    {
        ~SetBoolWhenDestroyed() { b = true; }

        bool& b;
    };

    const auto coro = [](auto& event, bool& b) -> AwaitableTask<void>
        {
            SetBoolWhenDestroyed toBeDestroyed{b};
            while (true)
            {
                co_await event;
            }
        };

    auto* event = new SingleEvent<void>();
    bool tested = false;
    coro(*event, tested);

    EXPECT_FALSE(tested);
    event->set();
    EXPECT_FALSE(tested);
    delete event;
    EXPECT_TRUE(tested);
}

}  // namespace basiccoro
