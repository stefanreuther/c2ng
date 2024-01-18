/**
  *  \file test/server/play/friendlycodepackertest.cpp
  *  \brief Test for server::play::FriendlyCodePacker
  */

#include "server/play/friendlycodepacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"

AFL_TEST("server.play.FriendlyCodePacker", a)
{
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Player list
    game::PlayerList& pl = root->playerList();
    game::Player* p3 = pl.create(3);
    a.check("01. create player", p3);
    p3->setName(game::Player::ShortName, "Threes");
    p3->setName(game::Player::AdjectiveName, "threeish");

    // Friendly code list
    game::spec::FriendlyCodeList& fcList = shipList->friendlyCodes();
    fcList.addCode(game::spec::FriendlyCode("pfc", "p,whatever", tx));
    fcList.addCode(game::spec::FriendlyCode("gs3", "s-3,give to %3", tx));
    fcList.addCode(game::spec::FriendlyCode("gs4", "s,give to %4", tx));
    afl::io::ConstMemoryStream ms(afl::string::toBytes("ab"));
    fcList.loadExtraCodes(ms, tx);

    // Testee
    server::play::FriendlyCodePacker testee(*shipList, *root, tx);
    a.checkEqual("11. getName", testee.getName(), "fcode");

    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access ap(result.get());

    a.checkEqual("21. getArraySize", ap.getArraySize(), 4U);
    a.checkEqual("22", ap[0]("NAME").toString(), "pfc");
    a.checkEqual("23", ap[0]("FLAGS").toString(), "p");
    a.checkEqual("24", ap[0]("DESCRIPTION").toString(), "whatever");
    a.checkEqual("25", ap[0]("RACES").toInteger(), -1);

    a.checkEqual("31", ap[1]("NAME").toString(), "gs3");
    a.checkEqual("32", ap[1]("FLAGS").toString(), "s");
    a.checkEqual("33", ap[1]("DESCRIPTION").toString(), "give to Threes");
    a.checkEqual("34", ap[1]("RACES").toInteger(), ~(1<<3));

    a.checkEqual("41", ap[2]("NAME").toString(), "gs4");
    a.checkEqual("42", ap[2]("FLAGS").toString(), "s");
    a.checkEqual("43", ap[2]("DESCRIPTION").toString(), "give to 4");
    a.checkEqual("44", ap[2]("RACES").toInteger(), -1);

    a.checkEqual("51", ap[3]("NAME").toString(), "ab");
    a.checkEqual("52", ap[3]("FLAGS").toString(), "x");
    a.checkEqual("53", ap[3]("DESCRIPTION").toString(), "");
    a.checkEqual("54", ap[3]("RACES").toInteger(), -1);
}
