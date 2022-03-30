/**
  *  \file game/proxy/scripteditorproxy.cpp
  *  \brief Class game::proxy::ScriptEditorProxy
  */

#include "game/proxy/scripteditorproxy.hpp"
#include "game/proxy/waitindicator.hpp"

namespace {
    /* Collect list of contexts in a vector */
    class ContextCollector : public interpreter::ContextReceiver {
     public:
        virtual void pushNewContext(interpreter::Context* p)
            { m_contexts.pushBackNew(p); }

        void collect(game::Session& session, game::interface::ContextProvider* ctxp)
            {
                if (ctxp != 0) {
                    ctxp->createContext(session, *this);
                }
            }

        afl::container::PtrVector<interpreter::Context>& get()
            { return m_contexts; }
     private:
        afl::container::PtrVector<interpreter::Context> m_contexts;
    };
}


// Constructor.
game::proxy::ScriptEditorProxy::ScriptEditorProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

void
game::proxy::ScriptEditorProxy::buildCompletionList(WaitIndicator& ind,
                                                    game::interface::CompletionList& result,
                                                    String_t text,
                                                    bool onlyCommands,
                                                    std::auto_ptr<game::interface::ContextProvider> ctxp)
{
    class Query : public util::Request<Session> {
     public:
        Query(game::interface::CompletionList& result, const String_t& text, bool onlyCommands, std::auto_ptr<game::interface::ContextProvider> ctxp)
            : m_result(result),
              m_text(text),
              m_onlyCommands(onlyCommands),
              m_contextProvider(ctxp)
            { }
        virtual void handle(Session& session)
            {
                ContextCollector c;
                c.collect(session, m_contextProvider.get());
                game::interface::buildCompletionList(m_result, m_text, session, false, c.get());
            }
     private:
        game::interface::CompletionList& m_result;
        String_t m_text;
        bool m_onlyCommands;
        std::auto_ptr<game::interface::ContextProvider> m_contextProvider;
    };

    Query q(result, text, onlyCommands, ctxp);
    ind.call(m_gameSender, q);
}

void
game::proxy::ScriptEditorProxy::buildPropertyList(WaitIndicator& ind,
                                                  game::interface::PropertyList& result,
                                                  std::auto_ptr<game::interface::ContextProvider> ctxp)
{
    class Query : public util::Request<Session> {
     public:
        Query(game::interface::PropertyList& result,
              std::auto_ptr<game::interface::ContextProvider> ctxp)
            : m_result(result),
              m_contextProvider(ctxp)
            { }
        virtual void handle(Session& session)
            {
                ContextCollector c;
                c.collect(session, m_contextProvider.get());
                for (size_t i = c.get().size(); i > 0; --i) {
                    if (const game::map::Object* obj = c.get()[i-1]->getObject()) {
                        game::interface::buildPropertyList(m_result, obj, session.world(), session.translator());
                        break;
                    }
                }
            }
     private:
        game::interface::PropertyList& m_result;
        std::auto_ptr<game::interface::ContextProvider> m_contextProvider;
    };
    Query q(result, ctxp);
    ind.call(m_gameSender, q);
}
