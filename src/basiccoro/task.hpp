#pragma once

#include <coroutine>
#include <exception>
#include <stdexcept>
#include <utility>

#include "types.hpp"

namespace basiccoro
{
namespace detail
{

template<class Derived>
struct PromiseBase
{
    auto get_return_object() { return std::coroutine_handle<Derived>::from_promise(static_cast<Derived&>(*this)); }
    void unhandled_exception() { std::terminate(); }
};

template<class Derived, class T>
struct ValuePromise : public PromiseBase<Derived>
{
    using value_type = T;
    T val;
    void return_value(T t) { val = std::move(t); }
};

template<class Derived>
struct ValuePromise<Derived, void> : public PromiseBase<Derived>
{
    using value_type = void;
    void return_void() {}
};

template<class T>
class AwaitablePromise : public ValuePromise<AwaitablePromise<T>, T>
{
public:
    auto initial_suspend() { return std::suspend_never(); }

    auto final_suspend()
    {
        if (waiting_)
        {
            waiting_.resume();
            waiting_ = nullptr;
        }

        return std::suspend_never();
    }

    void storeWaiting(std::coroutine_handle<> handle)
    {
        if (waiting_)
        {
            throw std::runtime_error("AwaitablePromise::storeWaiting(): already waiting");
        }

        waiting_ = handle;
    }

    ~AwaitablePromise()
    {
        if (waiting_ && !waiting_.done())
        {
            waiting_.destroy();
        }
    }

private:
    std::coroutine_handle<> waiting_ = nullptr;
};

template<class Promise>
class TaskBase
{
public:
    using promise_type = Promise;

    TaskBase(std::coroutine_handle<promise_type> handle);
    TaskBase(const TaskBase&) = delete;
    TaskBase(TaskBase&& other);
    TaskBase& operator=(const TaskBase&) = delete;
    TaskBase& operator=(TaskBase&& other) = delete;

    bool done() const { return handle_.done(); }

protected:
    std::coroutine_handle<promise_type> handle_;
};

template<class Promise>
TaskBase<Promise>::TaskBase(std::coroutine_handle<promise_type> handle)
    : handle_(handle)
{}

template<class Promise>
TaskBase<Promise>::TaskBase(TaskBase&& other)
    : handle_(std::exchange(other.handle_, nullptr))
{}

}  // namespace detail

template<class T>
class AwaitableTask : public detail::TaskBase<detail::AwaitablePromise<T>>
{
    using Base = detail::TaskBase<detail::AwaitablePromise<T>>;

public:
    AwaitableTask(std::coroutine_handle<typename Base::promise_type> handle)
        : Base(handle)
    {}

    class awaiter;
    friend class awaiter;
    awaiter operator co_await() const;
};

template<class T>
struct AwaitableTask<T>::awaiter
{
    bool await_ready()
    {
        return task_.done();
    }

    template<class Promise>
    void await_suspend(std::coroutine_handle<Promise> handle)
    {
        static_assert(!NeedsToBeDestroyedAfterDone<Promise>::value, "basiccoro::AwaitableTask::awaiter::await_suspend(): can be used only with auto destructing coroutines");
        task_.handle_.promise().storeWaiting(handle);
    }

    T await_resume()
    {
        if constexpr (!std::is_same_v<void, T>)
        {
            return std::move(task_.handle_.promise().val);
        }
    }

    const AwaitableTask& task_;
};

template<class T>
AwaitableTask<T>::awaiter AwaitableTask<T>::operator co_await() const
{
    return awaiter{*this};
}

}  // namespace basiccoro
