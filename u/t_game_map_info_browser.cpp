/**
  *  \file u/t_game_map_info_browser.cpp
  *  \brief Test for game::map::info::Browser
  */

#include "game/map/info/browser.hpp"

#include "t_game_map_info.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/info/scriptlinkbuilder.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/specificationloader.hpp"

using game::map::info::Nodes_t;
using game::map::Planet;
using game::Player;

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::map::info::ScriptLinkBuilder link;
        game::map::info::Browser browser;

        TestHarness()
            : tx(), fs(), session(tx, fs),
              browser(session, link)
            { }
    };

    void createTurn(TestHarness& h)
    {
        // Root
        afl::base::Ptr<game::Root> root = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
        h.session.setRoot(root);
        Player* p1 = root->playerList().create(1);
        p1->setName(Player::ShortName, "The First");
        p1->setName(Player::LongName, "The First Imperium");
        p1->setName(Player::AdjectiveName, "first");
        Player* p2 = root->playerList().create(2);
        p2->setName(Player::ShortName, "The Second");
        p2->setName(Player::LongName, "The Second Fleet");
        p2->setName(Player::AdjectiveName, "second");
        Player* p7 = root->playerList().create(7);
        p7->setName(Player::ShortName, "The Seven");
        p7->setName(Player::LongName, "The Seven Dwarves");
        p7->setName(Player::AdjectiveName, "dwarven");

        // Ship list
        afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
        h.session.setShipList(sl);
        game::test::initStandardBeams(*sl);
        game::test::initStandardTorpedoes(*sl);

        // Hulls and engines from default files
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("<spec>");
        dir->addStream("hullspec.dat", *new afl::io::ConstMemoryStream(game::test::getDefaultHulls()));
        dir->addStream("engspec.dat",  *new afl::io::ConstMemoryStream(game::test::getDefaultEngines()));
        dir->addStream("truehull.dat", *new afl::io::ConstMemoryStream(game::test::getDefaultHullAssignments()));
        game::v3::SpecificationLoader ldr(dir, std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                          h.session.translator(), h.session.log());
        ldr.loadHulls(*sl, *dir);
        ldr.loadHullAssignments(*sl, *dir);
        ldr.loadEngines(*sl, *dir);

        game::test::addOutrider(*sl);
        game::test::addGorbie(*sl);
        game::test::addAnnihilation(*sl);
        game::test::addNovaDrive(*sl);
        game::test::addTranswarp(*sl);

        // Game
        afl::base::Ptr<game::Game> g = new game::Game();
        h.session.setGame(g);
        g->currentTurn().setTurnNumber(33);
    }

    void populateTurn(TestHarness& h)
    {
        afl::charset::Utf8Charset cs;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::v3::Loader ldr(cs, tx, log);

        // create objects
        ldr.prepareUniverse(h.session.getGame()->currentTurn().universe());

        // planet.nm
        afl::io::ConstMemoryStream planetNameFile(game::test::getDefaultPlanetNames());
        ldr.loadPlanetNames(h.session.getGame()->currentTurn().universe(), planetNameFile);

        // xyplan.dat
        afl::io::ConstMemoryStream planetCoordinateFile(game::test::getDefaultPlanetCoordinates());
        ldr.loadPlanetCoordinates(h.session.getGame()->currentTurn().universe(), planetCoordinateFile);

        // result file
        afl::io::ConstMemoryStream resultFile(game::test::getComplexResultFile());
        ldr.loadResult(h.session.getGame()->currentTurn(), *h.session.getRoot(), *h.session.getGame(), resultFile, 7);

        // finish
        h.session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(7),
                                                                  game::PlayerSet_t(7),
                                                                  game::map::Object::Playable,
                                                                  h.session.getGame()->mapConfiguration(),
                                                                  h.session.getRoot()->hostVersion(),
                                                                  h.session.getRoot()->hostConfiguration(),
                                                                  61, *h.session.getShipList(), tx, log);
        h.session.getGame()->teamSettings().setViewpointPlayer(7);
    }

    template<typename T>
    String_t toString(const T& n)
    {
        afl::io::InternalSink sink;
        afl::io::xml::Writer(sink).visit(n);
        return afl::string::fromBytes(sink.getContent());
    }

    bool hasOption(const util::StringList& list, String_t title, uint8_t value)
    {
        for (size_t i = 0; i < list.size(); ++i) {
            String_t foundTitle;
            int32_t foundKey;
            if (list.get(i, foundKey, foundTitle) && foundKey == value && foundTitle == title) {
                return true;
            }
        }
        return false;
    }
}

/** Test empty session.
    For an empty session, all "render" calls exit with an exception (eUser). */
void
TestGameMapInfoBrowser::testNull()
{
    TestHarness h;
    Nodes_t out;
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::TotalsPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::MineralsPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::PlanetsPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::ColonyPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::StarbasePage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::StarshipPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::CapitalPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::StarchartPage, out), game::Exception);
    TS_ASSERT_THROWS(h.browser.renderPage(game::map::info::WeaponsPage, out), game::Exception);
}


void
TestGameMapInfoBrowser::testEmptyTotals()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::TotalsPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Totals</h1>"
                     "<table align=\"left\">"
                     "<tr><td width=\"10\">Planets:</td><td align=\"right\" width=\"6\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Starbases:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Starships:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 Capital ships:</td><td align=\"right\"><font color=\"green\">0</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Minerals</font></td><td align=\"right\" width=\"6\">(available)</td><td width=\"2\"></td><td align=\"right\" width=\"6\">(ground)</td><td width=\"2\"></td></tr>"
                     "<tr><td>Neutronium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Tritanium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Duranium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Molybdenum:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Colonies</font></td><td align=\"right\" width=\"6\"></td><td width=\"2\"></td></tr>"
                     "<tr><td>Colonists:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Money:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">mc</font></td></tr>"
                     "<tr><td>Supplies:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Mineral Mines:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Factories:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Defense Posts:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Production</font></td><td align=\"right\" width=\"6\"></td><td width=\"2\"></td><td align=\"right\" width=\"6\">(max)</td><td width=\"2\"></td></tr>"
                     "<tr><td>Neutronium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Tritanium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Duranium:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Molybdenum:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Money:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">mc</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">mc</font></td></tr>"
                     "<tr><td>Supplies:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\">kt</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyMinerals()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::MineralsPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Minerals</h1>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Neutronium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Tritanium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Duranium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Molybdenum Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyPlanets()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::PlanetsPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Planets</h1>"
                     "<table align=\"left\"><tr><td width=\"15\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">0</font></td></tr></table>");

}

void
TestGameMapInfoBrowser::testEmptyColony()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::ColonyPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Colony</h1>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Colonists Planets</font></td><td align=\"right\" width=\"8\">(clans)</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Money Planets</font></td><td align=\"right\" width=\"8\">(mc)</td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyStarbases()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarbasePage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starbases</h1>"
                     "<table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">0</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyStarships()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarshipPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starships</h1>"
                     "<table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">0</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyCapital()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::CapitalPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Capital Ships</h1>"
                     "<table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">0</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyStarchart()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarchartPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starchart</h1>"
                     "<table align=\"left\"><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Foreign Units</font></td><td align=\"right\" width=\"7\">Ships</td><td width=\"3\">(hist.)</td><td align=\"right\" width=\"7\">Planets</td><td width=\"3\">(hist.)</td><td align=\"right\" width=\"7\">Minefields</td></tr>"
                     "<tr><td>Total:</td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 Visual Contacts:</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"/><td/><td align=\"right\"/><td/></tr></table>"
                     "<table align=\"left\"><tr><td>Universal Minefield FCode:</td><td><font color=\"green\">none</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testEmptyWeapons()
{
    TestHarness h;
    createTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::WeaponsPage, out);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Weapons</h1>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Beams</font></td><td align=\"right\" width=\"4\">Ships</td><td align=\"right\" width=\"8\">Weapons</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Torpedoes</font></td><td align=\"right\" width=\"4\">Ships</td><td align=\"right\" width=\"8\">Torpedoes</td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Others</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td>Carriers</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Unarmed ships</td><td align=\"right\"><font color=\"green\">0</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleTotals()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::TotalsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::TotalsPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Totals</h1><table align=\"left\"><tr><td width=\"10\">Planets:</td><td align=\"right\" width=\"6\"><font color=\"green\">35</font></td></tr>"
                     "<tr><td>Starbases:</td><td align=\"right\"><font color=\"green\">5</font></td></tr>"
                     "<tr><td>Starships:</td><td align=\"right\"><font color=\"green\">37</font></td></tr>"
                     "<tr><td>\xC2\xA0 Capital ships:</td><td align=\"right\"><font color=\"green\">23</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Minerals</font></td><td align=\"right\" width=\"6\">(available)</td><td width=\"2\"></td><td align=\"right\" width=\"6\">(ground)</td><td width=\"2\"></td></tr>"
                     "<tr><td>Neutronium:</td><td align=\"right\"><font color=\"green\">18,296</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">62,260</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Tritanium:</td><td align=\"right\"><font color=\"green\">11,471</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">58,038</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Duranium:</td><td align=\"right\"><font color=\"green\">10,025</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">11,944</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Molybdenum:</td><td align=\"right\"><font color=\"green\">8,497</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">37,213</font></td><td><font color=\"green\">kt</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Colonies</font></td><td align=\"right\" width=\"6\"></td><td width=\"2\"></td></tr>"
                     "<tr><td>Colonists:</td><td align=\"right\"><font color=\"green\">3,061,600</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Money:</td><td align=\"right\"><font color=\"green\">26,938</font></td><td><font color=\"green\">mc</font></td></tr>"
                     "<tr><td>Supplies:</td><td align=\"right\"><font color=\"green\">23,964</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Mineral Mines:</td><td align=\"right\"><font color=\"green\">2,233</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Factories:</td><td align=\"right\"><font color=\"green\">3,232</font></td><td><font color=\"green\"></font></td></tr>"
                     "<tr><td>Defense Posts:</td><td align=\"right\"><font color=\"green\">1,230</font></td><td><font color=\"green\"></font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Production</font></td><td align=\"right\" width=\"6\"></td><td width=\"2\"></td><td align=\"right\" width=\"6\">(max)</td><td width=\"2\"></td></tr>"
                     "<tr><td>Neutronium:</td><td align=\"right\"><font color=\"green\">897</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,181</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Tritanium:</td><td align=\"right\"><font color=\"green\">333</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,294</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Duranium:</td><td align=\"right\"><font color=\"green\">357</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,132</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Molybdenum:</td><td align=\"right\"><font color=\"green\">460</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,394</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td>Money:</td><td align=\"right\"><font color=\"green\">2,243</font></td><td><font color=\"green\">mc</font></td><td align=\"right\"><font color=\"green\">2,618</font></td><td><font color=\"green\">mc</font></td></tr>"
                     "<tr><td>Supplies:</td><td align=\"right\"><font color=\"green\">3,286</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">3,344</font></td><td><font color=\"green\">kt</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleMinerals()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::MineralsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::MineralsPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Minerals</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Neutronium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,354\">Planet #354: Madonna</a></td><td align=\"right\"><font color=\"green\">10,133</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">36</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,495\">Planet #495: Ohular</a></td><td align=\"right\"><font color=\"green\">9,111</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,847</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">8,095</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,211</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,30\">Planet #30: Cestus 3</a></td><td align=\"right\"><font color=\"green\">7,358</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,569</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,361\">Planet #361: Garfield</a></td><td align=\"right\"><font color=\"green\">6,453</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">541</font></td><td><font color=\"green\">kt</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Tritanium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,336\">Planet #336: The Right Planet</a></td><td align=\"right\"><font color=\"green\">10,714</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">532</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,437\">Planet #437: Amascusin</a></td><td align=\"right\"><font color=\"green\">10,490</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">28</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,358\">Planet #358: World of Wonder</a></td><td align=\"right\"><font color=\"green\">9,886</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">659</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,155\">Planet #155: Mao 3</a></td><td align=\"right\"><font color=\"green\">7,605</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">542</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,30\">Planet #30: Cestus 3</a></td><td align=\"right\"><font color=\"green\">5,814</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">828</font></td><td><font color=\"green\">kt</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Duranium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,260\">Planet #260: Regenal</a></td><td align=\"right\"><font color=\"green\">5,324</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">3,513</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,332\">Planet #332: Kaye\'s World</a></td><td align=\"right\"><font color=\"green\">3,948</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">164</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,363\">Planet #363: Rambo 3</a></td><td align=\"right\"><font color=\"green\">2,120</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">126</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,361\">Planet #361: Garfield</a></td><td align=\"right\"><font color=\"green\">1,667</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">425</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,423\">Planet #423: Doggle</a></td><td align=\"right\"><font color=\"green\">741</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">739</font></td><td><font color=\"green\">kt</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Molybdenum Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,355\">Planet #355: Center</a></td><td align=\"right\"><font color=\"green\">10,812</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">664</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,299\">Planet #299: Sinatra\'s Keep</a></td><td align=\"right\"><font color=\"green\">9,705</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">137</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">9,260</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,383</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,167\">Planet #167: Owen 1717</a></td><td align=\"right\"><font color=\"green\">5,967</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,962</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,206\">Planet #206: Orkney World</a></td><td align=\"right\"><font color=\"green\">5,283</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">359</font></td><td><font color=\"green\">kt</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleMineralsMinedT()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.setPageOptions(game::map::info::MineralsPage, game::map::info::Minerals_ShowOnlyT + game::map::info::Minerals_SortByMined);
    h.browser.renderPage(game::map::info::MineralsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::MineralsPage), game::map::info::Minerals_ShowOnlyT + game::map::info::Minerals_SortByMined);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Minerals</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 24 Tritanium Planets</font></td><td align=\"right\" width=\"8\">(total)</td><td width=\"2\"></td><td align=\"right\" width=\"8\">(mined)</td><td width=\"2\"></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">1,067</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">1,065</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,30\">Planet #30: Cestus 3</a></td><td align=\"right\"><font color=\"green\">5,814</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">828</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,363\">Planet #363: Rambo 3</a></td><td align=\"right\"><font color=\"green\">828</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">827</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,52\">Planet #52: Rigel</a></td><td align=\"right\"><font color=\"green\">690</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">686</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,358\">Planet #358: World of Wonder</a></td><td align=\"right\"><font color=\"green\">9,886</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">659</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,495\">Planet #495: Ohular</a></td><td align=\"right\"><font color=\"green\">643</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">639</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,56\">Planet #56: Ikaal</a></td><td align=\"right\"><font color=\"green\">603</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">598</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,413\">Planet #413: Centouria</a></td><td align=\"right\"><font color=\"green\">590</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">585</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,260\">Planet #260: Regenal</a></td><td align=\"right\"><font color=\"green\">546</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">545</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,155\">Planet #155: Mao 3</a></td><td align=\"right\"><font color=\"green\">7,605</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">542</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,336\">Planet #336: The Right Planet</a></td><td align=\"right\"><font color=\"green\">10,714</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">532</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,423\">Planet #423: Doggle</a></td><td align=\"right\"><font color=\"green\">478</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">474</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,332\">Planet #332: Kaye\'s World</a></td><td align=\"right\"><font color=\"green\">397</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">394</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,452\">Planet #452: Helios</a></td><td align=\"right\"><font color=\"green\">349</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">344</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,492\">Planet #492: Tyre</a></td><td align=\"right\"><font color=\"green\">260</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">258</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,379\">Planet #379: Kennedy Center</a></td><td align=\"right\"><font color=\"green\">228</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">226</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,361\">Planet #361: Garfield</a></td><td align=\"right\"><font color=\"green\">209</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">206</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,355\">Planet #355: Center</a></td><td align=\"right\"><font color=\"green\">197</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">192</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,369\">Planet #369: Nothing Planet</a></td><td align=\"right\"><font color=\"green\">183</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">178</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,299\">Planet #299: Sinatra\'s Keep</a></td><td align=\"right\"><font color=\"green\">390</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">164</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,1\">Planet #1: Ceti Alpha one</a></td><td align=\"right\"><font color=\"green\">161</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">160</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,192\">Planet #192: Kugo</a></td><td align=\"right\"><font color=\"green\">244</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">120</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,338\">Planet #338: Big World</a></td><td align=\"right\"><font color=\"green\">4,877</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">118</font></td><td><font color=\"green\">kt</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,426\">Planet #426: Bootie</a></td><td align=\"right\"><font color=\"green\">288</font></td><td><font color=\"green\">kt</font></td><td align=\"right\"><font color=\"green\">107</font></td><td><font color=\"green\">kt</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSamplePlanets()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::PlanetsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::PlanetsPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Planets</h1><table align=\"left\"><tr><td width=\"15\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">35</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"12\"><font color=\"white\">Natives</font></td><td align=\"right\" width=\"6\">Planets</td><td align=\"right\" width=\"8\">Natives</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=0&quot;,&quot;p2&quot;\">none</a></td><td align=\"right\"><font color=\"green\">16</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=6&quot;,&quot;p2&quot;\">Insectoid</a></td><td align=\"right\"><font color=\"green\">5</font></td><td align=\"right\"><font color=\"green\">27,286,000</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=8&quot;,&quot;p2&quot;\">Ghipsoldal</a></td><td align=\"right\"><font color=\"green\">5</font></td><td align=\"right\"><font color=\"green\">14,416,200</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=3&quot;,&quot;p2&quot;\">Reptilian</a></td><td align=\"right\"><font color=\"green\">3</font></td><td align=\"right\"><font color=\"green\">11,736,700</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=4&quot;,&quot;p2&quot;\">Avian</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">4,097,700</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=1&quot;,&quot;p2&quot;\">Humanoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">4,544,500</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=2&quot;,&quot;p2&quot;\">Bovinoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">1,120,200</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=7&quot;,&quot;p2&quot;\">Amphibian</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">11,312,700</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=9&quot;,&quot;p2&quot;\">Siliconoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">1,193,200</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"12\"><font color=\"white\">Climate</font></td><td align=\"right\" width=\"6\">Planets</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=0 And Temp$&lt;=14 And Owner$=My.Race$&quot;,&quot;p2&quot;\">arctic</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=15 And Temp$&lt;=39 And Owner$=My.Race$&quot;,&quot;p2&quot;\">cool</a></td><td align=\"right\"><font color=\"green\">13</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=40 And Temp$&lt;=64 And Owner$=My.Race$&quot;,&quot;p2&quot;\">warm</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=65 And Temp$&lt;=84 And Owner$=My.Race$&quot;,&quot;p2&quot;\">tropical</a></td><td align=\"right\"><font color=\"green\">3</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=85 And Temp$&lt;=100 And Owner$=My.Race$&quot;,&quot;p2&quot;\">desert</a></td><td align=\"right\"><font color=\"green\">5</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"15\"><a href=\"q:UI.Search &quot;Defense&lt;10 And Owner$=My.Race$&quot;,&quot;p2&quot;\">Nearly undefended:</a></td><td align=\"right\" width=\"3\"><font color=\"green\">8</font></td></tr>"
                     "<tr><td width=\"15\"><a href=\"q:UI.Search &quot;Defense&lt;15 And Owner$=My.Race$&quot;,&quot;p2&quot;\">Visible by sensor scan:</a></td><td align=\"right\" width=\"3\"><font color=\"green\">9</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSamplePlanetsByNativeRace()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.setPageOptions(game::map::info::PlanetsPage, game::map::info::Planets_SortByRace);
    h.browser.renderPage(game::map::info::PlanetsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::PlanetsPage), game::map::info::Planets_SortByRace);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Planets</h1><table align=\"left\"><tr><td width=\"15\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">35</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"12\"><font color=\"white\">Natives</font></td><td align=\"right\" width=\"6\">Planets</td><td align=\"right\" width=\"8\">Natives</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=0&quot;,&quot;p2&quot;\">none</a></td><td align=\"right\"><font color=\"green\">16</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=1&quot;,&quot;p2&quot;\">Humanoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">4,544,500</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=2&quot;,&quot;p2&quot;\">Bovinoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">1,120,200</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=3&quot;,&quot;p2&quot;\">Reptilian</a></td><td align=\"right\"><font color=\"green\">3</font></td><td align=\"right\"><font color=\"green\">11,736,700</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=4&quot;,&quot;p2&quot;\">Avian</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">4,097,700</font></td></tr>"
                     "<tr><td>Amorphous</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=6&quot;,&quot;p2&quot;\">Insectoid</a></td><td align=\"right\"><font color=\"green\">5</font></td><td align=\"right\"><font color=\"green\">27,286,000</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=7&quot;,&quot;p2&quot;\">Amphibian</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">11,312,700</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=8&quot;,&quot;p2&quot;\">Ghipsoldal</a></td><td align=\"right\"><font color=\"green\">5</font></td><td align=\"right\"><font color=\"green\">14,416,200</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Natives.Race$=9&quot;,&quot;p2&quot;\">Siliconoid</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">1,193,200</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"12\"><font color=\"white\">Climate</font></td><td align=\"right\" width=\"6\">Planets</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=0 And Temp$&lt;=14 And Owner$=My.Race$&quot;,&quot;p2&quot;\">arctic</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=15 And Temp$&lt;=39 And Owner$=My.Race$&quot;,&quot;p2&quot;\">cool</a></td><td align=\"right\"><font color=\"green\">13</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=40 And Temp$&lt;=64 And Owner$=My.Race$&quot;,&quot;p2&quot;\">warm</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=65 And Temp$&lt;=84 And Owner$=My.Race$&quot;,&quot;p2&quot;\">tropical</a></td><td align=\"right\"><font color=\"green\">3</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Temp$&gt;=85 And Temp$&lt;=100 And Owner$=My.Race$&quot;,&quot;p2&quot;\">desert</a></td><td align=\"right\"><font color=\"green\">5</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"15\"><a href=\"q:UI.Search &quot;Defense&lt;10 And Owner$=My.Race$&quot;,&quot;p2&quot;\">Nearly undefended:</a></td><td align=\"right\" width=\"3\"><font color=\"green\">8</font></td></tr>"
                     "<tr><td width=\"15\"><a href=\"q:UI.Search &quot;Defense&lt;15 And Owner$=My.Race$&quot;,&quot;p2&quot;\">Visible by sensor scan:</a></td><td align=\"right\" width=\"3\"><font color=\"green\">9</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleColony()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::ColonyPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::ColonyPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Colony</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Colonists Planets</font></td><td align=\"right\" width=\"8\">(clans)</td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,363\">Planet #363: Rambo 3</a></td><td align=\"right\"><font color=\"green\">24,172</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,167\">Planet #167: Owen 1717</a></td><td align=\"right\"><font color=\"green\">698</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,413\">Planet #413: Centouria</a></td><td align=\"right\"><font color=\"green\">601</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,336\">Planet #336: The Right Planet</a></td><td align=\"right\"><font color=\"green\">443</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">383</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,260\">Planet #260: Regenal</a></td><td align=\"right\"><font color=\"green\">2,861</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,30\">Planet #30: Cestus 3</a></td><td align=\"right\"><font color=\"green\">2,260</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,452\">Planet #452: Helios</a></td><td align=\"right\"><font color=\"green\">1,853</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,75\">Planet #75: Sol 3</a></td><td align=\"right\"><font color=\"green\">1,792</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,167\">Planet #167: Owen 1717</a></td><td align=\"right\"><font color=\"green\">1,606</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 5 Money Planets</font></td><td align=\"right\" width=\"8\">(mc)</td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,260\">Planet #260: Regenal</a></td><td align=\"right\"><font color=\"green\">3,524</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">1,639</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,336\">Planet #336: The Right Planet</a></td><td align=\"right\"><font color=\"green\">1,612</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,167\">Planet #167: Owen 1717</a></td><td align=\"right\"><font color=\"green\">1,492</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,369\">Planet #369: Nothing Planet</a></td><td align=\"right\"><font color=\"green\">1,120</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleColonyTopSupplies()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.setPageOptions(game::map::info::ColonyPage, game::map::info::Colony_ShowOnlySupplies);
    h.browser.renderPage(game::map::info::ColonyPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::ColonyPage), game::map::info::Colony_ShowOnlySupplies);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Colony</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Top 24 Supplies Planets</font></td><td align=\"right\" width=\"8\">(kt)</td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,260\">Planet #260: Regenal</a></td><td align=\"right\"><font color=\"green\">2,861</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,30\">Planet #30: Cestus 3</a></td><td align=\"right\"><font color=\"green\">2,260</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,452\">Planet #452: Helios</a></td><td align=\"right\"><font color=\"green\">1,853</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,75\">Planet #75: Sol 3</a></td><td align=\"right\"><font color=\"green\">1,792</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,167\">Planet #167: Owen 1717</a></td><td align=\"right\"><font color=\"green\">1,606</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,363\">Planet #363: Rambo 3</a></td><td align=\"right\"><font color=\"green\">1,272</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,379\">Planet #379: Kennedy Center</a></td><td align=\"right\"><font color=\"green\">1,055</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,133\">Planet #133: Virgo Pegasi</a></td><td align=\"right\"><font color=\"green\">996</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,94\">Planet #94: Alycyone</a></td><td align=\"right\"><font color=\"green\">809</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,336\">Planet #336: The Right Planet</a></td><td align=\"right\"><font color=\"green\">750</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,361\">Planet #361: Garfield</a></td><td align=\"right\"><font color=\"green\">618</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,369\">Planet #369: Nothing Planet</a></td><td align=\"right\"><font color=\"green\">596</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,155\">Planet #155: Mao 3</a></td><td align=\"right\"><font color=\"green\">524</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,492\">Planet #492: Tyre</a></td><td align=\"right\"><font color=\"green\">519</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,413\">Planet #413: Centouria</a></td><td align=\"right\"><font color=\"green\">511</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,208\">Planet #208: New Ireland</a></td><td align=\"right\"><font color=\"green\">477</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,499\">Planet #499: Yukindo</a></td><td align=\"right\"><font color=\"green\">463</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,495\">Planet #495: Ohular</a></td><td align=\"right\"><font color=\"green\">414</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,332\">Planet #332: Kaye\'s World</a></td><td align=\"right\"><font color=\"green\">407</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,358\">Planet #358: World of Wonder</a></td><td align=\"right\"><font color=\"green\">357</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,52\">Planet #52: Rigel</a></td><td align=\"right\"><font color=\"green\">326</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,190\">Planet #190: Wooky World</a></td><td align=\"right\"><font color=\"green\">311</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,56\">Planet #56: Ikaal</a></td><td align=\"right\"><font color=\"green\">300</font></td></tr>"
                     "<tr><td><a href=\"q:UI.GotoScreen 2,338\">Planet #338: Big World</a></td><td align=\"right\"><font color=\"green\">261</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleStarbases()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarbasePage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::StarbasePage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starbases</h1><table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">5</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Tech.Hull=10&quot;,&quot;b2&quot;\">Tech 10 Hulls:</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Tech.Engine=10&quot;,&quot;b2&quot;\">Tech 10 Engines:</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td>Tech 10 Beams:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Tech 10 Torpedoes:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Building a ship:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Recycling a ship:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Repairing a ship:</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Storage.Hulls(0)+Storage.Engines(0)+Storage.Beams(0)+Storage.Launchers(0)&quot;,&quot;b2&quot;\">Have parts in storage:</a></td><td align=\"right\"><font color=\"green\">3</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"20\"><font color=\"white\">Ships Being Built</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td>(none)</td><td align=\"right\"/></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleStarships()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarshipPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::StarshipPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starships</h1><table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">37</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Orbit$=0 And Owner$=My.Race$&quot;,&quot;s2&quot;\">In free space:</a></td><td align=\"right\"><font color=\"green\">15</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Type.Short=&quot;T&quot; And Owner$=My.Race$\',&quot;s2&quot;\">Torpedo Ships:</a></td><td align=\"right\"><font color=\"green\">23</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Cargo.N=0 And Owner$=My.Race$&quot;,&quot;s2&quot;\">Ships w/o fuel:</a></td><td align=\"right\"><font color=\"green\">3</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"20\"><font color=\"white\">Ships by Hull Type</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=60&quot;,&quot;s2&quot;\">RUBY CLASS LIGHT CRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=61&quot;,&quot;s2&quot;\">EMERALD CLASS BATTLECRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=15&quot;,&quot;s2&quot;\">SMALL DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=16&quot;,&quot;s2&quot;\">MEDIUM DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=17&quot;,&quot;s2&quot;\">LARGE DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=14&quot;,&quot;s2&quot;\">NEUTRONIC FUEL CARRIER</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=46&quot;,&quot;s2&quot;\">METEOR CLASS BLOCKADE RUNNER</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=63&quot;,&quot;s2&quot;\">DIAMOND FLAME CLASS BATTLESHIP</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=66&quot;,&quot;s2&quot;\">OPAL CLASS TORPEDO BOAT</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleStarshipsByHullName()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.setPageOptions(game::map::info::StarshipPage, game::map::info::Ships_SortByName | game::map::info::Ships_HideTop);
    h.browser.renderPage(game::map::info::StarshipPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::StarshipPage), game::map::info::Ships_SortByName | game::map::info::Ships_HideTop);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starships</h1><table align=\"left\"><tr><td width=\"20\"><font color=\"white\">Ships by Hull Type</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=63&quot;,&quot;s2&quot;\">DIAMOND FLAME CLASS BATTLESHIP</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=61&quot;,&quot;s2&quot;\">EMERALD CLASS BATTLECRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=17&quot;,&quot;s2&quot;\">LARGE DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=16&quot;,&quot;s2&quot;\">MEDIUM DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=46&quot;,&quot;s2&quot;\">METEOR CLASS BLOCKADE RUNNER</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=14&quot;,&quot;s2&quot;\">NEUTRONIC FUEL CARRIER</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=66&quot;,&quot;s2&quot;\">OPAL CLASS TORPEDO BOAT</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=60&quot;,&quot;s2&quot;\">RUBY CLASS LIGHT CRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=15&quot;,&quot;s2&quot;\">SMALL DEEP SPACE FREIGHTER</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleCapital()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::CapitalPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::CapitalPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Capital Ships</h1><table align=\"left\"><tr><td width=\"17\">Total:</td><td align=\"right\" width=\"3\"><font color=\"green\">23</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Orbit$=0 And Owner$=My.Race$ And Type.Short&lt;&gt;&quot;F&quot;\',&quot;s2&quot;\">In free space:</a></td><td align=\"right\"><font color=\"green\">6</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Type.Short=&quot;T&quot; And Owner$=My.Race$ And Type.Short&lt;&gt;&quot;F&quot;\',&quot;s2&quot;\">Torpedo Ships:</a></td><td align=\"right\"><font color=\"green\">23</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Cargo.N=0 And Owner$=My.Race$ And Type.Short&lt;&gt;&quot;F&quot;\',&quot;s2&quot;\">Ships w/o fuel:</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"20\"><font color=\"white\">Ships by Hull Type</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=60&quot;,&quot;s2&quot;\">RUBY CLASS LIGHT CRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=61&quot;,&quot;s2&quot;\">EMERALD CLASS BATTLECRUISER</a></td><td align=\"right\"><font color=\"green\">10</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=46&quot;,&quot;s2&quot;\">METEOR CLASS BLOCKADE RUNNER</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=63&quot;,&quot;s2&quot;\">DIAMOND FLAME CLASS BATTLESHIP</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Hull$=66&quot;,&quot;s2&quot;\">OPAL CLASS TORPEDO BOAT</a></td><td align=\"right\"><font color=\"green\">1</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleStarchart()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::StarchartPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::StarchartPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Starchart</h1><table align=\"left\"><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">35</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Range:</td><td><font color=\"green\">561 ly from 1013 to 1573</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Range:</td><td><font color=\"green\">561 ly from 2247 to 2807</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">37</font></td></tr>"
                     "<tr><td>Unowned Planets:</td><td><font color=\"green\">1</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">500</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"10\"><font color=\"white\">Foreign Units</font></td><td align=\"right\" width=\"7\">Ships</td><td width=\"3\">(hist.)</td><td align=\"right\" width=\"7\">Planets</td><td width=\"3\">(hist.)</td><td align=\"right\" width=\"7\">Minefields</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=2&quot;,&quot;spo2&quot;\">The Second</a></td><td align=\"right\"><font color=\"green\">4</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=5&quot;,&quot;spo2&quot;\">Player 5</a></td><td align=\"right\"><font color=\"green\">2</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=9&quot;,&quot;spo2&quot;\">Player 9</a></td><td align=\"right\"><font color=\"green\">3</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total:</td><td align=\"right\"><font color=\"green\">9</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td><td><font color=\"green\"></font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 Visual Contacts:</td><td align=\"right\"><font color=\"green\">9</font></td><td align=\"right\"/><td/><td align=\"right\"/><td/></tr></table>"
                     "<table align=\"left\"><tr><td>Universal Minefield FCode:</td><td><font color=\"green\">none</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleWeapons()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.renderPage(game::map::info::WeaponsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::WeaponsPage), 0);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Weapons</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Beams</font></td><td align=\"right\" width=\"4\">Ships</td><td align=\"right\" width=\"8\">Weapons</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=0&quot;,&quot;s2&quot;\">No beams</a></td><td align=\"right\"><font color=\"green\">14</font></td><td align=\"right\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=2&quot;,&quot;s2&quot;\">X-Ray Laser</a></td><td align=\"right\"><font color=\"green\">3</font></td><td align=\"right\"><font color=\"green\">12</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=3&quot;,&quot;s2&quot;\">Plasma Bolt</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">8</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=4&quot;,&quot;s2&quot;\">Blaster</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=5&quot;,&quot;s2&quot;\">Positron Beam</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=6&quot;,&quot;s2&quot;\">Disruptor</a></td><td align=\"right\"><font color=\"green\">16</font></td><td align=\"right\"><font color=\"green\">97</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Beam$=7&quot;,&quot;s2&quot;\">Heavy Blaster</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">10</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Torpedoes</font></td><td align=\"right\" width=\"4\">Ships</td><td align=\"right\" width=\"8\">Torpedoes</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And IsEmpty(Aux)&quot;,&quot;s2&quot;\">No torps/fighters</a></td><td align=\"right\"><font color=\"green\">14</font></td><td align=\"right\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=1&quot;,&quot;s2&quot;\">Mark 1 Photon</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=2&quot;,&quot;s2&quot;\">Proton torp</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=4&quot;,&quot;s2&quot;\">Gamma Bomb</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">45</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=6&quot;,&quot;s2&quot;\">Mark 4 Photon</a></td><td align=\"right\"><font color=\"green\">17</font></td><td align=\"right\"><font color=\"green\">204</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=7&quot;,&quot;s2&quot;\">Mark 5 Photon</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr></table>"
                     "<table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Others</font></td><td align=\"right\" width=\"4\"/></tr>"
                     "<tr><td>Carriers</td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Unarmed ships</td><td align=\"right\"><font color=\"green\">14</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testSampleWeaponsOnlyTorps()
{
    TestHarness h;
    createTurn(h);
    populateTurn(h);

    Nodes_t out;
    h.browser.setPageOptions(game::map::info::WeaponsPage, game::map::info::Weapons_ShowOnlyTorpedoes);
    h.browser.renderPage(game::map::info::WeaponsPage, out);
    TS_ASSERT_EQUALS(h.browser.getPageOptions(game::map::info::WeaponsPage), game::map::info::Weapons_ShowOnlyTorpedoes);

    TS_ASSERT_EQUALS(toString(out),
                     "<h1>Weapons</h1><table align=\"left\"><tr><td width=\"16\"><font color=\"white\">Torpedoes</font></td><td align=\"right\" width=\"4\">Ships</td><td align=\"right\" width=\"8\">Torpedoes</td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And IsEmpty(Aux)&quot;,&quot;s2&quot;\">No torps/fighters</a></td><td align=\"right\"><font color=\"green\">14</font></td><td align=\"right\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=1&quot;,&quot;s2&quot;\">Mark 1 Photon</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=2&quot;,&quot;s2&quot;\">Proton torp</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Mark 2 Photon</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=4&quot;,&quot;s2&quot;\">Gamma Bomb</a></td><td align=\"right\"><font color=\"green\">2</font></td><td align=\"right\"><font color=\"green\">45</font></td></tr>"
                     "<tr><td>Mark 3 Photon</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=6&quot;,&quot;s2&quot;\">Mark 4 Photon</a></td><td align=\"right\"><font color=\"green\">17</font></td><td align=\"right\"><font color=\"green\">204</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Owner$=My.Race$ And Torp$=7&quot;,&quot;s2&quot;\">Mark 5 Photon</a></td><td align=\"right\"><font color=\"green\">1</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Mark 6 Photon</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Mark 7 Photon</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Mark 8 Photon</td><td align=\"right\"><font color=\"green\">0</font></td><td align=\"right\"><font color=\"green\">0</font></td></tr></table>");
}

void
TestGameMapInfoBrowser::testOptionsTotals()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::TotalsPage, out);

    TS_ASSERT(out.empty());
}

void
TestGameMapInfoBrowser::testOptionsMinerals()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::MineralsPage, out);

    TS_ASSERT(hasOption(out, "Sort by mined amount", game::map::info::Minerals_SortByMined));
    TS_ASSERT(hasOption(out, "Sort by total amount", game::map::info::Minerals_SortByTotal));
    TS_ASSERT(hasOption(out, "Show only Tritanium", game::map::info::Minerals_ShowOnlyT));
}

void
TestGameMapInfoBrowser::testOptionsMineralsMinedT()
{
    TestHarness h;
    util::StringList out;
    h.browser.setPageOptions(game::map::info::MineralsPage, game::map::info::Minerals_ShowOnlyT | game::map::info::Minerals_SortByMined);
    h.browser.renderPageOptions(game::map::info::MineralsPage, out);

    TS_ASSERT(hasOption(out, "Sort by mined amount", game::map::info::Minerals_ShowOnlyT | game::map::info::Minerals_SortByMined));
    TS_ASSERT(hasOption(out, "Sort by total amount", game::map::info::Minerals_ShowOnlyT | game::map::info::Minerals_SortByTotal));
    TS_ASSERT(hasOption(out, "Show only Tritanium",  game::map::info::Minerals_ShowOnlyT | game::map::info::Minerals_SortByMined));
    TS_ASSERT(hasOption(out, "Show only Molybdenum", game::map::info::Minerals_ShowOnlyM | game::map::info::Minerals_SortByMined));
}

void
TestGameMapInfoBrowser::testOptionsPlanets()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::PlanetsPage, out);

    TS_ASSERT(hasOption(out, "Sort by native race", game::map::info::Planets_SortByRace));
}

void
TestGameMapInfoBrowser::testOptionsColony()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::ColonyPage, out);

    TS_ASSERT(hasOption(out, "Show only Supplies", game::map::info::Colony_ShowOnlySupplies));
}

void
TestGameMapInfoBrowser::testOptionsStarbases()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::StarbasePage, out);

    TS_ASSERT(hasOption(out, "Show only ship list", game::map::info::Ships_HideTop));
    TS_ASSERT(hasOption(out, "Sort ships by name", game::map::info::Ships_SortByName));
    TS_ASSERT(hasOption(out, "Sort ships by hull mass", game::map::info::Ships_SortByMass));
}

void
TestGameMapInfoBrowser::testOptionsStarbasesShipsByMass()
{
    TestHarness h;
    util::StringList out;
    h.browser.setPageOptions(game::map::info::StarbasePage, game::map::info::Ships_HideTop | game::map::info::Ships_SortByMass);
    h.browser.renderPageOptions(game::map::info::StarbasePage, out);

    TS_ASSERT(hasOption(out, "Show all info",                                            game::map::info::Ships_SortByMass));
    TS_ASSERT(hasOption(out, "Sort ships by name",      game::map::info::Ships_HideTop | game::map::info::Ships_SortByName));
    TS_ASSERT(hasOption(out, "Sort ships by hull mass", game::map::info::Ships_HideTop | game::map::info::Ships_SortByMass));
}

void
TestGameMapInfoBrowser::testOptionsStarships()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::StarshipPage, out);

    TS_ASSERT(hasOption(out, "Show only hull list", game::map::info::Ships_HideTop));
    TS_ASSERT(hasOption(out, "Sort ships by name", game::map::info::Ships_SortByName));
    TS_ASSERT(hasOption(out, "Sort ships by hull mass", game::map::info::Ships_SortByMass));
}

void
TestGameMapInfoBrowser::testOptionsStarshipsByMass()
{
    TestHarness h;
    util::StringList out;
    h.browser.setPageOptions(game::map::info::StarshipPage, game::map::info::Ships_HideTop | game::map::info::Ships_SortByMass);
    h.browser.renderPageOptions(game::map::info::StarshipPage, out);

    TS_ASSERT(hasOption(out, "Show all info",                                            game::map::info::Ships_SortByMass));
    TS_ASSERT(hasOption(out, "Sort ships by name",      game::map::info::Ships_HideTop | game::map::info::Ships_SortByName));
    TS_ASSERT(hasOption(out, "Sort ships by hull mass", game::map::info::Ships_HideTop | game::map::info::Ships_SortByMass));
}

void
TestGameMapInfoBrowser::testOptionsCapital()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::CapitalPage, out);

    TS_ASSERT(hasOption(out, "Show only hull list", game::map::info::Ships_HideTop));
    TS_ASSERT(hasOption(out, "Sort ships by name", game::map::info::Ships_SortByName));
    TS_ASSERT(hasOption(out, "Sort ships by hull mass", game::map::info::Ships_SortByMass));
}

void
TestGameMapInfoBrowser::testOptionsStarchart()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::StarchartPage, out);

    TS_ASSERT(out.empty());
}


void
TestGameMapInfoBrowser::testOptionsWeapons()
{
    TestHarness h;
    util::StringList out;
    h.browser.renderPageOptions(game::map::info::WeaponsPage, out);

    TS_ASSERT(hasOption(out, "Show only beams", game::map::info::Weapons_ShowOnlyBeams));
    TS_ASSERT(hasOption(out, "Show all info", 0));
}
