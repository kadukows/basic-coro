#include <iostream>
#include <memory>

#include <basiccoro/task.hpp>
#include <basiccoro/awaitables.hpp>

basiccoro::AwaitableTask<std::unique_ptr<int>> consumer(basiccoro::SingleEvent<int>& event)
{
    std::cout << "consumer: start waiting" << std::endl;

    while (true)
    {
        const auto i = co_await event;
        std::cout << "consumer: received: " << i << std::endl;
    }
}

int main()
{
    basiccoro::SingleEvent<int> event;
    consumer(event);

    while (true)
    {
        int i = 0;

        std::cout << "Enter no.(1-9): ";
        std::cin >> i;

        if (i == 0)
        {
            break;
        }
        else if (1 <= i && i <= 9)
        {
            event.set(i);
        }
    }
}
