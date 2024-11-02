/**
  *  \file test/game/nu/registrationkeytest.cpp
  *  \brief Test for game::nu::RegistrationKey
  */

#include "game/nu/registrationkey.hpp"

#include "afl/test/testrunner.hpp"
#include "util/io.hpp"
#include "afl/data/access.hpp"

AFL_TEST("game.nu.RegistrationKey", a)
{
    // An "/account/load" response, heavily redacted/trimmed
    const char*const text =
        "{"
        "  \"account\": {"
        "    \"apikey\": \"...\","
        "    \"description\": \"\","
        "    \"displayname\": \"streu\","
	"    \"email\": \"streu@gmx.de\","
        "    \"username\": \"streu\","
        "    \"_officers\": [],"
        "    \"_title\": \"Midshipman\","
        "    \"_completedlevels\": 0,"
        "    \"path\": \"streu\","
        "    \"hubmail\": \"streu@hub.planets.nu\","
        "    \"isnew\": false,"
        "    \"id\": 860"
        "  },"
        "  \"isregistered\": true,"
        "  \"settings\": {"
        "    \"id\": 0"
        "  },"
        "  \"playergroups\": [],"
        "  \"success\": true"
        "}";
    std::auto_ptr<afl::data::Value> p(util::parseJSON(afl::string::toBytes(text)));

    // Object under test
    game::nu::RegistrationKey testee((afl::data::Access(p)));

    // Text
    a.checkEqual("01. Line1", testee.getLine(game::RegistrationKey::Line1), "streu, streu@gmx.de");
    a.checkEqual("02. Line2", testee.getLine(game::RegistrationKey::Line2), "Account #860");
    a.checkEqual("03. Line3", testee.getLine(game::RegistrationKey::Line3), "");
    a.checkEqual("04. Line4", testee.getLine(game::RegistrationKey::Line4), "");

    // Status
    a.checkEqual("11. status", testee.getStatus(), game::RegistrationKey::Registered);
    a.checkEqual("12. tech", testee.getMaxTechLevel(game::EngineTech), 10);

    // Modification
    a.checkEqual("21. set", testee.setLine(game::RegistrationKey::Line4, "x"), false);
}
