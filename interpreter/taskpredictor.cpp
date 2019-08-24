/**
  *  \file interpreter/taskpredictor.cpp
  *  \brief Class interpreter::TaskPredictor
  */

#include "interpreter/taskpredictor.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"

namespace {
    /** Read a single argument. Returns true on success, returns false or
        throws IntError on error.

        argument ::= string
                   | bool
                   | ["+"|"-"] number

        \param tok [in] Token stream
        \param args [in/out] Arguments collected here  */
    bool readOneArg(interpreter::Tokenizer& tok, afl::data::Segment& args)
    {
        if (tok.getCurrentToken() == tok.tString) {
            // String
            args.pushBackString(tok.getCurrentString());
            tok.readNextToken();
            return true;
        } else if (tok.getCurrentToken() == tok.tBoolean) {
            // Boolean
            args.pushBackNew(interpreter::makeBooleanValue(tok.getCurrentInteger()));
            tok.readNextToken();
            return true;
        } else {
            // Must be number. Let's accept signed numbers, just in case.
            int sign = +1;
            if (tok.checkAdvance(tok.tMinus)) {
                sign = -1;
            } else if (tok.checkAdvance(tok.tPlus)) {
                // nix
            } else {
                // auch nix
            }

            if (tok.getCurrentToken() == tok.tInteger) {
                // integer
                args.pushBackInteger(sign * tok.getCurrentInteger());
                tok.readNextToken();
                return true;
            } else if (tok.getCurrentToken() == tok.tFloat) {
                // float
                args.pushBackNew(interpreter::makeFloatValue(sign * tok.getCurrentFloat()));
                tok.readNextToken();
                return true;
            } else {
                return false;
            }
        }
    }

    /** Read list of arguments. Returns true on success, returns false or
        throws interpreter::Error on error.
        \param tok [in] Token stream
        \param args [out] Arguments */
    bool readArgs(interpreter::Tokenizer& tok, afl::data::Segment& args)
    {
        if (tok.getCurrentToken() == tok.tEnd) {
            // No args: valid
            return true;
        }

        while (1) {
            // Read one
            if (!readOneArg(tok, args)) {
                return false;
            }

            // Check delimiter
            if (tok.checkAdvance(tok.tComma)) {
                // ok
            } else if (tok.getCurrentToken() == tok.tEnd) {
                // ok
                return true;
            } else {
                return false;
            }
        }
    }
}

void
interpreter::TaskPredictor::predictTask(const TaskEditor& editor, size_t endPC)
{
    // IntAutoTaskPredictor::predictTask
    size_t startPC = editor.getPC();
    size_t pc = startPC;
    bool looped = false;

    /* Loop until we reach the configured end, or, when we have looped, the starting position */
    while (pc < endPC && pc < editor.getNumInstructions() && (!looped || pc < startPC)) {
        try {
            // Parse one line
            Tokenizer tok(editor[pc]);
            ++pc;

            // Extract command
            if (tok.getCurrentToken() == tok.tEnd) {
                continue;
            }
            if (tok.getCurrentToken() != tok.tIdentifier) {
                break;
            }
            String_t command = tok.getCurrentString();
            tok.readNextToken();

            // Build arguments
            afl::data::Segment args;
            if (!readArgs(tok, args)) {
                break;
            }

            // Call predictor
            if (command == "RESTART") {
                looped = true;
                pc = 0;
            } else {
                Arguments block(args, 0, args.size());
                if (!predictInstruction(command, block)) {
                    break;
                }
            }
        }
        catch (Error&) {
            break;
        }
    }
}

void
interpreter::TaskPredictor::predictStatement(const TaskEditor& editor, size_t pc)
{
    // ex IntAutoTaskPredictor::predictStatement
    if (pc < editor.getNumInstructions()) {
        try {
            // Parse one line
            Tokenizer tok(editor[pc]);

            if (tok.getCurrentToken() != tok.tIdentifier) {
                return;
            }
            String_t command = tok.getCurrentString();
            tok.readNextToken();

            // Build arguments and call predictor
            afl::data::Segment args;
            if (readArgs(tok, args) && command != "RESTART") {
                Arguments block(args, 0, args.size());
                predictInstruction(command, block);
            }
        }
        catch (Error&) { }
    }
}
