/**
  *  \file game/vcr/object.cpp
  *  \brief Class game::vcr::Object
  */

#include "game/vcr/object.hpp"
#include "afl/string/format.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;

// Default constructor.
game::vcr::Object::Object()
    : m_data(),
      m_name()
{
    // ex GVcrObject::GVcrObject
    m_data.beamKillRate    = 1;
    m_data.beamChargeRate  = 1;
    m_data.torpMissRate    = 35;
    m_data.torpChargeRate  = 1;
    m_data.crewDefenseRate = 0;
}

/** Destructor. */
game::vcr::Object::~Object()
{
    // ex GVcrObject::~GVcrObject
}

// Remember guessed hull.
void
game::vcr::Object::setGuessedHull(const game::spec::HullVector_t& hulls)
{
    // ex GVcrObject::setGuessedHull()
    setHull(getGuessedHull(hulls));
}

// Check if this could be the specified hull.
bool
game::vcr::Object::canBeHull(const game::spec::HullVector_t& hulls, int hullId) const
{
    // ex GVcrObject::canBeHull, vcrplay.pas:CanBe
    const game::spec::Hull* theHull = hulls.get(hullId);
    if (theHull == 0) {
        // Hull does not exist
        return false;
    } else if (isPlanet()) {
        // I'm a planet
        return false;
    } else if (getHull() != 0) {
        // Hull is known
        return getHull() == hullId;
    } else {
        /* This checks the same properties as PCC 1.x does. It does not check:
           - Mass. Normally, the ship shouldn't be lighter than its hull's mass,
             but since balancing approaches toy around with the mass, we don't
             trust it too much.
           - Crew. The crew can be larger (tow-capture bug) or smaller than
             the hull's standard crew. */

        /* Picture must match.
           THost has an easter egg where it reports Nebulas (picture 16) with picture 30
           instead when they have Transwarp Drives. */
        if (getPicture() != theHull->getExternalPictureNumber()
            && (getPicture() != 30
                || theHull->getExternalPictureNumber() != 16))
        {
            return false;
        }

        /* Must not have more beams/torps than hull allows */
        if (getNumBeams() > theHull->getMaxBeams() || getNumLaunchers() > theHull->getMaxLaunchers()) {
            return false;
        }

        /* For fighter bays, the only criterion is that ship has fighters but hull has not.
           The number of bays can be smaller (damage), zero (NTP) or larger (scotty bonus). */
        if (getNumBays() != 0 && theHull->getNumBays() == 0) {
            return false;
        }
        return true;
    }
}

// Guess this ship's hull.
int
game::vcr::Object::getGuessedHull(const game::spec::HullVector_t& hulls) const
{
    // ex GVcrObject::getGuessedHull, vcrplay.pas:IdentifyHull
    // planets don't have hulls
    if (isPlanet()) {
        return 0;
    }

    // see if PHost sent us the hull type
    if (int h = getHull()) {
        return h;
    }

    // otherwise, try all hulls.
    int type = 0;
    for (game::spec::Hull* p = hulls.findNext(0); p != 0; p = hulls.findNext(p->getId())) {
        int id = p->getId();
        if (canBeHull(hulls, id)) {
            if (type == 0) {
                type = id;
            } else {
                return 0;       // ambiguous, can't guess
            }
        }
    }
    return type;
}

int
game::vcr::Object::getGuessedShipPicture(const game::spec::HullVector_t& hulls) const
{
    // ex GVcrObject::getGuessedShipPicture, vcrplay.pas:GetVcrPicture
    if (isPlanet()) {
        return 0;
    } else if (const game::spec::Hull* hull = hulls.get(getGuessedHull(hulls))) {
        return hull->getInternalPictureNumber();
    } else {
        return getPicture();
    }
}

int
game::vcr::Object::getGuessedEngine(const game::spec::EngineVector_t& engines,
                                    const game::spec::Hull* pAssumedHull,
                                    bool withESB,
                                    const game::config::HostConfiguration& config) const
{
    // ex client/widgets/vcrinfomain.cc:identifyEngine, vcrplay.pas:IdentifyEngine
    // Don't guess if we don't know the hull
    if (isPlanet() || pAssumedHull == 0) {
        return 0;
    }

    // Compute effective ESB.
    int32_t esb;
    if (withESB) {
        esb = config[config.EngineShieldBonusRate](getOwner());
    } else {
        esb = 0;
    }

    if (config[config.NumExperienceLevels]() > 0 && getExperienceLevel() > 0) {
        esb += config[config.EModEngineShieldBonusRate](getExperienceLevel());
    }

    // Figure out mass that must be accounted for by ESB
    int32_t massDiff = getMass() - pAssumedHull->getMass();
    if (config.getPlayerRaceNumber(getOwner()) == 1) {
        // Scotty bonus
        massDiff -= 50;
    }

    // Is 360 kt bonus applicable?
    bool is360 = (getMass() > 140+360 && getNumBays() > 0);

    int result = 0;
    for (int i = 1, n = engines.size(); i <= n; ++i) {
        if (const game::spec::Engine* p = engines.get(i)) {
            int32_t thisESB = esb * p->cost().get(game::spec::Cost::Money) / 100;
            int32_t remain = massDiff - thisESB;
            if (remain == 0 || (is360 && remain == 360)) {
                if (result != 0) {
                    return 0;
                } else {
                    result = i;
                }
            }
        }
    }
    return result;
}

// Get mass for build point computation.
int
game::vcr::Object::getBuildPointMass(const game::config::HostConfiguration& config,
                                     const game::spec::ShipList& shipList,
                                     bool isPHost) const
{
    // vcrplay.pas::PALMass, game/classicvcr.cc:getBuildPointMass
    int guessedHull = getGuessedHull(shipList.hulls());
    if (isPlanet()) {
        // planet
        return getMass() - 100;
    } else if ((!isPHost || !config[config.PALIncludesESB](getOwner())) && guessedHull != 0) {
        // ship, type known, and we have HOST or PHost where PAL does not include ESB
        if (const game::spec::Hull* hull = shipList.hulls().get(guessedHull)) {
            return hull->getMass();
        } else {
            return getMass();
        }
    } else {
        // ship, type unknown, or build points include ESB
        return getMass();
    }
}

// Check for freighter.
bool
game::vcr::Object::isFreighter() const
{
    // ex GVcrObject::isFreighter, ccvcr.pas:IsFreighter
    return getNumBeams() == 0
        && getNumLaunchers() == 0
        && getNumBays() == 0;
}

// Apply classic shield limits.
void
game::vcr::Object::applyClassicLimits()
{
    // ex GVcrObject::applyClassicLimits
    setShield(std::max(0, std::min(getShield(), 100 - getDamage())));

    if (!isPlanet()) {
        if (isFreighter()) {
            setShield(0);
        }
    } else {
        if (getCrew() <= 0) {
            setShield(0);
        }
    }
}

// Format this object into human-readable form.
game::vcr::ObjectInfo
game::vcr::Object::describe(const TeamSettings* teamSettings, const Root* root, const game::spec::ShipList* shipList, afl::string::Translator& tx) const
{
    // ex WVcrSelector::drawWarrior, WVcrSelector::drawContent, vcrplay.pas:ShowWarrior
    // ex c2ng ClassicVcrDialog::prepareWarrior
    ObjectInfo result;

    // Environment
    if (root == 0 || shipList == 0) {
        // Low-fi version
        // FIXME: consider removing this case
        result.text[0] = getName();
        return result;
    }

    size_t line = 0;
    result.text[line] = Format(tx("%s (%s)"), getName(), getSubtitle(teamSettings, *root, *shipList, tx));
    result.color[line] = teamSettings != 0 ? teamSettings->getPlayerColor(getOwner()) : util::SkinColor::Static;
    ++line;

    // Shield, Damage, Crew
    int shield = std::max(0, getShield());
    result.text[line] = Format(tx("%d%% shield (%d kt), %d%% damaged"), shield, getMass(), getDamage());
    if (!isPlanet()) {
        util::addListItem(result.text[line], ", ", Format(tx("%d %1{crewman%|crewmen%}"), root->userConfiguration().formatNumber(getCrew())));
    }
    switch (getRole()) {
     case NoRole:                                                               break;
     case AggressorRole: util::addListItem(result.text[line], ", ", tx("aggressor")); break;
     case OpponentRole:  util::addListItem(result.text[line], ", ", tx("opponent"));  break;
    }
    ++line;

    // Beams
    if (getNumBeams() > 0) {
        if (const game::spec::Beam* b = shipList->beams().get(getBeamType())) {
            result.text[line] = Format("%d " UTF_TIMES " %s", getNumBeams(), b->getName(shipList->componentNamer()));
        } else {
            result.text[line] = Format("%d beam weapon%!1{s%}", getNumBeams());
        }
        ++line;
    }

    // Torps/Fighters
    if (getNumBays() > 0) {
        if (getNumLaunchers() > 0) {
            if (const game::spec::TorpedoLauncher* tl = shipList->launchers().get(getTorpedoType())) {
                result.text[line] = Format(tx("%d %1{%s%|%ss%} and %d %1{fighter%|fighters%}"), getNumTorpedoes(), tl->getName(shipList->componentNamer()), getNumFighters());
            } else {
                result.text[line] = Format(tx("%d torpedo%!1{es%} and %d %1{fighter%|fighters%}"), getNumTorpedoes(), getNumFighters());
            }
        } else {
            result.text[line] = Format(tx("%d fighter bay%!1{s%} with %d fighter%!1{s%}"), getNumBays(), getNumFighters());
        }
        ++line;
    } else if (getNumLaunchers() > 0) {
        if (const game::spec::TorpedoLauncher* tl = shipList->launchers().get(getTorpedoType())) {
            result.text[line] = Format(tx("%d \xC3\x97 %1{%s launcher%|%s launchers%} with %d torpedo%!1{es%}"), getNumLaunchers(), tl->getName(shipList->componentNamer()),
                                       root->userConfiguration().formatNumber(getNumTorpedoes()));
        } else {
            result.text[line] = Format(tx("%d \xC3\x97 torpedo launcher%!1{s%} with %d torpedo%!1{es%}"), getNumLaunchers(),
                                       root->userConfiguration().formatNumber(getNumTorpedoes()));
        }
        ++line;
    } else {
        // No auxiliary weapons.
        // We can still give more info.
        // When "NTP" is used, THost clears the "count" field, but keeps type and ammo count intact (thanks, Akseli, for that one!)
        // We can still give the number of bays if we know the hull.
        // PHost makes the ship appear with the correct weapon count, but no ammo, so this doesn't work here.
        if (const game::spec::Hull* pHull = shipList->hulls().get(getGuessedHull(shipList->hulls()))) {
            if (pHull->getNumBays() > 0) {
                result.text[line] = Format(tx("(%d fighter bay%!1{s%} %snot used)"),
                                           pHull->getNumBays(),
                                           (getNumFighters() > 0
                                            ? String_t(Format(tx("with %d fighter%!1{s%} "), root->userConfiguration().formatNumber(getNumFighters())))
                                            : String_t()));
                result.color[line] = util::SkinColor::Faded;
                ++line;
            } else if (pHull->getMaxLaunchers() > 0) {
                const game::spec::TorpedoLauncher* tl = shipList->launchers().get(getTorpedoType());
                result.text[line] = Format(tx("(up to %s %snot used)"),
                                           (tl != 0
                                            ? String_t(Format(tx("%d %s%!1{s%}"), pHull->getMaxLaunchers(), tl->getName(shipList->componentNamer())))
                                            : String_t(Format(tx("%d torpedo launcher%!1{s%}"), pHull->getMaxLaunchers()))),
                                           (getNumTorpedoes() > 0
                                            ? String_t(Format(tx("with %d torp%!1{s%} "), root->userConfiguration().formatNumber(getNumTorpedoes())))
                                            : String_t()));
                result.color[line] = util::SkinColor::Faded;
                ++line;
            } else {
                // No additional info possible
            }
        }
    }
    return result;
}

String_t
game::vcr::Object::getSubtitle(const TeamSettings* teamSettings, const Root& root, const game::spec::ShipList& shipList, afl::string::Translator& tx) const
{
    int viewpointPlayer = teamSettings != 0 ? teamSettings->getViewpointPlayer() : 0;

    // FIXME: this i18n approach is far from perfect
    // We have the following combinations:
    //    {A <race>|Our} {<Level>|(nothing)} {planet|<type>|starship}
    // Giving a total of 2x2x3 = 12 sentences.

    // Object title
    String_t adj = (getOwner() == viewpointPlayer
                    ? tx("our")
                    : String_t(Format(tx("a %s"), root.playerList().getPlayerName(getOwner(), Player::AdjectiveName))));

    // Experience
    String_t type;
    if (root.hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels]() > 0) {
        type += root.hostConfiguration().getExperienceLevelName(getExperienceLevel(), tx);
        type += ' ';
    }

    // Type
    if (isPlanet()) {
        type += tx("planet");
    } else if (const game::spec::Hull* pHull = shipList.hulls().get(getGuessedHull(shipList.hulls()))) {
        type += pHull->getName(shipList.componentNamer());
    } else {
        type += tx("starship");
    }

    return Format(tx("Id #%d, %s %s"), getId(), adj, type);
}
