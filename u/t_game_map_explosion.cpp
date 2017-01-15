/**
  *  \file u/t_game_map_explosion.cpp
  *  \brief Test for game::map::Explosion
  */

#include "game/map/explosion.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameMapExplosion::testName()
{
    // Infrastructure
    class NullInterpreterInterface : public game::InterpreterInterface {
     public:
        virtual afl::data::Value* evaluate(Scope /*scope*/, int /*id*/, String_t /*expr*/)
            { return 0; }
        virtual String_t getComment(Scope /*scope*/, int /*id*/)
            { return String_t(); }
        virtual bool hasTask(Scope /*scope*/, int /*id*/)
            { return false; }
        virtual bool getHullShortName(int /*nr*/, String_t& /*out*/)
            { return false; }
        virtual bool getPlayerAdjective(int /*nr*/, String_t& /*out*/)
            { return false; }
    };
    NullInterpreterInterface iface;
    afl::string::NullTranslator tx;

    // Default
    {
        game::map::Explosion t(0, game::map::Point(1,1));
        TS_ASSERT_EQUALS(t.getName(t.PlainName, tx, iface), "Explosion");
    }

    // Just a ship name
    {
        game::map::Explosion t(0, game::map::Point(1,1));
        t.setShipName("USS Dull");
        TS_ASSERT_EQUALS(t.getName(t.PlainName, tx, iface), "Explosion of USS Dull");
    }

    // Just a ship Id
    {
        game::map::Explosion t(0, game::map::Point(1,1));
        t.setShipId(42);
        TS_ASSERT_EQUALS(t.getName(t.PlainName, tx, iface), "Explosion of ship #42");
    }

    // Name and Id
    {
        game::map::Explosion t(0, game::map::Point(1,1));
        t.setShipName("USS Dull");
        t.setShipId(42);
        TS_ASSERT_EQUALS(t.getName(t.PlainName, tx, iface), "Explosion of USS Dull (#42)");
    }
}
