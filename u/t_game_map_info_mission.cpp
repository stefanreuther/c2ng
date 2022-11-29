/**
  *  \file u/t_game_map_info_mission.cpp
  *  \brief Test for game::map::info::Mission
  */

#include "game/map/info/mission.hpp"

#include "t_game_map_info.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/chunnelmission.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/shiplist.hpp"

using game::Element;
using game::map::Ship;

namespace {
    template<typename T>
    String_t toString(const T& n)
    {
        afl::io::InternalSink sink;
        afl::io::xml::Writer(sink).visit(n);
        return afl::string::fromBytes(sink.getContent());
    }

    class Environment {
     public:
        // ShipPredictor:
        game::map::Universe univ;
        game::UnitScoreDefinitionList scoreDefinitions;
        game::spec::ShipList shipList;
        game::map::Configuration mapConfig;
        game::config::HostConfiguration config;
        game::HostVersion hostVersion;
        game::test::RegistrationKey key;

        // renderShipPredictorUsedProperties:
        game::PlayerList playerList;

        Environment()
            : univ(), scoreDefinitions(), shipList(), mapConfig(), config(),
              hostVersion(game::HostVersion::PHost, MKVERSION(3,0,0)),
              key(game::RegistrationKey::Unknown, 6),
              playerList()
            {
                game::test::initStandardBeams(shipList);
                game::test::initStandardTorpedoes(shipList);
                game::test::addTranswarp(shipList);
                game::test::addOutrider(shipList);
                game::test::addGorbie(shipList);
                game::test::addAnnihilation(shipList);
            }
    };

    Ship& addShip(Environment& env, int shipId, int hullNr)
    {
        // Add a ship
        // - required properties
        Ship& s = *env.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(hullNr);
        s.setEngineType(game::test::TRANSWARP_ENGINE_ID);
        s.setPosition(game::map::Point(1000, 1000));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);
        return s;
    }
}

/** Test renderChunnelFailureReasons(). */
void
TestGameMapInfoMission::testRenderChunnelFailureReasons()
{
    afl::string::NullTranslator tx;
    afl::io::xml::TagNode node("ul");
    game::map::info::renderChunnelFailureReasons(node, game::map::ChunnelMission::chf_MateFuel, tx);

    TS_ASSERT_EQUALS(toString(node), "<ul><li>Mate needs fuel</li></ul>");
}

/** Test renderShipPredictorUsedProperties(), friendly code and mission. */
void
TestGameMapInfoMission::testRenderShipPredictorUsedProperties()
{
    // Prepare
    Environment env;
    Ship& sh = addShip(env, 99, game::test::ANNIHILATION_HULL_ID);
    sh.setNumLaunchers(10);
    sh.setTorpedoType(10);
    sh.setCargo(Element::fromTorpedoType(10), 100);
    sh.setMission(game::spec::Mission::msn_LayMines, 0, 0);
    sh.setFriendlyCode(String_t("mdh"));

    // Predict
    game::map::ShipPredictor pred(env.univ, 99, env.scoreDefinitions, env.shipList,
                                  env.mapConfig, env.config, env.hostVersion, env.key);
    pred.computeTurn();
    TS_ASSERT_EQUALS(pred.getNumTurns(), 1);
    TS_ASSERT(pred.getUsedProperties().contains(game::map::ShipPredictor::UsedMission));
    TS_ASSERT(pred.getUsedProperties().contains(game::map::ShipPredictor::UsedFCode));

    // Verify formatting
    afl::string::NullTranslator tx;
    {
        afl::io::xml::TagNode node("ul");
        game::map::info::renderShipPredictorUsedProperties(node, pred, "", env.playerList, tx);
        TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Ship mission</li><li>Ship friendly code</li></ul>");
    }

    // Verify formatting with explicitly provided mission name
    {
        afl::io::xml::TagNode node("ul");
        game::map::info::renderShipPredictorUsedProperties(node, pred, "lay it", env.playerList, tx);
        TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Ship mission<br/><font color=\"dim\">lay it</font></li><li>Ship friendly code</li></ul>");
    }

    // Verify formatting with data provided in spec
    {
        env.shipList.friendlyCodes().addCode(game::spec::FriendlyCode("mdh", "s,lay half", tx));
        env.shipList.missions().addMission(game::spec::Mission(game::spec::Mission::msn_LayMines, ",Lay Mines"));
        afl::io::xml::TagNode node("ul");
        game::map::info::renderShipPredictorUsedProperties(node, pred, "", env.playerList, tx);
        TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Ship mission<br/><font color=\"dim\">Lay Mines</font></li><li>Ship friendly code<br/><font color=\"dim\"><b>mdh</b>: lay half</font></li></ul>");
    }

    // Verify formatting with explicitly provided mission name and data in spec
    {
        afl::io::xml::TagNode node("ul");
        game::map::info::renderShipPredictorUsedProperties(node, pred, "lay it", env.playerList, tx);
        TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Ship mission<br/><font color=\"dim\">lay it</font></li><li>Ship friendly code<br/><font color=\"dim\"><b>mdh</b>: lay half</font></li></ul>");
    }
}

/** Test renderShipPredictorUsedProperties(), supply repair and damage limit. */
void
TestGameMapInfoMission::testRenderShipPredictorUsedProperties2()
{
    // Prepare
    Environment env;
    Ship& sh = addShip(env, 99, game::test::ANNIHILATION_HULL_ID);
    sh.setDamage(80);
    sh.setWarpFactor(9);
    sh.setCargo(Element::Supplies, 100);
    sh.setWaypoint(game::map::Point(2000, 2000));

    // Predict
    game::map::ShipPredictor pred(env.univ, 99, env.scoreDefinitions, env.shipList,
                                  env.mapConfig, env.config, env.hostVersion, env.key);
    pred.computeTurn();
    TS_ASSERT_EQUALS(pred.getNumTurns(), 1);
    TS_ASSERT(pred.getUsedProperties().contains(game::map::ShipPredictor::UsedRepair));
    TS_ASSERT(pred.getUsedProperties().contains(game::map::ShipPredictor::UsedDamageLimit));

    // Verify formatting
    afl::string::NullTranslator tx;
    afl::io::xml::TagNode node("ul");
    game::map::info::renderShipPredictorUsedProperties(node, pred, "", env.playerList, tx);
    TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Supply repair</li><li>Damage speed limit</li></ul>");
}

/** Test renderShipPredictorUsedProperties(), towee. */
void
TestGameMapInfoMission::testRenderShipPredictorUsedProperties3()
{
    // Prepare
    Environment env;
    Ship& sh = addShip(env, 99, game::test::ANNIHILATION_HULL_ID);
    sh.setWarpFactor(9);
    sh.setCargo(Element::Supplies, 100);
    sh.setWaypoint(game::map::Point(2000, 2000));
    sh.setMission(game::spec::Mission::msn_Tow, 0, 88);
    sh.setName(String_t("This Ship"));

    Ship& sh2 = addShip(env, 88, game::test::ANNIHILATION_HULL_ID);
    sh2.setName(String_t("Other Ship"));

    // Predict
    game::map::ShipPredictor towPred(env.univ, 88, env.scoreDefinitions, env.shipList, env.mapConfig, env.config, env.hostVersion, env.key);
    game::map::ShipPredictor pred(env.univ, 99, towPred, env.scoreDefinitions, env.shipList, env.mapConfig, env.config, env.hostVersion, env.key);
    pred.computeTurn();
    TS_ASSERT_EQUALS(pred.getNumTurns(), 1);
    TS_ASSERT(pred.getUsedProperties().contains(game::map::ShipPredictor::UsedTowee));

    // Verify formatting
    afl::string::NullTranslator tx;
    afl::io::xml::TagNode node("ul");
    game::map::info::renderShipPredictorUsedProperties(node, pred, "", env.playerList, tx);
    TS_ASSERT_EQUALS(toString(node), "<ul><li>Movement (1 turn)</li><li>Towed ship\'s prediction<br/><font color=\"dim\">Other Ship</font></li></ul>");
}

