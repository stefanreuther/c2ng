/**
  *  \file u/t_game_parser_messageparser.cpp
  *  \brief Test for game::parser::MessageParser
  */

#include "game/parser/messageparser.hpp"

#include "t_game_parser.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/log.hpp"
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
        TSM_ASSERT(msg, !"missing value");
        throw std::runtime_error("boom");
    }
}

/** Test parsing the host version (Configuration, non-continue). */
void
TestGameParserMessageParser::testHostVersion()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse messages
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-h000) PHOST v3.4a\n"
                                                     "HUL=DFC40C02\n"
                                                     "ENG=C9FFADD7\n"
                                                     "BEA=A3B33229\n"
                                                     "TOR=945A6730\n"
                                                     "TRU=74071860\n"
                                                     "PXY=1CDA17D2\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTVERSION", "HostVersion"), "v3.4a");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTTYPE",    "HostType"),    "PHost");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
                                                     "(For Ship Build Queue)\n"
                                                     "Southern   :    16\n"
                                                     "[...]\n"
                                                     "Northern   :    13\n"
                                                     "HOST Version 3.22.020\n"
                                                     "Compiled: Nov 26, 1997\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTVERSION", "HostVersion"), "3.22.020");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "HOSTTYPE",    "HostType"),    "Host");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-r1000)<<< Sub Space Message >>>\n"
                                                     "FROM: The Feds\n"
                                                     "TO: The Lizards\n"
                                                     "\n"
                                                     "This is war!\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 0U);
    }
}

/** Test parsing configuration. */
void
TestGameParserMessageParser::testConfig()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 3U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-g0000)< Message from your Host >\n"
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
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "GROUNDKILLFACTOR", "GroundKillFactor"), "1,20,1,10,,,1,,,,");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "SCANRANGE", "ScanRange"), "300");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "ALLOWHISS", "AllowHiss"), "YES");
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-g0000)< Shortened >\n"
                                                     "a2 hiss mission  YES\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Configuration);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageConfigurationValue_t>(*info[0], "ALLOWHISS", "AllowHiss"), "YES");
    }
}

/** Test mixed object information. */
void
TestGameParserMessageParser::testObjects()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 3U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-90306)<<< Captain's Log >>>\n"
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
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Planet);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 306);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Owner, "Owner"), 5);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalN, "TotalN"), 880);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalT, "TotalT"), 829);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalD, "TotalD"), 876);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalM, "TotalM"), 862);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetCash, "Money"), 0);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetHasBase, "HasBase"), 0);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-i0021)<<< ION Advisory >>>\n"
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
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::IonStorm);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 21);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 2468);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2639);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_IonVoltage, "Voltage"), 105);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Heading, "Heading"), 197);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Speed, "Speed"), 6);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Radius, "Money"), 167);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_IonStatus, "Status"), 1);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-x0005)<< Long Range Sensors >>\n"
                                                     "Distress call and explosion\n"
                                                     "detected from a starship at:\n"
                                                     "( 1930 , 2728 )\n"
                                                     "The name of the ship was the: \n"
                                                     "C.S.S. War03\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 1930);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2728);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Name"), "C.S.S. War03");
    }
}

/** Test a message that matches multiple patterns, not mergeable. */
void
TestGameParserMessageParser::testMulti()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-e0000)<< MESSAGE FROM ENEMY >>\n"
                                                     "<<< DISTRESS CALL! >>>\n"
                                                     "\n"
                                                     "C.S.S. Scout        \n"    // <- appears space-padded in host-generated files!
                                                     "ID #  162\n"
                                                     "Has struck a mine!\n"
                                                     "AT: (  2758 , 1709 )\n"
                                                     "Damage is at  400%\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 2U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Explosion Name"), "C.S.S. Scout");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "Explosion X"), 2758);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Explosion Y"), 1709);

        TS_ASSERT(info[1] != 0);
        TS_ASSERT_EQUALS(info[1]->getObjectType(), game::parser::MessageInformation::Ship);
        TS_ASSERT_EQUALS(info[1]->getObjectId(), 162);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageStringValue_t>(*info[1], game::parser::ms_Name, "Ship Name"), "C.S.S. Scout");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_X, "Ship X"), 2758);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_Y, "Ship Y"), 1709);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[1], game::parser::mi_Damage, "Ship Damage"), 400);
    }
    {
        // This generates just one record because the ship Id is 0
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-e0000)<< MESSAGE FROM ENEMY >>\n"
                                                     "USS Null\n"
                                                     "ID #0\n"
                                                     "Has struck a mine!\n"
                                                     "AT: (1234,4567)\n"
                                                     "Damage is at  400%\n",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "Explosion Name"), "USS Null");
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "Explosion X"), 1234);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Explosion Y"), 4567);
    }
}

/** Test score parsing. */
void
TestGameParserMessageParser::testScore()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
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
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::PlayerScore);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 2);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  1, "pl1"), 16);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  2, "pl2"), 15);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  3, "pl3"), 20);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  5, "pl5"), 20);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  6, "pl6"), 2);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  7, "pl7"), 7);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  8, "pl8"), 19);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0],  9, "pl9"), 15);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0], 10, "pl10"), 5);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageScoreValue_t>(*info[0], 11, "pl11"), 13);
    }
}

/** Test message that generates a delta value [this does not yet appear in msgparse.ini]. */
void
TestGameParserMessageParser::testDelta()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-q0200)<<< 3D Scanner >>>\n"
                                                     "Ship has 500 fuel on starbord, and 30 on portside.",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Ship);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 200);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_PlanetTotalN, "TotalN"), 530);
    }
}

/** Test tim-alliance handling. */
void
TestGameParserMessageParser::testTimAllies()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse message
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-c0000)<<< Priority Points >>>\n"
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
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Alliance);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);

        Offer o = getValue<game::parser::MessageAllianceValue_t>(*info[0], "thost.ally", "thost.ally");
        TS_ASSERT_EQUALS(o.oldOffer.get(3), Offer::Yes);
        TS_ASSERT_EQUALS(o.oldOffer.get(7), Offer::No);
        TS_ASSERT_EQUALS(o.oldOffer.get(9), Offer::Yes);
        TS_ASSERT_EQUALS(o.theirOffer.get(3), Offer::No);
        TS_ASSERT_EQUALS(o.theirOffer.get(7), Offer::No);
        TS_ASSERT_EQUALS(o.theirOffer.get(9), Offer::Yes);
    }
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-g0000)FF / ff Allies:\n"
                                                     "Race:  4 YES / YES\n"
                                                     "Race:  7 YES / yes\n"
                                                     "Race:  2 yes / no\n",
                                                     ifc, 30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT(info[0] != 0);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Alliance);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);

        Offer o = getValue<game::parser::MessageAllianceValue_t>(*info[0], "thost.ff", "thost.ff");
        TS_ASSERT_EQUALS(o.oldOffer.get(2), Offer::No);
        TS_ASSERT_EQUALS(o.oldOffer.get(4), Offer::Yes);
        TS_ASSERT_EQUALS(o.oldOffer.get(7), Offer::Yes);
        TS_ASSERT_EQUALS(o.theirOffer.get(2), Offer::No);
        TS_ASSERT_EQUALS(o.theirOffer.get(4), Offer::Yes);
        TS_ASSERT_EQUALS(o.theirOffer.get(7), Offer::No);
    }
}

/** Test failure to provide Id. */
void
TestGameParserMessageParser::testFailId()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 2U);
    MockDataInterface ifc;

    // Parse messages
    // - Ship (mandatory Id), fails
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("<<< Ship Scanner >>>\n"
                                                     "Ship has 500 fuel.",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT(info.empty());
    }

    // - Explosion (optional Id), succeeds
    {
        afl::container::PtrVector<game::parser::MessageInformation> info;
        TS_ASSERT_THROWS_NOTHING(testee.parseMessage("<<< Explosion Scanner >>>\n"
                                                     "Name was FRED",
                                                     ifc,
                                                     30, info, tx, log));
        TS_ASSERT_EQUALS(info.size(), 1U);
        TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::Explosion);
        TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);
        TS_ASSERT_EQUALS(getValue<game::parser::MessageStringValue_t>(*info[0], game::parser::ms_Name, "FRED"), "FRED");
    }
}

/** Test creation of markers. */
void
TestGameParserMessageParser::testMarker()
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
    TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    TS_ASSERT_EQUALS(testee.getNumTemplates(), 1U);
    MockDataInterface ifc;

    afl::container::PtrVector<game::parser::MessageInformation> info;
    TS_ASSERT_THROWS_NOTHING(testee.parseMessage("(-x0005)<< Long Range Sensors >>\n"
                                                 "Distress call and explosion\n"
                                                 "detected from a starship at:\n"
                                                 "( 1930 , 2728 )\n"
                                                 "The name of the ship was the: \n"
                                                 "C.S.S. War03\n",
                                                 ifc,
                                                 30, info, tx, log));
    TS_ASSERT_EQUALS(info.size(), 1U);
    TS_ASSERT(info[0] != 0);
    TS_ASSERT_EQUALS(info[0]->getObjectType(), game::parser::MessageInformation::MarkerDrawing);
    TS_ASSERT_EQUALS(info[0]->getObjectId(), 0);
    TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_X, "X"), 1930);
    TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Y, "Y"), 2728);
    TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_DrawingShape, "shape"), 3);
    TS_ASSERT_EQUALS(getValue<game::parser::MessageIntegerValue_t>(*info[0], game::parser::mi_Color, "color"), 5);
}
