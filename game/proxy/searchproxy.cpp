/**
  *  \file game/proxy/searchproxy.cpp
  *  \brief Class game::proxy::SearchProxy
  */

#include "game/proxy/searchproxy.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/string/translator.hpp"
#include "game/interface/referencelistcontext.hpp"
#include "interpreter/process.hpp"

namespace {
    struct QueryExtra : public game::Extra {
        game::SearchQuery query;
    };
    const game::ExtraIdentifier<game::Session, QueryExtra> SEARCHQUERY_ID = {{}};
}


/*
 *  SearchProxy::Responder
 *
 *  A process finalizer that reports the search result to the SearchProxy.
 */

class game::proxy::SearchProxy::Responder : public interpreter::Process::Finalizer {
 public:
    Responder(util::RequestSender<SearchProxy> reply, afl::string::Translator& tx)
        : m_reply(reply), m_translator(tx)
        { }
    virtual void finalizeProcess(interpreter::Process& p);

    void signalError(const String_t& err);
    void signalSuccess(const game::ref::List& list);

 public:
    util::RequestSender<SearchProxy> m_reply;
    afl::string::Translator& m_translator;
};

void
game::proxy::SearchProxy::Responder::finalizeProcess(interpreter::Process& p)
{
    bool ok = false;
    switch (p.getState()) {
     case interpreter::Process::Suspended:
        // Unexpected suspension
        // Can be caused by users by calling "Stop" in a function.
        signalError(m_translator("Search failed: query suspended unexpectedly (script called \"Stop\")"));
        ok = true;
        break;

     case interpreter::Process::Frozen:
     case interpreter::Process::Runnable:
     case interpreter::Process::Running:
     case interpreter::Process::Waiting:
        // Unexpected state (should not happen)
        break;
                        
     case interpreter::Process::Ended:
        // Success
        if (const game::interface::ReferenceListContext* ctx = dynamic_cast<const game::interface::ReferenceListContext*>(p.getResult())) {
            // Script produced a ReferenceList
            signalSuccess(ctx->getList());
        } else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(p.getResult())) {
            // Script produced a string, which means a message as-is
            signalError(sv->getValue());
        } else {
            // Script did not produce a ReferenceList - error in CCUI$Search
            signalError(m_translator("Internal error: search query produced unexpected result"));
        }
        ok = true;
        break;

     case interpreter::Process::Terminated:
        // Abnormal termination
        // Can be caused by users by calling "End" in a function.
        signalError(m_translator("Search failed: query did not produce a result (script called \"End\")"));
        ok = true;
        break;

     case interpreter::Process::Failed:
        // Abnormal termination (error)
        // CCUI$Search does not throw.
        signalError(afl::string::Format(m_translator("Internal error: search failed unexpectedly: %s"), p.getError().what()));
        ok = true;
        break;
    }

    if (!ok) {
        // Unexpected state or state not covered by switch()
        // This means the finalizer was called in a state where we didn't expect it.
        signalError(m_translator("Internal error: query stopped in wrong state"));
    }
}

void
game::proxy::SearchProxy::Responder::signalError(const String_t& err)
{
    class Task : public util::Request<SearchProxy> {
     public:
        Task(const String_t& err)
            : m_error(err)
            { }
        void handle(SearchProxy& proxy)
            { proxy.sig_error.raise(m_error); }
     private:
        String_t m_error;
    };
    m_reply.postNewRequest(new Task(err));
}

void
game::proxy::SearchProxy::Responder::signalSuccess(const game::ref::List& list)
{
    class Task : public util::Request<SearchProxy> {
     public:
        Task(const game::ref::List& list)
            : m_list(list)
            { }
        void handle(SearchProxy& proxy)
            { proxy.sig_success.raise(m_list); }
     private:
        game::ref::List m_list;
    };
    m_reply.postNewRequest(new Task(list));
}


/*
 *  SearchProxy
 */


game::proxy::SearchProxy::SearchProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_gameSender(gameSender)
{ }

void
game::proxy::SearchProxy::search(const SearchQuery& q, bool saveQuery)
{
    class Task : public util::Request<Session> {
     public:
        Task(const SearchQuery& q, bool saveQuery, util::RequestSender<SearchProxy> reply)
            : m_query(q), m_saveQuery(saveQuery), m_reply(reply)
            { }

        void handle(Session& session)
            {
                afl::string::Translator& tx = session.translator();
                try {
                    // Save it
                    if (m_saveQuery) {
                        savedQuery(session) = m_query;
                    }

                    // Start search driver in a process
                    interpreter::ProcessList& processList = session.processList();
                    interpreter::Process& proc = processList.create(session.world(), tx("Search query"));
                    proc.pushFrame(m_query.compile(session.world()), true);
                    proc.setNewFinalizer(new Responder(m_reply, tx));

                    uint32_t pgid = processList.allocateProcessGroup();
                    processList.resumeProcess(proc, pgid);
                    processList.startProcessGroup(pgid);
                    processList.run();
                    // FIXME: removeTerminatedProcesses()?
                }
                catch (std::exception& e) {
                    Responder(m_reply, tx).signalError(afl::string::Format(tx("Invalid search query: %s"), e.what()));
                }
            }
     private:
        SearchQuery m_query;
        bool m_saveQuery;
        util::RequestSender<SearchProxy> m_reply;
    };
    m_gameSender.postNewRequest(new Task(q, saveQuery, m_reply.getSender()));
}

game::SearchQuery&
game::proxy::SearchProxy::savedQuery(Session& session)
{
    return session.extra().create(SEARCHQUERY_ID).query;
}
