/**
  *  \file server/console/parser.hpp
  *  \brief Class server::console::Parser
  */
#ifndef C2NG_SERVER_CONSOLE_PARSER_HPP
#define C2NG_SERVER_CONSOLE_PARSER_HPP

#include <memory>
#include "afl/io/textreader.hpp"
#include "server/console/environment.hpp"
#include "afl/data/value.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace console {

    class CommandHandler;
    class Terminal;

    /** Shell command parser.
        This is the main command parser.
        It parses commands and executes them on the fly.
        It uses an Environment for variable expansion, a Terminal for output, a file system for input redirection,
        and a CommandHandler to actually execute commands. */
    class Parser {
     public:
        /** Evaluation result. */
        enum Result {
            End,                 /**< End of input reached, no command executed. */
            BlankLine,           /**< Blank line, no command executed. */
            Command              /**< Command executed normally. */
        };

        /** Constructor.
            \param env Environment for variable expansion
            \param term Terminal for output
            \param fs File system for input redirection
            \param handler Command execution */
        Parser(Environment& env, Terminal& term, afl::io::FileSystem& fs, CommandHandler& handler);

        /** Evaluate a single command.
            Reads one line for the command and possible continuation lines.
            \param in     [in] Input
            \param result [out] Command result goes here; pass null.
            \return parser result */
        Result evaluate(afl::io::TextReader& in, std::auto_ptr<afl::data::Value>& result);

        /** Evaluate a string.
            Interprets the string as a sequence of commands.
            \param in     [in] String
            \param result [out] Result of last command */
        void evaluateString(const String_t& str, std::auto_ptr<afl::data::Value>& result);

        /** Evaluate a string, return bool.
            Like evaluateString(), but converts the result to a boolean value.
            \param str [in] String
            \return value */
        bool evaluateStringToBool(const String_t& str);

        /** Access terminal.
            \return terminal as passed to constructor */
        Terminal& terminal();

     private:
        void handleInclude(afl::data::StringList_t& cmd, bool& hadLE);

        Environment& m_environment;
        Terminal& m_terminal;
        afl::io::FileSystem& m_fileSystem;
        CommandHandler& m_commandHandler;
    };

} }

#endif
