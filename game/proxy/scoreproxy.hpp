/**
  *  \file game/proxy/scoreproxy.hpp
  *  \brief Class game::proxy::ScoreProxy
  */
#ifndef C2NG_GAME_PROXY_SCOREPROXY_HPP
#define C2NG_GAME_PROXY_SCOREPROXY_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/score/scorebuilderbase.hpp"
#include "game/session.hpp"
#include "util/datatable.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "util/stringlist.hpp"

namespace game { namespace proxy {

    /** Proxy for score access.
        Wraps a game::score::ChartBuilder and a game::score::TableBuilder.
        To use,
        - construct
        - register listeners
        - configure; updates are sent upon every configuration call.
          No implicit updates are sent, so you need a (possibly no-op) call to get started.

        Bidirectional, synchronous:
        - retrieve possible score views

        Bidirectional, asynchronous:
        - configuring the view
        - update view content */
    class ScoreProxy {
     public:
        typedef game::score::ScoreBuilderBase::Variants_t Variants_t;

        /** Overview information. */
        struct Info {
            size_t numTurns;
            bool hasTeams;
            int viewpointPlayer;
            PlayerSet_t players;

            Info()
                : numTurns(0), hasTeams(false), viewpointPlayer(0), players()
                { }
        };

        /** Constructor.
            \param reply      RequestDispatcher to receive replies on
            \param gameSender Game sender */
        ScoreProxy(util::RequestDispatcher& reply, util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~ScoreProxy();

        /** Retrieve "chart" variants.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] out  Result
            \see game::score::ChartBuilder::getVariant */
        void getChartVariants(WaitIndicator& ind, Variants_t& out);

        /** Retrieve "table" variants.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] out  Result
            \see game::score::TableBuilder::getVariant */
        void getTableVariants(WaitIndicator& ind, Variants_t& out);

        /** Retrieve list of turns.
            Format of the returned list:
            - index is compatible with setTableTurnIndex()
            - key is turn number
            - string is the timestamp (Timestamp::getTimestampAsString())
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] out  List of turns */
        void getTurns(WaitIndicator& ind, util::StringList& out);

        /** Get overview information.
            This is ad-hoc information required to build a score view; extend as needed.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] out  Information */
        void getOverviewInformation(WaitIndicator& ind, Info& out);

        /** Select "chart" variant.
            The new content will be reported using sig_chartUpdate.
            \param Index into list of variants
            \see game::score::ChartBuilder::setVariantIndex */
        void setChartIndex(size_t index);

        /** Select "table" turn index.
            The new content will be reported using sig_tableUpdate.
            \param Index into list of variants
            \see game::score::TableBuilder::setTurnIndex */
        void setTableTurnIndex(size_t index);

        /** Select "table" turn pair to report differences.
            The new content will be reported using sig_tableUpdate.
            \param first Index of turn to display
            \param second Index of turn to subtract
            \see game::score::TableBuilder::setTurnDifferenceIndexes() */
        void setTableTurnDifferenceIndexes(size_t first, size_t second);

        /** Select by-team mode.
            The new content will be reported using sig_chartUpdate and sig_tableUpdate.
            \param flag false: build scores by player (default); true: build scores by team
            \see game::score::ChartBuilder::setByTeam, game::score::TableBuilder::setByTeam */
        void setByTeam(bool flag);

        /** Select cumulative mode.
            The new content will be reported using sig_chartUpdate.
            \param flag false: build individual scores (default); true: build cumulative (stacked) scores
            \see game::score::ChartBuilder::setCumulativeMode */
        void setCumulativeMode(bool flag);

        /** Signal: update of "chart" data.
            \param value New value (first listener can take it)
            \see game::score::ChartBuilder::build() */
        afl::base::Signal<void(std::auto_ptr<util::DataTable>&)> sig_chartUpdate;

        /** Signal: update of "table" data.
            \param value New value (first listener can take it)
            \see game::score::TableBuilder::build() */
        afl::base::Signal<void(std::auto_ptr<util::DataTable>&)> sig_tableUpdate;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<ScoreProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;

        template<typename Object, typename Parameter>
        void setOption(void (Object::*fcn)(Parameter), Parameter value);
    };

} }

#endif
