/**
  *  \file interpreter/error.hpp
  *  \brief Class interpreter::Error
  */
#ifndef C2NG_INTERPRETER_ERROR_HPP
#define C2NG_INTERPRETER_ERROR_HPP

#include <stdexcept>
#include "afl/string/string.hpp"

namespace interpreter {

    /** Interpreter error.
        The basic idea is to generate most error messages by explicit calls to constructor methods (e.g. Error::rangeError()).
        This allows us to associate a little bit of meta-information with errors,
        and generate the final message when the error is presented to the user.
        So far, only dumb strings are implemented however.

        Note that the backtrace of an error can be obtained from the process' state and need not be stored with the error message.
        It will be added to the message when the BCO executer finds the error.

        Error messages are not internationalized.

        FIXME: if a user process does
           Try / ... / Else / Throw System.Error / EndTry
        it would lose this meta-information because the error message is
        converted to a string inbetween. */
    class Error : public std::exception {
     public:
        Error(const char* error);
        Error(String_t error);
        ~Error() throw();

        const char* what() const throw();
        String_t getTrace() const
            { return m_trace; }

        void addTrace(String_t line);

        enum ExpectedType {
            ExpectNone,
            ExpectInteger,
            ExpectBool,                 // needed?
            ExpectNumeric,
            ExpectBaseType,
            ExpectString,
            ExpectIndexable,
            ExpectIterable,
            ExpectRecord,
            ExpectCallable,
            ExpectProcedure,
            ExpectKeymap,
            ExpectBlob,
            ExpectFile,
            ExpectArray
        };

        static Error unknownIdentifier(const String_t& name);
        static Error typeError(ExpectedType expectedType = ExpectNone);
        static Error internalError(const char* msg);
        static Error notSerializable();
        static Error notAssignable();
        static Error rangeError();
        static Error invalidMultiline();
        static Error expectKeyword(const char* kw);
        static Error expectKeyword(const char* kw1, const char* kw2);
        static Error expectSymbol(const char* sym);
        static Error expectSymbol(const char* sym1, const char* sym2);
        static Error misplacedKeyword(const char* kw);
        static Error garbageAtEnd(bool expression);
        static Error expectIdentifier(const char* what);
        static Error contextError();

     private:
        String_t m_error;
        String_t m_trace;
    };

}

#endif
