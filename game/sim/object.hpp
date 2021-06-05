/**
  *  \file game/sim/object.hpp
  *  \brief Class game::sim::Object
  */
#ifndef C2NG_GAME_SIM_OBJECT_HPP
#define C2NG_GAME_SIM_OBJECT_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/sim/ability.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace sim {

    class Configuration;

    /** Base class for simulator objects.
        Contains the definition and state of an object.
        All properties are freely editable.
        This object cannot be instantiated on its own; use Planet/Ship instead. */
    class Object : public afl::base::Deletable {
     public:
        /** Default constructor.
            Initializes all properties to default values. */
        Object();

        /** Copy constructor. */
        Object(const Object& obj);

        /** Destructor. */
        ~Object();

        /** Assignment operator.
            \param rhs Other object
            \return this */
        Object& operator=(const Object& other);


        /*
         *  Attributes
         */

        /** Get object Id.
            \return Id */
        Id_t getId() const;

        /** Set object Id.
            \param id Id */
        void setId(Id_t id);

        /** Get name.
            \return name */
        String_t getName() const;

        /** Set name.
            \param name Name */
        void setName(String_t name);

        /** Get friendly code.
            \return friendly code */
        String_t getFriendlyCode() const;

        /** Set friendly code.
            \param fcode friendly code */
        void setFriendlyCode(String_t fcode);

        /** Get damage.
            \return damage */
        int getDamage() const;

        /** Set damage.
            \param damage Damage */
        void setDamage(int damage);

        /** Get shield level.
            \return shield level */
        int getShield() const;

        /** Set shield level.
            \param shield shield level */
        void setShield(int shield);

        /** Get owner.
            \return owner */
        int getOwner() const;

        /** Set owner.
            \param owner owner */
        void setOwner(int owner);

        /** Get experience level.
            \return experience level */
        int getExperienceLevel() const;

        /** Set experience level.
            \param experienceLevel experience level */
        void setExperienceLevel(int experienceLevel);

        /** Get flags.
            \return flags (fl_XXX) */
        int32_t getFlags() const;

        /** Set flags.
            \param flags new flags (fl_XXX) */
        void setFlags(int32_t flags);

        /** Get FLAK rating override.
            \return value */
        int32_t getFlakRatingOverride() const;

        /** Set FLAK rating override.
            The value is used only if the fl_RatingOverride flag is set.
            \param r value */
        void setFlakRatingOverride(int32_t r);

        /** Get FLAK compensation override.
            \return value */
        int getFlakCompensationOverride() const;

        /** Set FLAK compensation override.
            The value is used only if the fl_RatingOverride flag is set.
            \param r value */
        void setFlakCompensationOverride(int r);

        /*
         *  Random Friendly Codes
         */

        /** Assign random friendly code if requested.
            Considers fl_RandomFC and the fl_RandomDigits flags to assign a new, (partially) numeric friendly code.
            \param rng Random Number Generator */
        void setRandomFriendlyCode(util::RandomNumberGenerator& rng);

        /** Assign random friendly code flags.
            Derives fl_RandomFC and fl_RandomDigits from the actual friendly code selected.
            \return true iff fl_RandomFC has been enabled */
        bool setRandomFriendlyCodeFlags();

        /*
         *  Abilities
         */

        /** Check effective availability of an ability.
            Checks whether the ability has been configured by the user (fl_XXXSet flag);
            otherwise, queries hasImpliedAbility().
            \param which Ability to check for
            \param opts Simulation options
            \param shipList Ship list
            \param config Host configuration
            \return true if ability is available */
        bool hasAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

        /** Check presence of any nonstandard ability.
            \retval true at least one ability has been configured by the user (fl_XXXSet)
            \retval false all abilities are at default values as configured by shiplist/host */
        bool hasAnyNonstandardAbility() const;

        /** Get set of all abilities.
            Returns all results of hasAbility() in one go.
            \param opts Simulation options
            \param shipList Ship list
            \param config Host configuration
            \return set of abilities */
        Abilities_t getAbilities(const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;

        /** Check availability of an ability according to ship list and host configuration.
            This function is called to determine the default abilities,
            if the availability of that ability has not been configured explicitly.
            \param which Ability to check for
            \param opts Simulation options
            \param shipList Ship list
            \param config Host configuration
            \return true if ability is available */
        virtual bool hasImpliedAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const = 0;

        /*
         *  Dirtiness
         */

        /** Mark dirty.
            Called by all modifier functions. */
        void markDirty();

        /** Mark clean. */
        void markClean();

        /** Check dirtiness.
            \return true if object has been modified since last markClean(). */
        bool isDirty() const;

        /*
         *  Flags
         *
         *  FIXME: the names are taken from PCC2 and probably sub-par.
         */

        static const int32_t fl_RandomFC          = 1;    ///< Friendly code randomisation enabled.
        static const int32_t fl_RandomFC1         = 2;    ///< Randomize first place of fcode.
        static const int32_t fl_RandomFC2         = 4;    ///< Randomize second place of fcode.
        static const int32_t fl_RandomFC3         = 8;    ///< Randomize third place of fcode.
        static const int32_t fl_RatingOverride    = 16;   ///< Use FLAK rating overrides.
        static const int32_t fl_Cloaked           = 32;   ///< Ship is cloaked.
        static const int32_t fl_Deactivated       = 64;   ///< Ignore this ship during simulation.
        static const int32_t fl_PlanetImmunity    = 128;  ///< Ship has Planet Immunity.
        static const int32_t fl_PlanetImmunitySet = 256;  ///< PlanetImmunity bit is effective.
        static const int32_t fl_FullWeaponry      = 512;  ///< Ship has Full Weaponry.
        static const int32_t fl_FullWeaponrySet   = 1024; ///< FullWeaponry bit is effective.
        static const int32_t fl_Commander         = 2048; ///< Ship is Commander.
        static const int32_t fl_CommanderSet      = 4096; ///< Commander bit is effective.

        static const int32_t fl_WasCaptured       = 8192; ///< (Runtime only) Set if ship was captured.

        static const int32_t fl_TripleBeamKill      = 1*65536;     ///< Ship has 3x beam kill.
        static const int32_t fl_TripleBeamKillSet   = 2*65536;     ///< TripleBeamKill bit is effective.
        static const int32_t fl_DoubleBeamCharge    = 4*65536;     ///< Ship has 2x beam recharge.
        static const int32_t fl_DoubleBeamChargeSet = 8*65536;     ///< DoubleBeamCharge bit is effective.
        static const int32_t fl_DoubleTorpCharge    = 16*65536;    ///< Ship has 2x torp recharge.
        static const int32_t fl_DoubleTorpChargeSet = 32*65536;    ///< DoubleTorpCharge bit is effective.
        static const int32_t fl_Elusive             = 64*65536;    ///< Ship sets enemy's torp hit odds to 10%
        static const int32_t fl_ElusiveSet          = 128*65536;   ///< Elusive bit is effective.
        static const int32_t fl_Squadron            = 256*65536;   ///< Ship is a fighter squadron (unkillable, respawns).
        static const int32_t fl_SquadronSet         = 512*65536;   ///< Squadron bit is effective.
        static const int32_t fl_ShieldGenerator     = 1024*65536;  ///< Ship has a shield generator.
        static const int32_t fl_ShieldGeneratorSet  = 2048*65536;  ///< Shield generator bit is effective.
        static const int32_t fl_CloakedBays         = 4096*65536;  ///< Ship has cloaked fighter bays.
        static const int32_t fl_CloakedBaysSet      = 8192*65536;  ///< Cloaked fighter bays bit is effective.

        static const int32_t fl_RandomDigits      = fl_RandomFC1 + fl_RandomFC2 + fl_RandomFC3;
        static const int32_t fl_FunctionSetBits   = (fl_PlanetImmunitySet | fl_CommanderSet | fl_FullWeaponrySet
                                                     | fl_TripleBeamKillSet | fl_DoubleBeamChargeSet | fl_DoubleTorpChargeSet
                                                     | fl_ElusiveSet | fl_SquadronSet | fl_ShieldGeneratorSet | fl_CloakedBaysSet);


        /** Description of an ability. */
        struct AbilityInfo {
            /** "set" bit.
                If this flag is clear, the unit has its ability at its default value.
                If this flag is set, presence of the ability is determined by the activeBit. */
            int32_t setBit;
            /** "active" bit.
                This flag is valid if the "set" bit is set, and determines whether the unit has the ability (ability is active). */
            int32_t activeBit;

            AbilityInfo(int32_t setBit, int32_t activeBit)
                : setBit(setBit), activeBit(activeBit)
                { }
        };

        /** Get description for a unit's ability.
            \param a Ability
            \return Description */
        static AbilityInfo getAbilityInfo(Ability a);

     private:
        // common
        Id_t m_id;                       // id
        String_t m_name;                 // name
        String_t m_friendlyCode;         // fcode
        int m_damage;                    // damage
        int m_shield;                    // shield
        int m_owner;                     // owner
        int m_experienceLevel;           // experience_level
        int32_t m_flags;                 // flags
        int32_t m_flakRatingOverride;    // flak_r_override
        int m_flakCompensationOverride;  // flak_c_override

        bool m_changed;

        /* PCC2 included a VcrStatItem / game::vcr::Statistic in GSimObject.
           We handle these externally. */
    };

} }

#endif
