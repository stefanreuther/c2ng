/**
  *  \file game/proxy/ionstormproxy.hpp
  *  \class game::proxy::IonStormProxy
  */
#ifndef C2NG_GAME_PROXY_IONSTORMPROXY_HPP
#define C2NG_GAME_PROXY_IONSTORMPROXY_HPP

#include <utility>
#include <vector>
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/point.hpp"
#include "game/proxy/objectobserver.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"

namespace game { namespace proxy {

    /** Ion storm proxy.

        This is essentially a CursorObserverProxy for the ion storm cursor.
        It implements the ObjectObserver interface and can therefore be used everywhere CursorObserverProxy can be used.
        In addition, it reports ion storm information in a pre-packaged way.

        Bidirectional, asynchronous:
        - report ion storm information
        - browsing */
    class IonStormProxy : public ObjectObserver {
     public:
        /** Index into IonStormInfo::text. */
        enum InfoLine {
            Radius,             ///< "89 ly"
            Heading,            ///< "150 deg"
            Speed,              ///< "warp 7"
            Voltage,            ///< "50 MeV"
            Status,             ///< "growing"
            ClassName           ///< "Class 3 (dangerous)"
        };
        static const size_t NUM_INFO_LINES = static_cast<size_t>(ClassName)+1;

        /** Information about an ion storm.
            Contains information in human-readable and machine-readable form,
            mostly ad-hoc for PCC2's GUI needs. */
        struct IonStormInfo {
            Id_t stormId;                              ///< Ion storm Id.
            game::map::Point center;                   ///< Center location.
            int radius;                                ///< Radius.
            int heading;                               ///< Heading.
            int voltage;                               ///< Voltage.
            int speed;                                 ///< Speed.
            String_t text[NUM_INFO_LINES];             ///< Textual information in human-readable form.
            game::map::IonStorm::Forecast_t forecast;  ///< Forecast.

            IonStormInfo()
                : stormId(), center(), radius(), heading(), voltage(), speed(), forecast()
                { }
        };

        /** Constructor.
            \param reply      RequestDispatcher to receive replies on
            \param gameSender Game sender */
        IonStormProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~IonStormProxy();

        /** Browse ion storms.
            Updated information will be reported on sig_stormChange.
            \param mode Mode
            \param marked true to accept only marked ion storms */
        void browse(game::map::ObjectCursor::Mode mode, bool marked);

        // ObjectObserver:
        virtual void addNewListener(ObjectListener* pl);

        /** Signal: ion storm changes.
            Emitted whenever the ion storm changes, or a different storm is selected on Cursors::currentIonStorm()
            using this proxy's methods or others.

            If the IonStormProxy is constructed for a game with no ion storms,
            this signal is emitted once with a stormId of zero.

            \param info Updated information */
        afl::base::Signal<void(const IonStormInfo&)> sig_stormChange;
     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<IonStormProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
