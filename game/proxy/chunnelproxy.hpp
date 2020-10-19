/**
  *  \file game/proxy/chunnelproxy.hpp
  *  \brief Class game::proxy::ChunnelProxy
  */
#ifndef C2NG_GAME_PROXY_CHUNNELPROXY_HPP
#define C2NG_GAME_PROXY_CHUNNELPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/data/stringlist.hpp"
#include "game/map/point.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Chunnel proxy.

        Bidirectional, asynchronous:
        - get list of possible chunnel targets (postCandidateRequest(), sig_candidateListUpdate), as list of positions.

        Bidirectional, synchronous:
        - get list of possible chunnel targets by location (getCandidates), as list of named units. */
    class ChunnelProxy {
     public:
        /** Possible chunnel target.
            This is a structure instead of just a game::map::Point so we can add more information as required. */
        struct Candidate {
            game::map::Point pos;

            Candidate()
                : pos()
                { }
            Candidate(game::map::Point pos)
                : pos(pos)
                { }

            bool operator==(const Candidate& b) const;
            bool operator!=(const Candidate& b) const;
            bool operator<(const Candidate& b) const;
        };

        /** List of possible chunnel targets. */
        struct CandidateList {
            int minDistance;
            std::vector<Candidate> candidates;

            CandidateList()
                : minDistance(0), candidates()
                { }

            bool operator==(const CandidateList& b) const;
            bool operator!=(const CandidateList& b) const;
        };


        /** Constructor.
            \param gameSender Sender
            \param reply RequestDispatcher to send replies back */
        ChunnelProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~ChunnelProxy();

        /** Asynchronous request for possible chunnel targets (locations).
            Arranges for sig_candidateListUpdate to be called with the CandidateList.
            \param shipId Initiator ship Id */
        void postCandidateRequest(Id_t shipId);

        /** Synchronous request for possible chunnel targets at a location (ships).
            \param link   [in] WaitIndicator
            \param shipId [in] Initiator ship Id
            \param pos    [in] Desired chunnel target position
            \param list   [out] Result */
        void getCandidates(WaitIndicator& link, Id_t shipId, game::map::Point pos, game::ref::UserList& list);

        /** Synchronous request to set up a chunnel.
            \param link       [in] WaitIndicator
            \param fromShipId [in] Initiator ship Id
            \param toShipId   [in] Mate ship Id
            \return List of possible failures user needs to fix */
        afl::data::StringList_t setupChunnel(WaitIndicator& link, Id_t fromShipId, Id_t toShipId);

        /** Signal: new CandidateList
            \see postCandidateRequest */
        afl::base::Signal<void(const CandidateList&)> sig_candidateListUpdate;

     private:
        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<ChunnelProxy> m_reply;
    };

} }

#endif
