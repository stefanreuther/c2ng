/**
  *  \file interpreter/commandsource.hpp
  *  \brief Class interpreter::CommandSource
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
        Provides a stream of script lines, and holds a Tokenizer, for use by the compiler.
        Derived classes implement readNextLine by calling setNextLine or setEOF. */
    class CommandSource {
     public:
        /** Default constructor. */
        CommandSource();

        /** Destructor. */
        virtual ~CommandSource();

        /** Read next line.
            This function must read a line from the input file and call setNextLine() or setEOF(). */
        virtual void readNextLine() = 0;

        /** Set character set.
            This implements the "Option Encoding" command and configures the character set for the file being read.
            The character set object is always newly-allocated, and the CommandSource will take ownership.

            The CommandSource may not support character sets.
            In this case, this method should delete \c cs and return \c false, failing the "Option Encoding" command.

            \param cs Newly-allocated character set
            \retval true Character set has been set
            \retval false This command source does not support character sets */
        virtual bool setCharsetNew(afl::charset::Charset* cs) = 0;

        /** Add trace to an error.
            This should call e.addTrace() with a string describing the current position
            (file name, line number) of this CommandSource, if these can be meaningfully defined.
            \param e [in/out] Error to annotate
            \param tx [in] Translator */
        virtual void addTraceTo(Error& e, afl::string::Translator& tx) = 0;

        /** Check for end-of-file.
            This returns true after readNextLine() calls setEOF().
            \return true End-of-file reached */
        bool isEOF() const;

        /** Get line number.
            Returns the current line number.
            Each call to setNextLine() advances the line number by one, starting with 1.
            \return line number */
        int getLineNumber() const;

        /** Access tokenizer.
            \return tokenizer */
        Tokenizer& tokenizer();

     protected:
        /** Set next input line.
            \param s Line (in UTF-8 encoding) as read from the file */
        void setNextLine(String_t s);

        /** Set end-of-file.
            Call if no more lines can be read. */
        void setEOF();

     private:
        Tokenizer m_tokenizer;
        int m_lineNr;
        bool m_eof;
    };

}

/**************************** Inline Functions ***************************/

inline void
interpreter::CommandSource::setEOF()
{
    // ex IntCommandSource::setEOF
    m_eof = true;
}

inline bool
interpreter::CommandSource::isEOF() const
{
    // ex IntCommandSource::isEOF
    return m_eof;
}

inline int
interpreter::CommandSource::getLineNumber() const
{
    // ex IntCommandSource::getLineNumber
    return m_lineNr;
}

inline interpreter::Tokenizer&
interpreter::CommandSource::tokenizer()
{
    // ex IntCommandSource::getTokenizer
    return m_tokenizer;
}

#endif
