/**
  *  \file game/actions/buildammo.hpp
  *  \brief Class game::actions::BuildAmmo
  */
#ifndef C2NG_GAME_ACTIONS_BUILDAMMO_HPP
#define C2NG_GAME_ACTIONS_BUILDAMMO_HPP

#include "afl/base/signalconnection.hpp"
#include "game/actions/cargocostaction.hpp"
#include "game/cargocontainer.hpp"
#include "game/element.hpp"
#include "game/exception.hpp"
#include "game/map/planet.hpp"
#include "game/root.hpp"

namespace game { namespace actions {

    /** Ammo building.

        This action allows building of ammunition (torpedoes, fighters).
        There can be up to three participants:
        - a planet with starbase that provides the tech levels.
        - a financier who provides minerals and cash. Typically the same as the planet but can be something else.
        - a receiver who receives the new stuff. Can be the planet or a ship.
        These participants can be the same object but don't have to.

        To use, construct the participants and the action, configure using add/addLimitCash, and commit. */
    class BuildAmmo {
     public:
        enum Status {
            Success,
            MissingResources,
            DisallowedTech,
            MissingRoom
        };

        /** Constructor.
            \param planet Planet. Needed for tech levels.
            \param financier Container that pays the transaction.
            \param receiver Container that receives the result. Can be the same as financier.
            \param shipList Ship list. Needed for unit costs and tech.
            \param root Root. Needed for host configuration and key. */
        BuildAmmo(game::map::Planet& planet,
                  CargoContainer& financier,
                  CargoContainer& receiver,
                  game::spec::ShipList& shipList,
                  const Root& root);

        /** Destructor. */
        ~BuildAmmo();

        /** Set undo information.
            This enables this transaction to undo former builds.
            This uses the universe's reverter, if any.
            Changes on the universe will automatically be propagated.
            \param univ Universe. Must live longer than the TechUpgrade action. */
        void setUndoInformation(game::map::Universe& univ);

        /** Add ammo.
            This function checks that we are allowed to build this component, but does not verify costs.
            \param type Weapon type (Fighters or a torpedo)
            \param count Number to add (negative to remove)
            \param partial true: allow partial add/remove; false: execute change completely or not at all
            \return Number added/removed. With partial=false, either 0 or amount. */
        int add(Element::Type type, int count, bool partial);

        /** Add ammo, limiting by cash.
            When called with a positive count on a valid transaction, will not make the transaction invalid.

            Note: when called with a negative amount, and the financier has limited room, this may overflow the financier.
            This is not usually a problem, as the financier usually is a planet.

            \param type Weapon type (Fighters or a torpedo)
            \param count Number to add (negative to remove)
            \return Number added/removed. */
        int addLimitCash(Element::Type type, int count);

        /** Get ammo that must remain.
            \param type Weapon type (Fighters or a torpedo)
            \return minimum permitted amount */
        int getMinAmount(Element::Type type) const;

        /** Get maximum ammo.
            If the receiver has limited room, this returns the number of items of this type that can be added.
            \param type Weapon type (Fighters or a torpedo)
            \return maximum permitted amount */
        int getMaxAmount(Element::Type type) const;

        /** Get current amount (number on receiver plus build order).
            \param type Weapon type (Fighters or a torpedo)
            \return current amount */
        int getAmount(Element::Type type) const;

        /** Get current status.
            If the action fails, this returns a failure reason.
            \return current status */
        Status getStatus();

        /** Commit.
            \throw game::Exception if this action is not valid */
        void commit();

        /** Check validity.
            \return true if this action can be committed */
        bool isValid();

        /** Access underlying CargoCostAction.
            \return CargoCostAction */
        const CargoCostAction& costAction() const;

        /** Access underlying receiver.
            \return receiver */
        const CargoContainer& receiver() const;

        /** Get item cost.
            \param [in]  type       Type
            \param [out] cost       Cost
            \param [out] techLevel  Tech level
            \return true on success, false if type is invalid */
        bool getItemCost(Element::Type type, game::spec::Cost& cost, int& techLevel) const;

        /** Check for valid combination.
            Given a ship and a planet, determines whether the ship is permitted to build ammo at the planet.
            \param [in]  planet Planet
            \param [in]  ship   Ship
            \param [out] ex     Exception
            \retval true Combination is valid
            \retval false Combination not valid; exception has been produced in \c ex */
        static bool isValidCombination(const game::map::Planet& planet, const game::map::Ship& ship, Exception& ex);

     private:
        game::map::Planet& m_planet;
        CargoCostAction m_costAction;
        CargoContainer& m_receiver;
        game::spec::ShipList& m_shipList;
        const Root& m_root;
        bool m_mustCommitReceiver;
        int m_totalTechLevel;

        afl::base::SignalConnection m_costActionChangeConnection;
        afl::base::SignalConnection m_receiverChangeConnection;
        afl::base::SignalConnection m_shipListChangeConnection;
        afl::base::SignalConnection m_planetChangeConnection;
        afl::base::SignalConnection m_universeChangeConnection;

        const game::map::Universe* m_pUniverse;

        void update();

        bool isAccessibleTechLevel(int techLevel) const;
    };

} }

#endif
