/**
  *  \file game/proxy/basestorageproxy.hpp
  *  \brief Class game::proxy::BaseStorageProxy
  */
#ifndef C2NG_GAME_PROXY_BASESTORAGEPROXY_HPP
#define C2NG_GAME_PROXY_BASESTORAGEPROXY_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "game/proxy/starbaseadaptor.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for starbase component storage.
        Provides access to names, status and amounts of starship components on a starbase.
        To use, retrieve data using the synchronous getParts() call; then listen to sig_update for changes.

        It provides two ways to access hulls:
        - allHulls=false (default): reports all hulls that can be built on the starbase, in specification order
        - allHulls=true: reports all hulls (those that cannot be built, with count 0), in alphabetical order

        Synchronous:
        - retrieve list of parts possibly in storage

        Asynchronous:
        - receive updates to part list */
    class BaseStorageProxy {
     public:
        /** Information about one part. */
        struct Part {
            int id;                 ///< Id number (for hulls, NOT slot number).
            int numParts;           ///< Number of available parts.
            TechStatus techStatus;  ///< Tech status.
            String_t name;          ///< Name.

            Part(int id, int numParts, TechStatus techStatus, String_t name)
                : id(id), numParts(numParts), techStatus(techStatus), name(name)
                { }
        };

        /** List of parts. */
        typedef std::vector<Part> Parts_t;


        /** Constructor (planet).
            Provides access to the given planet in viewpointTurn().
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive updates in this thread
            \param planetId   Planet Id
            \param allHulls   true to report all hulls, false (default) only those buildable on this base */
        BaseStorageProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, Id_t planetId, bool allHulls = false);

        /** Constructor (general).
            \param adaptorSender StarbaseAdaptor sender
            \param receiver      RequestDispatcher to receive updates in this thread
            \param allHulls   true to report all hulls, false (default) only those buildable on this base */
        BaseStorageProxy(util::RequestSender<StarbaseAdaptor> adaptorSender, util::RequestDispatcher& receiver, bool allHulls = false);

        /** Destructor. */
        ~BaseStorageProxy();

        /** Get list of parts, synchronously.
            \param [in]  ind     WaitIndicator
            \param [in]  level   Area to retrieve information for
            \param [out] result  Part information */
        void getParts(WaitIndicator& ind, TechLevel level, Parts_t& result);

        /** Check all-hulls status.
            If true, reports all hulls; if false, only those buildable on this base
            \return Value of allHulls parameter to constructor */
        bool hasAllHulls() const;

        /** Signal: part list update.
            Sent whenever the part list changes, e.g. by a change to the planet.
            \param area Area
            \param list List of parts (same as getParts() result for that area) */
        afl::base::Signal<void(TechLevel, const Parts_t&)> sig_update;

     private:
        struct Trampoline;
        class TrampolineFromAdaptor;
        util::RequestReceiver<BaseStorageProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
        const bool m_allHulls;
    };

} }

#endif
