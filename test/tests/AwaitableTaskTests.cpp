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
    struct SetBoolWhenDestroyed
    {
        ~SetBoolWhenDestroyed() { b = true; }

        bool& b;
    };

    SingleEvent<void> e_;
};

TEST_F(AwaitableTaskShould, ProperlyReportIfItsDone)
{
    const auto sut_done = [this]() -> AwaitableTask<void>
        {
            co_return;
        };
    const auto sut_not_done = [this]() -> AwaitableTask<void>
        {
            co_await e_;
            co_return;
        };

    EXPECT_TRUE(sut_done().done());
    EXPECT_FALSE(sut_not_done().done());
}

TEST_F(AwaitableTaskShould, StartItselfEagearlyThenResumeItself)
{
    const auto sut = [this](int &i) -> AwaitableTask<void>
        {
            i += 1;
            co_await e_;
            i += 1;
            co_await e_;
            i += 1;
            co_return;
        };

    int tested = 0;
    sut(tested);
    EXPECT_EQ(tested, 1);
    e_.set();
    EXPECT_EQ(tested, 2);
    e_.set();
    EXPECT_EQ(tested, 3);
}

TEST_F(AwaitableTaskShould, BeDestroyedWithTaskObjectIfItDoesntAwait)
{
    const auto sut = [](bool &b) -> AwaitableTask<std::unique_ptr<SetBoolWhenDestroyed>>
        {
            co_return std::make_unique<SetBoolWhenDestroyed>(b);
        };

    bool tested = false;
    {
        auto task = sut(tested);
        EXPECT_FALSE(tested);
    }
    EXPECT_TRUE(tested);
}

TEST_F(AwaitableTaskShould, PersistWithoutTaskObjectIfItAwaits)
{
    const auto sut = [this](bool &b) -> AwaitableTask<std::unique_ptr<SetBoolWhenDestroyed>>
        {
            co_await e_;
            co_return std::make_unique<SetBoolWhenDestroyed>(b);
        };

    bool tested = false;
    {
        auto task = sut(tested);
        EXPECT_FALSE(tested);
    }
    EXPECT_FALSE(tested);
    e_.set();
    EXPECT_TRUE(tested);
}

TEST_F(AwaitableTaskShould, SwitchBetweenAwaitingTargets)
{
    const auto coro_add = [this](int i, int j) -> AwaitableTask<int>
        {
            co_await e_;
            co_return i + j;
        };

    const auto sut = [this, &coro_add](int &i) -> AwaitableTask<void>
        {
            i += co_await coro_add(40, 2);
            co_await e_;
            i += 1;
        };

    int i = 0;
    sut(i);

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
    const auto assigner = [&coro_adder](auto& result, int i, int j) -> AwaitableTask<void>
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
    const auto coro_await_event = [](auto& event) -> AwaitableTask<void>
        {
            while (true)
            {
                co_await event;
            }
        };

    const auto sut = [&coro_await_event](auto& event, bool& b) -> AwaitableTask<void>
        {
            SetBoolWhenDestroyed toBeDestroyed{b};
            co_await coro_await_event(event);
        };

    auto* event = new SingleEvent<void>();
    bool tested = false;
    sut(*event, tested);

    EXPECT_FALSE(tested);
    event->set();
    EXPECT_FALSE(tested);
    delete event;
    EXPECT_TRUE(tested);
}

using DeathTest_AwaitableTaskShould = AwaitableTaskShould;

// Pleasse be aware that even if AwaitablePromise::storeWaiting method throws exception upon
// double await, this exception also triggers PromiseBase::unhandled_exception (and terminates the program)
TEST_F(DeathTest_AwaitableTaskShould, TerminateAfterDoubleAwaiting)
{
    const auto sut = [this]() -> AwaitableTask<void>
        {
            co_await e_;
        };
    const auto wait_on = [](auto& awaitable) -> AwaitableTask<void>
        {
            co_await awaitable;
        };

    auto task = sut();
    wait_on(task);

    EXPECT_EXIT(wait_on(task), KilledBySignal(SIGABRT), "");
}

TEST_F(DeathTest_AwaitableTaskShould, TerminateAfterUncaughtException)
{
    const auto sut = [this]() -> AwaitableTask<void>
        {
            co_await e_;
            throw std::runtime_error("");
        };

    sut();
    EXPECT_EXIT(e_.set(), KilledBySignal(SIGABRT), "");
}

}  // namespace basiccoro
