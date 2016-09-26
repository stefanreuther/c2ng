/**
  *  \file interpreter/commandsource.hpp
  */
#ifndef C2NG_INTERPRETER_COMMANDSOURCE_HPP
#define C2NG_INTERPRETER_COMMANDSOURCE_HPP

#include "interpreter/tokenizer.hpp"
#include "afl/charset/charset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace interpreter {

    class Error;

    /** Command source.
        Provides a stream of script lines, as well as a tokenizer, for use by the compiler.
        Derived classes implement readNextLine by calling setNextLine or setEOF. */
    class CommandSource {
     public:
        CommandSource();
        virtual ~CommandSource();
        virtual void readNextLine() = 0;
        virtual bool setCharsetNew(afl::charset::Charset* cs) = 0;
        virtual void addTraceTo(Error& e, afl::string::Translator& tx) = 0;

        bool isEOF() const;
        int getLineNumber() const;

        Tokenizer& tokenizer();

     protected:
        void setNextLine(String_t s);
        void setEOF();

     private:
        Tokenizer m_tokenizer;
        int m_lineNr;
        bool m_eof;
    };

}

/**************************** Inline Functions ***************************/

/** Set EOF. Call this from readNextLine when you reached end of file. */
inline void
interpreter::CommandSource::setEOF()
{
    // ex IntCommandSource::setEOF
    m_eof = true;
}

/** Check for EOF. */
inline bool
interpreter::CommandSource::isEOF() const
{
    // ex IntCommandSource::isEOF
    return m_eof;
}

/** Get line number. */
inline int
interpreter::CommandSource::getLineNumber() const
{
    // ex IntCommandSource::getLineNumber
    return m_lineNr;
}

/** Get embedded tokenizer. */
inline interpreter::Tokenizer&
interpreter::CommandSource::tokenizer()
{
    // ex IntCommandSource::getTokenizer
    return m_tokenizer;
}

#endif
