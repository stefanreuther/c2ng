/**
  *  \file game/proxy/minefieldproxy.hpp
  *  \brief Class game::proxy::MinefieldProxy
  */
#ifndef C2NG_GAME_PROXY_MINEFIELDPROXY_HPP
#define C2NG_GAME_PROXY_MINEFIELDPROXY_HPP

#include <utility>
#include <vector>
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/point.hpp"
#include "game/proxy/objectobserver.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"

namespace game { namespace proxy {

    /** Minefield proxy.

        This is essentially a CursorObserverProxy for the minefield cursor.
        It implements the ObjectObserver interface and can therefore be used everywhere CursorObserverProxy can be used.

        In addition, it implements a stateful computation of minefield passage information,
        This cannot be implemented with CursorObserverProxy alone because its ObjectListener's cannot be addressed from the UI side.

        Bidirectional, asynchronous:
        - report minefield information
        - report and update passage information

        Bidirectional, synchronous:
        - sweep information */
    class MinefieldProxy : public ObjectObserver {
     public:
        /** Index into MinefieldInfo::text. */
        enum InfoLine {
            Owner,              ///< "The Evil Empire".
            Radius,             ///< "89 ly radius".
            Units,              ///< "8,056 units".
            AfterDecay,         ///< "7,653 units (87 ly)"
            LastInfo,           ///< "this turn"
            ControlPlanet,      ///< "Star Curcuit"
            ControlPlayer       ///< "The Evil Empire"
        };
        static const size_t NUM_INFO_LINES = static_cast<size_t>(ControlPlayer)+1;

        /** Information about a minefield.
            Contains information in human-readable and machine-readable form,
            mostly ad-hoc for PCC2's GUI needs. */
        struct MinefieldInfo {
            Id_t minefieldId;                ///< Minefield Id.
            Id_t controllingPlanetId;        ///< Controlling planet Id; 0 if not known.
            game::map::Point center;         ///< Center location.
            int radius;                      ///< Radius.
            String_t text[NUM_INFO_LINES];   ///< Textual information in human-readable form.
            MinefieldInfo()
                : minefieldId(), controllingPlanetId(), center(), radius()
                { }
        };

        /** Information about minefield passage probabilities. */
        struct PassageInfo {
            double normalPassageRate;        ///< Normal passage rate [0,1].
            double cloakedPassageRate;       ///< Cloaked passage rate [0,1].
            int distance;                    ///< Distance used for computing passage rate.
            PassageInfo()
                : normalPassageRate(), cloakedPassageRate(), distance()
                { }
        };

        /** Item in SweepInfo::weapons. */
        struct SweepItem {
            int needed;                      ///< Number of required weapons (beams, fighters).
            int have;                        ///< Number of available weapons.
            String_t name;                   ///< Name.
            SweepItem(int needed, int have, String_t name)
                : needed(needed), have(have), name(name)
                { }
        };

        /** Information about mine sweep. */
        struct SweepInfo {
            int32_t units;                   ///< Number of units. Can differ from minefield's current size; see HostVersion::isMineLayingAfterMineDecay()
            bool isWeb;                      ///< True for web minefields.
            std::vector<SweepItem> weapons;  ///< Weapons usable against this minefield.
            SweepInfo()
                : units(), isWeb(), weapons()
                { }
        };

        /** Constructor.
            \param reply      RequestDispatcher to receive replies on
            \param gameSender Game sender */
        MinefieldProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~MinefieldProxy();

        /** Set passage distance.
            Updated values will be reported on sig_passageChange.
            \param distance Travel distance in ly. Defaults to radius of current minefield. */
        void setPassageDistance(int distance);

        /** Get minesweep informtion.
            \param [in,out] ind WaitIndicator for UI synchronisation
            \param [out]    out Result */
        void getSweepInfo(WaitIndicator& ind, SweepInfo& out);

        /** Browse minefields.
            Updated information will be reported on sig_passageChange, sig_minefieldChange.
            \param mode Mode
            \param marked true to accept only marked minefields */
        void browse(game::map::ObjectCursor::Mode mode, bool marked);

        /** Erase minefield by Id.
            If the minefield is currently being looked at, will emit sig_minefieldChange, sig_passageChange.
            \param id Minefield Id
            \see game::map::MinefieldType::erase() */
        void erase(Id_t id);

        // ObjectObserver:
        virtual void addNewListener(ObjectListener* pl);

        /** Signal: Minefield changes.
            Emitted whenever the minefield changes, or a different minefield is selected on Cursors::currentMinefield()
            using this proxy's methods or others.

            If the MinefieldProxy is constructed for a game with no minefields,
            or the last minefield is deleted, this signal is emitted once with a minefieldId of zero.

            \param info Updated information */
        afl::base::Signal<void(const MinefieldInfo&)> sig_minefieldChange;

        /** Signal: Passage information changes.
            Emitted whenever a different minefield is selected on Cursors::currentMinefield() using this proxy's methods or others,
            or the travel distance is changed using setPassageDistance().
            \param info Updated information */
        afl::base::Signal<void(const PassageInfo&)> sig_passageChange;
     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<MinefieldProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
