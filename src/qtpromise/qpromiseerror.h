#ifndef _QTPROMISE_QPROMISEERROR_H
#define _QTPROMISE_QPROMISEERROR_H

// QtPromise
#include "qpromiseglobal.h"

namespace QtPromise {

class QPromiseError
{
public:
    QPromiseError()
        : exception(nullptr)
    { }

    template <typename T>
    QPromiseError(const T& value)
        : exception(nullptr)
    {
        try {
            throw value;
        } catch(...) {
            exception = std::current_exception();
        }
    }

    QPromiseError(const std::exception_ptr& exception)
        : exception(exception)
    { }

    void rethrow() const
    {
        std::rethrow_exception(exception);
    }

private:
    std::exception_ptr exception;
};

} // namespace QtPromise

#endif // _QTPROMISE_QPROMISEERROR_H
