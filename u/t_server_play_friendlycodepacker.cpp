/**
  *  \file u/t_server_play_friendlycodepacker.cpp
  *  \brief Test for server::play::FriendlyCodePacker
  */

#include "server/play/friendlycodepacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/playerlist.hpp"
#include "game/player.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/constmemorystream.hpp"

void
TestServerPlayFriendlyCodePacker::testIt()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());

    // Player list
    game::PlayerList& pl = session.getRoot()->playerList();
    game::Player* p3 = pl.create(3);
    TS_ASSERT(p3);
    p3->setName(game::Player::ShortName, "Threes");
    p3->setName(game::Player::AdjectiveName, "threeish");

    // Friendly code list
    game::spec::FriendlyCodeList& fcList = session.getShipList()->friendlyCodes();
    fcList.addCode(game::spec::FriendlyCode("pfc", "p,whatever", tx));
    fcList.addCode(game::spec::FriendlyCode("gs3", "s-3,give to %3", tx));
    fcList.addCode(game::spec::FriendlyCode("gs4", "s,give to %4", tx));
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab"));
    fcList.loadExtraCodes(ms, tx);

    // Testee
    server::play::FriendlyCodePacker testee(session);
    TS_ASSERT_EQUALS(testee.getName(), "fcode");

    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access a(result.get());

    TS_ASSERT_EQUALS(a.getArraySize(), 4U);
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "pfc");
    TS_ASSERT_EQUALS(a[0]("FLAGS").toString(), "p");
    TS_ASSERT_EQUALS(a[0]("DESCRIPTION").toString(), "whatever");
    TS_ASSERT_EQUALS(a[0]("RACES").toInteger(), -1);

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "gs3");
    TS_ASSERT_EQUALS(a[1]("FLAGS").toString(), "s");
    TS_ASSERT_EQUALS(a[1]("DESCRIPTION").toString(), "give to Threes");
    TS_ASSERT_EQUALS(a[1]("RACES").toInteger(), ~(1<<3));

    TS_ASSERT_EQUALS(a[2]("NAME").toString(), "gs4");
    TS_ASSERT_EQUALS(a[2]("FLAGS").toString(), "s");
    TS_ASSERT_EQUALS(a[2]("DESCRIPTION").toString(), "give to 4");
    TS_ASSERT_EQUALS(a[2]("RACES").toInteger(), -1);

    TS_ASSERT_EQUALS(a[3]("NAME").toString(), "ab");
    TS_ASSERT_EQUALS(a[3]("FLAGS").toString(), "x");
    TS_ASSERT_EQUALS(a[3]("DESCRIPTION").toString(), "");
    TS_ASSERT_EQUALS(a[3]("RACES").toInteger(), -1);
}

