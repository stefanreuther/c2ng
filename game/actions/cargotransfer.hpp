/**
  *  \file game/actions/cargotransfer.hpp
  *  \brief Class game::actions::CargoTransfer
  */
#ifndef C2NG_GAME_ACTIONS_CARGOTRANSFER_HPP
#define C2NG_GAME_ACTIONS_CARGOTRANSFER_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/cargocontainer.hpp"
#include "game/cargospec.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace actions {

    /** Cargo transfer.

        This class provides everything necessary to transfer cargo between objects.
        It holds (and controls life of) a number of CargoContainer objects.

        To use,
        - create a CargoTransfer
        - add participants using addNew()
        - use move() to move stuff around
        - commit() if the transaction is valid

        CargoTransfer will not itself allow to create an invalid transaction (exceeding unit limits).
        However, a cargo transfer can become invalid by the underlying objects changing.
        An additional validitity criterion is that all temporary containers must be empty at the end of a transaction. */
    class CargoTransfer {
     public:
        /** Mode for distribute(). */
        enum DistributeMode {
            DistributeEqually,                 ///< Try to add the same amount to each unit.
            DistributeFreeSpace,               ///< Try to make each unit have the same amount of free space.
            DistributeProportionally           ///< Try to make each unit have an amount proportional to their capacity.
        };


        /** Default constructor.
            Makes an empty CargoTransfer. */
        CargoTransfer();

        /** Destructor.
            Destroying the CargoTransfer object without committing it cancels the transfer. */
        ~CargoTransfer();

        /** Add new participant.
            \param container New participant. Will become owned by CargoTransfer. Null is ignored. */
        void addNew(CargoContainer* container);

        /** Add new hold space.
            Hold space is a temporary container that can temporarily hold cargo.
            However, the transaction cannot be committed while it is nonempty.
            \param name Name */
        void addHoldSpace(const String_t& name);

        /** Get participant by index.
            \param index Index [0,getNumContainers())
            \return container; null if index is out of range */
        CargoContainer* get(size_t index);

        /** Get number of participants.
            \return number of participants */
        size_t getNumContainers() const;

        /** Set overload permission.
            With overload enabled, units accept more than the rules allow.
            \param enable New value */
        void setOverload(bool enable);

        /** Check overload mode.
            \return true if overload enabled */
        bool isOverload() const;

        /** Move cargo.
            \param type    Element type to move
            \param amount  Amount to move (kt, clans, mc, units)
            \param from    Index of source unit
            \param to      Index of target unit
            \param partial If true, allow partial transfer. If false, only allow complete transfer.
            \param sellSupplies If enabled, convert supplies to mc
            \return Amount moved; 0 or amount if partial=false.

            Special behaviour:
            - if from or source are out of range or identical, the call is a no-op and returns 0.
            - if either participant does not support the requested type, the call is a no-op and returns 0.
            - if sellSupplies is true, and type is Element::Supplies, and a CargoContainer::SupplySale
              takes part in the transfer, this will sell supplies.
            - if amount is negative, the direction is reversed. */
        int32_t move(Element::Type type, int32_t amount, size_t from, size_t to, bool partial, bool sellSupplies);

        /** Move cargo specified by a CargoSpec.
            \param amount       [in/out] On input, cargo to move. On output, cargo not moved. If amount.isZero(), everything was moved.
            \param shipList     [in] Ship list (needed to determine number of torpedo types)
            \param from         [in] Index of source unit
            \param to           [in] Index of target unit
            \param sellSupplies [in] If enabled, convert supplies to mc */
        void move(CargoSpec& amount, const game::spec::ShipList& shipList, size_t from, size_t to, bool sellSupplies);

        /** Move with extension.
            Move cargo from source unit to target unit; if source unit is empty, move from extension instead.
            Partial moves are always accepted.
            A negative amount will not exhibit special behaviour.

            \param type       Element type to move
            \param amount     Amount to move (kt, clans, mc, units)
            \param from       Index of source unit
            \param to         Index of target unit
            \param extension  Index of extension unit
            \param sellSupplies If enabled, convert supplies to mc */
        void moveExt(Element::Type type, int32_t amount, size_t from, size_t to, size_t extension, bool sellSupplies);

        /** Move all cargo to a given unit.
            Take cargo from all units and put it on the target unit.

            \param type       Element type to move
            \param to         Index of target unit
            \param except     Exception; this unit's cargo is not removed (use same as to to not except any unit)
            \param sellSupplies If enabled, convert supplies to mc */
        void moveAll(Element::Type type, size_t to, size_t except, bool sellSupplies);

        /** Distribute cargo.
            Take cargo from the source unit and distribute it to all other units according to the given mode.

            The following units do not receive cargo:
            - temporary (CargoContainer::Temporary)
            - the unit specified as except

            \param type       Element type to move
            \param from       Index of source unit
            \param except     Index of exception unit (use same as from to not except any unit)
            \param mode       Distribution mode */
        void distribute(Element::Type type, size_t from, size_t except, DistributeMode mode);

        /** Unload operation.
            This is a shortcut to transfer all industry resources (T/D/M/$/S/Clans), corresponding to the user interface "U" function.
            \param sellSupplies If enabled, convert supplies to mc
            \return true if function succeeded (see isUnloadAllowed()).
            Note that only reports whether the structural requirement was fulfilled, it does NOT report whether cargo was moved successfully. */
        bool unload(bool sellSupplies);

        /** Check whether unload is allowed.
            Unload requires
            - exactly one CargoContainer::UnloadTarget
            - at least one CargoContainer::UnloadSource
            \return true if unload() will succeed */
        bool isUnloadAllowed() const;

        /** Check whether supply sale is allowed.
            Supply sale requires at least one unit with CargoContainer::SupplySale present.
            \return true if condition fulfilled */
        bool isSupplySaleAllowed() const;

        /** Get permitted element types.
            An element type is permitted in the transfer if it is supported on ALL participants, and a nonzero amount is present.

            \param shipList     [in] Ship list (needed to determine number of torpedo types)

            \return set of all element types */
        ElementTypes_t getElementTypes(const game::spec::ShipList& shipList) const;

        /** Check validity of transaction.
            The transaction is valid if
            - all participants are valid
            - all CargoContainer::Temporary are empty
            \return true if condition fulfilled */
        bool isValid() const;

        /** Commit.
            \pre isValid()
            \throw Exception if condition not fulfilled */
        void commit();

        /** Change signal.
            Called whenever anything in any container changes. */
        afl::base::Signal<void()> sig_change;

     private:
        afl::container::PtrVector<CargoContainer> m_units;
        bool m_overload;

        int m_notificationSuppressed;
        bool m_notificationPending;
        class Deferrer;

        void notify();
    };

} }

#endif
