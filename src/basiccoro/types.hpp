#pragma once

#include <coroutine>
#include <type_traits>

namespace basiccoro
{

template<class Promise>
using NeedsToBeDestroyedAfterDone = std::is_same<decltype(std::declval<Promise>().final_suspend()), std::suspend_always>;

}  // nmaespace basiccoro
