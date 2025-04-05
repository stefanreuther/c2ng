/**
  *  \file game/sim/object.cpp
  *  \brief Class game::sim::Object
  */

#include "game/sim/object.hpp"
#include "util/updater.hpp"

using util::Updater;

const int32_t game::sim::Object::fl_RandomFC,
    game::sim::Object::fl_RandomFC1,
    game::sim::Object::fl_RandomFC2,
    game::sim::Object::fl_RandomFC3,
    game::sim::Object::fl_RatingOverride,
    game::sim::Object::fl_Cloaked,
    game::sim::Object::fl_Deactivated,
    game::sim::Object::fl_PlanetImmunity,
    game::sim::Object::fl_PlanetImmunitySet,
    game::sim::Object::fl_FullWeaponry,
    game::sim::Object::fl_FullWeaponrySet,
    game::sim::Object::fl_Commander,
    game::sim::Object::fl_CommanderSet,
    game::sim::Object::fl_WasCaptured,
    game::sim::Object::fl_TripleBeamKill,
    game::sim::Object::fl_TripleBeamKillSet,
    game::sim::Object::fl_DoubleBeamCharge,
    game::sim::Object::fl_DoubleBeamChargeSet,
    game::sim::Object::fl_DoubleTorpCharge,
    game::sim::Object::fl_DoubleTorpChargeSet,
    game::sim::Object::fl_Elusive,
    game::sim::Object::fl_ElusiveSet,
    game::sim::Object::fl_Squadron,
    game::sim::Object::fl_SquadronSet,
    game::sim::Object::fl_ShieldGenerator,
    game::sim::Object::fl_ShieldGeneratorSet,
    game::sim::Object::fl_CloakedBays,
    game::sim::Object::fl_CloakedBaysSet,
    game::sim::Object::fl_RandomDigits,
    game::sim::Object::fl_FunctionSetBits;

// Default constructor.
game::sim::Object::Object()
    : m_id(1),
      m_name("?"),
      m_friendlyCode("?""?""?"),  // avoid hitting trigraphs!
      m_damage(0),
      m_shield(100),
      m_owner(12),
      m_experienceLevel(0),
      m_flags(0),
      m_flakRatingOverride(0),
      m_flakCompensationOverride(0),
      m_changed(false)
{ }

// Copy constructor.
game::sim::Object::Object(const Object& obj)
    : m_id(obj.m_id),
      m_name(obj.m_name),
      m_friendlyCode(obj.m_friendlyCode),
      m_damage(obj.m_damage),
      m_shield(obj.m_shield),
      m_owner(obj.m_owner),
      m_experienceLevel(obj.m_experienceLevel),
      m_flags(obj.m_flags),
      m_flakRatingOverride(obj.m_flakRatingOverride),
      m_flakCompensationOverride(obj.m_flakCompensationOverride),
      m_changed(false)
{ }

// Destructor.
game::sim::Object::~Object()
{ }

// Assignment operator.
game::sim::Object&
game::sim::Object::operator=(const Object& other)
{
    setId(other.m_id);
    setName(other.m_name);
    setFriendlyCode(other.m_friendlyCode);
    setDamage(other.m_damage);
    setShield(other.m_shield);
    setOwner(other.m_owner);
    setExperienceLevel(other.m_experienceLevel);
    setFlags(other.m_flags);
    setFlakRatingOverride(other.m_flakRatingOverride);
    setFlakCompensationOverride(other.m_flakCompensationOverride);
    return *this;
}

// Get object Id.
game::Id_t
game::sim::Object::getId() const
{
    // ex GSimObject::getId
    return m_id;
}

// Set object Id.
void
game::sim::Object::setId(Id_t id)
{
    // ex GSimObject::setId
    if (Updater().set(m_id, id)) {
        markDirty();
    }
}

// Get name.
String_t
game::sim::Object::getName() const
{
    // ex GSimObject::getName
    return m_name;
}

// Set name.
void
game::sim::Object::setName(String_t name)
{
    // ex GSimObject::setName
    // FIXME: can we move this into Ship? Planet does not need it.
    if (Updater().set(m_name, name)) {
        markDirty();
    }
}

// Get friendly code.
String_t
game::sim::Object::getFriendlyCode() const
{
    // ex GSimObject::getFCode
    return m_friendlyCode;
}

// Set friendly code.
void
game::sim::Object::setFriendlyCode(String_t fcode)
{
    // ex GSimObject::setFCode
    if (Updater().set(m_friendlyCode, fcode)) {
        markDirty();
    }
}

// Get damage.
int
game::sim::Object::getDamage() const
{
    // ex GSimObject::getDamage
    return m_damage;
}

// Set damage.
void
game::sim::Object::setDamage(int damage)
{
    // ex GSimObject::setDamage
    if (Updater().set(m_damage, damage)) {
        markDirty();
    }
}

// Get shield level.
int
game::sim::Object::getShield() const
{
    // ex GSimObject::getShield
    return m_shield;
}

// Set shield level.
void
game::sim::Object::setShield(int shield)
{
    // ex GSimObject::setShield
    if (Updater().set(m_shield, shield)) {
        markDirty();
    }
}

// Get owner.
int
game::sim::Object::getOwner() const
{
    // ex GSimObject::getOwner
    return m_owner;
}

// Set owner.
void
game::sim::Object::setOwner(int owner)
{
    // ex GSimObject::setOwner
    if (Updater().set(m_owner, owner)) {
        markDirty();
    }
}

// Get experience level.
int
game::sim::Object::getExperienceLevel() const
{
    // ex GSimObject::getExperienceLevel
    return m_experienceLevel;
}

// Set experience level.
void
game::sim::Object::setExperienceLevel(int experienceLevel)
{
    // ex GSimObject::setExperienceLevel
    if (Updater().set(m_experienceLevel, experienceLevel)) {
        markDirty();
    }
}

// Get flags.
int32_t
game::sim::Object::getFlags() const
{
    // ex GSimObject::getFlags
    return m_flags;
}

// Set flags.
void
game::sim::Object::setFlags(int32_t flags)
{
    // ex GSimObject::setFlags
    if (Updater().set(m_flags, flags)) {
        markDirty();
    }
}

// Get FLAK rating override.
int32_t
game::sim::Object::getFlakRatingOverride() const
{
    // ex GSimObject::getFlakRatingOverride
    return m_flakRatingOverride;
}

// Set FLAK rating override.
void
game::sim::Object::setFlakRatingOverride(int32_t r)
{
    // ex GSimObject::setFlakRatingOverride
    if (Updater().set(m_flakRatingOverride, r)) {
        markDirty();
    }
}

// Get FLAK compensation override.
int
game::sim::Object::getFlakCompensationOverride() const
{
    // ex GSimObject::getFlakCompensationOverride
    return m_flakCompensationOverride;
}

// Set FLAK compensation override.
void
game::sim::Object::setFlakCompensationOverride(int r)
{
    // ex GSimObject::setFlakCompensationOverride
    if (Updater().set(m_flakCompensationOverride, r)) {
        markDirty();
    }
}

// Assign random friendly code if requested.
void
game::sim::Object::setRandomFriendlyCode(util::RandomNumberGenerator& rng)
{
    // ex GSimObject::assignRandomFCode, ccsim.pas:AssignRandomFCs
    if (m_flags & fl_RandomFC) {
        for (size_t i = 0; i < 3; ++i) {
            if (m_friendlyCode.size() <= i) {
                m_friendlyCode += ' ';
            }
            if (shouldRandomize(m_flags, i)) {
                m_friendlyCode[i] = static_cast<char>('0' + rng(10));
            }
        }
        markDirty();
    }
}

// Assign random friendly code flags.
bool
game::sim::Object::setRandomFriendlyCodeFlags()
{
    // ex GSimObject::assignRandomFCodeFlags
    int32_t newFlags = getFlags() & ~(fl_RandomFC | fl_RandomDigits);
    for (size_t i = 0; i < 3 && i < m_friendlyCode.size(); ++i) {
        if (m_friendlyCode[i] == '#') {
            newFlags |= (fl_RandomFC1 << i);
        }
    }
    if ((newFlags & fl_RandomDigits) != 0) {
        newFlags |= fl_RandomFC;
    }

    setFlags(newFlags);

    return (newFlags & fl_RandomFC) != 0;
}

// Check effective availability of an ability.
bool
game::sim::Object::hasAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    // ex GSimObject::hasFunction, ccsim.pas:SimShipDoes
    // Get bits (ex simf_to_value_bit, simf_to_set_bit)
    AbilityInfo info = getAbilityInfo(which);

    int32_t flags = getFlags();
    if ((flags & info.setBit) != 0) {
        return (flags & info.activeBit) != 0;
    } else {
        return hasImpliedAbility(which, opts, shipList, config);
    }
}

// Check presence of any nonstandard ability.
bool
game::sim::Object::hasAnyNonstandardAbility() const
{
    // ex GSimObject::hasAnyNonstandardFunction
    return (getFlags() & fl_FunctionSetBits) != 0;
}

// Get set of all abilities.
game::sim::Abilities_t
game::sim::Object::getAbilities(const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    Abilities_t result;
    for (int i = FIRST_ABILITY; i <= LAST_ABILITY; ++i) {
        Ability a = Ability(i);
        if (hasAbility(a, opts, shipList, config)) {
            result += a;
        }
    }
    return result;
}

// Mark dirty.
void
game::sim::Object::markDirty()
{
    // ex GSimObject::setChanged
    m_changed = true;
}

// Mark clean.
void
game::sim::Object::markClean()
{
    // ex GSimObject::setUnchanged
    m_changed = false;
}

// Check dirtiness.
bool
game::sim::Object::isDirty() const
{
    // ex GSimObject::isChanged
    return m_changed;
}

game::sim::Object::AbilityInfo
game::sim::Object::getAbilityInfo(Ability a)
{
    switch (a) {
     case PlanetImmunityAbility:       return AbilityInfo(fl_PlanetImmunitySet,   fl_PlanetImmunity);
     case FullWeaponryAbility:         return AbilityInfo(fl_FullWeaponrySet,     fl_FullWeaponry);
     case CommanderAbility:            return AbilityInfo(fl_CommanderSet,        fl_Commander);
     case TripleBeamKillAbility:       return AbilityInfo(fl_TripleBeamKillSet,   fl_TripleBeamKill);
     case DoubleBeamChargeAbility:     return AbilityInfo(fl_DoubleBeamChargeSet, fl_DoubleBeamCharge);
     case DoubleTorpedoChargeAbility:  return AbilityInfo(fl_DoubleTorpChargeSet, fl_DoubleTorpCharge);
     case ElusiveAbility:              return AbilityInfo(fl_ElusiveSet,          fl_Elusive);
     case SquadronAbility:             return AbilityInfo(fl_SquadronSet,         fl_Squadron);
     case ShieldGeneratorAbility:      return AbilityInfo(fl_ShieldGeneratorSet,  fl_ShieldGenerator);
     case CloakedBaysAbility:          return AbilityInfo(fl_CloakedBaysSet,      fl_CloakedBays);
    }
    return AbilityInfo(0, 0);
}

bool
game::sim::Object::shouldRandomize(int32_t flags, size_t pos)
{
    if ((flags & fl_RandomFC) != 0) {
        int32_t which = (flags & fl_RandomDigits);
        if (which == 0) {
            return true;
        } else {
            return ((which & (fl_RandomFC1 << pos)) != 0);
        }
    } else {
        return false;
    }
}
