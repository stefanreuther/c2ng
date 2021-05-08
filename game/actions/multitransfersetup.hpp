/**
  *  \file game/actions/multitransfersetup.hpp
  *  \brief Class game::actions::MultiTransferSetup
  */
#ifndef C2NG_GAME_ACTIONS_MULTITRANSFERSETUP_HPP
#define C2NG_GAME_ACTIONS_MULTITRANSFERSETUP_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/types.hpp"

namespace game { namespace actions {

    class CargoTransfer;

    /** Setup for multi-ship cargo transfer.
        To use,
        - construct
        - call setShipId(), setFleetOnly()
        - use getSupportedElementTypes() to select an element; call setCargoType().
        - use build() to configure a CargoTransfer object

        This is a data class that does not hold any references and can be copied as needed. */
    class MultiTransferSetup {
     public:
        /** Status for build(). */
        enum Status {
            Success,                     ///< Action built successfully.
            NoCargo,                     ///< Action built successfully but meaningless: nobody has the required cargo.
            NoPeer,                      ///< Action built successfully but meaningless: no other unit.
            Failure                      ///< Failure, action could not be built.
        };

        /** Result for build(). */
        struct Result {
            Status status;               ///< Status.
            size_t thisShipIndex;        ///< Index into CargoTransfer action of initiating ship.
            size_t extensionIndex;       ///< Index into CargoTransfer action of initial extension (hold space or planet).

            Result()
                : status(Failure), thisShipIndex(0), extensionIndex(0)
                { }
        };


        /** Constructor.
            Make blank object. */
        MultiTransferSetup();

        /** Set ship Id.
            \param shipId Ship Id; must be a played ship */
        void setShipId(Id_t shipId);

        /** Set fleet-only flag.
            If set, only members of the same fleet will be treated as possible partners.
            \param flag Flag */
        void setFleetOnly(bool flag);

        /** Set element type to transfer.
            Note that this only determines how the CargoTransfer is built
            (units must be able to carry this type, and there must be some of that type available).
            The action will also allow moving other cargo.
            \param type Type */
        void setElementType(Element::Type type);

        /** Get ship Id.
            \return ship Id */
        Id_t getShipId() const;

        /** Get fleet-only flag.
            \return flag */
        bool isFleetOnly() const;

        /** Get element type to transfer.
            \return element type */
        Element::Type getElementType() const;

        /** Get supported element types.
            If a valid ship Id has been configured, determines the cargo types it can carry.
            This function also checks preconditions; it will return an empty set if the ship is not applicable as a starter for multi-ship cargo transfer.
            \param univ      Universe
            \param shipList  Ship list */
        ElementTypes_t getSupportedElementTypes(const game::map::Universe& univ, const game::spec::ShipList& shipList) const;

        /** Build cargo transfer action.
            The cargo transfer action will contain:
            - hold space at first slot
            - all applicable ships in appropriate sort order
            - planet, if any

            \param [out] action  Action
            \param [in]  univ    Universe (participating units are taken from here)
            \param [in]  session Session (in particular, required for sorting)
            \return result, containing overall status */
        Result build(CargoTransfer& action, game::map::Universe& univ, Session& session) const;

     private:
        Id_t m_shipId;
        bool m_fleet;
        Element::Type m_element;
    };

} }

#endif
