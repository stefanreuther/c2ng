/**
  *  \file game/actions/cargocostaction.hpp
  *  \brief Class game::actions::CargoCostAction
  */
#ifndef C2NG_GAME_ACTIONS_CARGOCOSTACTION_HPP
#define C2NG_GAME_ACTIONS_CARGOCOSTACTION_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/base/uncopyable.hpp"
#include "game/cargocontainer.hpp"
#include "game/spec/cost.hpp"

namespace game { namespace actions {

    /** Basic action for things that cost resources.
        Manages a game::spec::Cost value (i.e. costs consisting of T/D/M, supplies, money)
        and allocates the cost on a CargoContainer.
        Supply sale is taken into account if supported by the CargoContainer.

        The transaction will be valid if the CargoContainer can pay the cost.
        It will be invalid if the cost exceeds the content of the CargoContainer. */
    class CargoCostAction : private afl::base::Uncopyable {
     public:
        /** Constructor.
            Initializes the object with cost zero.
            \param container Container (typically a PlanetStorage) */
        explicit CargoCostAction(CargoContainer& container);

        /** Destructor. */
        ~CargoCostAction();

        /** Set cost.
            \param cost New cost. */
        void setCost(game::spec::Cost cost);

        /** Get cost.
            \return current cost */
        const game::spec::Cost& getCost() const;

        /** Set reserved amount.

            This amount is considered reserved for other actions, and will not be spent by this action.
            Use this is this action is nested within another action's UI.

            PCC2 used cargo reservations for this purpose, which is a more general, but also more complex solution to this problem.

            \param cost Amount */
        void setReservedAmount(game::spec::Cost cost);

        /** Get remaining amount.
            This is the amount remaining after removing the cost.
            It may be outside the range allowed by the CargoContainer if the transaction is invalid.
            \param type Element type to query
            \return remaining amount */
        int32_t getRemainingAmount(Element::Type type) const;

        /** Get remaining amount as Cost structure.
            \return remaining amount */
        game::spec::Cost getRemainingAmountAsCost() const;

        /** Get missing amount.
            If this transaction is valid, this will return 0.
            Otherwise, it returns the amount that needs to be added to the container to make the transaction valid.
            \param type Element type to query
            \return remaining amount */
        int32_t getMissingAmount(Element::Type type) const;

        /** Get missing amount as Cost structure.
            \return remaining amount */
        game::spec::Cost getMissingAmountAsCost() const;

        /** Get available amount as Cost structure.
            This is a convenience method to access the underlying cargo container.
            \return available amount (before cost has been subtracted) */
        game::spec::Cost getAvailableAmountAsCost() const;

        /** Check validity.
            The transaction will be valid if the CargoContainer can pay the cost.
            \return validity */
        bool isValid() const;

        /** Commit transaction.
            This commits the cargo container. */
        void commit();

        /** Signal: change.
            This signal is raised when
            - the underlying CargoContainer changes
            - the cost changes

            In general, a change in the cost will trigger a change in the CargoContainer.
            However, if a cost like "100$ 0S" is changed to "0$ 100",
            it may not actually produce a change in the container, so this signal can be used in addition. */
        afl::base::Signal<void()> sig_change;

     private:
        CargoContainer& m_container;
        game::spec::Cost m_cost;
        game::spec::Cost m_reservedAmount;
        afl::base::SignalConnection m_changeConnection;

        bool m_updating;

        void update();
        void onChange();
    };

} }

#endif
