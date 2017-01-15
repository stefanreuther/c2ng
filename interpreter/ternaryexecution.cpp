/**
  *  \file interpreter/ternaryexecution.cpp
  */

#include "interpreter/ternaryexecution.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/error.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "interpreter/world.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/arguments.hpp"

using interpreter::Error;

namespace {
    afl::data::Value* FKeyAdd(interpreter::World& world, afl::data::Value* a, afl::data::Value* b, afl::data::Value* c)
    {
        /* Check null operation */
        if (a == 0 || b == 0 || c == 0)
            return 0;

        /* a must be a keymap */
        interpreter::KeymapValue* keymap = dynamic_cast<interpreter::KeymapValue*>(a);
        if (!keymap)
            throw Error::typeError(Error::ExpectKeymap);

        /* b must be a string (the key) */
        afl::data::StringValue* keysym = dynamic_cast<afl::data::StringValue*>(b);
        if (!keysym)
            throw Error::typeError(Error::ExpectString);

        util::Key_t keyval = 0;
        if (!util::parseKey(keysym->getValue(), keyval)) {
            throw Error("Invalid key name");
        }

        /* c must be a string or integer (the command) */
        util::Atom_t command;
        if (!interpreter::checkCommandAtomArg(command, c, world.atomTable())) {
            // cannot happen (already handled above)
            return 0;
        }

        /* Do it */
        keymap->getKeymap()->addKey(keyval, command, 0);

        return keymap->clone();
    }

    afl::data::Value* (*const ternary_ops[])(interpreter::World&,afl::data::Value*,afl::data::Value*,afl::data::Value*) = {
        FKeyAdd,
    };

}

/** Execute ternary operation.
    \param op Operation, IntTernaryOperation
    \param a,b,c User-supplied arguments taken from value stack
    \return New value to push on value stack */
afl::data::Value*
interpreter::executeTernaryOperation(interpreter::World& world, uint8_t op, afl::data::Value* a, afl::data::Value* b, afl::data::Value* c)
{
    if (op < countof(ternary_ops))
        return ternary_ops[op](world, a, b, c);
    else
        throw Error::internalError("invalid ternary operation");
}
