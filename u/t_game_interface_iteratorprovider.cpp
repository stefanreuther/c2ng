/**
  *  \file u/t_game_interface_iteratorprovider.cpp
  *  \brief Test for game::interface::IteratorProvider
  */

#include "game/interface/iteratorprovider.hpp"

#include "t_game_interface.hpp"

/** Interface test. */
void
TestGameInterfaceIteratorProvider::testIt()
{
    class Tester : public game::interface::IteratorProvider {
     public:
        virtual game::map::ObjectCursor* getCursor()
            { return 0; }
        virtual game::map::ObjectType* getType()
            { return 0; }
        virtual void store(interpreter::TagNode& /*out*/)
            { }
        virtual String_t toString()
            { return String_t(); }
    };
    Tester t;
}

