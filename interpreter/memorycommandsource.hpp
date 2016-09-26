/**
  *  \file interpreter/memorycommandsource.hpp
  */
#ifndef C2NG_INTERPRETER_MEMORYCOMMANDSOURCE_HPP
#define C2NG_INTERPRETER_MEMORYCOMMANDSOURCE_HPP

#include <vector>
#include "interpreter/commandsource.hpp"

namespace interpreter {

    /** Memory command source.
        Provides script lines from a memory buffer. */
    class MemoryCommandSource : public CommandSource {
     public:
        /** Constructor.
            Makes a blank command source. */
        MemoryCommandSource();

        /** Constructor.
            Makes a command source containing a single line.
            \param line The line */
        MemoryCommandSource(String_t line);

        /** Add line to this command source.
            \param line The line */
        void addLine(const String_t& line);

        virtual void readNextLine();
        virtual bool setCharsetNew(afl::charset::Charset* cs);
        virtual void addTraceTo(Error& e, afl::string::Translator& tx);

     private:
        std::vector<String_t> m_lines;
        size_t m_index;
    };

}

#endif
