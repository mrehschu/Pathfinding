#include "Coroutine.h"

Coroutine Coroutine::promise_type::get_return_object()
{
    return Coroutine { std::coroutine_handle<promise_type>::from_promise(*this) };
}

void Coroutine::promise_type::unhandled_exception()
{
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


suspend_if::suspend_if(const bool criterium) : criteriumValue(criterium)
{
    evaluationType = EvaluationType::ByValue;
}

suspend_if::suspend_if(std::function<bool()> criterium) : criteriumExpression(criterium)
{
    evaluationType = EvaluationType::ByExpression;
}

suspend_if::suspend_if(bool* criterium) : criteriumRefrence(criterium)
{
    evaluationType = EvaluationType::ByRefrence;
    if (criterium == nullptr) throw std::exception("Criterium can't be nullptr.");
}

bool suspend_if::await_ready()
{
    switch (evaluationType)
    {
        case EvaluationType::ByValue: return !criteriumValue;
        case EvaluationType::ByRefrence: return !*criteriumRefrence;
        case EvaluationType::ByExpression: return !criteriumExpression();
    }

    throw std::exception("I honestly don't know how you got here.");
}