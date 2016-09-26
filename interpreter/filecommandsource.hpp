/**
  *  \file interpreter/filecommandsource.hpp
  */
#ifndef C2NG_INTERPRETER_FILECOMMANDSOURCE_HPP
#define C2NG_INTERPRETER_FILECOMMANDSOURCE_HPP

#include "interpreter/commandsource.hpp"
#include "afl/io/textfile.hpp"

namespace interpreter {

    /** File command source.
        Provides script lines from a text file. */
    class FileCommandSource : public CommandSource {
     public:
        /** File command source.
            \param tf Text file to read from */
        FileCommandSource(afl::io::TextFile& tf);

        virtual void readNextLine();
        virtual bool setCharsetNew(afl::charset::Charset* cs);
        virtual void addTraceTo(Error& e, afl::string::Translator& tx);

     private:
        afl::io::TextFile& m_textFile;
    };

}

#endif
