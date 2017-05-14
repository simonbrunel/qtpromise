#ifndef _QTPROMISE_QPROMISEGLOBAL_H
#define _QTPROMISE_QPROMISEGLOBAL_H

// QtCore
#include <QtGlobal>

// STL
#include <functional>

namespace QtPromisePrivate
{
// https://rmf.io/cxx11/even-more-traits#unqualified_types
template <typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

/*!
 * \struct ArgsOf
 * http://stackoverflow.com/a/7943765
 * http://stackoverflow.com/a/27885283
 */
template <typename... Args>
struct ArgsTraits
{
    using types = std::tuple<Args...>;
    using first = typename std::tuple_element<0, types>::type;
};

template <>
struct ArgsTraits<>
{
    using types = std::tuple<>;
    using first = void;
};

template <typename T>
struct ArgsOf : public ArgsOf<decltype(&T::operator())>
{ };

template <>
struct ArgsOf<std::nullptr_t> : public ArgsTraits<>
{ };

template <typename TReturn, typename... Args>
struct ArgsOf<TReturn(Args...)> : public ArgsTraits<Args...>
{ };

template <typename TReturn, typename... Args>
struct ArgsOf<TReturn(*)(Args...)> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...)> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) const> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) volatile> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) const volatile> : public ArgsTraits<Args...>
{ };

template <typename T>
struct ArgsOf<std::function<T> > : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<volatile T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const volatile T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<volatile T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const volatile T&&> : public ArgsOf<T>
{ };

/*!
 * \fn to_exception_ptr
 */
template <typename T>
std::exception_ptr to_exception_ptr(const T& value)
{
    try {
        throw value;
    } catch(...) {
        return std::current_exception();
    }
}

template <>
std::exception_ptr to_exception_ptr(const std::exception_ptr& value)
{
    return value;
}

} // namespace QtPromisePrivate

#endif // ifndef _QTPROMISE_QPROMISEGLOBAL_H
