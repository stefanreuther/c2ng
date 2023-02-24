/**
  *  \file u/t_game_map_renderer.cpp
  *  \brief Test for game::map::Renderer
  */

#include <set>
#include "game/map/renderer.hpp"

#include "t_game_map.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/configuration.hpp"
#include "game/map/drawing.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/minefield.hpp"
#include "game/map/point.hpp"
#include "game/map/rendererlistener.hpp"
#include "game/map/ufo.hpp"
#include "game/map/viewport.hpp"
#include "game/parser/messageinformation.hpp"

/*
 *  Test driver
 */

using afl::string::Format;
using game::HostVersion;
using game::PlayerSet_t;
using game::config::HostConfiguration;
using game::map::Configuration;
using game::map::Drawing;
using game::map::Explosion;
using game::map::IonStorm;
using game::map::Minefield;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Ufo;
using game::map::Viewport;
using game::parser::MessageInformation;

namespace {
    const int TURN_NUMBER = 20;

    class RendererListenerMock : public game::map::RendererListener {
     public:
        virtual void drawGridLine(Point a, Point b)
            { addCommand("drawGridLine", Format("%s,%s", a.toString(), b.toString())); }
        virtual void drawBorderLine(Point a, Point b)
            { addCommand("drawBorderLine", Format("%s,%s", a.toString(), b.toString())); }
        virtual void drawBorderCircle(Point c, int radius)
            { addCommand("drawBorderCircle", Format("%s,%d", c.toString(), radius)); }
        virtual void drawSelection(Point p)
            { addCommand("drawSelection", p.toString()); }
        virtual void drawMessageMarker(Point p)
            { addCommand("drawMessageMarker", p.toString()); }
        virtual void drawPlanet(Point p, int id, int flags, String_t label)
            { addCommand("drawPlanet", Format("%s,%d,%s,%s", p.toString(), id, formatPlanetFlags(flags), label)); }
        virtual void drawShip(Point p, int id, Relation_t rel, int flags, String_t label)
            { addCommand("drawShip", Format("%s,%d,%s,%s,%s") << p.toString() << id << formatRelation(rel) << formatShipFlags(flags) << label); }
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled)
            { addCommand("drawMinefield", Format("%s,%d,%d,%s,%s,%s") << p.toString() << id << r << (isWeb?"web":"normal") << formatRelation(rel) << (filled?"fill":"empty")); }
        virtual void drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled)
            { addCommand("drawUfo", Format("%s,%d,%d,%d,%d,%d,%s") << p.toString() << id << r << colorCode << speed << heading << (filled?"fill":"empty")); }
        virtual void drawUfoConnection(Point a, Point b, int colorCode)
            { addCommand("drawUfoConnection", Format("%s,%s,%d", a.toString(), b.toString(), colorCode)); }
        virtual void drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled)
            { addCommand("drawIonStorm", Format("%s,%d,%d,%d,%d,%s") << p.toString() << r << voltage << speed << heading << (filled?"fill":"empty")); }
        virtual void drawUserCircle(Point pt, int r, int color)
            { addCommand("drawUserCircle", Format("%s,%d,%d", pt.toString(), r, color)); }
        virtual void drawUserLine(Point a, Point b, int color)
            { addCommand("drawUserLine", Format("%s,%s,%d", a.toString(), b.toString(), color)); }
        virtual void drawUserRectangle(Point a, Point b, int color)
            { addCommand("drawUserRectangle", Format("%s,%s,%d", a.toString(), b.toString(), color)); }
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label)
            { addCommand("drawUserMarker", Format("%s,%d,%d,%s", pt.toString(), shape, color, label)); }
        virtual void drawExplosion(Point p)
            { addCommand("drawExplosion", p.toString()); }
        virtual void drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age)
            { addCommand("drawShipTrail", Format("%s,%s,%s,%s,%d") << a.toString() << b.toString() << formatRelation(rel) << formatTrailFlags(flags) << age); }
        virtual void drawShipWaypoint(Point a, Point b, Relation_t rel)
            { addCommand("drawShipWaypoint", Format("%s,%s,%s", a.toString(), b.toString(), formatRelation(rel))); }
        virtual void drawShipVector(Point a, Point b, Relation_t rel)
            { addCommand("drawShipVector", Format("%s,%s,%s", a.toString(), b.toString(), formatRelation(rel))); }
        virtual void drawWarpWellEdge(Point a, Edge e)
            { addCommand("drawWarpWellEdge", Format("%s,%s", a.toString(), formatEdge(e))); }

        bool hasCommand(const String_t& name) const;
        bool hasCommand(const String_t& name, const String_t& arg) const;

     private:
        std::set<String_t> m_commands;
        std::set<String_t> m_commandsWithArgs;

        void addCommand(const String_t& name, const String_t& arg);
        String_t formatPlanetFlags(int flags) const;
        String_t formatShipFlags(int flags) const;
        String_t formatTrailFlags(int flags) const;
        String_t formatEdge(Edge e) const;
        String_t formatRelation(Relation_t rel) const;
    };

    /*
     *  GameEnvironment
     *
     *  Aggregates all objects for a game situation.
     */
    struct GameEnvironment {
        game::map::Universe univ;
        game::TeamSettings teams;
        game::UnitScoreDefinitionList shipScoreDefinitions;
        game::spec::ShipList shipList;
        Configuration mapConfig;
        HostConfiguration hostConfiguration;
        HostVersion host;

        GameEnvironment()
            : univ(),
              teams(),
              shipScoreDefinitions(),
              shipList(),
              mapConfig(),
              hostConfiguration(),
              host(HostVersion::PHost, MKVERSION(3,0,0))
            { }
    };

    Ship& addShipXY(GameEnvironment& env, int id, Point pt, int owner, int scanner)
    {
        Ship* sh = env.univ.ships().create(id);
        TS_ASSERT(sh != 0);
        sh->addShipXYData(pt, owner, /* mass */ 400, PlayerSet_t(scanner));
        sh->internalCheck(PlayerSet_t(scanner), TURN_NUMBER);
        return *sh;
    }

    Planet& addPlanetXY(GameEnvironment& env, int id, Point pt)
    {
        Planet* p = env.univ.planets().create(id);
        TS_ASSERT(p != 0);
        p->setPosition(pt);
        return *p;
    }

    Planet& finishPlanet(GameEnvironment& env, Planet& p, int scanner)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(env.mapConfig, PlayerSet_t(scanner), TURN_NUMBER, tx, log);
        return p;
    }

    Planet& addScannedPlanet(GameEnvironment& env, int id, Point pt, int owner)
    {
        Planet& p = addPlanetXY(env, id, pt);
        p.setOwner(owner);
        return finishPlanet(env, p, 12);
    }

    Planet& addBasePlanet(GameEnvironment& env, int id, Point pt, int owner)
    {
        Planet& p = addPlanetXY(env, id, pt);
        p.setOwner(owner);
        p.setBuildBaseFlag(1); // for foreign planets, means hasBase()
        finishPlanet(env, p, 12);
        TS_ASSERT(p.hasBase());
        return p;
    }

    Planet& addNativePlanet(GameEnvironment& env, int id, Point pt)
    {
        Planet& p = addPlanetXY(env, id, pt);
        p.setNativeRace(1);
        finishPlanet(env, p, 12);
        TS_ASSERT(p.hasAnyPlanetData());
        TS_ASSERT(p.getOwner().get() == 0);
        return p;
    }

    Planet& addUnscannedPlanet(GameEnvironment& env, int id, Point pt)
    {
        return finishPlanet(env, addPlanetXY(env, id, pt), 12);
    }

    /*
     *  LabelEnvironment
     *
     *  LabelExtra requires a Session.
     *  We give it one, but not connected with the other objects we provide.
     *  In particular, the change callbacks remain unconnected.
     *  We populate the labels manually and do not run the interpreter.
     */
    struct LabelEnvironment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::interface::LabelExtra& extra;

        LabelEnvironment()
            : tx(), fs(), session(tx, fs), extra(game::interface::LabelExtra::create(session))
            { }
    };

    /*
     *  RenderEnvironment
     *
     *  Aggregates all objects for rendering
     */
    struct RenderEnvironment {
        Viewport viewport;
        RendererListenerMock listener;

        // Environment without labels
        explicit RenderEnvironment(GameEnvironment& env)
            : viewport(env.univ, TURN_NUMBER, env.teams, 0, env.shipScoreDefinitions,
                       env.shipList, env.mapConfig, env.hostConfiguration, env.host),
              listener()
            { viewport.setRange(Point(900, 900), Point(3100, 3100)); }

        // Environment with labels
        RenderEnvironment(GameEnvironment& env, LabelEnvironment& lenv)
            : viewport(env.univ, TURN_NUMBER, env.teams, &lenv.extra, env.shipScoreDefinitions,
                       env.shipList, env.mapConfig, env.hostConfiguration, env.host),
              listener()
            { viewport.setRange(Point(900, 900), Point(3100, 3100)); }
    };

    void render(RenderEnvironment& renv)
    {
        game::map::Renderer(renv.viewport).render(renv.listener);
    }
}

bool
RendererListenerMock::hasCommand(const String_t& name) const
{
    return m_commands.find(name) != m_commands.end();
}

bool
RendererListenerMock::hasCommand(const String_t& name, const String_t& arg) const
{
    return m_commandsWithArgs.find(name + ":" + arg) != m_commandsWithArgs.end();
}

void
RendererListenerMock::addCommand(const String_t& name, const String_t& arg)
{
    m_commands.insert(name);
    m_commandsWithArgs.insert(name + ":" + arg);
}

String_t
RendererListenerMock::formatPlanetFlags(int flags) const
{
    String_t result;
    if ((flags & ripUnowned)            != 0) { result += "u"; }
    if ((flags & ripOwnPlanet)          != 0) { result += "o"; }
    if ((flags & ripAlliedPlanet)       != 0) { result += "a"; }
    if ((flags & ripEnemyPlanet)        != 0) { result += "e"; }
    if ((flags & ripHasBase)            != 0) { result += "b"; }
    if ((flags & ripOwnShips)           != 0) { result += "O"; }
    if ((flags & ripAlliedShips)        != 0) { result += "A"; }
    if ((flags & ripEnemyShips)         != 0) { result += "E"; }
    if ((flags & ripGuessedAlliedShips) != 0) { result += "g"; }
    if ((flags & ripGuessedEnemyShips)  != 0) { result += "G"; }
    if (result.empty()) {
        result += "0";
    }
    return result;
}

String_t
RendererListenerMock::formatShipFlags(int flags) const
{
    String_t result;
    if ((flags & risShowDot)     != 0) { result += "."; }
    if ((flags & risShowIcon)    != 0) { result += "i"; }
    if ((flags & risFleetLeader) != 0) { result += "f"; }
    if ((flags & risAtPlanet)    != 0) { result += "p"; }
    if (result.empty()) {
        result += "0";
    }
    return result;
}

String_t
RendererListenerMock::formatTrailFlags(int flags) const
{
    String_t result;
    if ((flags & TrailFromPosition) != 0) { result += "f"; }
    if ((flags & TrailToPosition)   != 0) { result += "t"; }
    if (result.empty()) {
        result += "0";
    }
    return result;
}

String_t
RendererListenerMock::formatEdge(Edge e) const
{
    switch (e) {
     case North: return "N";
     case East:  return "E";
     case South: return "S";
     case West:  return "W";
    }
    return "?";
}

String_t
RendererListenerMock::formatRelation(Relation_t rel) const
{
    switch (rel) {
     case game::TeamSettings::ThisPlayer:   return "me";
     case game::TeamSettings::AlliedPlayer: return "ally";
     case game::TeamSettings::EnemyPlayer:  return "enemy";
    }
    return "?";
}


/*
 *  Tests
 */

void
TestGameMapRenderer::testRectangularGrid()
{
    // Given an empty map with ShowGrid enabled...
    GameEnvironment env;
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowGrid, true);
    render(renv);

    // ...I expect a grid to be rendered (check specimen).
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1100,1000),(1100,3000)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1000,1500),(3000,1500)"));
}

void
TestGameMapRenderer::testCircularGrid()
{
    // Given an empty circular map with ShowGrid enabled...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(500, 500));
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowGrid, true);
    renv.viewport.setOption(Viewport::ShowOutsideGrid, false);
    render(renv);

    // ...I expect a grid to be rendered (check specimen).
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1500,2000),(2500,2000)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(2000,1500),(2000,2500)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1600,2300),(2400,2300)"));
}

void
TestGameMapRenderer::testCircularGridOutside()
{
    // Given an empty circular map with ShowGrid, ShowOutsideGrid enabled...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(500, 500));
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowGrid, true);
    renv.viewport.setOption(Viewport::ShowOutsideGrid, true);
    render(renv);

    // ...I expect the inside grid to be rendered (check specimen)...
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1500,2000),(2500,2000)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(2000,1500),(2000,2500)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(1600,2300),(2400,2300)"));

    // ...and an outside grid to be rendered (check specimen).
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(2400,2300),(2410,2297)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(2410,2297),(2419,2293)"));
    TS_ASSERT(renv.listener.hasCommand("drawGridLine", "(2419,2293),(2429,2289)"));
}

void
TestGameMapRenderer::testNoGrid()
{
    // Given a map with ShowGrid disabled...
    GameEnvironment env;
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowGrid, false);
    render(renv);

    // ...I expect no grid to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawGridLine"));
}

void
TestGameMapRenderer::testRectangularBorder()
{
    // Given an empty map with ShowBorders enabled...
    GameEnvironment env;
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowBorders, true);
    render(renv);

    // ...I expect border to be rendererd.
    TS_ASSERT(renv.listener.hasCommand("drawBorderLine", "(3000,1000),(3000,3000)"));
    TS_ASSERT(renv.listener.hasCommand("drawBorderLine", "(1000,1000),(3000,1000)"));
}

void
TestGameMapRenderer::testCircularBorder()
{
    // Given an empty circular map with ShowBorders enabled...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(500, 500));
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowBorders, true);
    render(renv);

    // ...I expect border to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawBorderCircle", "(2000,2000),500"));
}

void
TestGameMapRenderer::testNoBorder()
{
    // Given an empty map with ShowBorders disabled...
    GameEnvironment env;
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowBorders, false);
    render(renv);

    // ...I expect no border to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawBorderLine"));
}

void
TestGameMapRenderer::testMinefieldsNormal()
{
    // Given a map with a single minefield...
    GameEnvironment env;
    Minefield* mf = env.univ.minefields().create(99);
    mf->addReport(Point(1400, 2100), 7, Minefield::IsMine, Minefield::UnitsKnown, 400, TURN_NUMBER, Minefield::MinefieldScanned);
    mf->internalCheck(TURN_NUMBER, env.host, env.hostConfiguration);

    // ...and ShowMinefields enabled, FillMinefields/ShowMinefields disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMinefields, true);
    renv.viewport.setOption(Viewport::FillMinefields, false);
    renv.viewport.setOption(Viewport::ShowMineDecay, false);
    render(renv);

    // ...I expect the minefield to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawMinefield", "(1400,2100),99,20,normal,enemy,empty"));
}

void
TestGameMapRenderer::testMinefieldsFilled()
{
    // Given a map with a single minefield...
    GameEnvironment env;
    Minefield* mf = env.univ.minefields().create(99);
    mf->addReport(Point(1400, 2100), 7, Minefield::IsMine, Minefield::UnitsKnown, 400, TURN_NUMBER, Minefield::MinefieldScanned);
    mf->internalCheck(TURN_NUMBER, env.host, env.hostConfiguration);
    RenderEnvironment renv(env);

    // ...and ShowMinefields/FillMinefields enabled, ShowMinefields disabled...
    renv.viewport.setOption(Viewport::ShowMinefields, true);
    renv.viewport.setOption(Viewport::FillMinefields, true);
    renv.viewport.setOption(Viewport::ShowMineDecay, false);
    render(renv);

    // ...I expect the minefield to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawMinefield", "(1400,2100),99,20,normal,enemy,fill"));
}

void
TestGameMapRenderer::testMinefieldsOff()
{
    // Given a map with a single minefield...
    GameEnvironment env;
    Minefield* mf = env.univ.minefields().create(99);
    mf->addReport(Point(1400, 2100), 7, Minefield::IsMine, Minefield::UnitsKnown, 400, TURN_NUMBER, Minefield::MinefieldScanned);
    mf->internalCheck(TURN_NUMBER, env.host, env.hostConfiguration);

    // ...and ShowMinefields disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMinefields, false);
    render(renv);

    // ...I expect no minefield to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawMinefield"));
}

void
TestGameMapRenderer::testMinefieldsWrap()
{
    // Given a wrapped map with a single minefield...
    GameEnvironment env;
    Minefield* mf = env.univ.minefields().create(99);
    mf->addReport(Point(1900, 2100), 7, Minefield::IsMine, Minefield::UnitsKnown, 400, TURN_NUMBER, Minefield::MinefieldScanned);
    mf->internalCheck(TURN_NUMBER, env.host, env.hostConfiguration);
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and ShowMinefields enabled, FillMinefields/ShowMinefields disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMinefields, true);
    renv.viewport.setOption(Viewport::FillMinefields, false);
    renv.viewport.setOption(Viewport::ShowMineDecay, false);
    render(renv);

    // ...I expect the minefield to be rendered multiple times (check specimen).
    TS_ASSERT(renv.listener.hasCommand("drawMinefield", "(1900,2100),99,20,normal,enemy,empty"));
    TS_ASSERT(renv.listener.hasCommand("drawMinefield", "(900,1100),99,20,normal,enemy,empty"));
}

void
TestGameMapRenderer::testMinefieldsDecay()
{
    // Given a map with a single minefield, MineDecayRate=5...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::MineDecayRate].set(5);
    Minefield* mf = env.univ.minefields().create(99);
    mf->addReport(Point(1400, 2100), 7, Minefield::IsMine, Minefield::UnitsKnown, 400, TURN_NUMBER, Minefield::MinefieldScanned);
    mf->internalCheck(TURN_NUMBER, env.host, env.hostConfiguration);

    // ...and ShowMineDecay enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMinefields, true);
    renv.viewport.setOption(Viewport::FillMinefields, false);
    renv.viewport.setOption(Viewport::ShowMineDecay, true);
    render(renv);

    // ...I expect the minefield to be rendered with its size after decay.
    TS_ASSERT(renv.listener.hasCommand("drawMinefield", "(1400,2100),99,19,normal,enemy,empty"));
}

void
TestGameMapRenderer::testUfoNormal()
{
    // Given a map with a single Ufo...
    GameEnvironment env;
    Ufo* ufo = env.univ.ufos().addUfo(100, 50, /* color */ 3);
    ufo->setRadius(30);
    ufo->setPosition(Point(1300, 1500));
    ufo->postprocess(TURN_NUMBER, env.mapConfig);
    ufo->setWarpFactor(12);

    // ...and ShowUfos enabled, FillUfos disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowUfos, true);
    renv.viewport.setOption(Viewport::FillUfos, false);
    render(renv);

    // ...I expect the Ufo to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(1300,1500),1,30,3,12,-1,empty"));
}

void
TestGameMapRenderer::testUfoFill()
{
    // Given a map with a single Ufo...
    GameEnvironment env;
    Ufo* ufo = env.univ.ufos().addUfo(100, 50, /* color */ 3);
    ufo->setRadius(30);
    ufo->setPosition(Point(1300, 1500));
    ufo->postprocess(TURN_NUMBER, env.mapConfig);
    ufo->setHeading(320);

    // ...and ShowUfos/FillUfos enabled disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowUfos, true);
    renv.viewport.setOption(Viewport::FillUfos, true);
    render(renv);

    // ...I expect the Ufo to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(1300,1500),1,30,3,-1,320,fill"));
}

void
TestGameMapRenderer::testUfoOff()
{
    // Given a map with a single Ufo...
    GameEnvironment env;
    Ufo* ufo = env.univ.ufos().addUfo(100, 50, /* color */ 3);
    ufo->setRadius(30);
    ufo->setPosition(Point(1300, 1500));
    ufo->postprocess(TURN_NUMBER, env.mapConfig);

    // ...and ShowUfos disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowUfos, false);
    render(renv);

    // ...I expect no Ufo to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawUfo"));
}

void
TestGameMapRenderer::testUfoWrap()
{
    // Given a wrapped map with a single Ufo...
    GameEnvironment env;
    Ufo* ufo = env.univ.ufos().addUfo(100, 50, /* color */ 3);
    ufo->setRadius(30);
    ufo->setPosition(Point(1800, 1500));
    ufo->postprocess(TURN_NUMBER, env.mapConfig);
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and ShowUfos enabled, FillUfos disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowUfos, true);
    renv.viewport.setOption(Viewport::FillUfos, false);
    render(renv);

    // ...I expect the Ufo to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(1800,1500),1,30,3,-1,-1,empty"));
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(2800,2500),1,30,3,-1,-1,empty"));
}

void
TestGameMapRenderer::testUfoConnect()
{
    // Given a map with two connected Ufos...
    GameEnvironment env;
    Ufo* ufo = env.univ.ufos().addUfo(100, 50, /* color */ 3);
    ufo->setRadius(30);
    ufo->setPosition(Point(1300, 1500));
    ufo->postprocess(TURN_NUMBER, env.mapConfig);

    Ufo* ufo2 = env.univ.ufos().addUfo(101, 50, /* color */ 3);
    ufo2->setRadius(20);
    ufo2->setPosition(Point(1500, 1800));
    ufo2->postprocess(TURN_NUMBER, env.mapConfig);

    ufo->connectWith(*ufo2);

    // ...and ShowUfos enabled, FillUfos disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowUfos, true);
    renv.viewport.setOption(Viewport::FillUfos, false);
    render(renv);

    // ...I expect both Ufos and a connection to be rendered.
    // Note that ID is not the Ufo ID, but the index into UfoType!
    // Note that order of parameters in drawUfoConnection depends on positions, not Ufo IDs.
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(1300,1500),1,30,3,-1,-1,empty"));
    TS_ASSERT(renv.listener.hasCommand("drawUfo", "(1500,1800),2,20,3,-1,-1,empty"));
    TS_ASSERT(renv.listener.hasCommand("drawUfoConnection", "(1300,1500),(1500,1800),3"));
}

void
TestGameMapRenderer::testIonStormNormal()
{
    // Given a map with an ion storm...
    GameEnvironment env;
    IonStorm* p = env.univ.ionStorms().create(20);
    p->setRadius(30);
    p->setPosition(Point(1300, 1500));
    p->setVoltage(40);
    p->setWarpFactor(6);
    p->setHeading(120);

    // ...and ShowIonStorms enabled, FillIonStorms disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowIonStorms, true);
    renv.viewport.setOption(Viewport::FillIonStorms, false);
    render(renv);

    // ...I expect the storm to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawIonStorm", "(1300,1500),30,40,6,120,empty"));
}

void
TestGameMapRenderer::testIonStormFill()
{
    // Given a map with an ion storm...
    GameEnvironment env;
    IonStorm* p = env.univ.ionStorms().create(20);
    p->setRadius(30);
    p->setPosition(Point(1300, 1500));
    p->setVoltage(40);
    p->setWarpFactor(6);
    p->setHeading(120);

    // ...and ShowIonStorms/FillIonStorms enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowIonStorms, true);
    renv.viewport.setOption(Viewport::FillIonStorms, true);
    render(renv);

    // ...I expect the storm to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawIonStorm", "(1300,1500),30,40,6,120,fill"));
}

void
TestGameMapRenderer::testIonStormOff()
{
    // Given a map with an ion storm...
    GameEnvironment env;
    IonStorm* p = env.univ.ionStorms().create(20);
    p->setRadius(30);
    p->setPosition(Point(1300, 1500));
    p->setVoltage(40);
    p->setWarpFactor(6);
    p->setHeading(120);

    // ...and ShowIonStorms disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowIonStorms, false);
    render(renv);

    // ...I expect no storm to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawIonStorm"));
}

void
TestGameMapRenderer::testIonStormWrap()
{
    // Given a wrapped map with an ion storm...
    GameEnvironment env;
    IonStorm* p = env.univ.ionStorms().create(20);
    p->setRadius(30);
    p->setPosition(Point(1800, 1700));
    p->setVoltage(40);
    p->setWarpFactor(6);
    p->setHeading(120);
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and ShowIonStorms enabled, FillIonStorms disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowIonStorms, true);
    renv.viewport.setOption(Viewport::FillIonStorms, false);
    render(renv);

    // ...I expect the storm to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawIonStorm", "(1800,1700),30,40,6,120,empty"));
    TS_ASSERT(renv.listener.hasCommand("drawIonStorm", "(2800,2700),30,40,6,120,empty"));
}

void
TestGameMapRenderer::testDrawings()
{
    // Given a map with some drawings...
    GameEnvironment env;
    Drawing* d1 = new Drawing(Point(1600, 1800), Drawing::LineDrawing);
    d1->setPos2(Point(1700, 1850));
    d1->setColor(3);
    env.univ.drawings().addNew(d1);

    Drawing* d2 = new Drawing(Point(1500, 1400), Drawing::RectangleDrawing);
    d2->setPos2(Point(1200, 1500));
    d2->setColor(4);
    env.univ.drawings().addNew(d2);

    Drawing* d3 = new Drawing(Point(1700, 1750), Drawing::CircleDrawing);
    d3->setCircleRadius(30);
    d3->setColor(5);
    env.univ.drawings().addNew(d3);

    Drawing* d4 = new Drawing(Point(1666, 1777), Drawing::MarkerDrawing);
    d4->setMarkerKind(2);
    d4->setColor(6);
    d4->setComment("look here!");
    env.univ.drawings().addNew(d4);

    // ...and ShowDrawings enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, true);
    render(renv);

    // ...I expect the drawings to be rendered correctly.
    TS_ASSERT(renv.listener.hasCommand("drawUserLine", "(1600,1800),(1700,1850),3"));
    TS_ASSERT(renv.listener.hasCommand("drawUserRectangle", "(1500,1400),(1200,1500),4"));
    TS_ASSERT(renv.listener.hasCommand("drawUserCircle", "(1700,1750),30,5"));
    TS_ASSERT(renv.listener.hasCommand("drawUserMarker", "(1666,1777),2,6,look here!"));
}

void
TestGameMapRenderer::testDrawingsOff()
{
    // Given a map with a drawing...
    GameEnvironment env;
    Drawing* d1 = new Drawing(Point(1600, 1800), Drawing::LineDrawing);
    d1->setPos2(Point(1700, 1850));
    d1->setColor(3);
    env.univ.drawings().addNew(d1);

    // ...and ShowDrawings disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, false);
    render(renv);

    // ...I expect no drawing to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawUserLine"));
}

void
TestGameMapRenderer::testDrawingsWrap()
{
    // Given a wrapped map with a drawing...
    GameEnvironment env;
    Drawing* d1 = new Drawing(Point(1600, 1800), Drawing::LineDrawing);
    d1->setPos2(Point(1700, 1850));
    d1->setColor(3);
    env.univ.drawings().addNew(d1);
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and ShowDrawings disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, true);
    render(renv);

    // ...I expect the drawing to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawUserLine", "(1600,1800),(1700,1850),3"));
    TS_ASSERT(renv.listener.hasCommand("drawUserLine", "(2600,1800),(2700,1850),3"));
}

void
TestGameMapRenderer::testExplosions()
{
    // Given a map with an explosion...
    GameEnvironment env;
    env.univ.explosions().add(Explosion(0, Point(1600, 1800)));

    // ...and ShowDrawings enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, true);
    render(renv);

    // ...I expect that explosion to be rendered normally.
    TS_ASSERT(renv.listener.hasCommand("drawExplosion", "(1600,1800)"));
}

void
TestGameMapRenderer::testExplosionsOff()
{
    // Given a map with an explosion...
    GameEnvironment env;
    env.univ.explosions().add(Explosion(0, Point(1600, 1800)));

    // ...and ShowDrawings disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, false);
    render(renv);

    // ...I expect no explosion to be rendered.
    TS_ASSERT(!renv.listener.hasCommand("drawExplosion"));
}

void
TestGameMapRenderer::testExplosionsWrap()
{
    // Given a wrapped map with an explosion...
    GameEnvironment env;
    env.univ.explosions().add(Explosion(0, Point(1600, 1800)));
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and ShowDrawings enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowDrawings, true);
    render(renv);

    // ...I expect the explosion to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawExplosion", "(1600,1800)"));
    TS_ASSERT(renv.listener.hasCommand("drawExplosion", "(2600,2800)"));
}

void
TestGameMapRenderer::testShip()
{
    // Given a map with multiple ships...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4);
    addShipXY(env, 20, Point(1750, 1800), 5, 4);     // own
    addShipXY(env, 40, Point(1770, 1800), 7, 4);     // allied

    // ...and a team configuration...
    env.teams.setViewpointPlayer(5);
    env.teams.setPlayerTeam(7, 5);

    // ...and ShowShipDots, ShowTrails disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, false);
    render(renv);

    // ...I expect the ships to be rendered...
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1750,1800),20,me,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1770,1800),40,ally,i,"));

    // ...but no vectors.
    TS_ASSERT(!renv.listener.hasCommand("drawShipVector"));
}

void
TestGameMapRenderer::testShipLabel()
{
    // Given a map with a ship...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4);

    // ...and a label for that ship...
    LabelEnvironment lenv;
    lenv.extra.shipLabels().updateLabel(10, true, "the label");

    // ...and ShowShipDots disabled, ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowLabels, true);
    render(renv);

    // ...I expect the ship to be rendered in two passes.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,0,the label"));
}

void
TestGameMapRenderer::testShipLabelOff()
{
    // Given a map with a ship...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4);

    // ...and a label for that ship...
    LabelEnvironment lenv;
    lenv.extra.shipLabels().updateLabel(10, true, "the label");

    // ...and ShowShipDots disabled, ShowLabels disabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowLabels, false);
    render(renv);

    // ...I expect the ship to be rendered, but no label.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
}

void
TestGameMapRenderer::testShipLabelDot()
{
    // Given a map with a ship...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4);

    // ...and a label for that ship...
    LabelEnvironment lenv;
    lenv.extra.shipLabels().updateLabel(10, true, "the label");

    // ...and ShowShipDots/ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowShipDots, true);
    renv.viewport.setOption(Viewport::ShowLabels, true);
    render(renv);

    // ...I expect the ship to be rendered in a single path, with risShowDot flag.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,.,the label"));
}

void
TestGameMapRenderer::testShipLabelWrap()
{
    // Given a wrapped map with a ship...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4);
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));

    // ...and a label for that ship...
    LabelEnvironment lenv;
    lenv.extra.shipLabels().updateLabel(10, true, "the label");

    // ...and ShowShipDots disabled, ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowLabels, true);
    render(renv);

    // ...I expect the ship to be rendered multiple times, in two passes.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,0,the label"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2700,1800),10,enemy,0,the label"));
}

void
TestGameMapRenderer::testShipVector()
{
    // Given a map with a ship...
    GameEnvironment env;
    Ship& sh = addShipXY(env, 10, Point(1700, 1800), 3, 4);

    // ...with a current vector (scanned heading)...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER);
        info.addValue(game::parser::mi_Heading, 30);
        info.addValue(game::parser::mi_WarpFactor, 7);
        sh.addMessageInformation(info, PlayerSet_t(4));
    }

    // ...and a previous position...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER-1);
        info.addValue(game::parser::mi_Heading, 50);
        info.addValue(game::parser::mi_WarpFactor, 6);
        info.addValue(game::parser::mi_X, 1750);
        info.addValue(game::parser::mi_Y, 1790);
        sh.addMessageInformation(info, PlayerSet_t());
    }

    // ...and a disconnected previous position...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER-4);
        info.addValue(game::parser::mi_Heading, 90);
        info.addValue(game::parser::mi_WarpFactor, 9);
        info.addValue(game::parser::mi_X, 1600);
        info.addValue(game::parser::mi_Y, 1500);
        sh.addMessageInformation(info, PlayerSet_t());
    }

    // ...and ShowShipDots disabled, ShowTrails enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, true);
    render(renv);

    // ...I expect the ship to be rendered...
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));

    // ...and a vector to and from disconnected previous position...
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1560,1500),(1600,1500),enemy,t,4"));
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1600,1500),(1640,1500),enemy,f,3"));

    // ...and a vector to previous position...
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1736,1778),(1750,1790),enemy,t,1"));

    // ...and a vector from previous to current position...
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1750,1790),(1700,1800),enemy,ft,0"));

    // ...and a speed vector, but no waypoint.
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1700,1800),(1724,1842),enemy"));
    TS_ASSERT(!renv.listener.hasCommand("drawShipWaypoint"));
}

void
TestGameMapRenderer::testShipVectorWrapSeam()
{
    // Given a wrapped map with a ship...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));
    Ship& sh = addShipXY(env, 10, Point(1600, 1050), 3, 4);

    // ...with a current vector (scanned heading)...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER);
        info.addValue(game::parser::mi_Heading, 30);
        info.addValue(game::parser::mi_WarpFactor, 7);
        sh.addMessageInformation(info, PlayerSet_t(4));
    }

    // ...and a previous position across the seam...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER-1);
        info.addValue(game::parser::mi_X, 1150);
        info.addValue(game::parser::mi_Y, 2970);
        sh.addMessageInformation(info, PlayerSet_t());
    }

    // ...and ShowShipDots disabled, ShowTrails enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, true);
    render(renv);

    // ...I expect the ship and vector to be rendered multiple times...
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1600,1050),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1600,3050),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1600,1050),(1624,1092),enemy"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1600,3050),(1624,3092),enemy"));

    // ...and the trails to be wrapped across the seam...
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1150,2970),(1600,3050),enemy,ft,0"));
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1150,970),(1600,1050),enemy,ft,0"));
}

void
TestGameMapRenderer::testShipVectorWrapCircular()
{
    // Given a circular wrapped map with a ship...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(1000, 1000));
    Ship& sh = addShipXY(env, 10, Point(2000, 1050), 3, 4);

    // ...with a current vector (scanned heading)...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER);
        info.addValue(game::parser::mi_Heading, 30);
        info.addValue(game::parser::mi_WarpFactor, 7);
        sh.addMessageInformation(info, PlayerSet_t(4));
    }
    // ...and a previous position across the seam...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER-1);
        info.addValue(game::parser::mi_X, 2020);
        info.addValue(game::parser::mi_Y, 2970);
        sh.addMessageInformation(info, PlayerSet_t());
    }
    // ...and another previous position across the seam...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER-2);
        info.addValue(game::parser::mi_X, 2030);
        info.addValue(game::parser::mi_Y, 2900);
        sh.addMessageInformation(info, PlayerSet_t());
    }

    // ...and ShowShipDots disabled, ShowTrails enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, true);
    render(renv);

    // ...I expect the ship and vector to be rendered once...
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2000,1050),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(2000,1050),(2024,1092),enemy"));

    // ...and the trails to honor the wrap.
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(2030,2900),(2020,2970),enemy,ft,1"));   // stays in image
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(1979,970),(2000,1050),enemy,ft,0"));    // crosses seam
    TS_ASSERT(renv.listener.hasCommand("drawShipTrail", "(2020,2970),(2000,3050),enemy,ft,0"));   // crosses seam
}

void
TestGameMapRenderer::testShipVectorWrap()
{
    // Given a wrapped map with a ship...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    Ship& sh = addShipXY(env, 10, Point(1700, 1800), 3, 4);

    // ...with a current vector (scanned heading)...
    {
        MessageInformation info(MessageInformation::Ship, 10, TURN_NUMBER);
        info.addValue(game::parser::mi_Heading, 30);
        info.addValue(game::parser::mi_WarpFactor, 7);
        sh.addMessageInformation(info, PlayerSet_t(4));
    }

    // ...and ShowShipDots disabled, ShowTrails enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, true);
    render(renv);

    // ...I expect the ship and vector to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2700,2800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1700,1800),(1724,1842),enemy"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(2700,2800),(2724,2842),enemy"));
}

void
TestGameMapRenderer::testShipMessages()
{
    // Given a map with a ship with a message...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .messages().add(7);

    // ...and ShowShipDots disabled, ShowMessages enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowMessages, true);
    render(renv);

    // ...I expect the ship and a message marker to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(1700,1800)"));
}

void
TestGameMapRenderer::testShipMessagesOff()
{
    // Given a map with a ship with a message...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .messages().add(7);

    // ...and ShowShipDots/ShowMessages disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowMessages, false);
    render(renv);

    // ...I expect the ship to be rendered, but no message marker.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(!renv.listener.hasCommand("drawMessageMarker"));
}

void
TestGameMapRenderer::testShipMessagesWrap()
{
    // Given a wrapped map with a ship with a message...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .messages().add(7);

    // ...and ShowShipDots disabled, ShowMessages enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowMessages, true);
    render(renv);

    // ...I expect ship and message marker to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2700,2800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(2700,1800)"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(2700,1800)"));
}

void
TestGameMapRenderer::testShipSelection()
{
    // Given a map with a ship that is marked...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .setIsMarked(true);

    // ...and ShowShipDots disabled, ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect the ship and a selection marker to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(1700,1800)"));
}

void
TestGameMapRenderer::testShipSelectionOff()
{
    // Given a map with a ship that is marked...
    GameEnvironment env;
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .setIsMarked(true);

    // ...and ShowShipDots/ShowSelection disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowSelection, false);
    render(renv);

    // ...I expect the ship to be rendered, but no selection marker.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(!renv.listener.hasCommand("drawSelectionMarker"));
}

void
TestGameMapRenderer::testShipSelectionWrap()
{
    // Given a wrapped map with a ship that is marked...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addShipXY(env, 10, Point(1700, 1800), 3, 4)
        .setIsMarked(true);

    // ...and ShowShipDots disabled, ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect ship and selection marker to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2700,2800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2700,1800)"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2700,1800)"));
}

void
TestGameMapRenderer::testShipSelectionCircularWrap()
{
    // Given a wrapped map with a ship that is marked...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(1000, 1000));
    addShipXY(env, 10, Point(2000, 1050), 3, 4)
        .setIsMarked(true);

    // ...and ShowShipDots disabled, ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect ship and selection marker to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2000,1050),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(2000,3050),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2000,1050)"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2000,3050)"));
}

void
TestGameMapRenderer::testShipWaypoint()
{
    // Given a map with a ship with speed and waypoint...
    GameEnvironment env;
    Ship& sh = addShipXY(env, 10, Point(1700, 1800), 3, 4);
    sh.setWarpFactor(8);
    sh.setWaypoint(Point(1600, 1700));

    // ...and ShowShipDots disabled, ShowTrails enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, true);
    render(renv);

    // ...I expect the ship to be rendered with waypoint and heading vector.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShipWaypoint", "(1700,1800),(1600,1700),enemy"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1700,1800),(1655,1755),enemy"));
}

void
TestGameMapRenderer::testShipSingleTrail()
{
    // Given a map with multiple ships with speed and waypoint...
    GameEnvironment env;
    Ship& sh1 = addShipXY(env, 10, Point(1700, 1800), 3, 4);
    sh1.setWarpFactor(8);
    sh1.setWaypoint(Point(1600, 1700));

    Ship& sh2 = addShipXY(env, 20, Point(1500, 1800), 3, 4);
    sh2.setWarpFactor(8);
    sh2.setWaypoint(Point(1600, 1700));

    Ship& sh3 = addShipXY(env, 30, Point(1500, 1600), 3, 4);
    sh3.setWarpFactor(7);
    sh3.setWaypoint(Point(1600, 1700));

    // ...and ShowShipDots/ShowTrails disabled, but a ShipTrailId set...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    renv.viewport.setOption(Viewport::ShowTrails, false);
    renv.viewport.setShipTrailId(20);
    render(renv);

    // ...I expect all ships, and the selected ship's trail, to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1500,1800),20,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1500,1600),30,enemy,i,"));
    TS_ASSERT(renv.listener.hasCommand("drawShipWaypoint", "(1500,1800),(1600,1700),enemy"));
    TS_ASSERT(renv.listener.hasCommand("drawShipVector", "(1500,1800),(1545,1755),enemy"));
}

void
TestGameMapRenderer::testShipFleet()
{
    // Given a map with a ship that is a fleet leader...
    GameEnvironment env;
    Ship& sh = addShipXY(env, 10, Point(1700, 1800), 3, 4);
    sh.setFleetNumber(10);

    // ...and ShowShipDots disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowShipDots, false);
    render(renv);

    // ...I expect the ship to be rendered as fleet icon
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),10,enemy,if,"));
}

void
TestGameMapRenderer::testPlanet()
{
    // Given a map with some planets...
    GameEnvironment env;
    addUnscannedPlanet(env, 10, Point(1700, 1800));
    addScannedPlanet(env, 20, Point(1710, 1800), 0);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);
    addScannedPlanet(env, 40, Point(1730, 1800), 2);
    addScannedPlanet(env, 50, Point(1740, 1800), 3);
    addNativePlanet(env, 60, Point(1750, 1800));
    addBasePlanet(env, 70, Point(1760, 1800), 3);

    // ...and a team configuration...
    env.teams.setViewpointPlayer(2);
    env.teams.setPlayerTeam(3, 2);

    // ...and no particular settings...
    RenderEnvironment renv(env);
    render(renv);

    // ...I expect the planets to be rendered as expected.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1700,1800),10,0,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1710,1800),20,u,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1730,1800),40,o,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1740,1800),50,a,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1750,1800),60,u,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1760,1800),70,ab,"));
}

void
TestGameMapRenderer::testPlanetWrap()
{
    // Given a wrapped map with some planet...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and no particular settings...
    RenderEnvironment renv(env);
    render(renv);

    // ...I expect the planet to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2720,2800),30,e,"));
}

void
TestGameMapRenderer::testPlanetLabel()
{
    // Given a map with some planet...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and a label for that planet...
    LabelEnvironment lenv;
    lenv.extra.planetLabels().updateLabel(30, true, "the label");

    // ...and ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowLabels, true);
    render(renv);

    // ...I expect the planet to be rendered with label.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,the label"));
}

void
TestGameMapRenderer::testPlanetLabelWrap()
{
    // Given a wrapped map with some planet...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and a label for that planet...
    LabelEnvironment lenv;
    lenv.extra.planetLabels().updateLabel(30, true, "the label");

    // ...and ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowLabels, true);
    render(renv);

    // ...I expect the planet to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,the label"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2720,2800),30,e,the label"));
}

void
TestGameMapRenderer::testPlanetLabelOff()
{
    // Given a map with some planet...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and a label for that planet...
    LabelEnvironment lenv;
    lenv.extra.planetLabels().updateLabel(30, true, "the label");

    // ...and ShowLabels disabled...
    RenderEnvironment renv(env, lenv);
    renv.viewport.setOption(Viewport::ShowLabels, false);
    render(renv);

    // ...I expect the planet to be rendered without label.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
}

void
TestGameMapRenderer::testPlanetMessage()
{
    // Given a map with some planet that has a message...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .messages().add(12);

    // ...and ShowMessages enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMessages, true);
    render(renv);

    // ...I expect the message marker to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(1720,1800)"));
}

void
TestGameMapRenderer::testPlanetMessageWrap()
{
    // Given a wrapped map with some planet that has a message...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .messages().add(12);

    // ...and ShowMessages enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMessages, true);
    render(renv);

    // ...I expect the message marker to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(1720,1800)"));
    TS_ASSERT(renv.listener.hasCommand("drawMessageMarker", "(2720,1800)"));
}

void
TestGameMapRenderer::testPlanetMessageOff()
{
    // Given a map with some planet that has a message...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .setIsMarked(true);

    // ...and ShowMessages disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowMessages, false);
    render(renv);

    // ...I expect the message marker to be rendered, but no marker.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(!renv.listener.hasCommand("drawMessageMarker"));
}

void
TestGameMapRenderer::testPlanetSelection()
{
    // Given a map with some planet that is marked...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .setIsMarked(true);

    // ...and ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect the selection marker to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(1720,1800)"));
}

void
TestGameMapRenderer::testPlanetSelectionWrap()
{
    // Given a wrapped map with some planet that is marked...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .setIsMarked(true);

    // ...and ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect the selection marker to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(1720,1800)"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2720,1800)"));
}

void
TestGameMapRenderer::testPlanetSelectionOff()
{
    // Given a map with some planet that is marked...
    GameEnvironment env;
    addScannedPlanet(env, 30, Point(1720, 1800), 1)
        .setIsMarked(true);

    // ...and ShowSelection disabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowSelection, false);
    render(renv);

    // ...I expect the selection marker to be rendered, but no marker.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(!renv.listener.hasCommand("drawSelection"));
}

void
TestGameMapRenderer::testPlanetWarpWell()
{
    // Given a map with some planet...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(3);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and ShowWarpWells enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowWarpWells, true);
    render(renv);

    // ...I expect the warp wells to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));

    // Check one quadrant:
    //        X
    //    X X X . .
    //    X X X . .
    //  X X X o . . .
    //    . . . . .
    //    . . . . .
    //        .
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1717,1800),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1717,1800),S"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1718,1799),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1718,1798),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1718,1798),S"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1719,1798),S"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1720,1797),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1720,1797),S"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1720,1797),E"));
}

void
TestGameMapRenderer::testPlanetWarpWellWrap()
{
    // Given a wrapped map with some planet...
    GameEnvironment env;
    env.mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(1000, 1000));
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(3);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and ShowWarpWells enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowWarpWells, true);
    render(renv);

    // ...I expect the warp wells to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1717,1800),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(2717,1800),W"));
}

void
TestGameMapRenderer::testPlanetWarpWellSquare()
{
    // Given a map with some planet, and square warp wells...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(0);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(4);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and ShowWarpWells enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowWarpWells, true);
    render(renv);

    // ...I expect the warp wells to be rendered (check specimen).
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1716,1800),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1716,1804),W"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(1716,1804),N"));
}

void
TestGameMapRenderer::testPlanetWarpWellOff()
{
    // Given a map with some planet...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(3);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and ShowWarpWells disbled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowWarpWells, false);
    render(renv);

    // ...I expect no wells to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(!renv.listener.hasCommand("drawWarpWellEdge"));
}

void
TestGameMapRenderer::testPlanetWarpWellNone()
{
    // Given a map with some planet in a universe without warp wells...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(0);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(3);
    addScannedPlanet(env, 30, Point(1720, 1800), 1);

    // ...and ShowWarpWells ensbled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowWarpWells, true);
    render(renv);

    // ...I expect no wells to be rendered.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,e,"));
    TS_ASSERT(!renv.listener.hasCommand("drawWarpWellEdge"));
}

void
TestGameMapRenderer::testPlanetShips()
{
    // Given a map with some planets, orbited by ships...
    GameEnvironment env;
    addScannedPlanet(env, 10, Point(1700, 1800), 0);
    addShipXY       (env, 10, Point(1700, 1800), 3, 7);        // enemy
    addScannedPlanet(env, 20, Point(1710, 1800), 0);
    addShipXY       (env, 20, Point(1710, 1800), 4, 7);        // own
    addScannedPlanet(env, 30, Point(1720, 1800), 0);
    addShipXY       (env, 30, Point(1720, 1800), 5, 7);        // ally

    // ...and a team configuration...
    env.teams.setViewpointPlayer(4);
    env.teams.setPlayerTeam(5, 4);

    // ...and no particular settings...
    RenderEnvironment renv(env);
    render(renv);

    // ...I expect the planets to be rendered with ship markers (and no ships).
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1700,1800),10,uE,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1710,1800),20,uO,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1720,1800),30,uA,"));
    TS_ASSERT(!renv.listener.hasCommand("drawShip"));
}

void
TestGameMapRenderer::testPlanetShipSelection()
{
    // Given a map with a planet, orbited by a marked ship...
    GameEnvironment env;
    addScannedPlanet(env, 10, Point(1700, 1800), 0);
    addShipXY(env, 10, Point(1700, 1800), 3, 7)
        .setIsMarked(true);

    // ...and a team configuration...
    env.teams.setViewpointPlayer(4);
    env.teams.setPlayerTeam(5, 4);

    // ...and ShowSelection enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    render(renv);

    // ...I expect the selection to be drawn.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1700,1800),10,uE,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(1700,1800)"));
    TS_ASSERT(!renv.listener.hasCommand("drawShip"));
}

void
TestGameMapRenderer::testPlanetCircularWrap()
{
    // Given a wrapped map with a marked planet...
    GameEnvironment env;
    env.hostConfiguration[HostConfiguration::AllowGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::RoundGravityWells].set(1);
    env.hostConfiguration[HostConfiguration::GravityWellRange].set(3);
    env.mapConfig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(1000, 1000));
    addUnscannedPlanet(env, 10, Point(2000, 1050))
        .setIsMarked(true);

    // ...and ShowSelection/ShowWarpWells enabled...
    RenderEnvironment renv(env);
    renv.viewport.setOption(Viewport::ShowSelection, true);
    renv.viewport.setOption(Viewport::ShowWarpWells, true);
    render(renv);

    // ...I expect planet, warp wells, and selection to be rendered multiple times.
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2000,1050),10,0,"));
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(2000,3050),10,0,"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2000,1050)"));
    TS_ASSERT(renv.listener.hasCommand("drawSelection", "(2000,3050)"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(2000,1047),S"));
    TS_ASSERT(renv.listener.hasCommand("drawWarpWellEdge", "(2000,3047),S"));
}

void
TestGameMapRenderer::testPlanetShipLabel()
{
    // Given a map with some planets, orbited by ships...
    GameEnvironment env;
    addScannedPlanet(env, 10, Point(1700, 1800), 0);
    addShipXY(env, 33, Point(1700, 1800), 3, 7);        // enemy

    // ...and a team configuration...
    env.teams.setViewpointPlayer(4);
    env.teams.setPlayerTeam(5, 4);

    // ...and a ship label...
    LabelEnvironment lenv;
    lenv.extra.shipLabels().updateLabel(33, true, "ship label");

    // ...and ShowLabels enabled...
    RenderEnvironment renv(env, lenv);
    render(renv);

    // ...I expect the planets to be rendered with ship markers (and no ships).
    TS_ASSERT(renv.listener.hasCommand("drawPlanet", "(1700,1800),10,uE,"));
    TS_ASSERT(renv.listener.hasCommand("drawShip", "(1700,1800),33,enemy,p,ship label"));
}

