/**
  *  \file game/sim/consoleapplication.cpp
  *  \brief Class game::sim::ConsoleApplication
  */

#include <cstring>
#include "game/sim/consoleapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "afl/sys/time.hpp"
#include "game/limits.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/loader.hpp"
#include "game/sim/object.hpp"
#include "game/sim/parallelrunner.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/resultlist.hpp"
#include "game/sim/run.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/simplerunner.hpp"
#include "game/specificationloader.hpp"
#include "game/v3/rootloader.hpp"
#include "util/charsetfactory.hpp"
#include "util/stopsignal.hpp"
#include "util/string.hpp"
#include "version.hpp"
#include "game/exception.hpp"

using afl::base::Optional;
using afl::base::Ptr;
using afl::base::Ref;
using afl::except::CommandLineException;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using game::sim::Configuration;

namespace {
    /* Match a boolean parameter ("--foo" or "--no-foo").
       Returns true on success, value of the parameter in value. */
    bool matchBoolParameter(afl::sys::CommandLineParser& p,
                            const String_t& text,
                            const char* name,
                            bool& value,
                            afl::string::Translator& tx)
    {
        if (text == name) {
            // --foo or --foo=N
            String_t param;
            if (p.getFlags().contains(afl::sys::CommandLineParser::HasParameter) && p.getParameter(param)) {
                // --foo=N
                if (param == "0" || param == "off" || param == "no" || param == "false") {
                    value = false;
                } else if (param == "1" || param == "on" || param == "yes" || param == "true") {
                    value = true;
                } else {
                    throw CommandLineException(Format(tx("parameter to '--%s' must be 'yes' or 'no'"), text));
                }
            } else {
                // --foo
                value = true;
            }
            return true;
        } else {
            size_t nameLen = std::strlen(name);
            if (text.size() == 3+nameLen
                && text.compare(0, 3, "no-") == 0
                && text.compare(3, nameLen, name) == 0)
            {
                // --no-foo
                value = false;
                return true;
            } else {
                // not a match
                return false;
            }
        }
    }

    Configuration::VcrMode parseVcrMode(const String_t& value, const String_t& text, afl::string::Translator& tx)
    {
        if (value == "host") {
            return Configuration::VcrHost;
        } else if (value == "phost2") {
            return Configuration::VcrPHost2;
        } else if (value == "phost3") {
            return Configuration::VcrPHost3;
        } else if (value == "phost4") {
            return Configuration::VcrPHost4;
        } else if (value == "flak") {
            return Configuration::VcrFLAK;
        } else if (value == "nuhost") {
            return Configuration::VcrNuHost;
        } else {
            throw CommandLineException(Format(tx("parameter '%s' to '--%s' is not valid"), value, text));
        }
    }

    Configuration::BalancingMode parseBalancingMode(const String_t& value, const String_t& text, afl::string::Translator& tx)
    {
        if (value == "360") {
            return Configuration::Balance360k;
        } else if (value == "no" || value == "none" || value == "off") {
            return Configuration::BalanceNone;
        } else if (value == "master") {
            return Configuration::BalanceMasterAtArms;
        } else {
            throw CommandLineException(Format(tx("parameter '%s' to '--%s' is not valid"), value, text));
        }
    }

    void writeScalar(afl::io::TextWriter& out, String_t name, int value)
    {
        out.writeLine(Format("  %s: %d", name, value));
    }

    void writeItem(afl::io::TextWriter& out, String_t name, const game::sim::UnitResult::Item& item, const game::sim::ResultList& resultList)
    {
        out.writeLine(Format("  %s: %.1f (%d..%d)",
                             name,
                             double(item.totalScaled) / resultList.getCumulativeWeight(),
                             item.min,
                             item.max));
    }
}

/*
 *  Parameter structure
 */
struct game::sim::ConsoleApplication::Parameters {
    bool hadAction;
    Optional<String_t> saveFileName;                       // -o
    bool enableReport;                                     // -r
    bool enableVerify;                                     // --verify
    Optional<String_t> gameDirectoryName;                  // -G
    Optional<String_t> rootDirectoryName;                  // -R
    size_t numThreads;                                     // -j
    Optional<String_t> charsetName;                        // -C
    Optional<size_t> runSimCount;                          // --run
    bool runSimSeries;                                     // --run-series
    Optional<Configuration::VcrMode> vcrMode;              // --mode
    Optional<int> engineShieldBonus;                       // --esb
    Optional<bool> scottyBonus;                            // --scotty
    Optional<bool> randomLeftRight;                        // --random-sides
    Optional<bool> honorAlliances;                         // --alliances
    Optional<bool> onlyOneSimulation;                      // --one
    Optional<bool> seedControl;                            // --seed-control
    Optional<bool> randomizeFCodesOnEveryFight;            // --random-fc
    Optional<Configuration::BalancingMode> balancingMode;  // --balance
    Optional<uint32_t> seed;                               // --seed
    std::vector<String_t> loadFileNames;                   // file names

    Parameters()
        : hadAction(false), saveFileName(), enableReport(false), enableVerify(false), gameDirectoryName(), rootDirectoryName(),
          numThreads(0), charsetName(), runSimCount(), runSimSeries(false),
          vcrMode(), engineShieldBonus(), scottyBonus(), randomLeftRight(),
          honorAlliances(), onlyOneSimulation(), seedControl(), randomizeFCodesOnEveryFight(),
          balancingMode(), loadFileNames()
        { }
};

/*
 *  Session structure
 *
 *  We load the root/shipList only when needed.
 */

struct game::sim::ConsoleApplication::Session {
    Ptr<Root> root;
    Ptr<game::spec::ShipList> shipList;
};


/*
 *  ConsoleApplication class
 */

game::sim::ConsoleApplication::ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs),
      m_verbose(true)
{
    consoleLogger().setConfiguration("*@Error=raw:*=hide", translator());
}

void
game::sim::ConsoleApplication::appMain()
{
    afl::string::Translator& tx = translator();

    // Parse command line
    Parameters p;
    parseCommandLine(p);

    // Detect unintended use
    if (p.loadFileNames.empty()) {
        errorExit(tx("no input files specified"));
    }
    if (!p.hadAction) {
        errorExit(tx("no action specified"));
    }

    // Build character set
    std::auto_ptr<afl::charset::Charset> cs;
    if (const String_t* charsetName = p.charsetName.get()) {
        cs.reset(util::CharsetFactory().createCharset(*charsetName));
        if (!cs.get()) {
            errorExit(tx("the specified character set is not known"));
        }
    } else {
        cs.reset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    }

    // Load
    Setup setup;
    loadSetup(setup, *cs, p.loadFileNames);

    // Save
    if (const String_t* saveFileName = p.saveFileName.get()) {
        saveSetup(setup, *cs, *saveFileName);
    }

    // Verify
    Session session;
    if (p.enableVerify) {
        loadSession(session, p, *cs);
        verifySetup(setup, session);
    }

    // Report
    if (p.enableReport) {
        loadSession(session, p, *cs);
        showSetup(setup, session);
    }

    // Sim
    if (p.runSimSeries || p.runSimCount.isValid()) {
        loadSession(session, p, *cs);
        runSimulation(setup, session, p);
    }
}

void
game::sim::ConsoleApplication::parseCommandLine(Parameters& p)
{
    afl::string::Translator& tx = translator();

    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
    String_t text;
    bool option;
    bool flag;

    while (parser.getNext(option, text)) {
        if (option) {
            if (text == "h" || text == "help") {
                help();
            } else if (text == "o" || text == "save") {
                // Output file; in mergeccb this is '-c' with a slightly different syntax
                p.saveFileName = parser.getRequiredParameter(text);
                p.hadAction = true;
            } else if (text == "r" || text == "report") {
                // Report; in mergeccb this is '-r'
                p.enableReport = true;
                p.hadAction = true;
            } else if (text == "verify") {
                p.enableVerify = true;
                p.hadAction = true;
            } else if (text == "G" || text == "game") {
                p.gameDirectoryName = parser.getRequiredParameter(text);
            } else if (text == "R" || text == "root") {
                p.rootDirectoryName = parser.getRequiredParameter(text);
            } else if (text == "j" || text == "jobs") {
                String_t param = parser.getRequiredParameter(text);
                if (!afl::string::strToInteger(param, p.numThreads)) {
                    errorExit(Format(tx("invalid number of threads, '%s'"), param));
                }
            } else if (text == "C" || text == "charset") {
                p.charsetName = parser.getRequiredParameter(text);
            } else if (text == "q") {
                m_verbose = false;
            } else if (text == "log") {
                try {
                    consoleLogger().setConfiguration(parser.getRequiredParameter(text), tx);
                }
                catch (std::exception& e) {
                    errorExit(tx("parameter to '--log' is not valid"));
                }
            } else if (text == "run") {
                String_t param = parser.getRequiredParameter(text);
                size_t n = 0;
                if (!afl::string::strToInteger(param, n)) {
                    errorExit(Format(tx("invalid number of simulations, '%s'"), param));
                }
                p.runSimCount = n;
                p.hadAction = true;
            } else if (text == "run-series") {
                p.runSimSeries = true;
                p.hadAction = true;
            } else if (text == "mode") {
                p.vcrMode = parseVcrMode(parser.getRequiredParameter(text), text, tx);
            } else if (text == "esb") {
                String_t param = parser.getRequiredParameter(text);
                int n = 0;
                if (!afl::string::strToInteger(param, n) || n < 0 || n > 10000) {
                    errorExit(Format(tx("invalid engine/shield bonus, '%s'"), param));
                }
                p.engineShieldBonus = n;
            } else if (matchBoolParameter(parser, text, "scotty", flag, tx)) {
                p.scottyBonus = flag;
            } else if (matchBoolParameter(parser, text, "random-sides", flag, tx)) {
                p.randomLeftRight = flag;
            } else if (matchBoolParameter(parser, text, "alliances", flag, tx)) {
                p.honorAlliances = flag;
            } else if (matchBoolParameter(parser, text, "one", flag, tx)) {
                p.onlyOneSimulation = flag;
            } else if (matchBoolParameter(parser, text, "seed-control", flag, tx)) {
                p.seedControl = flag;
            } else if (matchBoolParameter(parser, text, "random-fc", flag, tx)) {
                p.randomizeFCodesOnEveryFight = flag;
            } else if (text == "balance") {
                p.balancingMode = parseBalancingMode(parser.getRequiredParameter(text), text, tx);
            } else if (text == "seed") {
                String_t param = parser.getRequiredParameter(text);
                uint32_t n = 0;
                if (!afl::string::strToInteger(param, n)) {
                    errorExit(Format(tx("invalid seed, '%s'"), param));
                }
                p.seed = n;
            } else {
                errorExit(Format(tx("invalid option '%s' specified. Use '%s -h' for help."), text, environment().getInvocationName()));
            }
        } else {
            p.loadFileNames.push_back(text);
        }
    }
}

void
game::sim::ConsoleApplication::help()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Battle Simulation Utility v%s - (c) 2020-2022 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-opts] FILE.ccb...\n\n"
                            "Utility will load all .ccb files, combine them, and work on the result.\n\n"
                            "%s\n"
                            "Report bugs to <Streu@gmx.de>").c_str(),
                         environment().getInvocationName(),
                         util::formatOptions(tx("Actions (at least one):\n"
                                                "--save/-o OUT.ccb\tSave combined .ccb file\n"
                                                "--report/-r\tReport ships\n"
                                                "--verify\tVerify simulation against ship list\n"
                                                "--run N\tRun N simulations\n"
                                                "--run-series\tRun a series\n"
                                                "\n"
                                                "Options:\n"
                                                "--game/-G DIR\tGame directory\n"
                                                "--root/-R DIR\tRoot directory\n"
                                                "--charset/-C CS\tSet game character set\n"
                                                "-q\tDo not show progress messages\n"
                                                "--log CONFIG\tConfigure log output\n"
                                                "\n"
                                                "Simulation options:\n"
                                                "--jobs/-j N\tSet number of threads for simulation\n"
                                                "--mode=MODE\tSet mode (host, phost[2-4], flak, nuhost)\n"
                                                "--esb=N\tSet engine-shield bonus\n"
                                                "--[no-]scotty\tScotty bonus\n"
                                                "--[no-]random-sides\tRandom left/right\n"
                                                "--[no-]alliances\tHonor alliances\n"
                                                "--[no-]one\tOnly one simulation\n"
                                                "--[no-]seed-control\tSeed control\n"
                                                "--[no-]random-fc\tRandom friendly codes on every fight\n"
                                                "--balance=MODE\tSet balancing mode (none, 360, master)\n"
                                                "--seed=N\tSet random-number seed\n"))));
    out.flush();
    exit(0);
}

void
game::sim::ConsoleApplication::loadSetup(Setup& setup, afl::charset::Charset& charset, const std::vector<String_t>& loadFileNames)
{
    // ex mergeccb.pas:LoadFile
    for (size_t i = 0, n = loadFileNames.size(); i < n; ++i) {
        // Open file
        Ref<Stream> file = fileSystem().openFile(loadFileNames[i], FileSystem::OpenRead);

        // Load into temporary setup
        Setup fileSetup;
        game::sim::Loader(charset, translator()).load(*file, fileSetup);
        if (m_verbose) {
            standardOutput().writeLine(Format(translator()("Loaded %s (%d unit%!1{s%})"), loadFileNames[i], fileSetup.getNumObjects()));
        }

        // Merge
        setup.merge(fileSetup);
    }
}

void
game::sim::ConsoleApplication::saveSetup(const Setup& setup, afl::charset::Charset& charset, const String_t& saveFileName)
{
    // ex mergeccb.pas:SaveFile
    Ref<Stream> file = fileSystem().openFile(saveFileName, FileSystem::Create);
    game::sim::Loader(charset, translator()).save(*file, setup);
    if (m_verbose) {
        standardOutput().writeLine(Format(translator()("Saved %s (%d unit%!1{s%})"), saveFileName, setup.getNumObjects()));
    }
}

void
game::sim::ConsoleApplication::loadSession(Session& session, const Parameters& params, afl::charset::Charset& charset)
{
    if (session.root.get() == 0) {
        // Environment
        afl::sys::Environment& env = environment();
        FileSystem& fs = fileSystem();
        afl::string::Translator& tx = translator();

        // Directories
        String_t defaultRoot = fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs");
        util::ProfileDirectory profile(environment(), fileSystem(), translator(), log());
        game::v3::RootLoader loader(fs.openDirectory(params.rootDirectoryName.orElse(defaultRoot)), &profile, 0 /* pCallback */, tx, consoleLogger(), fs);

        // Load root
        const game::config::UserConfiguration uc;
        session.root = loader.load(fs.openDirectory(fs.getAbsolutePathName(params.gameDirectoryName.orElse("."))), charset, uc, true);
        if (session.root.get() == 0) {
            // This should not happen because we pass forceEmpty=true
            errorExit(tx("no game data found"));
        }

        // Load spec
        bool ok = false;
        session.shipList = new game::spec::ShipList();
        session.root->specificationLoader().loadShipList(*session.shipList, *session.root, makeResultTask(ok))->call();
        if (!ok) {
            throw Exception(tx("unable to load ship list"));
        }
    }
}

void
game::sim::ConsoleApplication::verifySetup(const Setup& setup, const Session& session)
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();

    if (!setup.isMatchingShipList(*session.shipList)) {
        errorExit(tx("simulation does not match ship list"));
    } else {
        if (m_verbose) {
            out.writeLine(tx("Verification succeeded"));
        }
    }
}

void
game::sim::ConsoleApplication::showSetup(const Setup& setup, const Session& session)
{
    // ex mergeccb.pas:Report
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();

    out.writeLine(tx("ID# Pl  Type             Name             Eng  Beams   T/F  Ammo  Dam%  Lvl"));
    out.writeLine(   "--- --  ---------------  ---------------  ---  -----  ----- ----  ----  ---");
    for (size_t i = 0, n = setup.getNumShips(); i < n; ++i) {
        if (const Ship* sh = setup.getShip(i)) {
            String_t hullName;
            if (sh->getHullType() == 0) {
                hullName = tx("custom");
            } else if (const game::spec::Hull* pHull = session.shipList->hulls().get(sh->getHullType())) {
                hullName = pHull->getName(session.shipList->componentNamer());
            } else {
                hullName = Format("#%d", sh->getHullType());
            }

            String_t line = Format("%3d %2d  %-15.15s  %-15.15s  %3d  ") << sh->getId() << sh->getOwner() << hullName << sh->getName() << sh->getEngineType();
            if (sh->getNumBeams() != 0) {
                line += Format("%2dx%-2d", sh->getNumBeams(), sh->getBeamType());
            } else {
                line += "  -  ";
            }
            line += "  ";
            if (sh->getNumLaunchers() != 0) {
                line += Format("%2dx%-2d", sh->getNumLaunchers(), sh->getTorpedoType());
            } else if (sh->getNumBays() != 0) {
                line += Format("%2d FB", sh->getNumBays());
            } else {
                line += "  -  ";
            }
            line += Format(" %4d  %3d%%  %2d", sh->getAmmo(), sh->getDamage(), sh->getExperienceLevel());
            out.writeLine(line);
        }
    }
}

void
game::sim::ConsoleApplication::runSimulation(Setup& setup, const Session& session, const Parameters& params)
{
    // Build configuration
    Configuration opts;
    if (const Configuration::VcrMode* vcrMode = params.vcrMode.get()) {
        TeamSettings team;             // FIXME: configurable
        opts.setMode(*vcrMode, team, session.root->hostConfiguration());
    }
    if (const int* engineShieldBonus = params.engineShieldBonus.get()) {
        opts.setEngineShieldBonus(*engineShieldBonus);
    }
    if (const bool* scottyBonus = params.scottyBonus.get()) {
        opts.setScottyBonus(*scottyBonus);
    }
    if (const bool* randomLeftRight = params.randomLeftRight.get()) {
        opts.setRandomLeftRight(*randomLeftRight);
    }
    if (const bool* honorAlliances = params.honorAlliances.get()) {
        opts.setHonorAlliances(*honorAlliances);
    }
    if (const bool* onlyOneSimulation = params.onlyOneSimulation.get()) {
        opts.setOnlyOneSimulation(onlyOneSimulation);
    }
    if (const bool* seedControl = params.seedControl.get()) {
        opts.setSeedControl(*seedControl);
    }
    if (const bool* randomizeFCodesOnEveryFight = params.randomizeFCodesOnEveryFight.get()) {
        opts.setRandomizeFCodesOnEveryFight(*randomizeFCodesOnEveryFight);
    }
    if (const Configuration::BalancingMode* balancingMode = params.balancingMode.get()) {
        opts.setBalancingMode(*balancingMode);
    }

    // Build RNG
    util::RandomNumberGenerator rng(params.seed.orElse(afl::sys::Time::getTickCounter()));
    game::sim::prepareSimulation(setup, opts, rng);

    // Build runner
    std::auto_ptr<Runner> runner;
    if (params.numThreads <= 1) {
        runner.reset(new SimpleRunner(setup, opts, *session.shipList, session.root->hostConfiguration(), session.root->flakConfiguration(), consoleLogger(), rng));
    } else {
        runner.reset(new ParallelRunner(setup, opts, *session.shipList, session.root->hostConfiguration(), session.root->flakConfiguration(), consoleLogger(), rng, params.numThreads));
    }

    // Run first sim
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    if (!runner->init()) {
        out.writeLine(tx("Simulation did not produce any battles."));
        return;
    }

    // Run more sims
    util::StopSignal sig;
    if (params.runSimSeries) {
        runner->run(runner->makeSeriesLimit(), sig);
    } else {
        size_t n = params.runSimCount.orElse(0);
        if (n > 1) {
            runner->run(runner->makeFiniteLimit(n-1), sig);
        }
    }

    // Show results
    out.writeLine(Format(tx("Results after %d simulation%!1{s%}"), runner->resultList().getNumBattles()));
    out.writeLine();
    showClassResults(setup, session, runner->resultList());
    showUnitResults(setup, session, runner->resultList());
}

void
game::sim::ConsoleApplication::showClassResults(const Setup& /*setup*/, const Session& session, const ResultList& resultList)
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();

    out.writeLine(tx("Class Results\n"
                     "-------------\n"));
    for (size_t i = 0, n = resultList.getNumClassResults(); i < n; ++i) {
        if (const ClassResult* r = resultList.getClassResult(i)) {
            String_t line = Format("%7.2f%% : ", double(100.0 * r->getWeight()) / resultList.getCumulativeWeight());
            bool first = true;
            for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
                if (int numSurvivors = r->getClass().get(pl)) {
                    if (first) {
                        first = false;
                    } else {
                        line += ", ";
                    }
                    line += Format("%d x %s", numSurvivors, session.root->playerList().getPlayerName(pl, Player::AdjectiveName, tx));
                }
            }
            if (first) {
                line += tx("none");
            }
            out.writeLine(line);
        }
    }
    out.writeLine();
}

void
game::sim::ConsoleApplication::showUnitResults(const Setup& setup, const Session& /*session*/, const ResultList& resultList)
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();

    out.writeLine(tx("Unit Results\n"
                     "------------\n"));

    for (size_t i = 0, n = resultList.getNumUnitResults(); i < n; ++i) {
        const Object* obj = setup.getObject(i);
        const UnitResult* r = resultList.getUnitResult(i);
        if (obj && r) {
            out.writeLine(Format("%s (#%d):", obj->getName(), obj->getId()));
            writeScalar(out, tx("Survived"), r->getNumFightsWon());
            writeScalar(out, tx("Fought"),   r->getNumFights());
            writeScalar(out, tx("Captured"), r->getNumCaptures());

            // Formatting logic taken from WSimUnitStat::render
            // FIXME: use ResultList::describeUnitResult
            writeItem(out, tx("Damage taken"), r->getDamage(), resultList);
            writeItem(out, tx("Shields"), r->getShield(), resultList);
            if (/**const Planet* pl =*/ dynamic_cast<const Planet*>(obj)) {
                writeItem(out, tx("Defense Lost"), r->getCrewLeftOrDefenseLost(), resultList);
                writeItem(out, tx("SB Ftrs Lost"), r->getNumFightersLost(), resultList);
                if (r->getNumFights() != 0) {
                    writeItem(out, tx("Min Ftr aboard"), r->getMinFightersAboard(), resultList);
                }
            }
            if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
                writeItem(out, tx("Crew Left"), r->getCrewLeftOrDefenseLost(), resultList);
                if (sh->getNumBays() != 0) {
                    writeItem(out, tx("Fighters Lost"), r->getNumFightersLost(), resultList);
                    writeItem(out, tx("Fighters Left"), UnitResult::Item(r->getNumFightersLost(), sh->getAmmo(), resultList.getCumulativeWeight()), resultList);
                    writeItem(out, tx("Min Ftr aboard"), r->getMinFightersAboard(), resultList);
                } else {
                    writeItem(out, tx("Torps Launched"), r->getNumTorpedoesFired(), resultList);
                    writeItem(out, tx("Torps Left"), UnitResult::Item(r->getNumTorpedoesFired(), sh->getAmmo(), resultList.getCumulativeWeight()), resultList);
                    writeItem(out, tx("Torps Hit"), r->getNumTorpedoHits(), resultList);
                }
            }
        }
        out.writeLine();
    }
}
