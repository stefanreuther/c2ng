/**
  *  \file game/proxy/ufoproxy.hpp
  *  \brief Class game::proxy::UfoProxy
  */
#ifndef C2NG_GAME_PROXY_UFOPROXY_HPP
#define C2NG_GAME_PROXY_UFOPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/map/objectcursor.hpp"
#include "game/proxy/objectobserver.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Ufo proxy.

        This is essentially a CursorObserverProxy for the Ufo cursor.
        It implements the ObjectObserver interface and can therefore be used everywhere CursorObserverProxy can be used.
        In addition, it reports Ufo information in a pre-packaged way and allows ufo-specific operations.

        Bidirectional, asynchronous:
        - report Ufo information
        - browsing
        - toggle stored-in-history flag */
    class UfoProxy : public ObjectObserver {
     public:
        /** Index into UfoInfo::text. */
        enum InfoLine {
            Info1,              ///< First info line.
            Info2,              ///< Second info line
            Radius,             ///< "89 ly"
            Heading,            ///< "150 deg", "150 dec (+1,+2)
            Speed,              ///< "warp 7"
            PlanetRange,        ///< "150 ly"
            ShipRange,          ///< "150 ly"
            LastInfo,           ///< "3 turns ago"
            OtherEndName        ///< "Wormhole #9"
        };
        static const size_t NUM_INFO_LINES = static_cast<size_t>(OtherEndName)+1;

        /** Information about an Ufo.
            Contains information in human-readable and machine-readable form,
            mostly ad-hoc for PCC2's GUI needs. */
        struct UfoInfo {
            Id_t ufoId;                                ///< Ufo Id.
            game::map::Point center;                   ///< Center location.
            int radius;                                ///< Radius.
            String_t text[NUM_INFO_LINES];             ///< Textual information in human-readable form.
            bool hasOtherEnd;                          ///< true if Ufo has an other end.
            bool isStoredInHistory;                    ///< true if Ufo is stored in history.
            int colorCode;

            UfoInfo()
                : ufoId(), center(), radius(), hasOtherEnd(false), isStoredInHistory(false)
                { }
        };

        /** Constructor.
            \param reply      RequestDispatcher to receive replies on
            \param gameSender Game sender */
        UfoProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~UfoProxy();

        /** Browse Ufos.
            Updated information will be reported on sig_ufoChange.
            \param mode Mode
            \param marked true to accept only marked Ufos */
        void browse(game::map::ObjectCursor::Mode mode, bool marked);

        /** Browse to other end.
            Updated information will be reported on sig_ufoChange.
            \see game::map::Ufo::getOtherEnd */
        void browseToOtherEnd();

        /** Toggle "stored in history" flag.
            Updated information will be reported on sig_ufoChange.
            \see game::map::Ufo::setIsStoredInHistory */
        void toggleStoredInHistory();

        // ObjectObserver:
        virtual void addNewListener(ObjectListener* pl);

        /** Signal: Ufo changes.
            Emitted whenever the Ufo changes, or a different Ufo is selected on Cursors::currentUfo()
            using this proxy's methods or others.

            If the UfoProxy is constructed for a game with no Ufos,
            this signal is emitted once with a ufoId of zero.

            \param info Updated information */
        afl::base::Signal<void(const UfoInfo&)> sig_ufoChange;
     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<UfoProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
