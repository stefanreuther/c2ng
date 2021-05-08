/**
  *  \file game/v3/specificationloader.cpp
  */

#include <algorithm>
#include <vector>
#include "game/v3/specificationloader.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/log.hpp"
#include "game/hostversion.hpp"
#include "game/root.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/nullcomponentnameprovider.hpp"
#include "game/v3/structures.hpp"
#include "util/configurationfileparser.hpp"
#include "util/string.hpp"

namespace gs = game::spec;
namespace gt = game::v3::structures;
using afl::io::FileSystem;

namespace {
    void unpackCost(gs::Cost& out, const gt::Cost& in)
    {
        out.set(out.Money,      in.money);
        out.set(out.Tritanium,  in.tritanium);
        out.set(out.Duranium,   in.duranium);
        out.set(out.Molybdenum, in.molybdenum);
    }

    void clearHullFunctions(gs::ComponentVector<gs::Hull>& hulls)
    {
        // ex game/spec.cc:clearSpecials
        for (int i = 1, n = hulls.size(); i <= n; ++i) {
            if (gs::Hull* h = hulls.get(i)) {
                h->clearHullFunctions();
            }
        }
    }

    // HULLFUNC.TXT parser.
    class HullfuncParser : public util::ConfigurationFileParser {
     public:
        HullfuncParser(game::spec::ShipList& shipList,
                       const game::HostVersion& host,
                       const game::config::HostConfiguration& config,
                       afl::string::Translator& tx,
                       afl::sys::LogListener& log)
            : util::ConfigurationFileParser(tx),
              m_basicFunctionId(-1),
              m_assignToHull(true),
              m_levels(game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS)),
              m_hulls(),
              m_shipList(shipList),
              m_host(host),
              m_config(config),
              m_log(log)
            { }

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line);
        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        int m_basicFunctionId;
        bool m_assignToHull;
        game::ExperienceLevelSet_t m_levels;
        std::vector<bool> m_hulls;
        game::spec::ShipList& m_shipList;
        const game::HostVersion& m_host;
        const game::config::HostConfiguration& m_config;
        afl::sys::LogListener& m_log;

        void performPlayerAssignment(const String_t& fileName, int lineNr, String_t value, bool byRace);
        void performAssignments(game::spec::ModifiedHullFunctionList::Function_t function, game::PlayerSet_t add, game::PlayerSet_t remove);
    };
}

void
HullfuncParser::handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& /*line*/)
{
    // ex HullfuncParser::assign
    if (util::stringMatch("Initialize", name)) {
        // "Initialize = Clear" or "Initialize = Default"
        if (util::stringMatch("Clear", value)) {
            clearHullFunctions(m_shipList.hulls());
        } else if (util::stringMatch("Default", value)) {
            clearHullFunctions(m_shipList.hulls());
            m_shipList.basicHullFunctions().performDefaultAssignments(m_shipList.hulls());
        } else {
            handleError(fileName, lineNr, translator()("Invalid argument to `Initialize'"));
        }
    } else if (util::stringMatch("Hull", name)) {
        // Initialize hull set
        m_hulls.clear();
        m_hulls.resize(m_shipList.hulls().size());

        // Parse
        String_t acc = value;
        do {
            String_t first = afl::string::strUCase(afl::string::strTrim(afl::string::strFirst(acc, ",")));
            if (first.size() > 0) {
                if (first[0] >= '0' && first[0] <= '9') {
                    // Number or range
                    int min = 1, max = m_shipList.hulls().size();
                    String_t::size_type pos;
                    if (util::parseRange(first, min, max, pos) || (pos > 0 && util::parseRange(first.substr(0, pos), min, max, pos))) {
                        if (min > 0 && min <= max && max <= m_shipList.hulls().size()) {
                            for (int i = min; i <= max; ++i) {
                                m_hulls[i-1] = true;
                            }
                        }
                    } else {
                        // FIXME: print a warning?
                    }
                } else {
                    // Name or '*'
                    if (first == "*") {
                        std::fill(m_hulls.begin(), m_hulls.end(), true);
                    } else {
                        const gs::ComponentVector<gs::Hull>& hulls = m_shipList.hulls();
                        gs::NullComponentNameProvider ncnp;
                        for (int i = 1, n = hulls.size(); i <= n; ++i) {
                            if (const game::spec::Hull* h = hulls.get(i)) {
                                const String_t thisHullName = afl::string::strUCase(h->getName(ncnp));
                                if (thisHullName.size() >= first.size() && thisHullName.compare(0, first.size(), first) == 0) {
                                    m_hulls[i-1] = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        } while (afl::string::strRemove(acc, ","));
    } else if (util::stringMatch("Function", name)) {
        // "Function = Number (garbage)" or "Function = Name"
        int id = -1;
        String_t::size_type pos = 0;
        if ((afl::string::strToInteger(value, id, pos) || (pos > 0 && afl::string::strToInteger(value.substr(0, pos), id))) && id >= 0) {
            m_basicFunctionId = id;
        } else if (const game::spec::BasicHullFunction* hf = m_shipList.basicHullFunctions().getFunctionByName(value, true)) {
            m_basicFunctionId = hf->getId();
        } else {
            handleError(fileName, lineNr, translator()("Invalid hull function"));
        }
    } else if (util::stringMatch("Racesallowed", name)) {
        performPlayerAssignment(fileName, lineNr, value, true);
        m_levels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);
    } else if (util::stringMatch("Playersallowed", name)) {
        performPlayerAssignment(fileName, lineNr, value, false);
        m_levels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);
    } else if (util::stringMatch("Assignto", name)) {
        m_assignToHull = !m_host.hasShipSpecificFunctions() || util::stringMatch("Hull", value);
    } else if (util::stringMatch("Level", name)) {
        // "Level = 1", "Level = 1-", "Level = 1-2"
        int min, max;
        String_t::size_type pos;
        if (afl::string::strToInteger(value, min, pos)) {
            // just a number
            max = game::MAX_EXPERIENCE_LEVELS;
        } else {
            // a number followed by something
            if (!afl::string::strToInteger(value.substr(0, pos), min)) {
                handleError(fileName, lineNr, translator()("Invalid level number"));
                return;
            }

            // what follows behind the number?
            String_t rest = afl::string::strLTrim(value.substr(pos));
            if (!rest.size()) {
                // "Level = 1" means "Level = 1-"
                max = game::MAX_EXPERIENCE_LEVELS;
            } else if (rest[0] == '-') {
                if (rest.size() == 1) {
                    // "1-"
                    max = game::MAX_EXPERIENCE_LEVELS;
                } else if (afl::string::strToInteger(rest.substr(1), max)) {
                    // "1-2", ok
                } else {
                    handleError(fileName, lineNr, translator()("Invalid level range"));
                    return;
                }
            } else {
                handleError(fileName, lineNr, translator()("Invalid level number"));
                return;
            }
        }

        // We now have a range. Is it valid?
        if (min < 0 || max > game::MAX_EXPERIENCE_LEVELS || min > max) {
            handleError(fileName, lineNr, translator()("Invalid level range"));
        }

        m_levels = game::ExperienceLevelSet_t::allUpTo(max);
        if (min != 0) {
            m_levels -= game::ExperienceLevelSet_t::allUpTo(min-1);
        }
    } else {
        handleError(fileName, lineNr, translator()("Invalid keyword"));
    }
}

void
HullfuncParser::handleError(const String_t& fileName, int lineNr, const String_t& message)
{
    m_log.write(m_log.Warn, "game.v3.specloader", fileName, lineNr, message);
}

void
HullfuncParser::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }


/** Assign special function.
    \param fileName file name
    \param lineNr line number
    \param value list of players
    \param byRace true: assign race numbers. false: assign player numbers */
void
HullfuncParser::performPlayerAssignment(const String_t& fileName, int lineNr, String_t value, bool byRace)
{
    // ex HullfuncParser::assignSpecial
    // Verify status
    if (m_basicFunctionId < 0) {
        handleError(fileName, lineNr, translator()("No function selected for assignment"));
        return;
    }
    if (m_hulls.size() <= 0) {
        handleError(fileName, lineNr, translator()("No hull selected for assignment"));
        return;
    }

    // Prepare player lists
    game::PlayerSet_t playersToAdd;
    game::PlayerSet_t playersToRemove;
    if (!m_host.hasCumulativeHullfunc()) {
        playersToRemove += game::PlayerSet_t::allUpTo(game::MAX_PLAYERS);
    }

    // Parse
    while (!value.empty()) {
        bool negate = false;
        if (value[0] == '*') {
            // "*": add all
            value.erase(0, 1);
            playersToRemove = game::PlayerSet_t();
            playersToAdd = game::PlayerSet_t::allUpTo(game::MAX_PLAYERS);
        } else if (value[0] == '-') {
            // "-": remove all, "-N": remove N
            value.erase(0, 1);
            if (value.length() > 0 && value[0] >= '0' && value[0] <= '9') {
                negate = true;
                goto digit;
            }
            playersToAdd = game::PlayerSet_t();
            playersToRemove = game::PlayerSet_t::allUpTo(game::MAX_PLAYERS);
        } else if (value[0] == '+') {
            // "+": add all, "+N": add N
            value.erase(0, 1);
            if (value.length() > 0 && value[0] >= '0' && value[0] <= '9') {
                goto digit;
            }
            playersToRemove = game::PlayerSet_t();
            playersToAdd = game::PlayerSet_t::allUpTo(game::MAX_PLAYERS);
        } else if (value[0] >= '0' && value[0] <= '9') {
         digit:
            // add or remove single race
            String_t::size_type pos = 0;
            int num = 0;
            if (afl::string::strToInteger(value, num, pos)) {
                // whole string parses as a number
                value.clear();
            } else {
                if (pos > 0 && afl::string::strToInteger(value.substr(0, pos), num)) {
                    // string starts with a number
                    value.erase(0, pos);
                } else {
                    // only happens on overflow
                    handleError(fileName, lineNr, translator()("Invalid number"));
                    return;
                }
            }

            game::PlayerSet_t set;
            if (byRace) {
                set = m_config.getPlayersOfRace(num);
            } else {
                if (num > 0 && num <= game::MAX_PLAYERS) {
                    set += num;
                } else {
                    handleError(fileName, lineNr, translator()("Invalid player number"));
                    return;
                }
            }
            if (negate) {
                playersToAdd -= set;
                playersToRemove += set;
            } else {
                playersToAdd += set;
                playersToRemove -= set;
            }
        } else {
            value.erase(0, 1);
        }
    }

    // Assign it
    game::spec::ModifiedHullFunctionList::Function_t effectiveFunction =
        m_shipList.modifiedHullFunctions().getFunctionIdFromDefinition(game::spec::HullFunction(m_basicFunctionId, m_levels));
    performAssignments(effectiveFunction, playersToAdd, playersToRemove);
}

void
HullfuncParser::performAssignments(game::spec::ModifiedHullFunctionList::Function_t function, game::PlayerSet_t add, game::PlayerSet_t remove)
{
    // ex HullfuncParser::performAssignments
    gs::ComponentVector<gs::Hull>& hulls = m_shipList.hulls();
    for (int i = 1, n = hulls.size(); i <= n; ++i) {
        if (m_hulls[i-1]) {
            if (gs::Hull* h = hulls.get(i)) {
                h->changeHullFunction(function, add, remove, m_assignToHull);
            }
        }
    }
}


game::v3::SpecificationLoader::SpecificationLoader(afl::base::Ref<afl::io::Directory> dir,
                                                   std::auto_ptr<afl::charset::Charset> charset,
                                                   afl::string::Translator& tx,
                                                   afl::sys::LogListener& log)
    : m_directory(dir),
      m_charset(charset),
      m_translator(tx),
      m_log(log)
{ }

void
game::v3::SpecificationLoader::loadShipList(game::spec::ShipList& list, game::Root& root)
{
    // ex game/spec.cc:loadSpecification
    loadBeams(list, *m_directory);
    loadLaunchers(list, *m_directory);
    loadEngines(list, *m_directory);
    loadHulls(list, *m_directory);
    loadHullAssignments(list, *m_directory);
    loadHullFunctions(list, *m_directory, root.hostVersion(), root.hostConfiguration());
    list.componentNamer().load(*m_directory, m_translator, m_log);
    loadFriendlyCodes(list, *m_directory);
    loadMissions(list, *m_directory);

    list.sig_change.raise();
}

void
game::v3::SpecificationLoader::loadBeams(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/spec.cc:loadBeams
    // Start with empty beam list
    gs::ComponentVector<gs::Beam>& beams = list.beams();
    beams.clear();

    // Load it
    afl::base::Ref<afl::io::Stream> file = dir.openFile("beamspec.dat", FileSystem::OpenRead);
    for (int i = 1; i <= gt::NUM_BEAM_TYPES; ++i) {
        gt::Beam in;
        file->fullRead(afl::base::fromObject(in));
        if (gs::Beam* out = beams.create(i)) {
            out->setName(m_charset->decode(in.name));
            unpackCost(out->cost(), in.cost);
            out->setMass(in.mass);
            out->setTechLevel(in.techLevel);
            out->setKillPower(in.killPower);
            out->setDamagePower(in.damagePower);
        }
    }
}

void
game::v3::SpecificationLoader::loadLaunchers(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/spec.cc:loadTorps
    // Start with empty torpedo list
    gs::ComponentVector<gs::TorpedoLauncher>& torps = list.launchers();
    torps.clear();

    // Load it
    afl::base::Ref<afl::io::Stream> file = dir.openFile("torpspec.dat", FileSystem::OpenRead);
    for (int i = 1; i <= gt::NUM_TORPEDO_TYPES; ++i) {
        gt::Torpedo in;
        file->fullRead(afl::base::fromObject(in));
        if (gs::TorpedoLauncher* out = torps.create(i)) {
            out->setName(m_charset->decode(in.name));
            out->torpedoCost().set(gs::Cost::Tritanium, 1);
            out->torpedoCost().set(gs::Cost::Duranium, 1);
            out->torpedoCost().set(gs::Cost::Molybdenum, 1);
            out->torpedoCost().set(gs::Cost::Money, in.torpedoCost);
            unpackCost(out->cost(), in.launcherCost);
            out->setMass(in.launcherMass);
            out->setTechLevel(in.techLevel);
            out->setKillPower(in.killPower);
            out->setDamagePower(in.damagePower);
        }
    }
}

void
game::v3::SpecificationLoader::loadEngines(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/spec.cc:loadEngines
    // Start with empty engine list
    gs::ComponentVector<gs::Engine>& engines = list.engines();
    engines.clear();

    // Load it
    afl::base::Ref<afl::io::Stream> file = dir.openFile("engspec.dat", FileSystem::OpenRead);
    for (int i = 1; i <= gt::NUM_ENGINE_TYPES; ++i) {
        gt::Engine in;
        file->fullRead(afl::base::fromObject(in));
        if (gs::Engine* out = engines.create(i)) {
            out->setName(m_charset->decode(in.name));
            unpackCost(out->cost(), in.cost);
            out->setTechLevel(in.techLevel);

            static_assert(gs::Engine::MAX_WARP == gt::NUM_WARP_FACTORS, "Identical warp factor limit");
            for (int i = 0; i < gs::Engine::MAX_WARP; ++i) {
                out->setFuelFactor(i+1, in.fuelFactors[i]);
            }
        }
    }
}

void
game::v3::SpecificationLoader::loadHulls(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/spec.cc:loadHulls (part)
    gs::ComponentVector<gs::Hull>& hulls = list.hulls();
    hulls.clear();

    // Load it
    afl::base::Ref<afl::io::Stream> file = dir.openFile("hullspec.dat", FileSystem::OpenRead);
    gt::Hull in;
    int i = 0;
    while (file->read(afl::base::fromObject(in)) == sizeof(in)) {
        ++i;
        if (gs::Hull* out = hulls.create(i)) {
            out->clearHullFunctions();
            out->setName(m_charset->decode(in.name));
            out->setExternalPictureNumber(in.pictureNumber);
            out->setInternalPictureNumber(i == 104 ? 152 : i == 105 ? 153 : in.pictureNumber);
            out->cost().set(gs::Cost::Tritanium, in.tritanium);
            out->cost().set(gs::Cost::Duranium, in.duranium);
            out->cost().set(gs::Cost::Molybdenum, in.molybdenum);
            out->setMaxFuel(in.maxFuel);
            out->setMaxCrew(in.maxCrew);
            out->setNumEngines(in.numEngines);
            out->setMass(in.mass);
            out->setTechLevel(in.techLevel);
            out->setMaxCargo(in.maxCargo);
            out->setNumBays(in.numBays);
            out->setMaxLaunchers(in.maxLaunchers);
            out->setMaxBeams(in.maxBeams);
            out->cost().set(gs::Cost::Money, in.money);
        }
    }

    if (hulls.size() < 10) {
        // File is obviously broken. Typical file has 105.
        throw afl::except::FileTooShortException(*file);
    }
}

void
game::v3::SpecificationLoader::loadHullAssignments(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/spec.cc:loadHulls (part)
    gs::HullAssignmentList& hullAssignments = list.hullAssignments();
    hullAssignments.clear();

    // Load it
    afl::base::Ref<afl::io::Stream> file = dir.openFile("truehull.dat", FileSystem::OpenRead);
    gt::Truehull in;
    file->fullRead(afl::base::fromObject(in));
    for (int player = 0; player < gt::NUM_PLAYERS; ++player) {
        for (int slot = 0; slot < gt::NUM_HULLS_PER_PLAYER; ++slot) {
            int hull = in.hulls[player][slot];
            if (hull != 0) {
                if (list.hulls().get(hull) == 0) {
                    throw afl::except::FileFormatException(*file, m_translator.translateString("File is invalid"));
                }
                hullAssignments.add(player+1, slot+1, hull);
            }
        }
    }
}

void
game::v3::SpecificationLoader::loadHullFunctions(game::spec::ShipList& list, afl::io::Directory& dir,
                                                 const game::HostVersion& host,
                                                 const game::config::HostConfiguration& config)
{
    // ex game/spec.cc:loadHulls (part)
    // Load basic function definitions
    list.basicHullFunctions().clear();
    afl::base::Ptr<afl::io::Stream> ps = dir.openFileNT("hullfunc.usr", FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }
    ps = dir.openFileNT("hullfunc.cc", FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }

    // Default hull function assignments
    list.basicHullFunctions().performDefaultAssignments(list.hulls());

    // Shiplist hull function assignments
    ps = dir.openFileNT("shiplist.txt", FileSystem::OpenRead);
    if (ps.get()) {
        // shiplist.txt: PHost, new-style
        HullfuncParser p(list, host, config, m_translator, m_log);
        p.setCharsetNew(m_charset->clone());
        p.setSection("hullfunc", false);
        p.parseFile(*ps);
    } else {
        ps = dir.openFileNT("hullfunc.txt", FileSystem::OpenRead);
        if (ps.get()) {
            // hullfunc.txt: PHost, old-style
            HullfuncParser p(list, host, config, m_translator, m_log);
            p.setCharsetNew(m_charset->clone());
            p.setSection("hullfunc", true);
            p.parseFile(*ps);
        } else {
            // no file, THost default
        }
    }

    // Postprocess
    list.findRacialAbilities(config);
}

void
game::v3::SpecificationLoader::loadFriendlyCodes(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex ccmain.pas:LoadFCodes
    game::spec::FriendlyCodeList& fcs = list.friendlyCodes();
    fcs.clear();

    // Regular definitions
    afl::base::Ptr<afl::io::Stream> ps = dir.openFileNT("fcodes.cc", FileSystem::OpenRead);
    if (ps.get()) {
        fcs.load(*ps, m_log, m_translator);
    }
    ps = dir.openFileNT("fcodes.usr", FileSystem::OpenRead);
    if (ps.get()) {
        fcs.load(*ps, m_log, m_translator);
    }

    // Extra definitions
    ps = dir.openFileNT("xtrfcode.txt", FileSystem::OpenRead);
    if (ps.get()) {
        fcs.loadExtraCodes(*ps);
    }
}

void
game::v3::SpecificationLoader::loadMissions(game::spec::ShipList& list, afl::io::Directory& dir)
{
    // ex game/mission.cc:initMissions
    game::spec::MissionList& msns = list.missions();
    msns.clear();

    // Regular definitions
    afl::base::Ptr<afl::io::Stream> ps = dir.openFileNT("mission.usr", FileSystem::OpenRead);
    if (ps.get()) {
        msns.loadFromFile(*ps, m_log, m_translator);
    }
    ps = dir.openFileNT("mission.cc", FileSystem::OpenRead);
    if (ps.get()) {
        msns.loadFromFile(*ps, m_log, m_translator);
    }
    ps = dir.openFileNT("mission.ini", FileSystem::OpenRead);
    if (ps.get()) {
        msns.loadFromIniFile(*ps, *m_charset);
    }

    msns.sort();
}
