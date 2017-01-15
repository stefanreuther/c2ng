/**
  *  \file game/sim/object.hpp
  */
#ifndef C2NG_GAME_SIM_OBJECT_HPP
#define C2NG_GAME_SIM_OBJECT_HPP

#include "game/types.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"
#include "game/sim/ability.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace sim {

    // /*! \class GSimObject
    //   \brief Base class for simulator objects

    //   Contains the definition and state of an object. In addition to the
    //   normal specification data, this includes some counters for
    //   statistics tracking: the simulator updates these counters the same
    //   way as it updates the shield level. */
    class Object : public afl::base::Deletable {
     public:
        Object();
        ~Object();

        // Attributes:
        Id_t     getId() const;
        void     setId(Id_t id);
        String_t getName() const;
        void     setName(String_t name);
        String_t getFriendlyCode() const;
        void     setFriendlyCode(String_t fcode);
        int      getDamage() const;
        void     setDamage(int damage);
        int      getShield() const;
        void     setShield(int shield);
        int      getOwner() const;
        void     setOwner(int owner);
        int      getExperienceLevel() const;
        void     setExperienceLevel(int experienceLevel);
        int32_t  getFlags() const;
        void     setFlags(int32_t flags);
        int32_t  getFlakRatingOverride() const;
        void     setFlakRatingOverride(int32_t r);
        int      getFlakCompensationOverride() const;
        void     setFlakCompensationOverride(int r);

        // Random fcode:
        void     setRandomFriendlyCode();
        bool     setRandomFriendlyCodeFlags();

        // Abilities:
        bool     hasAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;
        bool     hasAnyNonstandardAbility() const;
        virtual bool hasImpliedAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const = 0;

        // Dirtiness:
        void markDirty();
        void markClean();
        bool isDirty() const;

        // Flags: (FIXME: rename?)
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

        static const int32_t fl_RandomDigits      = fl_RandomFC1 + fl_RandomFC2 + fl_RandomFC3;
        static const int32_t fl_FunctionSetBits   = (fl_PlanetImmunitySet | fl_CommanderSet | fl_FullWeaponrySet
                                                     | fl_TripleBeamKillSet | fl_DoubleBeamChargeSet | fl_DoubleTorpChargeSet
                                                     | fl_ElusiveSet | fl_SquadronSet);

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

        // FIXME:
        // VcrStatItem stats; // game::vcr::Statistic

        // VcrStatItem& getStat()
        //     { return stats; }
        // const VcrStatItem& getStat() const
        //     { return stats; }
    };

} }

#endif
