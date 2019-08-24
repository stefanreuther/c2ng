/**
  *  \file game/actions/taxationaction.hpp
  *  \brief Class game::actions::TaxationAction
  */
#ifndef C2NG_GAME_ACTIONS_TAXATIONACTION_HPP
#define C2NG_GAME_ACTIONS_TAXATIONACTION_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "util/numberformatter.hpp"

namespace game { namespace actions {

    /** Taxation action.
        This action contains functions to choose tax rates for a planet.
        It wraps the functions in planetformula.cpp into a stateful interface
        which emits a change event on appropriate places.

        Taxation has two areas (Colonists, Natives).
        When used on a planet that is not Playable, TaxationAction will still work and allow
        tax rates to be changed to inquire the effects, but will not allow committing a change.
        Likewise, a change to native taxes cannot be committed if the planet has no natives. */
    class TaxationAction {
     public:
        /** Taxation area. */
        enum Area {
            Colonists,          /**< Colonist taxes. */
            Natives             /**< Native taxes. */
        };

        /** Set of areas. */
        typedef afl::bits::SmallSet<Area> Areas_t;

        /** Direction. */
        enum Direction {
            Down,               /**< Downward (decrement). */
            Up                  /**< Upward (increment). */
        };

        /** Constructor.
            \param planet Planet. Needs to live longer than TaxationAction.
                          Changes cause sig_change to be raised.
            \param config Host configuration. Needs to live longer than TaxationAction.
                          Changes cause sig_change to be raised.
                          Affects income (ColonistTaxRate) and happiness (CrystalSinTempBehavior).
            \param host   Host version. Affects rounding/formula details. */
        TaxationAction(game::map::Planet& planet,
                       game::config::HostConfiguration& config,
                       const game::HostVersion& host);

        /** Destructor. */
        ~TaxationAction();

        /** Set number of buildings (mines + factories).
            This affects happiness changes.
            If not set, TaxationAction uses the number of buildings on the planet.
            \param mifa Buildings */
        void setNumBuildings(int mifa);

        /** Get tax rate for an area.
            \param a area
            \return tax rate */
        int getTax(Area a) const;

        /** Get amount due for an area.
            \param a area
            \return amount due
            \see game::map::getColonistDue(), game::map::getNativeDue() */
        int32_t getDue(Area a) const;

        /** Get amount due, limited to amount collected for an area.
            \param a area
            \return amount collected
            \see game::map::getColonistDueLimited(), game::map::getNativeDueLimited() */
        int32_t getDueLimited(Area a) const;

        /** Get happiness change.
            \param a area
            \return happiness change */
        int getHappinessChange(Area a) const;

        /** Get bovinoid supply contribution.
            If the planet doesn't actually have Bovinoid colonists, returns 0.
            \return supplies
            \see game::map::getBovinoidSupplyContribution() */
        int getBovinoidSupplyContribution() const;

        /** Get bovinoid supply contribution, limited to amount collected.
            If the planet doesn't actually have Bovinoid colonists, returns 0.
            \return supplies
            \see game::map::getBovinoidSupplyContributionLimited() */
        int getBovinoidSupplyContributionLimited() const;


        /** Check whether area is modifyable.
            \param a area
            \return true if taxes can be changed on that area */
        bool isModifyable(Area a) const;

        /** Check whether area is available.
            \param a area
            \return true if taxes can be examined on that area */
        bool isAvailable(Area a) const;

        /** Get minimum tax rate for an area.
            \param a area
            \return minimum tax rate (include) */
        int getMinTax(Area a) const;

        /** Get maximum tax rate for an area.
            \param a area
            \return maximum tax rate (include) */
        int getMaxTax(Area a) const;

        /** Describe current tax rate.
            For Colonists, returns a two-line string.
            For Natives, returns a three-line string.
            \param a area
            \return tx Translator
            \param fmt Number formatter
            \return text */
        String_t describe(Area a, afl::string::Translator& tx, const util::NumberFormatter& fmt) const;


        /** Check validity.
            A TaxationAction is valid if the current tax rates are within the limits given by getMinTax(), getMaxTax().
            \return validity */
        bool isValid() const;


        /** Set tax rate, unconditionally.
            If the new value is out of range, the TaxationAction will become invalid.
            \param a area
            \param value new tax rate */
        void setTax(Area a, int value);

        /** Set tax rate, limit to valid range.
            If the new value is out of range, it will be forced into the valid range.
            \param a area
            \param value new tax rate */
        void setTaxLimited(Area a, int value);

        /** Change tax rate for better/worse revenue.
            Set the lowest rate with higher income / the highest rate with lower income.
            If the new value is out of range, the TaxationAction will become invalid.
            \param a area
            \param d direction */
        void changeRevenue(Area a, Direction d);

        /** Change tax rate.
            If the new value is out of range, it will be forced into the valid range.
            \param a area
            \param delta difference */
        void changeTax(Area a, int delta);

        /** Set safe-tax for areas.
            If the new value is out of range, the TaxationAction will become invalid.
            \param as areas */
        void setSafeTax(Areas_t as);

        /** Revert tax rates.
            Returns the given areas to their original values.
            \param as areas */
        void revert(Areas_t as);

        /** Commit transaction.
            Writes changes back into data.
            \throw game::Exception if transaction is invalid */
        void commit();


        /** Access planet being worked on.
            \return planet */
        game::map::Planet& planet() const;


        /** Signal: change.
            Raised if the return value of any function in TaxationAction changes. */
        afl::base::Signal<void()> sig_change;

     private:
        game::map::Planet& m_planet;                    // Planet.
        game::config::HostConfiguration& m_config;      // Host configuration.
        const game::HostVersion m_hostVersion;          // Host version.

        afl::base::Optional<int> m_numBuildings;        // Number of buildings for happiness change computation. Nothing to use planet's current value.
        afl::base::Optional<int> m_tax[2];              // Current tax. Nothing to use planet's current value.

        afl::base::SignalConnection conn_planetChange;  // Planet::sig_change
        afl::base::SignalConnection conn_configChange;  // HostConfiguration::sig_change

        int getOriginalTax(Area a) const;
        int getNumBuildings() const;

        void onChange();
        void update();
    };

} }

#endif
