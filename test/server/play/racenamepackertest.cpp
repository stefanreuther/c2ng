/**
  *  \file test/server/play/racenamepackertest.cpp
  *  \brief Test for server::play::RaceNamePacker
  */

#include "server/play/racenamepacker.hpp"

#include "afl/base/ref.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/v3/utils.hpp"

namespace {
    void setup(game::Root& r)
    {
        // Default race names
        afl::charset::Utf8Charset cs;
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
        dir->addStream("race.nm", *new afl::io::ConstMemoryStream(game::test::getDefaultRaceNames()));
        game::v3::loadRaceNames(r.playerList(), *dir, cs);

        // Configuration
        r.hostConfiguration()[game::config::HostConfiguration::PlayerRace].set("5,4,3,2,1,11,10,9,8,7,6");
        r.hostConfiguration()[game::config::HostConfiguration::PlayerSpecialMission].set("2,4,6,8,10,1,3,5,7,9,11");
    }
}

/** Simple functionality test.
    A: create root; create RaceNamePacker
    E: correct values for all properties */
AFL_TEST("server.play.RaceNamePacker:basics", a)
{
    // Input data. For simplicity, load v3 defaults.
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    setup(*r);

    // Testee
    afl::string::NullTranslator tx;
    server::play::RaceNamePacker testee(*r, 0, tx);
    a.checkEqual("01. getName", testee.getName(), "racename");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Expect >12 elements (11 players + dummy)
    a.checkLessThan("11", 12U, ap.getArraySize());
    a.check("12", ap[0].isNull());
    a.check("13", !ap[1].isNull());
    a.check("14", !ap[11].isNull());

    // Verify all attributes of #1
    a.checkEqual("21", ap[1]("RACE").toString(), "The Solar Federation");
    a.checkEqual("22", ap[1]("RACE.ADJ").toString(), "Fed");
    a.checkEqual("23", ap[1]("RACE.SHORT").toString(), "The Feds");
    a.checkEqual("24", ap[1]("RACE.ID").toInteger(), 5);
    a.checkEqual("25", ap[1]("RACE.MISSION").toInteger(), 2);

    // Verify all attributes of #11
    a.checkEqual("31", ap[11]("RACE").toString(), "The Missing Colonies of Man");
    a.checkEqual("32", ap[11]("RACE.ADJ").toString(), "Colonial");
    a.checkEqual("33", ap[11]("RACE.SHORT").toString(), "The Colonies");
    a.checkEqual("34", ap[11]("RACE.ID").toInteger(), 6);
    a.checkEqual("35", ap[11]("RACE.MISSION").toInteger(), 11);
}

/** Test offset 1.
    A: create RaceNamePacker with firstSlot=1
    E: no dummy element returned, first element describes Feds */
AFL_TEST("server.play.RaceNamePacker:offset1", a)
{
    // Input data. For simplicity, load v3 defaults.
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    setup(*r);

    // Testee
    afl::string::NullTranslator tx;
    server::play::RaceNamePacker testee(*r, 1, tx);

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("01", ap[0]("RACE").toString(), "The Solar Federation");
    a.checkEqual("02", ap[10]("RACE").toString(), "The Missing Colonies of Man");
}
