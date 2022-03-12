/**
  *  \file game/proxy/imperialstatsproxy.cpp
  *  \brief Class game::proxy::ImperialStatsProxy
  */

#include "game/proxy/imperialstatsproxy.hpp"
#include "game/map/info/browser.hpp"

namespace {
    const char*const LOG_NAME = "game.proxy.imperial";
}

class game::proxy::ImperialStatsProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<ImperialStatsProxy> reply)
        : m_reply(reply),
          m_session(session),
          m_browser(session)
        { }

    void requestPageContent(game::map::info::Page page)
        {
            class Task : public util::Request<ImperialStatsProxy> {
             public:
                Task(game::map::info::Browser& bro, Session& session, game::map::info::Page page)
                    : m_result()
                    {
                        try {
                            bro.renderPage(page, m_result);
                        }
                        catch (std::exception& e) {
                            session.log().write(afl::sys::LogListener::Error, LOG_NAME, "requestPageContent", e);
                        }
                    }
                virtual void handle(ImperialStatsProxy& proxy)
                    { proxy.sig_pageContent.raise(m_result); }
             private:
                Nodes_t m_result;
            };
            m_reply.postNewRequest(new Task(m_browser, m_session, page));
        }

    void requestPageOptions(game::map::info::Page page)
        {
            class Task : public util::Request<ImperialStatsProxy> {
             public:
                Task(game::map::info::Browser& bro, game::map::info::Page page)
                    : m_result(), m_current(bro.getPageOptions(page))
                    { bro.renderPageOptions(page, m_result); }
                virtual void handle(ImperialStatsProxy& proxy)
                    { proxy.sig_pageOptions.raise(m_result, m_current); }
             private:
                util::StringList m_result;
                PageOptions_t m_current;
            };
            m_reply.postNewRequest(new Task(m_browser, page));
        }

    void setPageOptions(game::map::info::Page page, PageOptions_t opts)
        {
            m_browser.setPageOptions(page, opts);
        }

 private:
    util::RequestSender<ImperialStatsProxy> m_reply;
    Session& m_session;
    game::map::info::Browser m_browser;
};

class game::proxy::ImperialStatsProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<ImperialStatsProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<ImperialStatsProxy> m_reply;
};


game::proxy::ImperialStatsProxy::ImperialStatsProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

game::proxy::ImperialStatsProxy::~ImperialStatsProxy()
{ }

void
game::proxy::ImperialStatsProxy::requestPageContent(game::map::info::Page page)
{
    m_sender.postRequest(&Trampoline::requestPageContent, page);
}

void
game::proxy::ImperialStatsProxy::requestPageOptions(game::map::info::Page page)
{
    m_sender.postRequest(&Trampoline::requestPageOptions, page);
}

void
game::proxy::ImperialStatsProxy::setPageOptions(game::map::info::Page page, PageOptions_t opts)
{
    m_sender.postRequest(&Trampoline::setPageOptions, page, opts);
}
