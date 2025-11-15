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

// Set player restriction.
inline void
game::spec::HullFunction::setPlayers(PlayerSet_t players)
{
    m_players = players;
}

// Set level restriction.
inline void
game::spec::HullFunction::setLevels(ExperienceLevelSet_t levels)
{
    m_levels = levels;
}

// Set kind.
inline void
game::spec::HullFunction::setKind(Kind kind)
{
    m_kind = kind;
}

// Set host Id.
inline void
game::spec::HullFunction::setHostId(int hostId)
{
    m_hostId = hostId;
}

// Set basic function Id.
inline void
game::spec::HullFunction::setBasicFunctionId(int basicFunctionId)
{
    m_basicFunctionId = basicFunctionId;
}

// Get player restriction.
inline game::PlayerSet_t
game::spec::HullFunction::getPlayers() const
{
    return m_players;
}

// Get level restriction.
inline game::ExperienceLevelSet_t
game::spec::HullFunction::getLevels() const
{
    return m_levels;
}

// Get kind of assignment.
inline game::spec::HullFunction::Kind
game::spec::HullFunction::getKind() const
{
    return m_kind;
}

// Get host Id.
inline int
game::spec::HullFunction::getHostId() const
{
    return m_hostId;
}

// Get basic function Id.
inline int
game::spec::HullFunction::getBasicFunctionId() const
{
    return m_basicFunctionId;
}

#endif
