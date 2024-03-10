#pragma once
#include <coroutine>
#include <functional>
#include <iostream>

struct Coroutine
{
    public:

    struct promise_type
    {
        Coroutine get_return_object();
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; };
        void unhandled_exception();
        void return_void() noexcept {}
    };

    Coroutine() { isDummy = true; }
    Coroutine(std::coroutine_handle<promise_type> handle) : handle(handle) {}
    void operator() () { if (!isDummy) handle.resume(); }
    bool isDone() { return isDummy || handle.done(); }

    private:

    std::coroutine_handle<promise_type> handle;
    bool isDummy = false;
};

struct suspend_if
{
    private:

    enum class EvaluationType {
        ByValue,
        ByRefrence,
        ByExpression
    };

    EvaluationType evaluationType;
    bool criteriumValue = false;
    bool* criteriumRefrence = nullptr;
    std::function<bool()> criteriumExpression;

    public:

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle) {}
    void await_resume() {}

    suspend_if(const bool criterium);
    suspend_if(bool* criterium);
    suspend_if(std::function<bool()> criterium);
};