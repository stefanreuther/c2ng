/**
  *  \file test/game/interface/iteratorprovidertest.cpp
  *  \brief Test for game::interface::IteratorProvider
  */

#include "game/interface/iteratorprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.interface.IteratorProvider")
{
    class Tester : public game::interface::IteratorProvider {
     public:
        virtual game::map::ObjectCursor* getCursor()
            { return 0; }
        virtual game::map::ObjectType* getType()
            { return 0; }
        virtual int getCursorNumber()
            { return 0; }
        virtual void store(interpreter::TagNode& /*out*/)
            { }
        virtual game::Session& getSession()
            { throw "boom"; }
        virtual String_t toString()
            { return String_t(); }
    };
    Tester t;
}
