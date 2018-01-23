/**
  *  \file server/console/parser.cpp
  *  \brief Class server::console::Parser
  *
  *  PCC2 Comment:
  *
  *  General syntax is:
  *  - a command is a list of words
  *  - words can be quoted with double or single quotes; much like in a
  *    shell. Within a double-quoted word, backslash quotes. The escape
  *    sequences "\n", "\t", "\r", "\0" are supported, as are "\xZZ" and
  *    "\uZZZZ" to produce a raw byte or an UTF-8 encoded character,
  *    respectively. There must be precisely 2 or 4 hex digits. Within an
  *    unquoted or double-quoted word, '$a' is a variable with a one-
  *    character name '${abc}' is a variable with a multi-character name.
  *  - '{' starts a nestable, multiline quote which collects everything
  *    up to a matching '}' in one word. The content would be syntactically
  *    valid c2console code, but no expansion happens in it yet.
  *  - a word starting with/preceded by a '<' is replaced by the content
  *    of the so-named file, for use in e.g.
  *        file put target/file.dat <host/file.dat
  *  - commands can be separated by pipes. The left-hand side command is
  *    executed, and its result appended to the next command. For example,
  *        redis keys foo* | redis del
  *    to delete all keys returned by 'keys foo*',
  *        redis get foo | setenv foo
  *    to get a value from redis and place it in an environment variable.
  *  - comments start with "#" (use this when input is redirected).
  *
  *  FIXME: The syntax seems to imply that ${${a}} were valid, but it isn't.
  *  It queries a variable '${a' instead and attaches a single '}'.
  *  Brace-quotes in turn consider that matching braces.
  *      c2console-ng> setenv '${a' zz
  *      c2console-ng> setenv a q
  *      c2console-ng> setenv q z
  *      c2console-ng> echo ${${a}}
  *      zz}                            <- should be "z" instead
  *
  *  FIXME: if a command fails, but is followed by a pipe and a braced multiline string:
  *     fail | whatever {
  *  the following lines are interpreted although they should be skipped as part of the
  *  multiline string.
  */

#include <stdexcept>
#include "server/console/parser.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/visitor.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "interpreter/values.hpp"
#include "server/console/commandhandler.hpp"
#include "server/console/terminal.hpp"
#include "server/types.hpp"

using afl::string::Format;

namespace {
    bool handleHexDigit(int32_t& value, char c)
    {
        if (c >= '0' && c <= '9') {
            value = 16*value + c - '0';
            return true;
        } else if (c >= 'A' && c <= 'F') {
            value = 16*value + c - 'A' + 10;
            return true;
        } else if (c >= 'a' && c <= 'f') {
            value = 16*value + c - 'a' + 10;
            return true;
        } else {
            return false;
        }
    }

    void handlePipe(afl::data::StringList_t& thisCommand, afl::data::StringList_t& previousPipeResult)
    {
        if (thisCommand.size() == 0) {
            throw std::runtime_error("No verb in command");
        }
        for (afl::data::StringList_t::iterator i = previousPipeResult.begin(); i != previousPipeResult.end(); ++i) {
            thisCommand.push_back(*i);
        }
        previousPipeResult.clear();
    }

}

// Constructor.
server::console::Parser::Parser(Environment& env, Terminal& term, afl::io::FileSystem& fs, CommandHandler& handler)
    : m_environment(env),
      m_terminal(term),
      m_fileSystem(fs),
      m_commandHandler(handler)
{ }

// Evaluate a single command.
server::console::Parser::Result
server::console::Parser::evaluate(afl::io::TextReader& in, std::auto_ptr<afl::data::Value>& result)
{
    // ex planetscentral/console/console.cc:evalLine

    // Read first line
    String_t line;
    if (!in.readLine(line)) {
        return End;
    }

    // Parse it
    afl::data::StringList_t previousPipeResult;   // FIXME should be segment
    afl::data::StringList_t thisCommand;          // FIXME should be segment
    enum State {
        Blank,                  // in whitespace, next character starts word
        Word,                   // in word
        WordVar,                // '$' in word
        WordVarBrace,           // '${' in word
        DQ,                     // in double-quoted word
        DQQ,                    // in double-quoted word, after backslash
        DQX,                    // in double-quoted word, after '\x'
        DQU,                    // in double-quoted word, after '\u'
        DQVar,                  // '$' in double-quoted
        DQVarBrace,             // '${' in double-quoted
        SQ,                     // in single-quoted word
        LE,                     // after '<'
        Brace,                  // '{' block
        BraceDQ,                // '{"
        BraceDQQ,               // '{"\'
        BraceSQ,                // '{''
        Pipe                    // after pipe (temporary state)
    };
    bool hadLE = false;         // this word had a '<' in front
    bool hadNonblank = false;
    State s = Blank;
    String_t varName;
    int32_t braceLevel = 0;
    int32_t charValue = 0;
    int charDigits = 0;
 again:
    for (String_t::size_type i = 0; i < line.size(); ++i) {
        // Get character
        uint8_t ch = line[i];
        State chClass = (ch == ' ' || ch == '\r' || ch == '\n')
            ? Blank
            : ch == '"'
            ? DQ
            : ch == '\''
            ? SQ
            : ch == '<'
            ? LE
            : ch == '|'
            ? Pipe
            : ch == '$'
            ? WordVar
            : ch == '{'
            ? Brace
            : Word;

        // Basic syntax checks
        if (ch == '#' && (s == Blank || s == Word || s == LE)) {
            break;
        }

        if (chClass != Blank) {
            hadNonblank = true;
        }

        // Process it
        switch (s) {
         case Blank:
            if (chClass == Pipe || chClass == LE || chClass == Blank) {
                // State change without action
            } else if (chClass == Word) {
                // Normal word
                thisCommand.push_back(String_t(1, char(ch)));
            } else {
                // Quoted word or variable
                thisCommand.push_back(String_t());
            }
            s = chClass;
            break;

         case Word:
            if (chClass == Pipe || chClass == LE || chClass == Blank) {
                // End word
                handleInclude(thisCommand, hadLE);
                s = chClass;
            } else if (chClass == Word) {
                // Word char
                thisCommand.back() += char(ch);
            } else {
                // Quote or variable
            }
            s = chClass;
            break;

         case DQ:
            if (ch == '"') {
                s = Word;
            } else if (ch == '\\') {
                s = DQQ;
            } else if (ch == '$') {
                s = DQVar;
            } else {
                thisCommand.back() += char(ch);
            }
            break;

         case DQQ:
            if (ch == 'x') {
                charValue = 0;
                charDigits = 0;
                s = DQX;
            } else if (ch == 'u') {
                charValue = 0;
                charDigits = 0;
                s = DQU;
            } else {
                if (ch == '0') {
                    thisCommand.back() += '\0';
                } else if (ch == 'n') {
                    thisCommand.back() += '\n';
                } else if (ch == 't') {
                    thisCommand.back() += '\t';
                } else if (ch == 'r') {
                    thisCommand.back() += '\r';
                } else {
                    thisCommand.back() += char(ch);
                }
                s = DQ;
            }
            break;

         case DQX:
            if (handleHexDigit(charValue, ch)) {
                ++charDigits;
                if (charDigits >= 2) {
                    thisCommand.back() += char(charValue);
                    s = DQ;
                }
            } else {
                throw std::runtime_error("Expecting hex digit after '\\x'");
            }
            break;

         case DQU:
            if (handleHexDigit(charValue, ch)) {
                ++charDigits;
                if (charDigits >= 4) {
                    afl::charset::Utf8().append(thisCommand.back(), charValue);
                    s = DQ;
                }
            } else {
                throw std::runtime_error("Expecting hex digit after '\\u'");
            }
            break;

         case SQ:
            if (ch == '\'') {
                s = Word;
            } else {
                thisCommand.back() += char(ch);
            }
            break;

         case LE:
            if (chClass == Blank) {
                // ok
            } else if (chClass == Pipe || chClass == LE) {
                throw std::runtime_error("Expecting name after '<'");
            } else {
                if (chClass == Word) {
                    thisCommand.push_back(String_t(1, ch));
                } else {
                    thisCommand.push_back(String_t());
                }
                hadLE = true;
                s = chClass;
            }
            break;

         case DQVar:
         case WordVar:
            if (ch == '{') {
                varName.clear();
                s = (s == DQVar ? DQVarBrace : WordVarBrace);
            } else if (chClass == Word) {
                afl::data::Value* value(m_environment.get(String_t(1, ch)));
                // FIXME: append real value, not string
                thisCommand.back().append(interpreter::toString(value, false));
                s = (s == DQVar ? DQ : Word);
            } else {
                throw std::runtime_error("Invalid variable reference");
            }
            break;

         case DQVarBrace:
         case WordVarBrace:
            if (ch == '}') {
                afl::data::Value* value(m_environment.get(varName));
                // FIXME: real value, not string
                thisCommand.back().append(interpreter::toString(value, false));
                s = (s == DQVarBrace ? DQ : Word);
            } else {
                varName += char(ch);
            }
            break;

         case Brace:
            if (ch == '}' && braceLevel == 0) {
                s = Word;
            } else {
                if (ch == '"') {
                    s = BraceDQ;
                } else if (ch == '\'') {
                    s = BraceSQ;
                } else if (ch == '}') {
                    --braceLevel;
                } else if (ch == '{') {
                    ++braceLevel;
                }
                thisCommand.back() += char(ch);
            }
            break;

         case BraceDQ:
            thisCommand.back() += char(ch);
            if (ch == '\\') {
                s = BraceDQQ;
            } else if (ch == '"') {
                s = Brace;
            }
            break;

         case BraceDQQ:
            thisCommand.back() += char(ch);
            s = BraceDQ;
            break;

         case BraceSQ:
            thisCommand.back() += char(ch);
            if (ch == '\'') {
                s = Brace;
            }
            break;

         case Pipe:
            break;
        }

        if (s == Pipe) {
            // Prepare
            handlePipe(thisCommand, previousPipeResult);

            // Execute
            afl::data::Segment seg;
            seg.pushBackElements(thisCommand);
            interpreter::Arguments args(seg, 1, seg.size()-1);
            String_t cmd = thisCommand[0];
            std::auto_ptr<afl::data::Value> val;
            if (!m_commandHandler.call(cmd, args, *this, val)) {
                throw std::runtime_error(Format("Unknown command: %s", cmd));
            }
            thisCommand.clear();

            // Postprocess and remember result for piping.
            // This implements roughly the same repertoire as consoleapplication.cpp:showValue.
            // In particular, an empty vector must produce an empty previousPipeResult.
            class Flattener : public afl::data::Visitor {
             public:
                Flattener(afl::data::StringList_t& out)
                    : m_out(out)
                    { }
                virtual void visitString(const String_t& str)
                    { m_out.push_back(str); }
                virtual void visitInteger(int32_t iv)
                    { m_out.push_back(Format("%d", iv)); }
                virtual void visitFloat(double fv)
                    { m_out.push_back(Format("%.25g", fv)); }
                virtual void visitBoolean(bool bv)
                    { m_out.push_back(bv ? "true" : "false"); }
                virtual void visitHash(const afl::data::Hash& /*hv*/)
                    {
                        // We do not support hashes.
                    }
                virtual void visitVector(const afl::data::Vector& vv)
                    {
                        for (size_t i = 0; i < vv.size(); ++i) {
                            m_out.push_back(server::toString(vv[i]));
                        }
                    }
                virtual void visitOther(const afl::data::Value& /*other*/)
                    { m_out.push_back("#<other>"); }
                virtual void visitNull()
                    { m_out.push_back(""); }
                virtual void visitError(const String_t& /*source*/, const String_t& str)
                    { m_out.push_back(Format("#<error:%s>", str)); }
             private:
                afl::data::StringList_t& m_out;
            };
            Flattener(previousPipeResult).visit(val.get());
            s = Blank;
        }
    }

    // End of line
    switch (s) {
     case Blank:
        break;

     case Word:
        handleInclude(thisCommand, hadLE);
        break;

     case DQ:
     case DQQ:
     case DQX:
     case DQU:
     case SQ:
     case BraceDQ:
     case BraceDQQ:
     case BraceSQ:
        throw std::runtime_error("Quote not closed at end of line");

     case WordVar:
     case WordVarBrace:
     case DQVar:
     case DQVarBrace:
        throw std::runtime_error("Variable reference not finished at end of line");

     case LE:
        throw std::runtime_error("Expecting name after '<'");

     case Pipe:
        throw std::runtime_error("Pipe not allowed at end of line");

     case Brace:
        m_terminal.printSecondaryPrompt();
        if (!in.readLine(line)) {
            throw std::runtime_error("Unexpected end of file");
        }
        if (!thisCommand.back().empty()) {
            thisCommand.back() += "\n";
        }
        goto again;
    }

    // Quick exit
    if (!hadNonblank) {
        return BlankLine;
    }

    // Prepare
    handlePipe(thisCommand, previousPipeResult);

    // Execute
    afl::data::Segment seg;
    seg.pushBackElements(thisCommand);
    interpreter::Arguments args(seg, 1, seg.size()-1);
    String_t cmd = thisCommand[0];
    result.reset();
    if (!m_commandHandler.call(cmd, args, *this, result)) {
        throw std::runtime_error(Format("Unknown command: %s", cmd));
    }
    return Command;
}

// Evaluate a string.
void
server::console::Parser::evaluateString(const String_t& str, std::auto_ptr<afl::data::Value>& result)
{
    // ex planetscentral/console/console.cc:evalSequence

    // Prepare a stream
    afl::io::InternalStream ss;
    ss.write(afl::string::toBytes(str));
    ss.setPos(0);
    ss.setWritePermission(false);
    afl::io::TextFile tf(ss);

    // Execute
    while (1) {
        try {
            if (evaluate(tf, result) == End) {
                break;
            }
        }
        catch (...) {
            // FIXME: std::cout << "ERROR: in nested command '" << line << "':\n";
            throw;
        }
    }
}

// Evaluate a string, return bool.
bool
server::console::Parser::evaluateStringToBool(const String_t& str)
{
    // ex planetscentral/console/console.cc:evalSequenceToBool
    std::auto_ptr<afl::data::Value> result;
    evaluateString(str, result);
    return interpreter::getBooleanValue(result.get()) > 0;
}

// Access terminal.
server::console::Terminal&
server::console::Parser::terminal()
{
    return m_terminal;
}

void
server::console::Parser::handleInclude(afl::data::StringList_t& cmd, bool& hadLE)
{
    // ex planetscentral/console/console.cc:handleLE
    if (hadLE && cmd.size()) {
        try {
            afl::base::Ref<afl::io::Stream> s = m_fileSystem.openFile(cmd.back(), afl::io::FileSystem::OpenRead);
            String_t result;
            while (1) {
                uint8_t tmp[1024];
                afl::base::Bytes_t m(tmp);
                m.trim(s->read(m));
                if (m.empty()) {
                    break;
                }
                result.append(afl::string::fromBytes(m));
            }
            m_terminal.printMessage(Format("(loaded %s, %d bytes)", cmd.back(), result.size()));
            cmd.back() = result;
        }
        catch (std::exception& e) {
            if (cmd[0] == "fatal") {
                // FIXME: std::cout << "ERROR: " << e.what() << "\n";
                // FIXME: std::exit(1);
            }
            throw;
        }
    }
    hadLE = false;
}
