/**
  *  \file game/spec/hullfunction.hpp
  *  \brief Class game::spec::HullFunction
  */
#ifndef C2NG_GAME_SPEC_HULLFUNCTION_HPP
#define C2NG_GAME_SPEC_HULLFUNCTION_HPP

#include "game/experiencelevelset.hpp"
#include "game/playerset.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

    class Hull;

    /** Hull function.
        This class has two purposes:
        - it can store a restricted/modified function definition for use in mapping our internal numbers / host's numbers to functions
        - it can report a broken-down function assignment for users

        In the first case, the fields Levels, HostId, BasicFunctionId are relevant,
        in the second case the fields Levels, Players, Kind, and BasicFunctionId. */
    class HullFunction {
     public:
        /** Assignment kind. */
        enum Kind {
            /** Assigned to ship.
                Not all ships of this type/owner may have this function.
                If the ship changes ownership, the function follows. */
            AssignedToShip,
            /** Assigned to hull.
                All ships of this type/owner have this function.
                If the ship changes ownership, the function may get lost/appear if it is player specific. */
            AssignedToHull,
            /** Assigned to race.
                All ships of this owner have this function.
                If the ship changes ownership, the function may get lost/appear. */
            AssignedToRace
        };

        /** Standard hull functions numbers.
            These are used by our core to check for presence of specific ship abilities.
            We do not require actual hull function numbers to be one of these constants.
            Hull functions must be defined in hullfunc.cc to be visible to users. */
        static const int MerlinAlchemy     = 0;        ///< 0 = Merlin Alchemy: 9 Sup -> 3 Min
        static const int NeutronicRefinery = 1;        ///< 1 = Neutronic Refinery: Min + Sup -> Fuel
        static const int AriesRefinery     = 2;        ///< 2 = Aries Refinery: Min -> Fuel
        static const int HeatsTo50         = 3;        ///< 3 = Bohemian Terraformer
        static const int CoolsTo50         = 4;        ///< 4 = Eros Terraformer
        static const int HeatsTo100        = 5;        ///< 5 = Onyx Terraformer
        static const int Hyperdrive        = 6;        ///< 6
        static const int Gravitonic        = 7;        ///< 7
        static const int ScansAllWormholes = 8;        ///< 8 = Bohemian
        static const int LadyRoyale        = 9;        ///< 9 = Lady Royale
        static const int LokiAnticloak     = 10;       ///< 10 = Loki
        static const int ImperialAssault   = 11;       ///< 11 = SSD
        static const int FirecloudChunnel  = 12;       ///< 12 = Firecloud
        static const int Ramscoop          = 13;       ///< 13 = Cobol
        static const int FullBioscan       = 14;       ///< 14 = Pawn
        static const int AdvancedCloak     = 15;       ///< 15 = Dark Wing
        static const int Cloak             = 16;       ///< 16
        static const int Bioscan           = 17;       ///< 17
        static const int SaberGlory        = 18;       ///< 18 = Saber (10% damage to own ships)
        static const int D19bGlory         = 19;       ///< 19 = D19b (20% damage to own ships)
        static const int Unclonable        = 20;       ///< 20
        static const int CloneOnce         = 21;       ///< 21
        static const int Ungiveable        = 22;       ///< 22
        static const int GiveOnce          = 23;       ///< 23
        static const int Level2Tow         = 24;       ///< 24
        static const int Tow               = 25;       ///< 25 = depends on AllowOneEngineTowing setting
        static const int ChunnelSelf       = 26;       ///< 26
        static const int ChunnelOthers     = 27;       ///< 27
        static const int ChunnelTarget     = 28;       ///< 28
        static const int PlanetImmunity    = 29;       ///< 29 = Rebels, Klingons, if configured, plus SSD
        static const int OreCondenser      = 30;       ///< 30
        static const int Boarding          = 31;       ///< 31 = Privs, Crystals
        static const int AntiCloakImmunity = 32;       ///< 32 = implied by AntiCloakImmunity option
        static const int Academy           = 33;       ///< 33
        static const int Repairs           = 34;       ///< 34
        static const int FullWeaponry      = 35;       ///< 35 = Feds, if configured
        static const int HardenedEngines   = 36;       ///< 36
        static const int Commander         = 37;       ///< 37
        static const int IonShield         = 38;       ///< 38
        static const int HardenedCloak     = 39;       ///< 39
        static const int AdvancedAntiCloak = 40;       ///< 40

        /** Constructor.
            Construct a hull function object referring to a basic, unrestricted function.
            The function is assigned to all players, all levels, assigned to the ship.
            \param basicFunctionId Basic function Id, see BasicHullFunction::getId() */
        explicit HullFunction(int basicFunctionId = -1);

        /** Constructor.
            Construct a hull function object referring to a function restricted to a certain set of levels.
            \param basicFunctionId Basic function Id, see BasicHullFunction::getId()
            \param levels Levels at which this function works */
        HullFunction(int basicFunctionId, ExperienceLevelSet_t levels);

        /** Set player restriction.
            \param players Player set */
        void setPlayers(PlayerSet_t players);

        /** Set level restriction.
            This value should not be limited by host configuration NumExperienceLevels,
            so it need not be updated when the host configuration changes.
            \param levels Level set */
        void setLevels(ExperienceLevelSet_t levels);

        /** Set kind.
            This describes how this function is assigned to a ship.
            \param kind Kind of assignment */
        void setKind(Kind kind);

        /** Set host Id.
            This is the number under which a level-restricted function is known to the host.
            \param hostId id */
        void setHostId(int hostId);

        /** Set basic function Id.
            \param basicFunctionId function Id, see BasicHullFunction::getId() */
        void setBasicFunctionId(int basicFunctionId);

        /** Get player restriction.
            \return player set */
        PlayerSet_t getPlayers() const;

        /** Get level restriction.
            \return level set */
        ExperienceLevelSet_t getLevels() const;

        /** Get kind of assignment.
            \return kind of assignment */
        Kind getKind() const;

        /** Get host Id.
            \return host id, or -1 if none */
        int getHostId() const;

        /** Get basic function Id.
            \return basic function Id */
        int getBasicFunctionId() const;

        /** Check whether two functions name the same hull function.
            This compares just the function data (basic function and experience levels), not assignment information (player, kind, host Id).
            \param other other function
            \return true if both are the same function */
        bool isSame(const HullFunction& other);

        /** Get default assignments for a basic function.
            Some hull functions have a variable default assignment, depending on the configuration or hull properties.
            In host, the Init=Default statement will consult the current configuration, and set the functions accordingly.

            We want to be able to support configuration that changes on the fly without reloading hull functions.
            That is, when the player configures AllowOneEngineTowing=Yes, all ships magically receive the Tow ability.

            This function determines the variable default for a hull/device.

            Note that all variable defaults are AssignedToHull and apply to all levels.
            This function does not handle fixed defaults ("44-46 = Gravitonic"); those are in BasicHullFunction/BasicHullFunctionList.

            \param basicFunctionId Function
            \param config Host configuration
            \param hull Hull
            \return default assignment for this basic function (AssignedToHull, all levels) */
        static PlayerSet_t getDefaultAssignment(int basicFunctionId, const game::config::HostConfiguration& config, const Hull& hull);

     private:
        int m_basicFunctionId;
        PlayerSet_t m_players;
        ExperienceLevelSet_t m_levels;
        Kind m_kind;
        int m_hostId;
    };

} }

#endif
