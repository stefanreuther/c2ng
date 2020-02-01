/**
  *  \file u/t_game_map_planetinfo.cpp
  *  \brief Test for game::map::PlanetInfo
  */

#include "game/map/planetinfo.hpp"

#include "t_game_map.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/visitor.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/parser/messagevalue.hpp"
#include "game/test/root.hpp"
#include "game/test/specificationloader.hpp"

using afl::string::Format;
using afl::string::NullTranslator;
using game::Element;
using game::HostVersion;
using game::config::HostConfiguration;
using game::map::Planet;
using game::map::PlanetMineralInfo;
namespace gp = game::parser;

namespace {
    const int TURN = 77;
    const int PLAYER = 3;

    /** Environment for "describe" methods. */
    struct Environment {
        afl::io::xml::Nodes_t nodes;
        NullTranslator tx;
        game::test::Root root;

        Environment()
            : nodes(), tx(), root(HostVersion(HostVersion::PHost, MKVERSION(3, 2, 0)))
            { }
    };

    /** Make a scanned planet. */
    Planet makeScannedPlanet()
    {
        Planet pl(12);
        pl.setPosition(game::map::Point(1000, 1000));

        gp::MessageInformation info(gp::MessageInformation::Planet, pl.getId(), TURN);
        info.addValue(gp::mi_Owner, PLAYER);
        info.addValue(gp::mi_PlanetDensityN, 50);
        info.addValue(gp::mi_PlanetDensityT, 30);
        info.addValue(gp::mi_PlanetDensityD,  5);
        info.addValue(gp::mi_PlanetDensityM, 75);
        info.addValue(gp::mi_PlanetAddedN, 500);  // Added produces Ground ore
        info.addValue(gp::mi_PlanetAddedT, 300);
        info.addValue(gp::mi_PlanetAddedD, 200);
        info.addValue(gp::mi_PlanetAddedM, 100);
        info.addValue(gp::mi_PlanetMinedN, 1000);
        info.addValue(gp::mi_PlanetMinedT, 3000);
        info.addValue(gp::mi_PlanetMinedD, 2000);
        info.addValue(gp::mi_PlanetMinedM, 4000);
        pl.addMessageInformation(info);

        return pl;
    }

    /** Make a played planet. */
    Planet makePlayedPlanet()
    {
        game::map::PlanetData pd;
        pd.owner = PLAYER;
        pd.friendlyCode = "xyz";
        pd.numMines = 10;
        pd.numFactories = 20;
        pd.numDefensePosts = 5;
        pd.minedNeutronium = 200;
        pd.minedTritanium = 300;
        pd.minedDuranium = 400;
        pd.minedMolybdenum = 500;
        pd.colonistClans = 100;
        pd.supplies = 70;
        pd.money = 200;
        pd.groundNeutronium = 700;
        pd.groundTritanium = 800;
        pd.groundDuranium = 900;
        pd.groundMolybdenum = 1000;
        pd.densityNeutronium = 70;
        pd.densityTritanium = 60;
        pd.densityDuranium = 50;
        pd.densityMolybdenum = 40;
        pd.colonistTax = 5;
        pd.nativeTax = 7;
        pd.colonistHappiness = 93;
        pd.nativeHappiness = 96;
        pd.nativeGovernment = 6;
        pd.nativeClans = 5000;
        pd.nativeRace = game::ReptilianNatives;
        pd.temperature = 50;
        pd.baseFlag = 0;

        Planet p(77);
        p.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        p.setPlayability(game::map::Object::Playable);

        return p;
    }

    /** Quick and dirty stringification of a node list. */
    String_t toString(const afl::io::xml::Nodes_t& nodes)
    {
        class Visitor : public afl::io::xml::Visitor {
         public:
            Visitor(String_t& result)
                : m_result(result)
                { }

            virtual void visitPI(const afl::io::xml::PINode&)
                { TS_ASSERT(!"unexpected PI"); }
            virtual void visitTag(const afl::io::xml::TagNode& node)
                {
                    m_result += "<" + node.getName() + ">";
                    visitNodes(node.getChildren());
                    m_result += "</" + node.getName() + ">";
                }
            virtual void visitText(const afl::io::xml::TextNode& node)
                { m_result += node.get(); }

            void visitNodes(const afl::io::xml::Nodes_t& nodes)
                {
                    for (size_t i = 0, n = nodes.size(); i < n; ++i) {
                        visit(*nodes[i]);
                    }
                }

         private:
            String_t& m_result;
        };

        String_t result;
        Visitor(result).visitNodes(nodes);
        return result;
    }

    /** Quick and dirty stringification of a DefenseEffectInfo list. */
    String_t toString(const game::map::DefenseEffectInfos_t& list)
    {
        String_t result;
        for (size_t i = 0, n = list.size(); i < n; ++i) {
            const game::map::DefenseEffectInfo& e = list[i];
            if (e.isDetail) {
                result += "  ";
            }
            result += Format("%s (+%d)", e.name, e.nextAt);
            if (!e.isAchievable) {
                result += " (unachievable)";
            }
            result += "\n";
        }
        return result;
    }
}


/** Test packPlanetMineralInfo(), simple regression test. */
void
TestGameMapPlanetInfo::testPackPlanetMineralInfo()
{
    Planet pl = makeScannedPlanet();
    HostConfiguration config;
    HostVersion host(HostVersion::PHost, MKVERSION(3, 2, 0));
    NullTranslator tx;

    PlanetMineralInfo info = game::map::packPlanetMineralInfo(pl, Element::Tritanium, TURN, config, host, afl::base::Nothing, tx);

    // Amounts
    TS_ASSERT_EQUALS(info.status, PlanetMineralInfo::Scanned);
    TS_ASSERT_EQUALS(info.age.orElse(-1), 0);
    TS_ASSERT_EQUALS(info.ageLabel, "current turn");
    TS_ASSERT_EQUALS(info.minedAmount.orElse(0), 3000);
    TS_ASSERT_EQUALS(info.groundAmount.orElse(0), 300);
    TS_ASSERT_EQUALS(info.density.orElse(0), 30);
    TS_ASSERT_EQUALS(info.groundSummary, "rare");
    TS_ASSERT_EQUALS(info.densitySummary, "dispersed");

    // No mining information because we don't have any number of mines
    TS_ASSERT_EQUALS(info.miningPerTurn.isValid(), false);
    TS_ASSERT_EQUALS(info.miningDuration.isValid(), false);
}

/** Test packPlanetMineralInfo(), number-of-mines variations. */
void
TestGameMapPlanetInfo::testPackPlanetMineralInfoMineOverride()
{
    HostConfiguration config;
    HostVersion host(HostVersion::PHost, MKVERSION(3, 2, 0));
    NullTranslator tx;

    // Mine override given: 50 mines x 30% = 15 kt/turn = 20 turns
    {
        PlanetMineralInfo info = game::map::packPlanetMineralInfo(makeScannedPlanet(), Element::Tritanium, TURN, config, host, 50, tx);
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 15);
        TS_ASSERT_EQUALS(info.miningDuration.orElse(-1), 20);
    }

    // Number of mines on planet: 100 mines x 30% = 30 kt/turn = 10 turns
    {
        Planet p = makeScannedPlanet();
        p.setNumBuildings(game::MineBuilding, 100);
        PlanetMineralInfo info = game::map::packPlanetMineralInfo(p, Element::Tritanium, TURN, config, host, afl::base::Nothing, tx);
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 30);
        TS_ASSERT_EQUALS(info.miningDuration.orElse(-1), 10);
    }

    // Mine override given: 0 mines
    {
        PlanetMineralInfo info = game::map::packPlanetMineralInfo(makeScannedPlanet(), Element::Tritanium, TURN, config, host, 0, tx);
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 0);
        TS_ASSERT_EQUALS(info.miningDuration.isValid(), false);
    }

    // Number of mines on planet and override
    {
        Planet p = makeScannedPlanet();
        p.setNumBuildings(game::MineBuilding, 50);
        PlanetMineralInfo info = game::map::packPlanetMineralInfo(p, Element::Tritanium, TURN, config, host, 10, tx);
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 3);
        TS_ASSERT_EQUALS(info.miningDuration.orElse(-1), game::map::MAX_MINING_DURATION);
    }
}

/** Test packPlanetMineralInfo(), empty (unknown) planet. */
void
TestGameMapPlanetInfo::testPackPlanetMineralInfoEmpty()
{
    HostConfiguration config;
    HostVersion host(HostVersion::PHost, MKVERSION(3, 2, 0));
    NullTranslator tx;
    PlanetMineralInfo info = game::map::packPlanetMineralInfo(Planet(99), Element::Tritanium, TURN, config, host, afl::base::Nothing, tx);

    TS_ASSERT_EQUALS(info.status, PlanetMineralInfo::Unknown);
    TS_ASSERT_EQUALS(info.age.isValid(), false);
    TS_ASSERT_EQUALS(info.ageLabel, "");
    TS_ASSERT_EQUALS(info.minedAmount.isValid(), false);
    TS_ASSERT_EQUALS(info.groundAmount.isValid(), false);
    TS_ASSERT_EQUALS(info.density.isValid(), false);
    TS_ASSERT_EQUALS(info.groundSummary, "");
    TS_ASSERT_EQUALS(info.densitySummary, "");
}

/** Test describePlanetClimate().
    This is mostly a regression test, in particular also for ports. */
void
TestGameMapPlanetInfo::testDescribePlanetClimate()
{
    Environment env;
    describePlanetClimate(env.nodes, makePlayedPlanet(), TURN, env.root, PLAYER, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Climate type: warm</li>"
                     "<li>Average temperature: 50\xC2\xB0""F</li>"
                     "<li>Supports 10,000,000 Player 3s</li>"
                     "</ul>");
}

/** Test describePlanetClimate(), empty (unknown) planet. */
void
TestGameMapPlanetInfo::testDescribePlanetClimateEmpty()
{
    Environment env;
    describePlanetClimate(env.nodes, Planet(77), TURN, env.root, 6, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>No information on climate available.</li>"
                     "</ul>");
}

/** Test describePlanetClimate(), different players. */
void
TestGameMapPlanetInfo::testDescribePlanetClimateDifferent()
{
    const int VIEWPOINT = 7;
    static_assert(PLAYER != VIEWPOINT, "PLAYER");

    Environment env;
    describePlanetClimate(env.nodes, makePlayedPlanet(), TURN, env.root, VIEWPOINT, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Climate type: warm</li>"
                     "<li>Average temperature: 50\xC2\xB0""F</li>"
                     "<li>Supports 10,000,000 Player 3s</li>"
                     "<li>Supports 5,000,000 Player 7s</li>"
                     "</ul>");
}

/** Test describePlanetNatives(). */
void
TestGameMapPlanetInfo::testDescribePlanetNatives()
{
    Environment env;
    describePlanetNatives(env.nodes, makePlayedPlanet(), TURN, env.root, PLAYER, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Native race: Reptilian"
                     "<ul><li>Double mining rates</li></ul></li>"
                     "<li>Population: 500,000</li>"
                     "<li>Government: Monarchy (120%)</li>"
                     "<li>Base Tax Rate: 9% (54 mc)</li>"
                     "<li>Max Tax Rate: 44% (264 mc)</li>"
                     "</ul>");
}

/** Test describePlanetNatives(), empty (unknown) planet. */
void
TestGameMapPlanetInfo::testDescribePlanetNativesEmpty()
{
    Environment env;
    describePlanetNatives(env.nodes, Planet(77), TURN, env.root, 6, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>No information on natives available.</li>"
                     "</ul>");
}

/** Test describePlanetColony(). */
void
TestGameMapPlanetInfo::testDescribePlanetColony()
{
    Environment env;
    describePlanetColony(env.nodes, makePlayedPlanet(), TURN, env.root, PLAYER, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Colonists: Player 3</li>"
                     "<li>Population: 10,000</li>"
                     "<li>20 factories, 10 mines, 5 DPs</li>"
                     "<li>200 mc, 70 supplies</li>"
                     "<li>Friendly code: xyz</li>"
                     "</ul>");
}

/** Test describePlanetColony(), empty (unknown) planet. */
void
TestGameMapPlanetInfo::testDescribePlanetColonyEmpty()
{
    Environment env;
    describePlanetColony(env.nodes, Planet(77), TURN, env.root, 6, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>No information on colonists available.</li>"
                     "</ul>");
}

/** Test describePlanetColony(), RGA case. */
void
TestGameMapPlanetInfo::testDescribePlanetColonyRGA()
{
    const int VIEWPOINT = 10;
    static_assert(PLAYER != VIEWPOINT, "PLAYER");

    Environment env;
    describePlanetColony(env.nodes, makePlayedPlanet(), TURN, env.root, VIEWPOINT, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Colonists: Player 3</li>"
                     "<li>Population: 10,000</li>"
                     "<li>RGA max. 17 turns</li>"
                     "<li>20 factories, 10 mines, 5 DPs</li>"
                     "<li>200 mc, 70 supplies</li>"
                     "<li>Friendly code: xyz</li>"
                     "</ul>");
}

/** Test describePlanetBuildingEffects(). */
void
TestGameMapPlanetInfo::testDescribePlanetBuildingEffects()
{
    Environment env;
    describePlanetBuildingEffects(env.nodes, makePlayedPlanet(), env.root, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Sensor visibility: <font>67%, light</font></li>"
                     "<li>Colonist Tax: <font>5% (1 mc)</font>"
                     "<ul><li><font>They LOVE you. (+5)</font></li></ul></li>"
                     "<li>Native Tax: <font>7% (42 mc)</font>"
                     "<ul><li><font>They like your leadership. (+1)</font></li></ul></li>"
                     "</ul>");
}

/** Test describePlanetBuildingEffects(), empty (unknown) planet. */
void
TestGameMapPlanetInfo::testDescribePlanetBuildingEffectsEmpty()
{
    Environment env;
    describePlanetBuildingEffects(env.nodes, Planet(77), env.root, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "</ul>");
}

/** Test describePlanetDefenseEffects. */
void
TestGameMapPlanetInfo::testDescribePlanetDefenseEffects()
{
    NullTranslator tx;

    game::spec::ShipList shipList;
    for (int i = 1; i <= 10; ++i) {
        shipList.beams().create(i)->setName(Format("Beam %d", i));
        shipList.launchers().create(i)->setName(Format("Beam %d", i));
    }

    Planet p = makePlayedPlanet();

    // Initial query; planet has 5 defense.
    {
        game::test::Root root(HostVersion(HostVersion::PHost, MKVERSION(3, 2, 0)));
        game::map::DefenseEffectInfos_t result;
        describePlanetDefenseEffects(result,
                                     p,
                                     root,
                                     shipList,
                                     game::UnitScoreDefinitionList(),
                                     tx);

        TS_ASSERT_EQUALS(toString(result),
                         "1 beam (+2)\n"
                         "  Beam 2 (+8)\n"
                         "2 fighters (+2)\n"
                         "2 fighter bays (+2)\n"
                         "3% shield loss from enemy fighter (+1)\n"
                         "3% damage from enemy fighter (+1)\n");
    }

    // Try again with 7 defense, does value adapt?
    {
        game::test::Root root(HostVersion(HostVersion::PHost, MKVERSION(3, 2, 0)));
        p.setNumBuildings(game::DefenseBuilding, p.getNumBuildings(game::DefenseBuilding).orElse(0) + 2);

        game::map::DefenseEffectInfos_t result;
        describePlanetDefenseEffects(result,
                                     p,
                                     root,
                                     shipList,
                                     game::UnitScoreDefinitionList(),
                                     tx);

        TS_ASSERT_EQUALS(toString(result),
                         "2 beams (+12)\n"
                         "  Beam 2 (+6)\n"
                         "3 fighters (+6)\n"
                         "3 fighter bays (+6)\n"
                         "2% shield loss from enemy fighter (+213) (unachievable)\n"
                         "2% damage from enemy fighter (+213) (unachievable)\n");
    }
}

/** Test packGroundDefenseInfo(). */
void
TestGameMapPlanetInfo::testPackGroundDefenseInfo()
{
    // Create a root with some players
    game::test::Root root(HostVersion(HostVersion::PHost, MKVERSION(3, 2, 0))) ;
    root.playerList().create(1)->setName(game::Player::LongName, "Fed");
    root.playerList().create(2)->setName(game::Player::LongName, "Lizard");
    root.playerList().create(3)->setName(game::Player::LongName, "Romulan");
    root.playerList().create(4)->setName(game::Player::LongName, "Klingon");
    root.playerList().create(5)->setName(game::Player::LongName, "Orion");
    root.playerList().create(6)->setName(game::Player::LongName, "Borg");

    game::map::GroundDefenseInfo info = game::map::packGroundDefenseInfo(makePlayedPlanet(), root);

    TS_ASSERT_EQUALS(info.defender, PLAYER);
    TS_ASSERT_EQUALS(info.isPlayable, true);

    TS_ASSERT_EQUALS(info.name.get(1), "Fed");
    TS_ASSERT_EQUALS(info.name.get(6), "Borg");
    TS_ASSERT_EQUALS(info.name.get(7), "");

    static_assert(PLAYER == 3, "PLAYER");
    TS_ASSERT_EQUALS(info.strength.get(1), 125);
    TS_ASSERT_EQUALS(info.strength.get(2), 5);
    TS_ASSERT_EQUALS(info.strength.get(3), 100);
    TS_ASSERT_EQUALS(info.strength.get(4), 9);
    TS_ASSERT_EQUALS(info.strength.get(5), 125);
    TS_ASSERT_EQUALS(info.strength.get(6), 125);
}

