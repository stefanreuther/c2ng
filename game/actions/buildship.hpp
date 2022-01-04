/**
  *  \file game/actions/buildship.hpp
  *  \brief Class game::actions::BuildShip
  */
#ifndef C2NG_GAME_ACTIONS_BUILDSHIP_HPP
#define C2NG_GAME_ACTIONS_BUILDSHIP_HPP

#include "game/actions/basebuildaction.hpp"
#include "game/shipbuildorder.hpp"
#include "game/shipquery.hpp"
#include "game/spec/component.hpp"
#include "util/stringlist.hpp"

namespace game { namespace actions {

    /** Building ships.

        This action allows to configure a ship build order.
        When commited, all (missing) parts for that ship will be built,
        and the build order will be set on the starbase.

        This action uses build orders with hull numbers, not truehull indexes.
        This allows build orders to be configured that you cannot build.
        Those will fail in commit(). */
    class BuildShip : public BaseBuildAction {
     public:
        /** Identificator for a weapon.
            For setting amounts. */
        enum Weapon {
            BeamWeapon,
            TorpedoWeapon
        };

        /** Constructor.
            \param planet    Planet to work on. Must have a played starbase.
            \param container Container to bill the builds on. Usually a PlanetStorage for the same planet.
            \param shipList  Ship list. Needed to access component costs and hull slots.
            \param root      Game root. Needed to access host configuration and registration key. */
        BuildShip(game::map::Planet& planet,
                  CargoContainer& container,
                  game::spec::ShipList& shipList,
                  Root& root);

        /** Destructor. */
        ~BuildShip();

        /** Choose whether parts from storage will be used.
            If enabled, ship building will use parts if possible.
            If disabled, everything will be built anew, even when there is already a matching part in storage.
            \param flag new setting */
        void setUsePartsFromStorage(bool flag);

        /** Check whether usage of stored parts is enabled.
            \return value */
        bool isUsePartsFromStorage() const;

        /** Get current build order.
            The build order uses a hull Id (not truehull index).
            \return build order */
        ShipBuildOrder getBuildOrder() const;

        /** Set build order.
            The build order uses a hull Id (not truehull index).
            \param o New build order */
        void setBuildOrder(const ShipBuildOrder& o);

        /** Get current build order as ShipQuery.
            \return query */
        ShipQuery getQuery() const;

        /** Set part type for a tech area.
            Note that validity of the Id is not verified.
            Setting the hull type will update weapon counts.
            \param area Area to change.
            \param id   Part Id (type) */
        void setPart(TechLevel area, int id);

        /** Set number of weapons.
            \param area   Area to change
            \param amount New number. Out-of-range values will be forced into range. */
        void setNumParts(Weapon area, int amount);

        /** Change number of weapons.
            \param area   Area to change
            \param amount Amount to add. Out-of-range values will be forced into range. */
        void addParts(Weapon area, int amount);

        /** Check whether this action is a change to an existing build order.
            \retval true there already is a different ship build order
            \retval false the starbase has no build order, or this is the same */
        bool isChange() const;

        /** Commit the transaction.
            This will build the parts and set the build order.
            \throw Exception if build order is invalid */
        void commit();

        // BaseBuildAction:
        virtual void perform(BaseBuildExecutor& exec);


        /** Prepare a build order.
            \param o [in/out] The build order
            \param pl Planet
            \param config Host configuration
            \param shipList Ship list
            \retval true we're re-using the base's build order
            \retval false this is a new build order
            \throw Exception Configuration does not allow this player to build */
        // FIXME: check whether we need this guy in the public interface?
        static bool prepareBuildOrder(ShipBuildOrder& o,
                                      const game::map::Planet& pl,
                                      const game::config::HostConfiguration& config,
                                      const game::spec::ShipList& shipList);

     private:
        ShipBuildOrder m_order;
        bool m_usePartsFromStorage;

        int getBuildAmount(int need, int have) const;
        void doTechUpgrade(game::TechLevel area, BaseBuildExecutor& exec, const game::spec::Component* component) const;

        bool getNewOrder(ShipBuildOrder& o) const;
    };

} }

#endif
