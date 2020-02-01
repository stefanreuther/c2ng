/**
  *  \file client/proxy/chunnelproxy.hpp
  *  \brief Class client::proxy::ChunnelProxy
  */
#ifndef C2NG_CLIENT_PROXY_CHUNNELPROXY_HPP
#define C2NG_CLIENT_PROXY_CHUNNELPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "client/downlink.hpp"
#include "game/map/point.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "afl/data/stringlist.hpp"

namespace client { namespace proxy {

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
            \param reply RequestDispatcher to send replies back
            \param gameSender Sender */
        ChunnelProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender);

        /** Destructor. */
        ~ChunnelProxy();

        /** Asynchronous request for possible chunnel targets (locations).
            Arranges for sig_candidateListUpdate to be called with the CandidateList.
            \param shipId Initiator ship Id */
        void postCandidateRequest(game::Id_t shipId);

        /** Synchronous request for possible chunnel targets at a location (ships).
            \param link   [in] Downlink
            \param shipId [in] Initiator ship Id
            \param pos    [in] Desired chunnel target position
            \param list   [out] Result */
        void getCandidates(Downlink& link, game::Id_t shipId, game::map::Point pos, game::ref::UserList& list);

        /** Synchronous request to set up a chunnel.
            \param link       [in] Downlink
            \param fromShipId [in] Initiator ship Id
            \param toShipId   [in] Mate ship Id
            \return List of possible failures user needs to fix */
        afl::data::StringList_t setupChunnel(Downlink& link, game::Id_t fromShipId, game::Id_t toShipId);

        /** Signal: new CandidateList
            \see postCandidateRequest */
        afl::base::Signal<void(const CandidateList&)> sig_candidateListUpdate;

     private:
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<ChunnelProxy> m_reply;
    };

} }

#endif
