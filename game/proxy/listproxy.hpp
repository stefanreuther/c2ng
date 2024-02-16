/**
  *  \file game/proxy/listproxy.hpp
  *  \brief Class game::proxy::ListProxy
  */
#ifndef C2NG_GAME_PROXY_LISTPROXY_HPP
#define C2NG_GAME_PROXY_LISTPROXY_HPP

#include <memory>
#include "game/map/point.hpp"
#include "game/ref/list.hpp"
#include "game/session.hpp"
#include "game/spec/costsummary.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Ship list proxy.
        Provides ability to retrieve a list of ships, intended for the "visual scan" function.

        Bidirectional, synchronous:
        - load list of current or future ships
        - get cargo summary

        Information is generated in the moment the respective synchronous call is made;
        results are cached and not automatically updated. */
    class ListProxy {
     public:
        /** Constructor.
            Initializes cached information for an empty list.
            @param sender Sender */
        explicit ListProxy(util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~ListProxy();

        /** Build list of current ships.
            Lists all ships at the given point, in viewpoint turn, according to given options.

            @param ind          WaitIndicator for UI synchronisation
            @param pos          Position
            @param options      Options to select which objects to add
            @param excludeShip  Do not include this ship Id even if it is otherwise eligible (0=exclude none)

            @see game::ref::List::addObjectsAt() */
        void buildCurrent(WaitIndicator& ind, game::map::Point pos, game::ref::List::Options_t options, Id_t excludeShip);

        /** Build list of ships according to prediction.
            Computes next-turn ship positions, starting at viewpoint turn, and lists all ships at a position.

            @param ind          WaitIndicator for UI synchronisation
            @param pos          Position
            @param fromShip     If a valid ship Id, list ships at this ship's next position instead of @c pos
            @param options      Options (IncludeForeignShips, SafeShipsOnly) */
        void buildNext(WaitIndicator& ind, game::map::Point pos, Id_t fromShip, game::ref::List::Options_t options);

        /** Get cargo summary.
            If list was loaded using buildCurrent(), builds a list of current ship's cargo.
            If list was loaded using buildNext(), builds a list of predicted ship's next-turn cargo.
            @param ind WaitIndicator for UI synchronisation
            @return cargo summary */
        game::spec::CostSummary getCargoSummary(WaitIndicator& ind);

        /** Get list.
            @return cached list from previous build() call */
        const game::ref::List getList() const;

        /** Get status.
            @return true if list was built with buildCurrent(), false if list was built with buildNext() */
        bool isCurrent() const;

        /** Check for unique playable unit.
            @return true if list contains a single playable unit. false if more or fewer ships, or not playable. */
        bool isUniquePlayable() const;

        /** Check status of remote control support.
            @return host configuration CPEnableRemote setting */
        bool hasRemoteControl() const;

        /** Check whether buildCurrent() excludeShip parameter was honored.
            @return true excludeShip referred to an eligible ship and was excluded;
                    false if ship was not specified or not eligible */
        bool hasExcludedShip() const;

        /** Check whether the given location refers to a planet that may be hiding ships.
            @return status */
        bool hasHidingPlanet() const;

        /** Get name of planet that may be hiding ships.
            @return name */
        const String_t& getHidingPlanetName() const;

     private:
        util::RequestSender<Session> m_gameSender;

        struct State;
        std::auto_ptr<State> m_state;
        static void setCommon(Session& session, State& st);
    };

} }

#endif
