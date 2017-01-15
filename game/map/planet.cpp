/**
  *  \file game/map/planet.cpp
  */

#include <algorithm>
#include "game/map/planet.hpp"
#include "afl/string/format.hpp"
#include "afl/base/memory.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"

namespace {
    const char LOG_NAME[] = "game.map.planet";

    bool isUsed(const game::map::PlanetData& d)
    {
        return d.owner.isValid()
            || d.friendlyCode.isValid()
            || d.numMines.isValid()
            || d.numFactories.isValid()
            || d.numDefensePosts.isValid()
            || d.minedNeutronium.isValid()
            || d.minedTritanium.isValid()
            || d.minedDuranium.isValid()
            || d.minedMolybdenum.isValid()
            || d.colonistClans.isValid()
            || d.supplies.isValid()
            || d.money.isValid()
            || d.groundNeutronium.isValid()
            || d.groundTritanium.isValid()
            || d.groundDuranium.isValid()
            || d.groundMolybdenum.isValid()
            || d.densityNeutronium.isValid()
            || d.densityTritanium.isValid()
            || d.densityDuranium.isValid()
            || d.densityMolybdenum.isValid()
            || d.colonistTax.isValid()
            || d.nativeTax.isValid()
            || d.colonistHappiness.isValid()
            || d.nativeHappiness.isValid()
            || d.nativeGovernment.isValid()
            || d.nativeClans.isValid()
            || d.nativeRace.isValid()
            || d.temperature.isValid()
            || d.baseFlag.isValid();
    }

    bool isUsed(const game::map::BaseData& d)
    {
        return d.owner.isValid()
            || d.numBaseDefensePosts.isValid()
            || d.damage.isValid()
            || d.techLevels[0].isValid()
            || d.techLevels[1].isValid()
            || d.techLevels[2].isValid()
            || d.techLevels[3].isValid()
            || d.engineStorage.isValid()
            || d.hullStorage.isValid()
            || d.beamStorage.isValid()
            || d.launcherStorage.isValid()
            || d.torpedoStorage.isValid()
            || d.numFighters.isValid()
            || d.shipyardId.isValid()
            || d.shipyardAction.isValid()
            || d.mission.isValid();
    }
}

game::map::Planet::Planet(Id_t id)
    : m_id(id),
      m_name("?"),
      m_position(),
      m_knownToNotExist(false),
      m_currentPlanetData(),
      m_previousPlanetData(),
      m_currentBaseData(),
      m_previousBaseData(),
      m_baseKind(UnknownBase),
      m_planetKind(NoPlanet),
      m_planetSource(),
      m_baseSource(),
      m_isPlanetKnownToHaveNatives(false),
      m_industryLevel(),
      m_queuePosition(),
      m_queuePriority(),
      m_unitScores()
{
    // ex GPlanet::MapInfo::MapInfo etc.
    m_autobuildGoals[MineBuilding]         = 1000;
    m_autobuildGoals[FactoryBuilding]      = 1000;
    m_autobuildGoals[DefenseBuilding]      = 1000;
    m_autobuildGoals[BaseDefenseBuilding]  = 1000;
    m_autobuildSpeeds[MineBuilding]        = 5;
    m_autobuildSpeeds[FactoryBuilding]     = 10;
    m_autobuildSpeeds[DefenseBuilding]     = 3;
    m_autobuildSpeeds[BaseDefenseBuilding] = 2;

    // ex GPlanet::GPlanet
    afl::base::Memory<int>(m_historyTimestamps).fill(0);
}

game::map::Planet::~Planet()
{ }

// /** Add .dat file entry.
//     This assumes that when we see a planet through several .dat files, we get the same one each time.
//     \param dat parsed .dat file entry
//     \param source source flag value to use for this entry */
void
game::map::Planet::addCurrentPlanetData(const PlanetData& data, PlayerSet_t source)
{
    // ex GPlanet::addPlanetData
    // FIXME: older PHost versions clear the following fields of a
    // planet when sending a pdata planet target for exploration of an
    // unowned planet (owned planets never generate a target):
    // colonists, supplies, credits, mines, factories, defense,
    // coltax, nattax. We may want to merge that information somehow,
    // or detect and/or merge it.
    m_currentPlanetData = data;
    m_planetSource += source;
}

void
game::map::Planet::addPreviousPlanetData(const PlanetData& data)
{
    m_previousPlanetData = data;
}

// /** Add starbase .dat file entry. See addPlanetData() for details. */
void
game::map::Planet::addCurrentBaseData(const BaseData& data, PlayerSet_t source)
{
    // ex GPlanet::addBaseData
    m_currentBaseData = data;
    m_baseSource += source;
}

void
game::map::Planet::addPreviousBaseData(const BaseData& data)
{
    m_previousBaseData = data;
}

// /** Set planet position. */
void
game::map::Planet::setPosition(Point pt)
{
    // ex GPlanet::setXY
    m_position = pt;
    markDirty();
}

// /** Set planet name. */
void
game::map::Planet::setName(const String_t& name)
{
    // ex GPlanet::setName
    m_name = name;
    markDirty();
}

// /** Set whether non-existance of this planet is known. There is no way
//     to explicitly specify that a planet does not exist. To build maps
//     with fewer than 500 planets, people move planets to odd positions.
//     Recent PHosts send a util.dat message whenever they consider a
//     planet to be non-existant, to make sure that the clients' idea of
//     which planets do exist agrees with PHost's. */
void
game::map::Planet::setKnownToNotExist(bool value)
{
    m_knownToNotExist = value;
    markDirty();
}

// /** Do internal checks for this planet.
//     Internal checks do not require a partner to interact with.
//     This will fix the problems, and display appropriate messages. */
void
game::map::Planet::internalCheck(const Configuration& config,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log)
{
    // ex GPlanet::internalCheck

    // Does this planet exist?
    // FIXME: the isValidPlanetCoordinate() check should probably be moved into the loader, because it's a v3 thing.
    // On the other hand, this allows live re-configuration of map wrap.
    Point pos;
    bool posKnown = m_position.get(pos);
    bool exists = !m_knownToNotExist && posKnown && config.isValidPlanetCoordinate(pos);

    // Check PDATA. If we have a PDATA entry, it must exist.
    if (!m_planetSource.empty() && !exists) {
        String_t fmt = (m_knownToNotExist
                        ? tx.translateString("Planet #%d has data although it is reported as non-existant, host confused?")
                        : tx.translateString("Planet #%d exists for Host, but is outside valid range"));
        log.write(log.Warn, LOG_NAME, afl::string::Format(fmt.c_str(), m_id));
        exists = true;
    }

    // Set planet kind
    if (!exists) {
        if (isUsed(m_currentPlanetData)) {
            m_planetKind = HiddenPlanet;
        } else {
            m_planetKind = NoPlanet;
        }
    } else if (!m_planetSource.empty()) {
        m_planetKind = CurrentPlanet;
    } else if (isUsed(m_currentPlanetData)) {
        m_planetKind = KnownPlanet;
    } else {
        m_planetKind = UnknownPlanet;
    }

    // Check BDATA. If we have BDATA, we also must have PDATA.
    if (!m_baseSource.empty() && m_planetSource.empty()) {
        // FIXME this will make PCC2 write invalid files, i.e. where BDATA.DAT and BDATA.DIS disagree.
        log.write(log.Warn, LOG_NAME, afl::string::Format(tx.translateString("Starbase #%d does not have a planet, deleting it").c_str(), m_id));
        m_currentBaseData = BaseData();
        m_previousBaseData = BaseData();
        m_baseSource = PlayerSet_t();
    }

    // Set base kind
    int baseFlag;
    if (!m_baseSource.empty()) {
        m_baseKind = CurrentBase;
    } else if (isUsed(m_currentBaseData)) {
        m_baseKind = KnownBase;
    } else if (!m_currentPlanetData.baseFlag.get(baseFlag)) {
        m_baseKind = UnknownBase;
    } else if (baseFlag == 0) {
        m_baseKind = NoBase;
    } else {
        // This case also applies for an own planet with "build me a base" set. combinedCheck2() will fix that up.
        m_baseKind = ExistingBase;
    }
}

// /** Combined checks, phase 2.
//     This will do all post-processing which needs a partner to interact with.
//     It requires the playability to be filled in. */
void
game::map::Planet::combinedCheck2(const Universe& univ, PlayerSet_t availablePlayers, int turnNumber)
{
    // FIXME: remove parameter?
    (void) univ;

    if (isUsed(m_currentPlanetData)) {
        if (m_planetKind == CurrentPlanet) {
            // We have seen this planet this turn
            m_historyTimestamps[MineralTime]
                = m_historyTimestamps[ColonistTime]
                = m_historyTimestamps[NativeTime]
                = m_historyTimestamps[CashTime]
                = turnNumber;
        } else {
            // We have taken this planet from the history
            int owner;
            if (getOwner(owner) && availablePlayers.contains(owner)) {
                // planet is played by us, but we do no longer own it
                m_currentPlanetData.owner = 0;
                m_previousPlanetData.owner = 0;
                m_currentPlanetData.colonistClans = LongProperty_t();
                m_previousPlanetData.colonistClans = LongProperty_t();
            }
        }

        int owner;
        if (getOwner(owner) && availablePlayers.contains(owner) && m_baseKind != NoBase && m_baseKind != CurrentBase) {
            // We play this planet, and have history information about a base, but that base isn't there.
            // -or- We're building a base here (this will set the status to ExistingBase).
            // Delete the base.
            m_baseKind = NoBase;
        }
    }
}


String_t
game::map::Planet::getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const
{
    // ex GPlanet::getName
    // FIXME: make the default name "", and handle that specially here
    switch (which) {
     case PlainName:
        return m_name;

     case LongName:
     case DetailedName: {
        String_t result = afl::string::Format(tx.translateString("Planet #%d: %s").c_str(), m_id, m_name);
        if (which == DetailedName) {
            String_t comment = iface.getComment(InterpreterInterface::Planet, m_id);
            if (!comment.empty()) {
                result += ": ";
                result += comment;
            }
        }
        return result;
     }
    }
    return String_t();
}

game::Id_t
game::map::Planet::getId() const
{
    // ex GPlanet::getId
    return m_id;
}

bool
game::map::Planet::getOwner(int& result) const
{
    // ex GPlanet::getOwner, GPlanet::isOwnerKnown
    return m_currentPlanetData.owner.get(result);
}

bool
game::map::Planet::getPosition(Point& result) const
{
    // ex GPlanet::getPos
    return !m_knownToNotExist && m_position.get(result);
}


// /*
//  *  Planet Status Accessors:
//  */
//
// /** Check whether planet is visible. */
bool
game::map::Planet::isVisible() const
{
    // ex/ GPlanet::isVisible
    return m_planetKind != NoPlanet
        && m_planetKind != HiddenPlanet;
}

// /** Get planet source flags. This is the set of players whose PDATA
//     file contains a copy of this planet (usually a unit set, but may
//     be larger for unowned planets). */
game::PlayerSet_t
game::map::Planet::getPlanetSource() const
{
    // ex GPlanet::getPlanetSource
    return m_planetSource;
}

void
game::map::Planet::addPlanetSource(PlayerSet_t p)
{
    m_planetSource += p;
}


// /** Check whether we have any information about this planet.
//     Note that the planet may not exist even if it has information (HiddenPlanet).
//     \return true iff we have any information, full or partial */
bool
game::map::Planet::hasAnyPlanetData() const
{
    // ex GPlanet::hasAnyPlanetData
    // Note that this is implemented differently than in PCC2!
    return m_planetKind != NoPlanet
        && m_planetKind != UnknownPlanet;
}

// /** Check whether we have full planet data. */
bool
game::map::Planet::hasFullPlanetData() const
{
    // ex GPlanet::hasFullPlanetData
    return !m_planetSource.empty();
}

// /** Get history timestamp. */
int
game::map::Planet::getHistoryTimestamp(Timestamp kind) const
{
    return m_historyTimestamps[kind];
}


// /** Get base source flags. This is the set of players whose BDATA file
//     contains a copy of this base (usually a unit set). */
game::PlayerSet_t
game::map::Planet::getBaseSource() const
{
    // ex GPlanet::getBaseSource
    return m_baseSource;
}

void
game::map::Planet::addBaseSource(PlayerSet_t p)
{
    m_baseSource += p;
}

// /** Check for starbase.
//     \retval true this planet has a starbase
//     \retval false this planet has no starbase, or we don't know */
bool
game::map::Planet::hasBase() const
{
    // ex GPlanet::hasBase
    return m_baseKind != UnknownBase
        && m_baseKind != NoBase;
}

// /** True iff we have full starbase information.
//     \return true iff we have full, playable data. If yes, all base accessors will work. */
bool
game::map::Planet::hasFullBaseData() const
{
    // ex GPlanet::hasFullBaseData
    return !m_baseSource.empty();
}



// /** Set owner.
//     \param owner new owner */
void
game::map::Planet::setOwner(IntegerProperty_t owner)
{
    // ex GPlanet::setOwner
    m_currentPlanetData.owner = owner;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getNumBuildings(PlanetaryBuilding kind) const
{
    // ex GPlanet::getStructures
    switch (kind) {
     case MineBuilding:
        return m_currentPlanetData.numMines;
     case FactoryBuilding:
        return m_currentPlanetData.numFactories;
     case DefenseBuilding:
        return m_currentPlanetData.numDefensePosts;
     case BaseDefenseBuilding:
        return m_baseKind == UnknownBase ? IntegerProperty_t() : m_currentBaseData.numBaseDefensePosts;
    }
    return IntegerProperty_t();
}

void
game::map::Planet::setNumBuildings(PlanetaryBuilding kind, IntegerProperty_t n)
{
    // ex GPlanet::setStructures
    switch (kind) {
     case MineBuilding:
        m_currentPlanetData.numMines = n;
        break;
     case FactoryBuilding:
        m_currentPlanetData.numFactories = n;
        break;
     case DefenseBuilding:
        m_currentPlanetData.numDefensePosts = n;
        break;
     case BaseDefenseBuilding:
        // FIXME: what to do if we do not have a base? PCC2 does that same as this:
        m_currentBaseData.numBaseDefensePosts = n;
        break;
    }
    markDirty();
}

// /** Get industry level of this planet. Reports the industry level from
//     known structure counts if available, otherwise from sensor scans. */
game::IntegerProperty_t
game::map::Planet::getIndustryLevel(const HostVersion& host) const
{
    // ex GPlanet::getIndustryLevel
    int mines, factories;
    if (getNumBuildings(MineBuilding).get(mines) && getNumBuildings(FactoryBuilding).get(factories)) {
        return getIndustryLevel(mines+factories, host);
    } else {
        return m_industryLevel;
    }
}

// /** Get industry level for a given structure count.
//     \param mifa Mines+Factories
//     \return level (ind_XXX) */
int
game::map::Planet::getIndustryLevel(int mifa, const HostVersion& host)
{
    // ex GPlanet::getIndustryLevel
    switch (host.getKind()) {
     case HostVersion::PHost:
        return std::min(int(HeavyIndustry), mifa/30);

     case HostVersion::Host:
     case HostVersion::SRace:
     case HostVersion::NuHost:
        if (mifa >= 100) {
            return HeavyIndustry;
        } else {
            return LightIndustry;
        }

     case HostVersion::Unknown:
        break;
    }

    // Fallback
    return HeavyIndustry;
}

// /** Set industry level for this planet. This routine only makes sense
//     for planets we do not play. */
void
game::map::Planet::setIndustryLevel(IntegerProperty_t level, const HostVersion& host)
{
    // ex GPlanet::setIndustryLevel

    // If we're playing this planet, setting the industry level is useless
    if (hasFullPlanetData()) {
        return;
    }

    // We're not playing the planet, so set the level and check for conflicts with our other data
    m_industryLevel = level;

    int rawLevel;
    if (level.get(rawLevel)) {
        int mines = 0, factories = 0;
        bool minesKnown = getNumBuildings(MineBuilding).get(mines);
        bool factoriesKnown = getNumBuildings(FactoryBuilding).get(factories);
        if (minesKnown && factoriesKnown) {
            // We know mines and factories. Does this conflict with our level?
            if (getIndustryLevel(mines + factories, host) != rawLevel) {
                // yes, so forget mi+fa
                setNumBuildings(MineBuilding, IntegerProperty_t());
                setNumBuildings(FactoryBuilding, IntegerProperty_t());
            }
        } else if (minesKnown || factoriesKnown) {
            // We know mines or factories, but not both. Does this conflict with our level?
            int mifa = 0;
            if (minesKnown) {
                mifa += mines;
            }
            if (factoriesKnown) {
                mifa += factories;
            }
            if (getIndustryLevel(mifa, host) > rawLevel) {
                // our stored mine/factory count would yield a larger level than reported.
                // This means our stored mi/fa is // outdated.
                setNumBuildings(MineBuilding, IntegerProperty_t());
                setNumBuildings(FactoryBuilding, IntegerProperty_t());
            }
        }
    }
    markDirty();
}


game::NegativeProperty_t
game::map::Planet::getColonistHappiness() const
{
    // ex GPlanet::getColonistHappiness
    return m_currentPlanetData.colonistHappiness;
}

void
game::map::Planet::setColonistHappiness(NegativeProperty_t happiness)
{
    // ex GPlanet::setColonistHappiness
    m_currentPlanetData.colonistHappiness = happiness;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getColonistTax() const
{
    // ex GPlanet::getColonistTax
    return m_currentPlanetData.colonistTax;
}

void
game::map::Planet::setColonistTax(IntegerProperty_t tax)
{
    // ex GPlanet::setColonistTax
    m_currentPlanetData.colonistTax = tax;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getNativeGovernment() const
{
    // ex GPlanet::getNativeGovernment
    return m_currentPlanetData.nativeGovernment;
}

void
game::map::Planet::setNativeGovernment(IntegerProperty_t gov)
{
    // ex GPlanet::setNativeGovernment
    m_currentPlanetData.nativeGovernment = gov;
    markDirty();
}

game::NegativeProperty_t
game::map::Planet::getNativeHappiness() const
{
    // ex GPlanet::getNativeHappiness
    return m_currentPlanetData.nativeHappiness;
}

void
game::map::Planet::setNativeHappiness(NegativeProperty_t happiness)
{
    // ex GPlanet::setNativeHappiness
    m_currentPlanetData.nativeHappiness = happiness;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getNativeRace() const
{
    // ex GPlanet::getNativeRace
    return m_currentPlanetData.nativeRace;
}

void
game::map::Planet::setNativeRace(IntegerProperty_t race)
{
    // ex GPlanet::setNativeRace
    m_currentPlanetData.nativeRace = race;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getNativeTax() const
{
    // ex GPlanet::getNativeTax
    return m_currentPlanetData.nativeTax;
}

void
game::map::Planet::setNativeTax(IntegerProperty_t tax)
{
    // ex GPlanet::setNativeTax
    m_currentPlanetData.nativeTax = tax;
    markDirty();
}

game::LongProperty_t
game::map::Planet::getNatives() const
{
    // ex GPlanet::getNatives
    // FIXME: rename?
    return m_currentPlanetData.nativeClans;
}

void
game::map::Planet::setNatives(LongProperty_t natives)
{
    // ex GPlanet::setNatives
    // FIXME: rename?
    m_currentPlanetData.nativeClans = natives;
    markDirty();
}

bool
game::map::Planet::isKnownToHaveNatives() const
{
    // ex GPlanet::isKnownToHaveNatives
    // We claim the planet has natives if
    //        ...we know that it has some from RGA
    //        ...we know the race (bioscan)
    //        ...we know the population (pillage)
    int race;
    int32_t pop;
    return m_isPlanetKnownToHaveNatives
        || (getNativeRace().get(race) && race != NoNatives)
        || (getNatives().get(pop) && pop != 0);
}

void
game::map::Planet::setKnownToHaveNatives(bool known)
{
    // ex GPlanet::setKnownToHaveNatives
    m_isPlanetKnownToHaveNatives = known;
}

game::StringProperty_t
game::map::Planet::getFriendlyCode() const
{
    // ex GPlanet::getFCode, GPlanet::isFCodeKnown
    return m_currentPlanetData.friendlyCode;
}

void
game::map::Planet::setFriendlyCode(StringProperty_t fc)
{
    // ex GPlanet::setFCode
    m_currentPlanetData.friendlyCode = fc;
    markDirty();
}

bool
game::map::Planet::isBuildingBase() const
{
    // ex GPlanet::isBuildingBase
    int value;
    return hasFullPlanetData()
        && m_currentPlanetData.baseFlag.get(value)
        && value != 0;
}

void
game::map::Planet::setBuildBaseFlag(bool b)
{
    // ex GPlanet::setBuildBaseFlag
    if (b != isBuildingBase()) {
        m_currentPlanetData.baseFlag = b;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Planet::getOreDensity(Element::Type type) const
{
    // ex GPlanet::getOreDensity
    switch (type) {
     case Element::Neutronium:
        return m_currentPlanetData.densityNeutronium;
     case Element::Tritanium:
        return m_currentPlanetData.densityTritanium;
     case Element::Duranium:
        return m_currentPlanetData.densityDuranium;
     case Element::Molybdenum:
        return m_currentPlanetData.densityMolybdenum;
     default:
        return IntegerProperty_t();
    }
}

void
game::map::Planet::setOreDensity(Element::Type type, IntegerProperty_t amount)
{
    // ex GPlanet::setOreDensity
    switch (type) {
     case Element::Neutronium:
        m_currentPlanetData.densityNeutronium = amount;
        break;
     case Element::Tritanium:
        m_currentPlanetData.densityTritanium = amount;
        break;
     case Element::Duranium:
        m_currentPlanetData.densityDuranium = amount;
        break;
     case Element::Molybdenum:
        m_currentPlanetData.densityMolybdenum = amount;
        break;
     default:
        break;
    }
    markDirty();
}

game::LongProperty_t
game::map::Planet::getOreGround(Element::Type type) const
{
    // ex GPlanet::getOreGround
    switch (type) {
     case Element::Neutronium:
        return m_currentPlanetData.groundNeutronium;
     case Element::Tritanium:
        return m_currentPlanetData.groundTritanium;
     case Element::Duranium:
        return m_currentPlanetData.groundDuranium;
     case Element::Molybdenum:
        return m_currentPlanetData.groundMolybdenum;
     default:
        return LongProperty_t();
    }
}

void
game::map::Planet::setOreGround(Element::Type type, LongProperty_t amount)
{
    // ex GPlanet::setOreGround
    switch (type) {
     case Element::Neutronium:
        m_currentPlanetData.groundNeutronium = amount;
        break;
     case Element::Tritanium:
        m_currentPlanetData.groundTritanium = amount;
        break;
     case Element::Duranium:
        m_currentPlanetData.groundDuranium = amount;
        break;
     case Element::Molybdenum:
        m_currentPlanetData.groundMolybdenum = amount;
        break;
     default:
        break;
    }
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getTemperature() const
{
    return m_currentPlanetData.temperature;
}

void
game::map::Planet::setTemperature(IntegerProperty_t value)
{
    m_currentPlanetData.temperature = value;
    markDirty();
}

// /** Get cargo, as it is in underlying data structure. */
game::LongProperty_t
game::map::Planet::getCargo(Element::Type type) const
{
    // ex GPlanet::getCargo, GPlanet::getCargoRaw, GPlanet::getColonists
    int n;
    switch (type) {
     case Element::Neutronium:
        return m_currentPlanetData.minedNeutronium;
     case Element::Tritanium:
        return m_currentPlanetData.minedTritanium;
     case Element::Duranium:
        return m_currentPlanetData.minedDuranium;
     case Element::Molybdenum:
        return m_currentPlanetData.minedMolybdenum;

     case Element::Supplies:
        return m_currentPlanetData.supplies;

     case Element::Money:
        return m_currentPlanetData.money;

     case Element::Fighters:
        if (m_baseKind == NoBase) {
            return 0;
        } else {
            return m_currentBaseData.numFighters;
        }

     case Element::Colonists:
        return m_currentPlanetData.colonistClans;

     default:
        if (Element::isTorpedoType(type, n)) {
            if (m_baseKind == NoBase) {
                return 0;
            } else {
                return m_currentBaseData.torpedoStorage.get(n);
            }
        } else {
            return LongProperty_t();
        }
    }
}

void
game::map::Planet::setCargo(Element::Type type, LongProperty_t amount)
{
    // ex GPlanet::setCargoRaw
    int n;
    switch (type) {
     case Element::Neutronium:
        m_currentPlanetData.minedNeutronium = amount;
        break;
     case Element::Tritanium:
        m_currentPlanetData.minedTritanium = amount;
        break;
     case Element::Duranium:
        m_currentPlanetData.minedDuranium = amount;
        break;
     case Element::Molybdenum:
        m_currentPlanetData.minedMolybdenum = amount;
        break;

     case Element::Supplies:
        m_currentPlanetData.supplies = amount;
        break;

     case Element::Money:
        m_currentPlanetData.money = amount;
        break;

     case Element::Fighters:
        m_currentBaseData.numFighters = amount;
        break;

     case Element::Colonists:
        m_currentPlanetData.colonistClans = amount;
        break;

     default:
        if (Element::isTorpedoType(type, n)) {
            if (IntegerProperty_t* p = m_currentBaseData.torpedoStorage.at(n)) {
                *p = amount;
            }
        }
        break;
    }
}


game::IntegerProperty_t
game::map::Planet::getBaseDamage() const
{
    // ex GPlanet::getBaseDamage
    return m_currentBaseData.damage;
}

void
game::map::Planet::setBaseDamage(IntegerProperty_t n)
{
    // ex GPlanet::setBaseDamage
    m_currentBaseData.damage = n;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getBaseMission() const
{
    // GPlanet::getBaseMission
    return m_currentBaseData.mission;
}

void
game::map::Planet::setBaseMission(IntegerProperty_t mission)
{
    // ex GPlanet::setBaseMission
    m_currentBaseData.mission = mission;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getBaseTechLevel(TechLevel level) const
{
    // ex GPlanet::getBaseTechLevel
    return m_currentBaseData.techLevels[level];
}

void
game::map::Planet::setBaseTechLevel(TechLevel level, IntegerProperty_t value)
{
    // ex GPlanet::setBaseTechLevel
    m_currentBaseData.techLevels[level] = value;
    markDirty();
}

game::IntegerProperty_t
game::map::Planet::getBaseShipyardAction() const
{
    // ex GPlanet::getBaseShipyardOrder
    return m_currentBaseData.shipyardAction;
}

game::IntegerProperty_t
game::map::Planet::getBaseShipyardId() const
{
    // ex GPlanet::getBaseShipyardId
    return m_currentBaseData.shipyardId;
}

void
game::map::Planet::setBaseShipyardOrder(IntegerProperty_t action, IntegerProperty_t id)
{
    // ex GPlanet::setBaseShipyardOrder
    m_currentBaseData.shipyardAction = action;
    m_currentBaseData.shipyardId = id;
    markDirty();
}


game::IntegerProperty_t
game::map::Planet::getBaseEngineStore(int slot) const
{
    // ex GPlanet::getBaseEngineStore
    return m_currentBaseData.engineStorage.get(slot);
}

game::IntegerProperty_t
game::map::Planet::getBaseBeamStore(int slot) const
{
    // ex GPlanet::getBaseBeamStore
    return m_currentBaseData.beamStorage.get(slot);
}

game::IntegerProperty_t
game::map::Planet::getBaseHullStoreSlot(int slot) const
{
    // ex GPlanet::getBaseHullStoreSlot
    return m_currentBaseData.hullStorage.get(slot);
}

game::IntegerProperty_t
game::map::Planet::getBaseLauncherStore(int slot) const
{
    // ex GPlanet::getBaseLauncherStore
    return m_currentBaseData.launcherStorage.get(slot);
}

void
game::map::Planet::setBaseEngineStore(int slot, IntegerProperty_t amount)
{
    // ex GPlanet::setBaseEngineStore
    if (IntegerProperty_t* p = m_currentBaseData.engineStorage.at(slot)) {
        *p = amount;
        markDirty();
    }
}

void
game::map::Planet::setBaseBeamStore(int slot, IntegerProperty_t amount)
{
    // ex GPlanet::setBaseBeamStore
    if (IntegerProperty_t* p = m_currentBaseData.beamStorage.at(slot)) {
        *p = amount;
        markDirty();
    }
}

void
game::map::Planet::setBaseHullStoreSlot(int slot, IntegerProperty_t amount)
{
    // ex GPlanet::setBaseHullStoreSlot
    if (IntegerProperty_t* p = m_currentBaseData.hullStorage.at(slot)) {
        *p = amount;
        markDirty();
    }
}

void
game::map::Planet::setBaseLauncherStore(int slot, IntegerProperty_t amount)
{
    // ex GPlanet::setBaseLauncherStore
    if (IntegerProperty_t* p = m_currentBaseData.launcherStorage.at(slot)) {
        *p = amount;
        markDirty();
    }
}

game::IntegerProperty_t
game::map::Planet::getBaseBuildHull(const game::config::HostConfiguration& config, const game::spec::HullAssignmentList& map) const
{
    // ex GPlanet::getBaseBuildHull
    int owner, index;
    if (getOwner(owner) && getBaseBuildOrderHullIndex().get(index)) {
        if (int hull = map.getHullFromIndex(config, owner, index)) {
            return hull;
        } else {
            return IntegerProperty_t();
        }
    } else {
        return IntegerProperty_t();
    }
}

game::ShipBuildOrder
game::map::Planet::getBaseBuildOrder() const
{
    // ex GPlanet::getBaseBuildOrder. Note different semantic.
    return m_currentBaseData.shipBuildOrder;
}

void
game::map::Planet::setBaseBuildOrder(const ShipBuildOrder& order)
{
    // ex GPlanet::setBaseBuildOrder. Note different semantic.
    // FIXME: we refuse to set this on foreign bases. Reconsider.
    if (!m_baseSource.empty()) {
        m_currentBaseData.shipBuildOrder = order;
        markDirty();
    }
}

// /** Get hull slot used for ship being built. Whereas the above functions deal
//     with real hull numbers, this one returns a slot; this simplifies things
//     every once in a while. */
game::IntegerProperty_t
game::map::Planet::getBaseBuildOrderHullIndex() const
{
    // ex GPlanet::getBaseBuildOrderHullSlot
    return m_currentBaseData.shipBuildOrder.getHullIndex();
}

game::IntegerProperty_t
game::map::Planet::getBaseQueuePosition() const
{
    // ex GPlanet::getBaseQueuePosition
    return m_queuePosition;
}

void
game::map::Planet::setBaseQueuePosition(IntegerProperty_t pos)
{
    // ex GPlanet::setBaseQueuePosition
    m_queuePosition = pos;
    markDirty();
}

game::LongProperty_t
game::map::Planet::getBaseQueuePriority() const
{
    // ex GPlanet::getBaseQueuePriority
    return m_queuePriority;
}

void
game::map::Planet::setBaseQueuePriority(LongProperty_t pri)
{
    // ex GPlanet::setBaseQueuePriority
    m_queuePriority = pri;
    markDirty();
}

// /** Get autobuild goal for a structure. Known for all planets. */
int
game::map::Planet::getAutobuildGoal(PlanetaryBuilding ps) const
{
    // ex GPlanet::getAutobuildGoal
    return m_autobuildGoals[ps];
}

// /** Set autobuild goal for a structure. */
void
game::map::Planet::setAutobuildGoal(PlanetaryBuilding ps, int value)
{
    // ex GPlanet::setAutobuildGoal
    if (value != m_autobuildGoals[ps]) {
        m_autobuildGoals[ps] = value;
        markDirty();
    }
}

// /** Get autobuild speed for a structure. Known for all planets. */
int
game::map::Planet::getAutobuildSpeed(PlanetaryBuilding ps) const
{
    // ex GPlanet::getAutobuildSpeed
    return m_autobuildSpeeds[ps];
}

// /** Set autobuild speed for a structure. */
void
game::map::Planet::setAutobuildSpeed(PlanetaryBuilding ps, int value)
{
    // ex GPlanet::setAutobuildSpeed
    if (value != m_autobuildSpeeds[ps]) {
        m_autobuildSpeeds[ps] = value;
        markDirty();
    }
}


game::UnitScoreList&
game::map::Planet::unitScores()
{
    return m_unitScores;
}

const game::UnitScoreList&
game::map::Planet::unitScores() const
{
    return m_unitScores;
}


// /** Add history file entry. */
// void
// GPlanet::addHistoryData(const TDbPlanet& data)
// {
//     if (planet_source.empty()) {
//         // Accept only when not scanned otherwise
//         PlanetInfo& pi = createPlanetInfo();
//
//         // FIXME: This assumes only one history data source. Otherwise, we have to merge.
//         pi.data               = data.planet;
//         pi.time[ts_Minerals]  = data.time[ts_Minerals];
//         pi.time[ts_Colonists] = data.time[ts_Colonists];
//         pi.time[ts_Natives]   = data.time[ts_Natives];
//         pi.time[ts_Cash]      = data.time[ts_Cash];
//         if (data.planet.factories >= 30000) {
//             pi.data.factories = mp16_t().getRawValue();
//             pi.industry_level = mp16_t(data.planet.factories - 30000);
//         } else {
//             pi.industry_level = mp16_t();
//         }
//         pi.known_to_have_natives = (data.known_to_have_natives != 0);
//     }
// }
//
//
//
//
//
// /** Get planet data record for storage in data files.
//     \param dat [out] planet data */
// void
// GPlanet::getPlanetData(TPlanet& dat) const
// {
//     dat = getDisplayedPlanet();
// }
//
// /** Get starbase data record for storage in data files.
//     \param bdat [out] base data */
// void
// GPlanet::getBaseData(TStarbase& bdat) const
// {
//     bdat = getDisplayedBase();
// }
//
//
// /** Get planet history record for storage in database file.
//     \param data [out] History record */
// void
// GPlanet::getHistoryData(TDbPlanet& data) const
// {
//     if (planet_info != 0) {
//         data.planet             = planet_info->data;
//         data.time[ts_Minerals]  = planet_info->time[ts_Minerals];
//         data.time[ts_Colonists] = planet_info->time[ts_Colonists];
//         data.time[ts_Natives]   = planet_info->time[ts_Natives];
//         data.time[ts_Cash]      = planet_info->time[ts_Cash];
//         data.known_to_have_natives = planet_info->known_to_have_natives;
//         if (planet_info->industry_level.isKnown() && mp16_t(planet_info->data.factories).isKnown()) {
//             data.planet.factories = planet_info->industry_level + 30000;
//         }
//         if (base_info) {
//             data.planet.build_base = 1;
//         }
//     } else {
//         data.planet                = blank_planet;
//         data.time[ts_Minerals]     = data.time[ts_Colonists] = data.time[ts_Natives] = data.time[ts_Cash] = 0;
//         data.known_to_have_natives = 0;
//     }
// }
//
//
//
// /** Notify planet of ship in orbit. This will compute the planet markings. */
// void
// GPlanet::setShipInOrbit(const GShip& s)
// {
//     switch (getPlayerRelation(s.getOwner())) {
//      case is_Me:
//         map_info.orbit_flags |= FlagOwnShipsInOrbit;
//         break;
//      case is_Enemy:
//         if (s.isReliablyVisible(0))
//             map_info.orbit_flags |= FlagEnemyShipsInOrbit;
//         else
//             map_info.orbit_flags |= FlagGuessedEnemyInOrbit;
//         break;
//      case is_Ally:
//         if (s.isReliablyVisible(0))
//             map_info.orbit_flags |= FlagAlliedShipsInOrbit;
//         else
//             map_info.orbit_flags |= FlagGuessedAllyInOrbit;
//         break;
//     }
// }
//
// /** Get planet kind. This determines that available information on the planet. */
// GPlanet::PlanetKind
// GPlanet::getPlanetKind() const
// {
//     return map_info.planet_kind;
// }
//
// /** Get starbase kind. This determines that available information on the starbase. */
// GPlanet::BaseKind
// GPlanet::getBaseKind() const
// {
//     return m_baseKind;
// }
//
// /*
//  *  Cargo Accessors
//  */
//
// void
// GPlanet::changeCargoRaw(GCargoType type, int32 amount)
// {
//     if (!amount)
//         return;
//
//     // FIXME
//     ASSERT(hasFullPlanetData());
//     markDirty();
//
//     switch(type) {
//      case el_Neutronium:
//      case el_Tritanium:
//      case el_Duranium:
//      case el_Molybdenum:
//         planet_info->data.ore_mined[type] += amount;
//         return;
//      case el_Supplies:
//         planet_info->data.supplies += amount;
//         return;
//      case el_Money:
//         planet_info->data.money += amount;
//         return;
//      case el_Fighters:
//         ASSERT(base_info);
//         base_info->data.ammo_store[10] += amount;
//         return;
//      case el_Colonists:
//         planet_info->data.colonists += amount;
//         return;
//      default:
//         ASSERT(base_info);
//         ASSERT(type >= el_Torps && type < el_Torps + NUM_TORPS);
//         base_info->data.ammo_store[type - el_Torps] += amount;
//         return;
//     }
// }
//
//
// /** Get amount of ammunition. This is a convenience function to simplify implementation
//     of the script interface.
//     \param slot [1,11], where [1,10] are the torps, 11 is fighters */
// mp16_t
// GPlanet::getBaseAmmoStore(int slot) const
// {
//     ASSERT(slot > 0 && slot <= NUM_TORPS+1);
//     mn32_t tmp;
//     if (slot > 0 && slot <= NUM_TORPS) {
//         tmp = getCargo(getCargoTypeFromTorpType(slot));
//     } else {
//         tmp = getCargo(el_Fighters);
//     }
//     if (tmp.isKnown() && tmp >= 0) {
//         return mp16_t(int32_t(tmp));
//     } else {
//         return mp16_t();
//     }
// }
//
//
//
//
//
// /** Get ship which is trying to clone at this planet. This runs
//     through the ship list, so use with care.
//     \param univ universe which contains the ships */
// int
// GPlanet::findShipCloningHere(const GUniverse& univ) const
// {
//     // FIXME: check whether 'cln' works
//     if (!config.AllowShipCloning())
//         return 0;
//     for (int i = univ.ty_any_ships.findNextIndex(0); i != 0; i = univ.ty_any_ships.findNextIndex(i)) {
//         if (univ.getShip(i).isCloningAt(*this)) {
//             return i;
//         }
//     }
//     return 0;
// }
