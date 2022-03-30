/**
  *  \file game/proxy/scripteditorproxy.hpp
  *  \brief Class game::proxy::ScriptEditorProxy
  */
#ifndef C2NG_GAME_PROXY_SCRIPTEDITORPROXY_HPP
#define C2NG_GAME_PROXY_SCRIPTEDITORPROXY_HPP

#include "game/interface/completionlist.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/interface/propertylist.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Proxy for context-dependant script-editing tasks.
        Provides bidirectional, synchronous, stateless access.
        User specifies the context using a ContextProvider. */
    class ScriptEditorProxy {
     public:
        /** Constructor.
            @param gameSender Game sender. Provides access to global names, file system. */
        explicit ScriptEditorProxy(util::RequestSender<Session> gameSender);

        /** Build completion list.
            Lists possible continuations of the given text.

            @param [in,out] ind           WaitIndicator
            @param [out]    result        Result
            @param [in]     text          Text to complete
            @param [in]     onlyCommands  true to complete only command names (and nothing when not at command position); false to determine valid types from context
            @param [in]     ctxp          ContextProvider; can be null (output will only include global names, file names).

            @see game::interface::buildCompletionList() */
        void buildCompletionList(WaitIndicator& ind,
                                 game::interface::CompletionList& result,
                                 String_t text,
                                 bool onlyCommands,
                                 std::auto_ptr<game::interface::ContextProvider> ctxp);

        /** Build property list.
            Lists user-defined properties of the object given by the ContextProvider.

            @param [in,out] ind           WaitIndicator
            @param [out]    result        Result
            @param [in]     ctxp          ContextProvider; can be null (output will be empty).

            @see game::interface::buildPropertyList() */
        void buildPropertyList(WaitIndicator& ind,
                               game::interface::PropertyList& result,
                               std::auto_ptr<game::interface::ContextProvider> ctxp);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
