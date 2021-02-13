#pragma once

#include <coroutine>
#include <optional>
#include <type_traits>
#include <vector>

namespace basiccoro
{
namespace detail
{

template<class Event>
class AwaiterBase
{
public:
    AwaiterBase(Event& event)
        : event_(event)
    {}

    bool await_ready()
    {
        if (event_.isSet())
        {
            // unset already set event, than continue coroutine
            event_.isSet_ = false;
            return true;
        }

        return false;
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        event_.waiting_.push_back(handle);
    }

    typename Event::value_type await_resume()
    {
        if constexpr (!std::is_same_v<typename Event::value_type, void>)
        {
            return event_.value();
        }
    }

private:
    Event& event_;
};

class SingleEventBase
{
public:
    ~SingleEventBase()
    {
        for (auto handle : waiting_)
        {
            handle.destroy();
        }
    }

    template<class T>
    friend class AwaiterBase;

    bool isSet() const { return isSet_; }

protected:
    void set_common()
    {
        if (!isSet_)
        {
            if (waiting_.empty())
            {
                isSet_ = true;
            }
            else
            {
                auto temp = std::move(waiting_);

                for (auto handle : temp)
                {
                    handle.resume();
                }
            }
        }
    }

private:
    std::vector<std::coroutine_handle<>> waiting_;
    bool isSet_ = false;
};

}  // namespace detail

template<class T>
class SingleEvent : public detail::SingleEventBase
{
public:
    using value_type = T;
    using awaiter = detail::AwaiterBase<SingleEvent<T>>;

    void set(T t) { result = t; set_common(); }
    const T& value() const { return result.value(); }
    awaiter operator co_await() { return awaiter{*this}; }

private:
    std::optional<T> result;
};

template<>
class SingleEvent<void> : public detail::SingleEventBase
{
public:
    using value_type = void;
    using awaiter = detail::AwaiterBase<SingleEvent<void>>;

    void set() { set_common(); }
    awaiter operator co_await() { return awaiter{*this}; }
};

}  // namespace basiccoro
