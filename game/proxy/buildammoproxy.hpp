/**
  *  \file game/proxy/buildammoproxy.hpp
  *  \brief Class game::proxy::BuildAmmoProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDAMMOPROXY_HPP
#define C2NG_GAME_PROXY_BUILDAMMOPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "game/element.hpp"
#include "game/session.hpp"
#include "game/spec/info/types.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for building ammunition.
        Proxies a game::actions::BuildAmmo.

        A BuildAmmoProxy is constructed for a planet which must be played and have a starbase.
        The planet will operate as the "financier" of the BuildAmmo action.
        To build ammo, use setPlanet() or setShip() to define the "receiver".
        After adjusting amounts, use commit() to perform the action.

        Bidirectional, synchronous:
        - getStatus

        Bidirectional, asynchronous:
        - set receiver (setPlanet(), setShip())
        - build/scrap parts
        - commit the action
        - status update (sig_update) */
    class BuildAmmoProxy {
     public:
        /** Information about a part. */
        struct Part {
            // Identification of the part
            Element::Type type;           ///< Part type. For use in addLimitCash().
            game::spec::info::Page page;  ///< Page in SpecBrowserProxy.
            Id_t id;                      ///< Id in SpecBrowserProxy.
            String_t name;                ///< Part name.
            game::spec::Cost cost;        ///< Part cost.

            // Tech status
            TechStatus techStatus;        ///< Tech level status for this part.
            bool isAccessible;            ///< true if part is accessible (receiver can hold it).
            int techLevel;                ///< Tech level of part.

            // Storage
            int amount;                   ///< Current mount (including modifications).
            int maxAmount;                ///< Maximum amount.

            Part()
                : type(), page(), id(), name(), cost(), techStatus(), isAccessible(), techLevel(), amount(), maxAmount()
                { }
        };

        /** Vector of parts. */
        typedef std::vector<Part> Parts_t;

        /** Action status. */
        struct Status {
            // Target
            String_t targetName;
            int availableTech;

            // Parts
            Parts_t parts;                ///< Status of all parts.

            // Cost
            game::spec::Cost cost;        ///< Total cost of all parts. @see game::spec::CargoCostAction::getCost()
            game::spec::Cost available;   ///< Available amounts. @see game::actions::CargoCostAction::getAvailableAmountAsCost()
            game::spec::Cost remaining;   ///< Remaining amounts. @see game::actions::CargoCostAction::getRemainingAmount()
            game::spec::Cost missing;     ///< Missing amounts. @see game::actions::CargoCostAction::getMissingAmount()

            Status()
                : targetName(), availableTech(), parts(), cost(), available(), remaining(), missing()
                { }
        };



        /** Constructor.
            \param gameSender Game sender
            \param reply      RequestDispatcher to receive updates in this thread
            \param planetId   Planet Id */
        BuildAmmoProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, Id_t planetId);

        /** Destructor. */
        ~BuildAmmoProxy();

        /** Get status, synchronously.
            \param [in,out] ind    WaitIndicator for UI synchronisation
            \param [out]    result Status */
        void getStatus(WaitIndicator& ind, Status& result);

        // FIXME: routine to request candidate ships

        /** Select planet as receiver.
            Built ammunition will be placed in the starbase's storage.
            If a previous build has not been committed, it will be discarded. */
        void setPlanet();

        /** Select ship as receiver.
            Built ammunition will be placed in the ship; only the ship's ammo type can be build.
            If a previous build has not been committed, it will be discarded.
            \param shipId Ship Id */
        void setShip(Id_t shipId);

        /** Add ammo, limiting by cash.
            Add or removes the given amount, but will not make the transaction invalid.
            The updated status will be reported using sig_update.
            \param type Weapon type (Fighters or a torpedo)
            \param count Number to add (negative to remove)
            \see game::actions::BuildAmmo::addLimitCash */
        void addLimitCash(Element::Type type, int count);

        /** Commit.
            Performs the requested build and finishes the transaction.
            Use setPlanet(), setShip() to start a new build if desired.
            \see game::actions::BuildAmmo::commit */
        void commit();

        /** Signal: status update.
            \param st Status update */
        afl::base::Signal<void(const Status&)> sig_update;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<BuildAmmoProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
