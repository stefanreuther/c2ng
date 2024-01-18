/**
  *  \file test/game/parser/messageparsertest.cpp
  *  \brief Test for game::parser::MessageParser
  */

#include "game/parser/messageparser.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "util/stringparser.hpp"

namespace {
    // FIXME: duplicated/extended from _messagetemplate.cpp
    class MockDataInterface : public game::parser::DataInterface {
     public:
        MockDataInterface()
            { setupPlayerList(); }
        virtual int getPlayerNumber() const
            { return 0; }
        virtual int parseName(Name which, const String_t& name) const
            {
                util::StringParser sp(name);
                bool ok = false;
                switch (which) {
                 case ShortRaceName:     ok = sp.parseString("s"); break;
                 case LongRaceName:      ok = sp.parseString("f"); break;
                 case AdjectiveRaceName: ok = sp.parseString("a"); break;
                 case HullName:          ok = sp.parseString("h"); break;
                }
                int n;
                if (ok && sp.parseInt(n) && sp.parseEnd()) {
                    return n;
                } else {
                    return 0;
                }
            }
        virtual String_t expandRaceNames(String_t name) const
            {
                afl::string::NullTranslator tx;
                return m_playerList.expandNames(name, true, tx);
            }
     private:
        void setupPlayerList()
            {
                for (int i = 1; i <= 11; ++i) {
                    if (game::Player* pl = m_playerList.create(i)) {
                        pl->setName(game::Player::ShortName, afl::string::Format("s%d", i));
                        pl->setName(game::Player::AdjectiveName, afl::string::Format("a%d", i));
                        pl->setOriginalNames();
                    }
                }
            }
        game::PlayerList m_playerList;
    };

    // Helper
    template<typename MV>
    typename MV::Value_t getValue(const game::parser::MessageInformation& info, typename MV::Index_t index, const char* msg)
    {
        for (game::parser::MessageInformation::Iterator_t i = info.begin(), e = info.end(); i != e; ++i) {
            if (const MV* p = dynamic_cast<const MV*>(*i)) {
                if (p->getIndex() == index) {
                    return p->getValue();
                }
            }
        }
        throw std::runtime_error(String_t("not found: ") + msg);
    }
}

/** Test parsing the host version (Configuration, non-continue). */
AFL_TEST("game.parser.MessageParser:host-version", a)
{
    // Prepare
    static const char* FILE =
        "; Host Version Detection\n"
        "\n"
        "config,THost PBP Message\n"
        "  kind   = c\n"
        "  check  = Priority Points\n"
        "  check  = Build Queue\n"
        "  parse  = Host Version $\n"
        "  assign = HostVersion\n"
        "  value  = Host\n"
        "  assign = HostType\n"
        "\n"
        "config,PHost Version Message\n"
        "  kind   = h\n"
        "  check  = HUL=\n"
        "  check  = PXY=\n"
        "  parse  = =1,PHost $\n"
        "  assign = HostVersion\n"
        "  value  = PHost\n"
        "  assign = HostType\n"
        "\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse messages
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-h000) PHOST v3.4a\n"
                                                                      "HUL=DFC40C02\n"
                                                                      "ENG=C9FFADD7\n"
                                                                      "BEA=A3B33229\n"
                                                                      "TOR=945A6730\n"
                                                                      "TRU=74071860\n"
                                                                      "PXY=1CDA17D2\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        a.checkEqual("15. HOSTVERSION", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTVERSION", "HostVersion"), "v3.4a");
        a.checkEqual("16. HOSTTYPE", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTTYPE",    "HostType"),    "PHost");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("17. parseMessage"), testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
                                                                      "(For Ship Build Queue)\n"
                                                                      "Southern   :    16\n"
                                                                      "[...]\n"
                                                                      "Northern   :    13\n"
                                                                      "HOST Version 3.22.020\n"
                                                                      "Compiled: Nov 26, 1997\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("18. size", info.size(), 1U);
        a.checkNonNull("19. info", info[0]);
        a.checkEqual("20. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        a.checkEqual("21. HOSTVERSION", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTVERSION", "HostVersion"), "3.22.020");
        a.checkEqual("22. HOSTTYPE", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTTYPE",    "HostType"),    "Host");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("23. parseMessage"), testee.parseMessage("(-r1000)<<< Sub Space Message >>>\n"
                                                                      "FROM: The Feds\n"
                                                                      "TO: The Lizards\n"
                                                                      "\n"
                                                                      "This is war!\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("24. size", info.size(), 0U);
    }
}

/** Test parsing configuration. */
AFL_TEST("game.parser.MessageParser:config", a)
{
    const char* FILE =
        "config,GroundKillFactor\n"
        "  kind     = g\n"
        "  check    = Ground Attack Kill Ratio\n"
        "  array    = +1,$ $ : 1\n"
        "  assign   = Index:Race.Adj, GroundKillFactor\n"
        "  continue = y\n"
        "\n"
        "config,ScanRange\n"
        "  kind     = g\n"
        "  parse    = Ships are visible at $\n"
        "  assign   = ScanRange\n"
        "  continue = y\n"
        "\n"
        "config,AllowHiss\n"
        "  kind     = g\n"
        "  parse    = %-2 hiss mission $\n"
        "  assign   = AllowHiss\n"
        "  continue = y\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 3U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-g0000)< Message from your Host >\n"
                                                                      "Ground Attack Kill Ratio\n"
                                                                      "  a1           1  : 1\n"
                                                                      "  a2           20 : 1\n"
                                                                      "  a3           1  : 1\n"
                                                                      "  a4           10 : 1\n"
                                                                      "  a7           1  : 1\n"
                                                                      "Ships are visible at  300\n"
                                                                      "a2 hiss mission  YES\n"
                                                                      "a10 ground attack  YES\n"
                                                                      "a1 super refit  YES\n"
                                                                      "Web mines  YES",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        a.checkEqual("15. GROUNDKILLFACTOR", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "GROUNDKILLFACTOR", "GroundKillFactor"), "1,20,1,10,,,1,,,,");
        a.checkEqual("16. SCANRANGE", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "SCANRANGE", "ScanRange"), "300");
        a.checkEqual("17. ALLOWHISS", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "ALLOWHISS", "AllowHiss"), "YES");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("18. parseMessage"), testee.parseMessage("(-g0000)< Shortened >\n"
                                                                      "a2 hiss mission  YES\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("19. size", info.size(), 1U);
        a.checkNonNull("20. info", info[0]);
        a.checkEqual("21. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        a.checkEqual("22. ALLOWHISS", getValue<game::parser::MessageConfigurationValue_t>(*info[0], "ALLOWHISS", "AllowHiss"), "YES");
    }
}

/** Test mixed object information. */
AFL_TEST("game.parser.MessageParser:objects", a)
{
    const char* FILE =
        "ionstorm,Ion Storm Warning\n"
        "  kind   = i\n"
        "  values = Id\n"
        "  assign = Id\n"
        "  parse  = Centered at: ($,$)\n"
        "  assign = X, Y\n"
        "  parse  = Voltage : $\n"
        "  assign = Voltage\n"
        "  parse  = Heading : $\n"
        "  assign = Heading\n"
        "  parse  = Speed $ Warp $\n"
        "  assign = _, Speed\n"
        "  parse  = Radius  : $\n"
        "  assign = Radius\n"
        "  parse  = System is $\n"
        "  assign = Status:weakening/growing\n"
        "\n"
        "planet,Dark Sense\n"
        "  kind   = 9\n"
        "  check  = dark sense\n"
        "  parse  = there are $\n"
        "  assign = Owner:Race.Adj\n"
        "  check  = Minerals on\n"
        "  parse  = +1,N: $\n"
        "  assign = Total.N\n"
        "  parse  = +0,T: $\n"
        "  assign = Total.T\n"
        "  parse  = +0,D: $\n"
        "  assign = Total.D\n"
        "  parse  = +0,M: $\n"
        "  assign = Total.M\n"
        "  parse  = Megacredits : $\n"
        "  assign = Money\n"
        "  find   = They have a starbase\n"
        "  assign = Base\n"
        "  value  = Id\n"
        "  assign = Id\n"
        "\n"
        "explosion,THost\n"
        "  kind   = x\n"
        "  parse  = ($,$)\n"
        "  assign = X, Y\n"
        "  check  = The name of the ship\n"
        "  parse  = +1,$\n"
        "  assign = Name";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 3U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-90306)<<< Captain's Log >>>\n"
                                                                      "(using the dark sense)\n"
                                                                      "I have a feeling that\n"
                                                                      "there are a5\n"
                                                                      "colonists living on\n"
                                                                      "Neinmen\n"
                                                                      "Planet ID#  306\n"
                                                                      " Minerals on/in planet\n"
                                                                      "N: 880 M: 862 T: 829 D: 876\n"
                                                                      "  Megacredits :  0\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType",    info[0]->getObjectType(), game::parser::MessageInformation::Planet);
        a.checkEqual("15. getObjectId",      info[0]->getObjectId(), 306);
        a.checkEqual("16. mi_Owner",         getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Owner, "Owner"), 5);
        a.checkEqual("17. mi_TotalN",        getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalN, "TotalN"), 880);
        a.checkEqual("18. mi_TotalT",        getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalT, "TotalT"), 829);
        a.checkEqual("19. mi_TotalD",        getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalD, "TotalD"), 876);
        a.checkEqual("20. mi_TotalM",        getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalM, "TotalM"), 862);
        a.checkEqual("21. mi_Money",         getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetCash, "Money"), 0);
        a.checkEqual("22. mi_PlanetHasBase", getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetHasBase, "HasBase"), 0);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("23. parseMessage"), testee.parseMessage("(-i0021)<<< ION Advisory >>>\n"
                                                                      "Ion Disturbance\n"
                                                                      "ID Number:  21\n"
                                                                      "Centered At: (  2468, 2639)\n"
                                                                      "West of Regula\n"
                                                                      "Planet ID Number  45\n"
                                                                      " 31 LY from planet\n"
                                                                      "Voltage : 105\n"
                                                                      "Heading : 197\n"
                                                                      "Speed   :  Warp 6\n"
                                                                      "Radius  : 167\n"
                                                                      "Class :  Level 3\n"
                                                                      "  Strong\n"
                                                                      "System is growing",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("24. size", info.size(), 1U);
        a.checkNonNull("25. info", info[0]);
        a.checkEqual("26. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::IonStorm);
        a.checkEqual("27. getObjectId",   info[0]->getObjectId(), 21);
        a.checkEqual("28. mi_Y",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 2468);
        a.checkEqual("29. mi_Y",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2639);
        a.checkEqual("30. mi_IonVoltage", getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_IonVoltage, "Voltage"), 105);
        a.checkEqual("31. mi_Heading",    getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Heading, "Heading"), 197);
        a.checkEqual("32. mi_WarpFactor", getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_WarpFactor, "Speed"), 6);
        a.checkEqual("33. mi_Radius",     getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Radius, "Money"), 167);
        a.checkEqual("34. mi_IonStatus",  getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_IonStatus, "Status"), 1);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("35. parseMessage"), testee.parseMessage("(-x0005)<< Long Range Sensors >>\n"
                                                                      "Distress call and explosion\n"
                                                                      "detected from a starship at:\n"
                                                                      "( 1930 , 2728 )\n"
                                                                      "The name of the ship was the: \n"
                                                                      "C.S.S. War03\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("36. size", info.size(), 1U);
        a.checkNonNull("37. info", info[0]);
        a.checkEqual("38. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        a.checkEqual("39. getObjectId", info[0]->getObjectId(), 0);
        a.checkEqual("40. mi_X",    getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 1930);
        a.checkEqual("41. mi_Y",    getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2728);
        a.checkEqual("42. mi_Name", getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Name"), "C.S.S. War03");
    }
}

/** Test a message that matches multiple patterns, not mergeable. */
AFL_TEST("game.parser.MessageParser:multiple", a)
{
    const char* FILE =
        "explosion,Mine Hit\n"
        "  kind     = e\n"
        "  parse    = Has struck a mine\n"
        "  check    = ID #\n"
        "  parse    = -1,$\n"
        "  assign   = Name\n"
        "  parse    = At: ($,$)\n"
        "  assign   = X, Y\n"
        "  continue = y\n"
        "\n"
        "ship,Mine Hit\n"
        "  ; Match the same things again, but this time produce a ship,\n"
        "  ; not an explosion.\n"
        "  kind     = e\n"
        "  parse    = Has struck a mine\n"
        "  parse    = ID #$\n"
        "  assign   = Id\n"
        "  parse    = -1,$\n"
        "  assign   = Name\n"
        "  parse    = At: ($,$)\n"
        "  assign   = X, Y\n"
        "  parse    = Damage is at $\n"
        "  assign   = Damage\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-e0000)<< MESSAGE FROM ENEMY >>\n"
                                                                      "<<< DISTRESS CALL! >>>\n"
                                                                      "\n"
                                                                      "C.S.S. Scout        \n"    // <- appears space-padded in host-generated files!
                                                                      "ID #  162\n"
                                                                      "Has struck a mine!\n"
                                                                      "AT: (  2758 , 1709 )\n"
                                                                      "Damage is at  400%\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 2U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        a.checkEqual("15. getObjectId",   info[0]->getObjectId(), 0);
        a.checkEqual("16. ms_Name",       getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Explosion Name"), "C.S.S. Scout");
        a.checkEqual("17. mi_X",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "Explosion X"), 2758);
        a.checkEqual("18. mi_Y",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Explosion Y"), 1709);

        a.checkNonNull("21. info", info[1]);
        a.checkEqual("22. getObjectType", info[1]->getObjectType(), game::parser::MessageInformation::Ship);
        a.checkEqual("23. getObjectId",   info[1]->getObjectId(), 162);
        a.checkEqual("24. ms_Name",       getValue<game::parser::MessageStringValue_t>(*info[1], game::parser::ms_Name, "Ship Name"), "C.S.S. Scout");
        a.checkEqual("25. ms_X",          getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_X, "Ship X"), 2758);
        a.checkEqual("26. ms_Y",          getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_Y, "Ship Y"), 1709);
        a.checkEqual("27. mi_Damage",     getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_Damage, "Ship Damage"), 400);
    }
    {
        // This generates just one record because the ship Id is 0
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("28. parseMessage"), testee.parseMessage("(-e0000)<< MESSAGE FROM ENEMY >>\n"
                                                                      "USS Null\n"
                                                                      "ID #0\n"
                                                                      "Has struck a mine!\n"
                                                                      "AT: (1234,4567)\n"
                                                                      "Damage is at  400%\n",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("29. size", info.size(), 1U);
        a.checkNonNull("30. info", info[0]);
        a.checkEqual("31. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        a.checkEqual("32. getObjectId",   info[0]->getObjectId(), 0);
        a.checkEqual("33. ms_Name",       getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Explosion Name"), "USS Null");
        a.checkEqual("34. mi_X",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "Explosion X"), 1234);
        a.checkEqual("35. mi_Y",          getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Explosion Y"), 4567);
    }
}

/** Test score parsing. */
AFL_TEST("game.parser.MessageParser:score", a)
{
    const char* FILE =
        "playerscore,PBPs\n"
        "  kind   = c\n"
        "  check  = Priority Points\n"
        "  check  = Build Queue\n"
        "  array  = +1,$ $\n"
        "  assign = Index:Race.Adj+Allies, Score\n"
        "  values = 2\n"
        "  assign = Id\n"
        "  continue = y\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
                                                                      "(For Ship Build Queue)\n"
                                                                      "a1   :          16\n"
                                                                      "a2              15\n"
                                                                      "a3   :          20\n"
                                                                      "bogus4   :      3\n"
                                                                      "a5         :    20\n"
                                                                      "a6       :      2\n"
                                                                      "a7            : 7\n"
                                                                      "a8           !  19\n"
                                                                      "a9     +! :     15\n"
                                                                      "a10   +! :      5\n"
                                                                      "a11        :    13\n"
                                                                      "HOST Version 3.22.020\n"
                                                                      "Compiled: Nov 26, 1997",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::PlayerScore);
        a.checkEqual("15. getObjectId", info[0]->getObjectId(), 2);
        a.checkEqual("16. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  1, "pl1"), 16);
        a.checkEqual("17. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  2, "pl2"), 15);
        a.checkEqual("18. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  3, "pl3"), 20);
        a.checkEqual("19. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  5, "pl5"), 20);
        a.checkEqual("20. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  6, "pl6"), 2);
        a.checkEqual("21. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  7, "pl7"), 7);
        a.checkEqual("22. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  8, "pl8"), 19);
        a.checkEqual("23. score", getValue<game::parser::MessageScoreValue_t>(*info[0],  9, "pl9"), 15);
        a.checkEqual("24. score", getValue<game::parser::MessageScoreValue_t>(*info[0], 10, "pl10"), 5);
        a.checkEqual("25. score", getValue<game::parser::MessageScoreValue_t>(*info[0], 11, "pl11"), 13);
    }
}

/** Test message that generates a delta value [this does not yet appear in msgparse.ini]. */
AFL_TEST("game.parser.MessageParser:delta", a)
{
    const char* FILE =
        "ship,Delta\n"
        "  check  = 3D Scanner\n"
        "  parse  = Ship has $ fuel on starbord, and $ on portside.\n"
        "  assign = Total.N, +Total.N\n"
        "  values = Id\n"
        "  assign = Id\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-q0200)<<< 3D Scanner >>>\n"
                                                                      "Ship has 500 fuel on starbord, and 30 on portside.",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType",   info[0]->getObjectType(), game::parser::MessageInformation::Ship);
        a.checkEqual("15. getObjectId",     info[0]->getObjectId(), 200);
        a.checkEqual("16. mi_PlanetTotalN", getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalN, "TotalN"), 530);
    }
}

/** Test tim-alliance handling. */
AFL_TEST("game.parser.MessageParser:tim-allies", a)
{
    using game::alliance::Offer;

    const char* FILE =
        "alliance,Classic allies\n"
        "  kind   = c\n"
        "  check  = Priority Points\n"
        "  check  = Build Queue\n"
        "  array  = +1,$ $\n"
        "  assign = Flags, _\n"
        "  values = thost.ally\n"
        "  assign = Name\n"
        "alliance,Strong allies\n"
        "  kind   = g\n"
        "  check  = FF allies\n"
        "  array  = +1,Race: $ $ / $\n"
        "  assign = Index, ToFF, FromFF\n"
        "  values = thost.ff\n"
        "  assign = Name\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
                                                                      "(For Ship Build Queue)\n"
                                                                      "a1   :          16\n"
                                                                      "a2              15\n"
                                                                      "a3  + :         20\n"
                                                                      "bogus4   :      3\n"
                                                                      "a5         :    20\n"
                                                                      "a6       :      2\n"
                                                                      "a7            : 7\n"
                                                                      "a8           !  19\n"
                                                                      "a9     +! :     15\n"
                                                                      "a10   +! :      5\n"
                                                                      "a11        :    13\n"
                                                                      "HOST Version 3.22.020\n"
                                                                      "Compiled: Nov 26, 1997",
                                                                      ifc, 30, info, tx, log));
        a.checkEqual("12. size", info.size(), 1U);
        a.checkNonNull("13. info", info[0]);
        a.checkEqual("14. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Alliance);
        a.checkEqual("15. getObjectId", info[0]->getObjectId(), 0);

        Offer o = getValue<game::parser::MessageAllianceValue_t>(*info[0], "thost.ally", "thost.ally");
        a.checkEqual("21. oldOffer", o.oldOffer.get(3), Offer::Yes);
        a.checkEqual("22. oldOffer", o.oldOffer.get(7), Offer::No);
        a.checkEqual("23. oldOffer", o.oldOffer.get(9), Offer::Yes);
        a.checkEqual("24. theirOffer", o.theirOffer.get(3), Offer::No);
        a.checkEqual("25. theirOffer", o.theirOffer.get(7), Offer::No);
        a.checkEqual("26. theirOffer", o.theirOffer.get(9), Offer::Yes);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("27. parseMessage"), testee.parseMessage("(-g0000)FF / ff Allies:\n"
                                                                      "Race:  4 YES / YES\n"
                                                                      "Race:  7 YES / yes\n"
                                                                      "Race:  2 yes / no\n",
                                                                      ifc, 30, info, tx, log));
        a.checkEqual("28. size", info.size(), 1U);
        a.checkNonNull("29. info", info[0]);
        a.checkEqual("30. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Alliance);
        a.checkEqual("31. getObjectId", info[0]->getObjectId(), 0);

        Offer o = getValue<game::parser::MessageAllianceValue_t>(*info[0], "thost.ff", "thost.ff");
        a.checkEqual("41. oldOffer", o.oldOffer.get(2), Offer::No);
        a.checkEqual("42. oldOffer", o.oldOffer.get(4), Offer::Yes);
        a.checkEqual("43. oldOffer", o.oldOffer.get(7), Offer::Yes);
        a.checkEqual("44. theirOffer", o.theirOffer.get(2), Offer::No);
        a.checkEqual("45. theirOffer", o.theirOffer.get(4), Offer::Yes);
        a.checkEqual("46. theirOffer", o.theirOffer.get(7), Offer::No);
    }
}

/** Test failure to provide Id. */
AFL_TEST("game.parser.MessageParser:error:missing-id", a)
{
    const char* FILE =
        "ship,Fail\n"
        "  check  = Ship Scanner\n"
        "  parse  = Ship has $ fuel.\n"
        "  assign = Total.N, Id\n"
        "explosion,Fail\n"
        "  check  = Explosion Scanner\n"
        "  parse  = Name was $\n"
        "  assign = Name, Id\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse messages
    // - Ship (mandatory Id), fails
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("<<< Ship Scanner >>>\n"
                                                                      "Ship has 500 fuel.",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.check("12. info", info.empty());
    }

    // - Explosion (optional Id), succeeds
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        AFL_CHECK_SUCCEEDS(a("21. parseMessage"), testee.parseMessage("<<< Explosion Scanner >>>\n"
                                                                      "Name was FRED",
                                                                      ifc,
                                                                      30, info, tx, log));
        a.checkEqual("22. size", info.size(), 1U);
        a.checkEqual("23. getObjectType", info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        a.checkEqual("24. getObjectId", info[0]->getObjectId(), 0);
        a.checkEqual("25. ms_Name", getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "FRED"), "FRED");
    }
}

/** Test creation of markers. */
AFL_TEST("game.parser.MessageParser:marker", a)
{
    const char* FILE =
        "marker,Test\n"
        "  check  = Distress call\n"
        "  check  = starship at:\n"
        "  parse  = +1,( $, $ )\n"
        "  assign = X, Y\n"
        "  values = 3, 5\n"
        "  assign = Shape, Color\n";
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    // Load
    game::parser::MessageParser testee;
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    a.checkEqual("02. getNumTemplates", testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    afl::container::PtrVector<game::parser::MessageInformation> info;
    AFL_CHECK_SUCCEEDS(a("11. parseMessage"), testee.parseMessage("(-x0005)<< Long Range Sensors >>\n"
                                                                  "Distress call and explosion\n"
                                                                  "detected from a starship at:\n"
                                                                  "( 1930 , 2728 )\n"
                                                                  "The name of the ship was the: \n"
                                                                  "C.S.S. War03\n",
                                                                  ifc,
                                                                  30, info, tx, log));
    a.checkEqual("12. size", info.size(), 1U);
    a.checkNonNull("13. info", info[0]);
    a.checkEqual("14. getObjectType",   info[0]->getObjectType(), game::parser::MessageInformation::MarkerDrawing);
    a.checkEqual("15. getObjectId",     info[0]->getObjectId(), 0);
    a.checkEqual("16. mi_X",            getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 1930);
    a.checkEqual("17. mi_Y",            getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2728);
    a.checkEqual("18. mi_DrawingShape", getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_DrawingShape, "shape"), 3);
    a.checkEqual("19. mi_Color",        getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Color, "color"), 5);
}
