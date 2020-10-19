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
#include "game/test/simpleturn.hpp"

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

    /** Make a visited unowned planet. */
    Planet makeUnownedPlanet()
    {
        // Planet New Georgia (#459), Manos-3 turn 5
        game::map::PlanetData pd;
        pd.owner = 0;
        pd.densityNeutronium = 70;
        pd.densityTritanium = 42;
        pd.densityDuranium = 74;
        pd.densityMolybdenum = 83;
        pd.groundNeutronium = 4748;
        pd.groundTritanium = 349;
        pd.groundDuranium = 408;
        pd.groundMolybdenum = 130;
        pd.minedNeutronium = 84;
        pd.minedTritanium = 9;
        pd.minedDuranium = 34;
        pd.minedMolybdenum = 12;
        pd.nativeRace = 3;
        pd.nativeClans = 46336;
        pd.nativeGovernment = 2;
        pd.temperature = 4;
        pd.money = 0;
        pd.supplies = 0;
        pd.friendlyCode = "358";

        Planet pl(459);
        pl.setPosition(game::map::Point(1000, 1000));
        pl.addCurrentPlanetData(pd, game::PlayerSet_t::allUpTo(11));
        pl.setPlayability(game::map::Object::Playable);

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

    /** Make a history planet. */
    Planet makeHistoryPlanet()
    {
        Planet p(77);

        // Colonist scan
        gp::MessageInformation cinfo(gp::MessageInformation::Planet, p.getId(), TURN - 5);
        cinfo.addValue(gp::mi_Owner, PLAYER);
        cinfo.addValue(gp::ms_FriendlyCode, "xyz");
        cinfo.addValue(gp::mi_PlanetMines, 10);
        cinfo.addValue(gp::mi_PlanetFactories, 20);
        cinfo.addValue(gp::mi_PlanetDefense, 30);
        cinfo.addValue(gp::mi_PlanetColonists, 100);
        cinfo.addValue(gp::mi_PlanetSupplies, 70);
        cinfo.addValue(gp::mi_PlanetCash, 200);
        p.addMessageInformation(cinfo);

        // Native scan
        gp::MessageInformation ninfo(gp::MessageInformation::Planet, p.getId(), TURN - 1);
        ninfo.addValue(gp::mi_PlanetNativeHappiness, 96);
        ninfo.addValue(gp::mi_PlanetNativeGov, 6);
        ninfo.addValue(gp::mi_PlanetNatives, 5000);
        ninfo.addValue(gp::mi_PlanetNativeRace, game::BovinoidNatives);
        ninfo.addValue(gp::mi_PlanetTemperature, 50);
        p.addMessageInformation(ninfo);

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

/** Test describePlanetClimate().
    Test that format parameters are honored. */
void
TestGameMapPlanetInfo::testDescribePlanetClimateFormat()
{
    Environment env;
    env.root.userConfiguration()[game::config::UserConfiguration::Display_ThousandsSep].set(0);
    env.root.userConfiguration()[game::config::UserConfiguration::Display_Clans].set(1);
    describePlanetClimate(env.nodes, makePlayedPlanet(), TURN, env.root, PLAYER, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Climate type: warm</li>"
                     "<li>Average temperature: 50\xC2\xB0""F</li>"
                     "<li>Supports 100000c Player 3s</li>"
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

/** Test describePlanetClimate(), THost climate deaths. */
void
TestGameMapPlanetInfo::testDescribePlanetClimateDeath()
{
    Environment env;
    env.root.hostVersion() = HostVersion(HostVersion::Host, MKVERSION(3, 22, 40));
    env.root.hostConfiguration()[HostConfiguration::ClimateDeathRate].set(25);

    Planet p = makePlayedPlanet();
    p.setCargo(Element::Colonists, 200);
    p.setTemperature(10);

    describePlanetClimate(env.nodes, p, TURN, env.root, PLAYER, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Climate type: arctic</li>"
                     "<li>Average temperature: 10\xC2\xB0""F</li>"
                     "<li>Supports 2,300 Player 3s"
                     "<ul><li>won\'t die if less than 9,200</li></ul></li>"
                     "</ul>");
}

/** Test describePlanetClimate(), scanned planet.
    This is mostly a regression test. */
void
TestGameMapPlanetInfo::testDescribePlanetClimateUnowned()
{
    Environment env;
    env.root.hostVersion() = HostVersion(HostVersion::Host, MKVERSION(3, 22, 40));

    Planet p = makeUnownedPlanet();

    describePlanetClimate(env.nodes, p, TURN, env.root, 6, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Climate type: arctic</li>"
                     "<li>Average temperature: 4\xC2\xB0""F</li>"
                     "<li>Supports 1,100 Player 6s"
                     "<ul><li>won\'t die if less than 11,000</li></ul></li>"
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

/** Test describePlanetNatives(), aged information. */
void
TestGameMapPlanetInfo::testDescribePlanetNativesAged()
{
    const int VIEWPOINT = 4;
    static_assert(PLAYER != VIEWPOINT, "PLAYER");

    Environment env;
    describePlanetNatives(env.nodes, makeHistoryPlanet(), TURN, env.root, PLAYER, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Native race: Bovinoid"
                     "<ul><li>Pay additional supplies</li>"
                     "<li>50 kt supplies per turn</li></ul></li>"
                     "<li>Population: 500,000</li>"
                     "<li>Government: Monarchy (120%)"
                     "<ul><li><font>previous turn</font></li></ul></li>"    // <- timestamp
                     "<li>Base Tax Rate: 9% (54 mc)</li>"
                     "<li>Max Tax Rate: 44% (264 mc)</li>""</ul>");
}

/** Test describePlanetNatives(), unowned visited planet. */
void
TestGameMapPlanetInfo::testDescribePlanetNativesUnowned()
{
    Environment env;
    env.root.hostVersion() = HostVersion(HostVersion::Host, MKVERSION(3, 22, 40));

    describePlanetNatives(env.nodes, makeUnownedPlanet(), TURN, env.root, 7, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Native race: Reptilian"
                     "<ul><li>Double mining rates</li></ul></li>"
                     "<li>Population: 4,633,600</li>"
                     "<li>Government: Pre-Tribal (40%)</li>"
                     "<li>Base Tax Rate: 5% (93 mc)</li>"
                     "<li>Max Tax Rate: 40% (741 mc)</li>"
                     "</ul>");
}

/** Test describePlanetNatives(), unowned visited planet, visitor is borg. */
void
TestGameMapPlanetInfo::testDescribePlanetNativesUnownedBorg()
{
    Environment env;
    env.root.hostVersion() = HostVersion(HostVersion::Host, MKVERSION(3, 22, 40));

    describePlanetNatives(env.nodes, makeUnownedPlanet(), TURN, env.root, 6, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Native race: Reptilian"
                     "<ul><li>Double mining rates</li></ul></li>"
                     "<li>Population: 4,633,600</li>"
                     "<li>Government: Pre-Tribal (40%)</li>"
                     "<li>Base Tax Rate: 5% (93 mc)</li>"
                     "<li>Max Tax Rate: 20% (371 mc)</li>"              // <- limit applied
                     "<li>Assimilated in 13 turns by 10 clans</li>"     // <- borg only
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

/** Test describePlanetColony() with UnloadInfo. */
void
TestGameMapPlanetInfo::testDescribePlanetColonyGroundAttack()
{
    // Use lizards as attackers for some nontrivial attack factor
    const int VIEWPOINT = 2;
    static_assert(PLAYER != VIEWPOINT, "PLAYER");

    game::map::UnloadInfo u;
    u.hostileUnload = 7;
    u.hostileUnloadIsAssumed = true;

    Environment env;
    describePlanetColony(env.nodes, makePlayedPlanet(), TURN, env.root, VIEWPOINT, u, env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Colonists: Player 3</li>"
                     "<li>Population: 10,000</li>"
                     "<li>20 factories, 10 mines, 5 DPs</li>"
                     "<li>200 mc, 70 supplies</li>"
                     "<li>Friendly code: xyz</li>"
                     "<li>Assuming, we\'d beam down 7 clans."
                     "<ul><li><font>Chance to win ground combat: 38%</font><br></br>"
                     "<font>Up to 3 of our clans survive.</font><br></br>"
                     "<font>Up to 45 of their clans survive.</font></li></ul></li>"
                     "</ul>");
}

/** Test describePlanetColony(), aged information. */
void
TestGameMapPlanetInfo::testDescribePlanetColonyAged()
{
    const int VIEWPOINT = 4;
    static_assert(PLAYER != VIEWPOINT, "PLAYER");

    Environment env;
    describePlanetColony(env.nodes, makeHistoryPlanet(), TURN, env.root, PLAYER, game::map::UnloadInfo(), env.tx);
    TS_ASSERT_EQUALS(toString(env.nodes),
                     "<ul>"
                     "<li>Colonists: Player 3</li>"
                     "<li>Population: 10,000</li>"
                     "<li>20 factories, 10 mines, 30 DPs"
                     "<ul><li><font>5 turns ago</font></li></ul></li>"    // <- timestamp
                     "<li>200 mc, 70 supplies"
                     "<ul><li><font>5 turns ago</font></li></ul></li>"    // <- timestamp
                     "<li>Last known friendly code: xyz</li>"
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
        shipList.launchers().create(i)->setName(Format("Torp %d", i));
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

    // Retry with PlanetsHaveTubes
    {
        game::test::Root root(HostVersion(HostVersion::PHost, MKVERSION(3, 2, 0)));
        root.hostConfiguration()[HostConfiguration::PlanetsHaveTubes].set(1);

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
                         "1 torpedo launcher (+4)\n"
                         "  Torp 2 (+8)\n"
                         "3 torpedoes (+4)\n"
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

/** Test prepareUnloadInfo(). */
void
TestGameMapPlanetInfo::testPrepareUnloadInfo()
{
    using game::map::Ship;
    const int PLANET_ID = 77;
    const int VIEWPOINT = 4;

    game::config::HostConfiguration config;

    game::test::SimpleTurn t;
    t.setPosition(game::map::Point(1000, 1000));
    t.addPlanet(PLANET_ID, 3, Planet::ReadOnly);

    // Affected ships
    int shipId = 1;
    {
        Ship& s = t.addShip(shipId++, VIEWPOINT, Ship::Playable);
        s.setTransporterTargetId(Ship::UnloadTransporter, PLANET_ID);
        s.setTransporterCargo(Ship::UnloadTransporter, Element::Colonists, 5);
    }
    {
        Ship& s = t.addShip(shipId++, VIEWPOINT, Ship::Playable);
        s.setTransporterTargetId(Ship::UnloadTransporter, PLANET_ID);
        s.setTransporterCargo(Ship::UnloadTransporter, Element::Colonists, 7);
    }

    // Not affected (foreign)
    {
        Ship& s = t.addShip(shipId++, VIEWPOINT+1, Ship::Playable);
        s.setTransporterTargetId(Ship::UnloadTransporter, PLANET_ID);
        s.setTransporterCargo(Ship::UnloadTransporter, Element::Colonists, 9);
    }

    // Not affected (elsewhere)
    t.setPosition(game::map::Point(1000, 2000));
    {
        Ship& s = t.addShip(shipId++, VIEWPOINT, Ship::Playable);
        s.setTransporterTargetId(Ship::UnloadTransporter, PLANET_ID);
        s.setTransporterCargo(Ship::UnloadTransporter, Element::Colonists, 11);
    }

    game::map::UnloadInfo info = game::map::prepareUnloadInfo(t.universe(), PLANET_ID, VIEWPOINT, game::UnitScoreDefinitionList(), t.shipList(), config);

    TS_ASSERT_EQUALS(info.hostileUnload, 12);
    TS_ASSERT_EQUALS(info.friendlyUnload, 0);
    TS_ASSERT_EQUALS(info.hostileUnloadIsAssault, false);
    TS_ASSERT_EQUALS(info.hostileUnloadIsAssumed, false);
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

