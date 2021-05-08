/**
  *  \file game/proxy/planetpredictorproxy.hpp
  *  \brief Class game::proxy::PlanetPredictorProxy
  */
#ifndef C2NG_GAME_PROXY_PLANETPREDICTORPROXY_HPP
#define C2NG_GAME_PROXY_PLANETPREDICTORPROXY_HPP

#include "game/actions/taxationaction.hpp"
#include "game/map/planeteffectors.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy for planet prediction.
        Provides bidirectional access to a PlanetPredictor object.

        To use, construct, and then at least configure number of turns (setNumTurns).
        Obtain data using sig_update or getStatus().

        Asynchronous, bidirectional:
        - configure
        - report updated prediction

        Synchronous, bidirectional:
        - query prediction
        - query PlanetEffectors */
    class PlanetPredictorProxy {
     public:
        typedef game::actions::TaxationAction::Area Area_t;
        typedef std::vector<int32_t> Vector_t;

        /** Status.
            Each of the vectors contains a prediction series.
            - empty vector: this category is not predicted (e.g. no natives)
            - nonempty: first value is current status, following values are predictions

            Because the PlanetPredictorProxy starts with 0 turns lookahead (setNumTurns(0)),
            vectors on the initial report/query will either contain 0 or 1 element. */
        struct Status {
            Vector_t colonistClans;       ///< Number of colonist clans.
            Vector_t nativeClans;         ///< Number of native clans.
            Vector_t experiencePoints;    ///< Experience points.
            Vector_t experienceLevel;     ///< Experience level (derived from points).
            String_t effectorLabel;       ///< Description of PlanetEffectors.
        };

        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive replies
            \param planetId   Planet Id*/
        PlanetPredictorProxy(util::RequestDispatcher& reply,
                             util::RequestSender<Session> gameSender,
                             Id_t planetId);

        /** Destructor. */
        ~PlanetPredictorProxy();

        /** Get effectors.
            After constructing the object, this object represents the current situation (number of ships hissing etc.).
            \param ind WaitIndicator for UI synchronisation
            \return PlanetEffectors
            \see game::map::preparePlanetEffectors */
        game::map::PlanetEffectors getEffectors(WaitIndicator& ind);

        /** Get current status, synchronously.
            \param [in]  ind WaitIndicator for UI synchronisation
            \param [out] out Result */
        void getStatus(WaitIndicator& ind, Status& out);

        /** Set effectors.
            The updated prediction will arrive as sig_update callback.
            \param eff New PlanetEffectors */
        void setEffectors(const game::map::PlanetEffectors& eff);

        /** Set number of turns for prediction.
            The updated prediction will arrive as sig_update callback.
            \param n Number of turns. Default is 0. */
        void setNumTurns(int n);

        /** Set number of buildings for prediction.
            Can be used to predict a hypothetical situation.
            The updated prediction will arrive as sig_update callback.
            \param which Building type
            \param n     New count
            \see game::map::Planet::setNumBuildings */
        void setNumBuildings(PlanetaryBuilding which, int n);

        /** Set taxes.
            Can be used to predict a hypothetical situation.
            The updated prediction will arrive as sig_update callback.
            \param area  Area
            \param rate  New tax rate
            \see game::map::Planet::setNativeTax, game::map::Planet::setColonistTax */
        void setTax(Area_t area, int rate);

        /** Signal: new data.
            \param status New status */
        afl::base::Signal<void(const Status&)> sig_update;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<PlanetPredictorProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;

        template<typename Addr>
        void setProperty(Addr a, int32_t value);
    };

} }

#endif
