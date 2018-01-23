/**
  *  \file server/console/integercommandhandler.cpp
  *  \brief Class server::console::IntegerCommandHandler
  */

#include <stdexcept>
#include "server/console/integercommandhandler.hpp"
#include "afl/string/parse.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::string::strToInteger;

namespace {
    int32_t toIntegerOrDie(String_t s)
    {
        int32_t n;
        if (!strToInteger(s, n)) {
            throw std::runtime_error("Parameter is not numeric");
        }
        return n;
    }
}


bool
server::console::IntegerCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    if (cmd == "int") {
        /** @q int VALUE (Global Console Command)
            Convert the VALUE into an integer and return it.
            If the value is not convertible to integer, returns nothing.
            @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCount(1);
        int32_t n;
        if (strToInteger(toString(args.getNext()), n)) {
            result.reset(makeIntegerValue(n));
        }
        return true;
    } else if (cmd == "int_not") {
        /* @q int_not VALUE (Global Console Command)
           Convert the VALUE into an integer and returns its logical inverse:
           If the result is nonzero, returns 0; otherwise return 1.
           If the value is not convertible to integer, returns nothing.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        args.checkArgumentCount(1);
        int32_t n;
        if (strToInteger(toString(args.getNext()), n)) {
            result.reset(makeIntegerValue(!n));
        }
        return true;
    } else if (cmd == "int_add") {
        /* @q int_add VALUE:Int (Global Console Command)
           Add all values given as parameter and returns the sum.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        int32_t sum = 0;
        while (args.getNumArgs() > 0) {
            sum += toIntegerOrDie(toString(args.getNext()));
        }
        result.reset(makeIntegerValue(sum));
        return true;
    } else if (cmd == "int_seq") {
        /* @q int_seq MIN:Int MAX:Int (Global Console Command)
           Produces a list of nubmers between MIN and MAX, inclusive.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        args.checkArgumentCount(2);
        int32_t low = toIntegerOrDie(toString(args.getNext()));
        int32_t high = toIntegerOrDie(toString(args.getNext()));

        afl::data::Vector::Ref_t v = afl::data::Vector::create();
        while (low <= high) {
            v->pushBackNew(makeIntegerValue(low));
            ++low;
        }
        result.reset(new afl::data::VectorValue(v));
        return true;
    } else {
        return false;
    }
}
