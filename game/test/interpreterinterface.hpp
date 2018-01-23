/**
  *  \file game/test/interpreterinterface.hpp
  *  \brief Class game::test::InterpreterInterface
  */
#ifndef C2NG_GAME_TEST_INTERPRETERINTERFACE_HPP
#define C2NG_GAME_TEST_INTERPRETERINTERFACE_HPP

#include "game/interpreterinterface.hpp"

namespace game { namespace test {

    /** Test support: interpreter interface.
        Reports failure/null results on all questions. */
    class InterpreterInterface : public game::InterpreterInterface {
     public:
        virtual afl::data::Value* evaluate(Scope scope, int id, String_t expr);
        virtual String_t getComment(Scope scope, int id);
        virtual bool hasTask(Scope scope, int id);
        virtual bool getHullShortName(int nr, String_t& out);
        virtual bool getPlayerAdjective(int nr, String_t& out);
    };

} }

#endif
