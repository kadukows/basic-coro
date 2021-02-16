#include <array>

#include <gmock/gmock.h>

#include <basiccoro/AwaitableTask.hpp>
#include <basiccoro/SingleEvent.hpp>

namespace basiccoro
{

using namespace ::testing;

class SingleEventShould : public Test
{
protected:
    struct AddOneWhenDestroyed
    {
        ~AddOneWhenDestroyed() { i += 1; }

        int& i;
    };
};

TEST_F(SingleEventShould, ResumeAllAwaitingCoroutines)
{
    const auto lazyAddOne = [](auto& awaitable, int& result) -> AwaitableTask<void>
        {
            co_await awaitable;
            result += 1;
        };

    SingleEvent<void> sut;
    int tested = 0;

    constexpr std::size_t CHILD_NO = 10;
    AwaitableTask<void> tasks[CHILD_NO];
    for (auto&& task : tasks)
    {
        task = lazyAddOne(sut, tested);
    }

    EXPECT_EQ(tested, 0);
    sut.set();
    EXPECT_EQ(tested, CHILD_NO);
}

TEST_F(SingleEventShould, DestroyAllAwaitingCoroutinesWithTheirself)
{
    const auto addOneAfterDestruction = [](auto& awaitable, int& result) -> AwaitableTask<void>
        {
            AddOneWhenDestroyed a{result};
            while (true)
            {
                co_await awaitable;
            }
        };

    auto* sut = new SingleEvent<void>();
    int tested = 0;

    constexpr std::size_t CHILD_NO = 25;
    AwaitableTask<void> tasks[CHILD_NO];
    for (auto&& task : tasks)
    {
        task = addOneAfterDestruction(*sut, tested);
    }

    EXPECT_EQ(tested, 0);
    sut->set();
    EXPECT_EQ(tested, 0);
    delete sut;
    EXPECT_EQ(tested, CHILD_NO);
}

}  // namespace basiccoro
