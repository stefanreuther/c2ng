/**
  *  \file game/interface/consolecommands.cpp
  *  \brief Console Commands
  */

#include "game/interface/consolecommands.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/data/stringvalue.hpp"
#include "game/extra.hpp"
#include "game/root.hpp"
#include "game/stringverifier.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using afl::charset::Unichar_t;
using afl::charset::Utf8;
using afl::charset::Utf8Reader;
using afl::io::TextReader;
using afl::io::TextWriter;

namespace {
    /*
     *  Console Handling
     *
     *  We stash away the console handles in a session extra.
     *  This means we can access it from a regular SimpleProcedure without having to make a closure object.
     */

    class ConsoleExtra : public game::Extra {
     public:
        ConsoleExtra(Ref<TextReader> in, Ref<TextWriter> out)
            : m_in(in),
              m_out(out)
            { }

        TextReader& input()
            { return *m_in; }

        TextWriter& output()
            { return *m_out; }

     private:
        Ref<TextReader> m_in;
        Ref<TextWriter> m_out;
    };

    const game::ExtraIdentifier<game::Session, ConsoleExtra> ID = {{}};

    ConsoleExtra* getConsole(game::Session& session)
    {
        return session.extra().get(ID);
    }


    /*
     *  Utilities
     */

    /* Flags for UTF-8 handling */
    const uint32_t UTF_FLAGS = 0;

    /* Check whether an Unicode character should be accepted. */
    bool acceptUnicode(game::Session& session, Unichar_t uni, int32_t flags)
    {
        if (uni < 32 || uni == 127) {
            // Refuse controls
            return false;
        }
        if ((flags & 1) != 0 && (uni < '0' || uni > '9')) {
            // Refuse nondigits if requested
            return false;
        }
        if ((flags & 2) != 0 && (uni >= 128)) {
            // Refuse non-ASCII if requested
            return false;
        }
        if ((flags & 16) != 0) {
            // Refuse non-game characters if requested
            game::Root* pRoot = session.getRoot().get();
            if (pRoot == 0 || !pRoot->stringVerifier().isValidCharacter(game::StringVerifier::Unknown, uni)) {
                return false;
            }
        }
        return true;
    }
}

/* Global command "UI.Input", console version. */
void
game::interface::IFUIInput(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex int/if/conif.cc:IFUIInput
    // UI.Input <prompt>[, <title>, <maxChars>, <flags>, <default>]
    args.checkArgumentCount(1, 5);

    String_t prompt;
    String_t title;
    String_t defaultText;
    int32_t  maxChars = 255;
    int32_t  flags = 0;
    int32_t  width = 0;

    // Mandatory argument
    if (!interpreter::checkStringArg(prompt, args.getNext())) {
        return;
    }

    // Optional arguments
    title = prompt;
    interpreter::checkStringArg(title, args.getNext());
    interpreter::checkIntegerArg(maxChars, args.getNext(), 0, 32000);
    interpreter::checkFlagArg(flags, &width, args.getNext(), "NHPFGM");
    interpreter::checkStringArg(defaultText, args.getNext());

    // Check status
    ConsoleExtra* cx = getConsole(session);
    if (!cx) {
        throw interpreter::Error("No console");
    }

    // Convert flags
    //   N = 1 = numeric
    //   H = 2 = no high ASCII
    //   P = 4 = password masking [ignored]
    //   F = 8 = frame [ignored]
    //   G = 16 = game charset
    //   M = 32 = width is in ems [ignored]
    while (1) {
        // Show prompt and get initial input
        String_t line;
        cx->output().writeText(prompt + "> ");
        cx->output().flush();
        if (!cx->input().readLine(line)) {
            // EOF
            proc.setVariable("UI.RESULT", 0);
            return;
        }

        // Check whether input is valid, by decoding and sanitizing the UTF-8.
        String_t result;
        bool hadInvalidChars = false;
        Utf8Reader rdr(afl::string::toBytes(line), UTF_FLAGS);
        for (int32_t i = 0; rdr.hasMore() && i < maxChars; ++i) {
            Unichar_t ch = rdr.eat();
            if (acceptUnicode(session, ch, flags)) {
                Utf8(UTF_FLAGS).append(result, ch);
            } else {
                hadInvalidChars = true;
                Utf8(UTF_FLAGS).append(result, (flags & 1) ? '0' : '?');
            }
        }
        if (!hadInvalidChars) {
            // Accepted input
            afl::data::StringValue sv(result);
            proc.setVariable("UI.RESULT", &sv);
            return;
        }

        // Not accepted, continue
        cx->output().writeLine(session.translator()("Your input contains characters that are not permitted at this place.\nPlease try again.\n"));
    }
}

void
game::interface::registerConsoleCommands(Session& session, afl::base::Ref<afl::io::TextReader> in, afl::base::Ref<afl::io::TextWriter> out)
{
    // ex initInterpreterConsoleInterface
    // Create the console
    session.extra().setNew(ID, new ConsoleExtra(in, out));

    // Command
    session.world().setNewGlobalValue("UI.INPUT", new interpreter::SimpleProcedure<Session&>(session, IFUIInput));
    session.world().setNewGlobalValue("SYSTEM.GUI", interpreter::makeBooleanValue(false));
}
