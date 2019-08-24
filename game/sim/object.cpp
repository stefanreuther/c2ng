/**
  *  \file game/sim/object.cpp
  *  \brief Class game::sim::Object
  */

#include <cstdlib>
#include "game/sim/object.hpp"

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

// Destructor.
game::sim::Object::~Object()
{ }


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
    m_id = id;
    markDirty();
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
    m_name = name;
    markDirty();
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
    m_friendlyCode = fcode;
    markDirty();
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
    m_damage = damage;
    markDirty();
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
    m_shield = shield;
    markDirty();
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
    m_owner = owner;
    markDirty();
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
    m_experienceLevel = experienceLevel;
    markDirty();
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
    m_flags = flags;
    markDirty();
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
    m_flakRatingOverride = r;
    markDirty();
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
    m_flakCompensationOverride = r;
    markDirty();
}

// Assign random friendly code if requested.
void
game::sim::Object::setRandomFriendlyCode()
{
    // ex GSimObject::assignRandomFCode
    if (m_flags & fl_RandomFC) {
        int32_t which = m_flags & fl_RandomDigits;
        if (which == 0) {
            which = fl_RandomDigits;
        }
        for (size_t i = 0; i < 3; ++i) {
            if (m_friendlyCode.size() <= i) {
                m_friendlyCode += ' ';
            }
            if ((which & (fl_RandomFC1 << i)) != 0) {
                m_friendlyCode[i] = static_cast<char>('0' + (std::rand() % 10));
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
game::sim::Object::hasAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    // ex GSimObject::hasFunction
    // Get bits (ex simf_to_value_bit, simf_to_set_bit)
    int32_t setBit = 0, activeBit = 0;
    switch (which) {
     case PlanetImmunityAbility:       setBit = fl_PlanetImmunitySet;   activeBit = fl_PlanetImmunity;   break;
     case FullWeaponryAbility:         setBit = fl_FullWeaponrySet;     activeBit = fl_FullWeaponry;     break;
     case CommanderAbility:            setBit = fl_CommanderSet;        activeBit = fl_Commander;        break;
     case TripleBeamKillAbility:       setBit = fl_TripleBeamKillSet;   activeBit = fl_TripleBeamKill;   break;
     case DoubleBeamChargeAbility:     setBit = fl_DoubleBeamChargeSet; activeBit = fl_DoubleBeamCharge; break;
     case DoubleTorpedoChargeAbility:  setBit = fl_DoubleTorpChargeSet; activeBit = fl_DoubleTorpCharge; break;
     case ElusiveAbility:              setBit = fl_ElusiveSet;          activeBit = fl_Elusive;          break;
     case SquadronAbility:             setBit = fl_SquadronSet;         activeBit = fl_Squadron;         break;
     case ShieldGeneratorAbility:      setBit = fl_ShieldGeneratorSet;  activeBit = fl_ShieldGenerator;  break;
     case CloakedBaysAbility:          setBit = fl_CloakedBaysSet;      activeBit = fl_CloakedBays;      break;
    }

    int32_t flags = getFlags();
    if ((flags & setBit) != 0) {
        return (flags & activeBit) != 0;
    } else {
        return hasImpliedAbility(which, shipList, config);
    }
}

// Check presence of any nonstandard ability.
bool
game::sim::Object::hasAnyNonstandardAbility() const
{
    // ex GSimObject::hasAnyNonstandardFunction
    return (getFlags() & fl_FunctionSetBits) != 0;
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

// FIXME: do we need these?
// /** Get maximum permitted damage level. Considers owner race. Intended
//     for display / editing only, so this may allow more than the host
//     allows. */
// int
// GSimObject::getMaxDamage() const
// {
//     if (raceId(getOwner()) == 2)
//         return 150;
//     else
//         return 99;
// }

// /** Get maximum permitted shield level. Considers owner race and
//     current damage. Intended for display / editing only, so this may
//     allow more than the host allows. */
// int
// GSimObject::getMaxShield() const
// {
//     int limit = getMaxDamage() - getDamage() + 1;
//     if (limit > 100)
//         limit = 100;
//     return limit;
// }
