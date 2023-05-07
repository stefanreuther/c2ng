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
        virtual String_t getComment(Scope scope, int id) const;
        virtual bool hasTask(Scope scope, int id) const;
        virtual afl::base::Optional<String_t> getHullShortName(int nr) const;
        virtual afl::base::Optional<String_t> getPlayerAdjective(int nr) const;
    };

} }

#endif
