/**
  *  \file u/t_server_play_racenamepacker.cpp
  *  \brief Test for server::play::RaceNamePacker
  */

#include "server/play/racenamepacker.hpp"

#include "t_server_play.hpp"
#include "afl/base/ref.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestServerPlayRaceNamePacker::testIt()
{
    // Input data. For simplicity, load v3 defaults.
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    setup(*r);

    // Testee
    afl::string::NullTranslator tx;
    server::play::RaceNamePacker testee(*r, 0, tx);
    TS_ASSERT_EQUALS(testee.getName(), "racename");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Expect >12 elements (11 players + dummy)
    TS_ASSERT_LESS_THAN(12U, a.getArraySize());
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[1].isNull());
    TS_ASSERT(!a[11].isNull());

    // Verify all attributes of #1
    TS_ASSERT_EQUALS(a[1]("RACE").toString(), "The Solar Federation");
    TS_ASSERT_EQUALS(a[1]("RACE.ADJ").toString(), "Fed");
    TS_ASSERT_EQUALS(a[1]("RACE.SHORT").toString(), "The Feds");
    TS_ASSERT_EQUALS(a[1]("RACE.ID").toInteger(), 5);
    TS_ASSERT_EQUALS(a[1]("RACE.MISSION").toInteger(), 2);

    // Verify all attributes of #11
    TS_ASSERT_EQUALS(a[11]("RACE").toString(), "The Missing Colonies of Man");
    TS_ASSERT_EQUALS(a[11]("RACE.ADJ").toString(), "Colonial");
    TS_ASSERT_EQUALS(a[11]("RACE.SHORT").toString(), "The Colonies");
    TS_ASSERT_EQUALS(a[11]("RACE.ID").toInteger(), 6);
    TS_ASSERT_EQUALS(a[11]("RACE.MISSION").toInteger(), 11);
}

/** Test offset 1.
    A: create RaceNamePacker with firstSlot=1
    E: no dummy element returned, first element describes Feds */
void
TestServerPlayRaceNamePacker::testOffset1()
{
    // Input data. For simplicity, load v3 defaults.
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    setup(*r);

    // Testee
    afl::string::NullTranslator tx;
    server::play::RaceNamePacker testee(*r, 1, tx);

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    TS_ASSERT_EQUALS(a[0]("RACE").toString(), "The Solar Federation");
    TS_ASSERT_EQUALS(a[10]("RACE").toString(), "The Missing Colonies of Man");
}

