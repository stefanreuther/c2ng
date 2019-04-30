/**
  *  \file game/cargocontainer.hpp
  *  \brief Base class game::CargoContainer
  */
#ifndef C2NG_GAME_CARGOCONTAINER_HPP
#define C2NG_GAME_CARGOCONTAINER_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "game/element.hpp"
#include "util/vector.hpp"

namespace game {

    /** Cargo container for generic cargo access.
        All actions that consume or move cargo use CargoContainer descendants to describe the underlying objects.
        Derived classes must...
        - implement the given methods
        - ensure that sig_change is called whenever the underlying object is changed

        Users can interrogate this object, and configure a listener.
        This object stores a delta to the current object, and allows you to commit the change.
        Nothing needs to be done to the underlying data for cancelling a change.

        @change Whereas PCC2 implemented an elaborate scheme with cargo reservations to deal with "concurrent" modifications,
        c2ng uses a "detect and fix" method.
        A concurrent modification may cause the change configured in this CargoContainer to become invalid.
        The caller must attempt to clean up (e.g. by resetting). */
    class CargoContainer : public afl::base::Deletable {
     public:
        /** Constructor. */
        CargoContainer();

        /** Destructor. */
        ~CargoContainer();

        /** Flags describing this unit.
            - the user "unload" function is enabled if a cargo transfer contains some UnloadSources and one UnloadTarget.
            - the user "supply to mc" function is enabled if a cargo transfer contains a SupplySale unit
            - a Temporary must not contain anything to make a cargo transfer valid */
        enum Flag {
            UnloadSource,           ///< This unit is a possible source for unloading (a ship).
            UnloadTarget,           ///< This unit is a possible target for unloading (a planet).
            SupplySale,             ///< This unit allows supply sale (an owned planet).
            Temporary               ///< This is a temporary container.
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;


        /*
         *  Derived-Class Functions
         */

        /** Get name.
            \param tx Translator
            \return Name */
        virtual String_t getName(afl::string::Translator& tx) const = 0;

        /** Get flags decribing this unit.
            \return flags */
        virtual Flags_t getFlags() const = 0;

        /** Check whether this container can contain the given element.
            \param type element type
            \return true Container can contain this element */
        virtual bool canHaveElement(Element::Type type) const = 0;

        /** Get maximum amount of this element the container can have.
            If this element shares space with others, their changes are taken into account.
            That is, for a ship
            - getMax(Neutronium) returns Hull::getMaxFuel() because Neutronium does not share space with anything
            - getMax(Tritanium) returns Hull::getMaxCargo() reduced by getAmount() and getDelta() for all cargo types other than Tritanium

            If isOverload(), this function should apply relaxed checking and allow the user to bypass rule checking.

            \param type element type
            \return current maximum amount */
        virtual int32_t getMaxAmount(Element::Type type) const = 0;

        /** Get minimum amount of this element the container can have.
            Typically, this is 0, but can be negative if we're working with an opportunistic/virtual container such as the source for a "beam up" mission.
            \param type element type
            \return current minimum amount */
        virtual int32_t getMinAmount(Element::Type type) const = 0;

        /** Get current amount of this element.
            Returns the unchanged element amount from the underlying object.
            \param type element type
            \return current amount */
        virtual int32_t getAmount(Element::Type type) const = 0;

        /** Commit.
            Updates the underlying object with the current deltas.
            This function must be called at most once for each instance.

            \pre isValid().
            Note that although the precondition is that this is a valid change, this function should NOT attempt to validate that.
            Since this call is part of a transaction, refusing a change midway will get universe totals out of balance.
            Whereas out-of-balance is a red error, having an overloaded ship is only a yellow one. */
        virtual void commit() = 0;


        /*
         *  CargoContainer functions
         */

        /** Change amount of an element.
            Updates the delta.
            \param type element type
            \param delta add/remove this many from that element */
        void change(Element::Type type, int32_t delta);

        /** Get current change.
            \param type element type
            \return current delta */
        int32_t getChange(Element::Type type) const;

        /** Get effective amount.
            This is the amount adjusted by change.
            \param type element type
            \return current effective amount */
        int32_t getEffectiveAmount(Element::Type type) const;

        /** Clear everything.
            Reverts all changes. */
        void clear();

        /** Check validity.
            \return true if all new amounts are within valid range. */
        bool isValid() const;

        /** Check emptiness.
            \return true if this contains no changes. */
        bool isEmpty() const;

        /** Set overload permission.
            With overload enabled, this unit accepts more than the rules allow.
            \param enable New value */
        void setOverload(bool enable);

        /** Check overload mode.
            \return true if overload enabled */
        bool isOverload() const;

        /** Get upper limit for type.
            \return type T such that getChange(t)==0 for all t >= T. */
        Element::Type getTypeLimit() const;

        /** Signal: anything changed.
            Called whenever the deltas or the underlying object changed */
        afl::base::Signal<void()> sig_change;

     private:
        util::Vector<int32_t,Element::Type> m_delta;    ///< Current deltas, indexed by Element::Type. Nonpresent values treated as 0.
        bool m_overload;                 ///< Overload flag.
    };

}

#endif
