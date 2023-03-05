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

        Error is used for errors during compilation and execution.

        FIXME: if a user process does
           Try / ... / Else / Throw System.Error / EndTry
        it would lose this meta-information because the error message is
        converted to a string inbetween. */
    class Error : public std::exception {
     public:
        /** Construct from string literal.
            \param error text */
        Error(const char* error);

        /** Construct from string.
            \param error text */
        Error(String_t error);

        /** Destructor. */
        ~Error() throw();

        /** Get error text.
            \return text */
        const char* what() const throw();

        /** Get trace.
            \return trace as a single text */
        String_t getTrace() const;

        /** Add line to trace.
            If the trace is already nonempty, adds a newline before appending the new line.
            \param line New line */
        void addTrace(String_t line);

        /** Type identification for typeError(). */
        enum ExpectedType {
            ExpectNone,
            ExpectInteger,
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

        /** Generate an "unknown identifier" error for a given identifier.
            \param name Identifier
            \return Error object */
        static Error unknownIdentifier(const String_t& name);

        /** Generate a "type mismatch" error.
            \param expectedType Expected Type
            \return Error object */
        static Error typeError(ExpectedType expectedType = ExpectNone);

        /** Generate an internal error.
            \param msg Error identification (not translated)
            \return Error object */
        static Error internalError(const char* msg);

        /** Generate an "object not serializable" error.
            Generate this error in your implementation of BaseValue::store if your object cannot serialize.
            \return Error object */
        static Error notSerializable();

        /** Generate a "not assignable" error.
            Generate this error in your implementation of Context::set or IndexableValue::set
            when asked to assign to a read-only property.
            \return Error object */
        static Error notAssignable();

        /** Generate a range error.
            Generate this error whenever some out-of-range operation is attempted.
            \return Error object */
        static Error rangeError();

        /** Generate a "multi-line not allowed" error.
            Generate this error during compilation when a multiline statement appears in a place it is not allowed.
            \return Error object */
        static Error invalidMultiline();

        /** Generate a "expecting keyword" error, one permitted keyword.
            \param kw Expected keyword (in TitleCase)
            \return Error object */
        static Error expectKeyword(const char* kw);

        /** Generate a "expecting keyword" error, two permitted keywords.
            \param kw1 Expected keyword (in TitleCase)
            \param kw2 Alternative keyword (in TitleCase)
            \return Error object */
        static Error expectKeyword(const char* kw1, const char* kw2);

        /** Generate a "expecting symbol" error, one permitted symbol.
            \param sym Expected symbol
            \return Error object */
        static Error expectSymbol(const char* sym);

        /** Generate a "expecting symbol" error, two permitted symbols.
            \param sym1 Expected symbol
            \param sym2 Alternative symbol
            \return Error object */
        static Error expectSymbol(const char* sym1, const char* sym2);

        /** Generate a "keyword not expected here" error.
            \param kw Keyword (in TitleCase if possible)
            \return Error object */
        static Error misplacedKeyword(const char* kw);

        /** Generate an "expecting end of line" error.
            Generate this error during compilation if you expected the line to end but there are still tokens following.
            \param expression true if the last production was an expression
            \return Error object */
        static Error garbageAtEnd(bool expression);

        /** Generate an "expecting identifier" error.
            \param what What the identifier will name (e.g. "keymap name")
            \return Error object */
        static Error expectIdentifier(const char* what);

        /** Generate a "command not valid in this context" error.
            Generate this error if the current runtime situation does not permit a command to be executed
            (e.g. GUI command when GUI is not running).
            \return Error object */
        static Error contextError();

        /** Generate a "too complex" error.
            Generate this error if an interpreter limit (not a game or language limit) is exceeded.
            \return Error object */
        static Error tooComplex();

        /** Generate a "too many arguments" error.
            \param fn Function/subroutine name */
        static Error tooManyArguments(const String_t& fn);

        /** Generate a "too few arguments" error.
            \param fn Function/subroutine name */
        static Error tooFewArguments(const String_t& fn);

     private:
        String_t m_error;
        String_t m_trace;
    };

}

inline String_t
interpreter::Error::getTrace() const
{
    return m_trace;
}

#endif
