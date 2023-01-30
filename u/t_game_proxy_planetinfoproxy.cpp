/**
  *  \file u/t_game_proxy_planetinfoproxy.cpp
  *  \brief Test for game::proxy::PlanetInfoProxy
  */

#include "game/proxy/planetinfoproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/visitor.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/root.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace gp = game::parser;
using game::test::Counter;

namespace {
    void makeScannedPlanet(game::map::Planet& pl)
    {
        pl.setPosition(game::map::Point(1000, 1000));

        gp::MessageInformation info(gp::MessageInformation::Planet, pl.getId(), 33);
        info.addValue(gp::mi_Owner, 4);
        info.addValue(gp::mi_PlanetColonists, 100);
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
        info.addValue(gp::mi_PlanetTemperature, 35);
        info.addValue(gp::mi_PlanetMines, 5);
        info.addValue(gp::mi_PlanetFactories, 10);
        info.addValue(gp::mi_PlanetDefense, 15);
        pl.addMessageInformation(info);
    }

    /** Quick and dirty stringification of a node list.
        FIXME: dupe from t_game_map_planetinfo */
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
}

void
TestGameProxyPlanetInfoProxy::testIt()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);

    const int ID = 77;
    game::test::SessionThread s;
    s.session().setShipList(new game::spec::ShipList());
    s.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))).asPtr());
    s.session().setGame(new game::Game());
    makeScannedPlanet(*s.session().getGame()->currentTurn().universe().planets().create(ID));

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::PlanetInfoProxy testee(s.gameSender(), disp);

    Counter c;
    testee.sig_change.add(&c, &Counter::increment);

    // Select planet
    testee.setPlanet(ID);
    while (c.get() == 0) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify
    // - getMineralInfo // see testPackPlanetMineralInfo
    {
        const game::map::PlanetMineralInfo& info = testee.getMineralInfo(game::proxy::PlanetInfoProxy::Tritanium);
        TS_ASSERT_EQUALS(info.groundAmount.orElse(-1), 300);
        TS_ASSERT_EQUALS(info.groundSummary, "rare");
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 1);
    }

    // - getClimateInfo
    {
        String_t s = toString(testee.getClimateInfo());
        TS_ASSERT_EQUALS(s, "<ul><li>Climate type: cool</li>"
                         "<li>Average temperature: 35\xC2\xB0""F</li>"
                         "<li>Supports 8,910,000 Player 4s</li>"
                         "<li>Supports 8,910,000 unowneds</li>"
                         "</ul>");
    }

    // - getColonyInfo
    {
        String_t s = toString(testee.getColonyInfo());
        TS_ASSERT_EQUALS(s, "<ul><li>Colonists: Player 4</li>"
                         "<li>Population: 10,000</li>"
                         "<li>10 factories, 5 mines, 15 DPs<ul>"
                         "<li><font>turn 33</font></li></ul></li></ul>");
    }

    // - getNativeInfo
    {
        String_t s = toString(testee.getNativeInfo());
        TS_ASSERT_EQUALS(s, "<ul><li>No information on natives available.</li></ul>");
    }

    // - getBuildingEffectsInfo
    {
        String_t s = toString(testee.getBuildingEffectsInfo());
        TS_ASSERT_EQUALS(s, "<ul><li>Sensor visibility: <font>0%, minimal</font></li></ul>");
    }

    // - getDefenseEffectsInfo
    {
        const game::map::DefenseEffectInfos_t& info = testee.getDefenseEffectsInfo();
        TS_ASSERT(!info.empty());
        TS_ASSERT_EQUALS(info[0].name, "2 beams");
        TS_ASSERT_EQUALS(info[0].nextAt, 4);
        TS_ASSERT_EQUALS(info[0].isAchievable, true);
        TS_ASSERT_EQUALS(info[0].isDetail, false);
    }

    // - getUnloadInfo
    {
        const game::map::UnloadInfo& info = testee.getUnloadInfo();
        TS_ASSERT_EQUALS(info.hostileUnload, 0);
        TS_ASSERT_EQUALS(info.friendlyUnload, 0);
        TS_ASSERT_EQUALS(info.hostileUnloadIsAssault, false);
        TS_ASSERT_EQUALS(info.hostileUnloadIsAssumed, false);
    }

    // - getGroundDefenseInfo
    {
        const game::map::GroundDefenseInfo& info = testee.getGroundDefenseInfo();
        TS_ASSERT_EQUALS(info.defender, 4);
    }
}

void
TestGameProxyPlanetInfoProxy::testOverride()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);

    const int ID = 77;
    game::test::SessionThread s;
    s.session().setShipList(new game::spec::ShipList());
    s.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))).asPtr());
    s.session().setGame(new game::Game());
    makeScannedPlanet(*s.session().getGame()->currentTurn().universe().planets().create(ID));

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::PlanetInfoProxy testee(s.gameSender(), disp);

    Counter c;
    testee.sig_change.add(&c, &Counter::increment);

    // Set building override; setting this one before setting the planet will not yet produce a callback
    testee.setBuildingOverride(game::MineBuilding, 100);

    // Select planet
    testee.setPlanet(ID);
    while (c.get() < 1) {
        TS_ASSERT(disp.wait(1000));
    }

    // Set attack override. This must be set after choosing the planet and will create a callback.
    testee.setAttackingClansOverride(1000);
    while (c.get() < 2) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify
    // - getMineralInfo // see testPackPlanetMineralInfo
    {
        const game::map::PlanetMineralInfo& info = testee.getMineralInfo(game::proxy::PlanetInfoProxy::Tritanium);
        TS_ASSERT_EQUALS(info.groundAmount.orElse(-1), 300);
        TS_ASSERT_EQUALS(info.groundSummary, "rare");
        TS_ASSERT_EQUALS(info.miningPerTurn.orElse(-1), 30);  // modified by override
    }

    // - getUnloadInfo
    {
        const game::map::UnloadInfo& info = testee.getUnloadInfo();
        TS_ASSERT_EQUALS(info.hostileUnload, 1000);
        TS_ASSERT_EQUALS(info.friendlyUnload, 0);
        TS_ASSERT_EQUALS(info.hostileUnloadIsAssault, false);
        TS_ASSERT_EQUALS(info.hostileUnloadIsAssumed, true);
    }
}

