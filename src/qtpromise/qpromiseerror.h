#ifndef QTPROMISE_QPROMISEERROR_H
#define QTPROMISE_QPROMISEERROR_H

// QtPromise
#include "qpromiseglobal.h"

namespace QtPromise {

class QPromiseError
{
public:
    template <typename T>
    QPromiseError(const T& value)
    {
        try {
            throw value;
        } catch (...) {
            m_exception = std::current_exception();
        }
    }

    QPromiseError(const std::exception_ptr& exception)
        : m_exception(exception)
    { }

    QPromiseError(const QPromiseError& error)
        : m_exception(error.m_exception)
    { }

    QPromiseError(QPromiseError&& other)
        : m_exception(nullptr)
    {
        swap(other);
    }

    QPromiseError& operator =(QPromiseError other)
    {
        swap(other);
        return *this;
    }

    void swap(QPromiseError& other)
    {
        qSwap(m_exception, other.m_exception);
    }

    void rethrow() const
    {
        std::rethrow_exception(m_exception);
    }

private:
    std::exception_ptr m_exception;
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEERROR_H
