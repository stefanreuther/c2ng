/**
  *  \file server/console/fundamentalcommandhandler.cpp
  *  \brief Class server::console::FundamentalCommandHandler
  */

#include "server/console/fundamentalcommandhandler.hpp"
#include "server/types.hpp"
#include "server/console/environment.hpp"
#include "server/console/parser.hpp"
#include "afl/base/optional.hpp"
#include "server/console/terminal.hpp"
#include "afl/string/format.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

server::console::FundamentalCommandHandler::FundamentalCommandHandler(Environment& env)
    : CommandHandler(),
      m_environment(env)
{ }

server::console::FundamentalCommandHandler::~FundamentalCommandHandler()
{ }

bool
server::console::FundamentalCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
{
    if (cmd == "foreach") {
        /* @q foreach VAR:Env BODY:Code ITEMS... (Global Console Command)
           Iterate through ITEMS, and execute some CODE with VAR set to the respective value.
           For example,
           <pre>foreach i {echo $i} 1 2 3</pre>
           will print three lines: 1, 2, 3.

           The unconventional syntax of listing the items to iterate over last allows for the form
           <pre>command | foreach i {code...}</pre>
           where %command is a command producing a list of output (e.g. "redis smembers SET", "host gamelist id").

           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCountAtLeast(2);
        String_t argName = toString(args.getNext());
        String_t body    = toString(args.getNext());
        while (args.getNumArgs() > 0) {
            Environment::ValuePtr_t oldValue(m_environment.pushNew(argName, Environment::ValuePtr_t(afl::data::Value::cloneOf(args.getNext()))));
            try {
                std::auto_ptr<afl::data::Value> tmp;
                parser.evaluateString(body, tmp);
                m_environment.popNew(argName, oldValue);
            }
            catch (...) {
                m_environment.popNew(argName, oldValue);
                throw;
            }
        }
        return true;
    } else if (cmd == "if") {
        /* @q if COND:Code THEN:Code [elsif COND:Code ELSIF:Code] [else ELSE:Code] (Global Console Command)
           Condition.
           Executes the condition COND.
           If that returns nonzero, executes the THEN code.
           Otherwise, looks for the first elsif COND that returns true, and executes that code.
           If none matches, executes the else's code.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        afl::base::Optional<String_t> thenPart;

        // then part
        args.checkArgumentCountAtLeast(2);
        if (parser.evaluateStringToBool(toString(args.getNext()))) {
            thenPart = toString(args.getNext());
        } else {
            args.getNext();
        }

        // elsif/else parts
        while (args.getNumArgs() > 0) {
            String_t keyword = toString(args.getNext());
            if (keyword == "elsif") {
                // elsif COND CODE [...]
                args.checkArgumentCountAtLeast(2);
                String_t cond = toString(args.getNext());
                String_t code = toString(args.getNext());
                if (!thenPart.isValid()) {
                    if (parser.evaluateStringToBool(cond)) {
                        thenPart = code;
                    }
                }
            } else if (keyword == "else") {
                // else CODE
                args.checkArgumentCount(1);
                if (!thenPart.isValid()) {
                    thenPart = toString(args.getNext());
                } else {
                    args.getNext();
                }
            } else {
                throw std::runtime_error("Invalid keyword in \"if\"");
            }
        }

        // Do it
        if (const String_t* p = thenPart.get()) {
            std::auto_ptr<afl::data::Value> tmp;
            parser.evaluateString(*p, tmp);
        }
        return true;
    } else if (cmd == "echo") {
        /* @q echo TEXT:Any.... (Global Console Command)
           Print all arguments to the console.
           Produces no return value.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        String_t acc;
        while (args.getNumArgs() > 0) {
            acc += toString(args.getNext());
            if (args.getNumArgs() > 0) {
                acc += " ";
            }
        }
        parser.terminal().printMessage(acc);
        return true;
    } else if (cmd == "setenv") {
        /* @q setenv VAR:Env VALUE:Any (Global Console Command)
           Set a local environment variable.
           Note that this command is called "setenv", not "set", to avoid a clash with the redis "set" command.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCount(2);
        String_t name = toString(args.getNext());
        m_environment.setNew(name, Environment::ValuePtr_t(afl::data::Value::cloneOf(args.getNext())));
        return true;
    } else if (cmd == "env") {
        /* @q env (Global Console Command)
           Returns the current environment as a list of names and values.
           @change PCC2 and PCC2ng produce the result in a different order.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCount(0);         // this check not in -classic
        afl::data::Vector::Ref_t p = afl::data::Vector::create();
        m_environment.listContent(*p);
        result.reset(new afl::data::VectorValue(p));
        return true;
    } else {
        return false;
    }
}
