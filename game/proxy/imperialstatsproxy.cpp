/**
  *  \file game/proxy/imperialstatsproxy.cpp
  *  \brief Class game::proxy::ImperialStatsProxy
  */

#include "game/proxy/imperialstatsproxy.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/info/browser.hpp"
#include "game/map/info/nulllinkbuilder.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"
#include "util/doc/htmlrenderer.hpp"
#include "util/doc/renderoptions.hpp"
#include "util/string.hpp"

namespace {
    const char*const LOG_NAME = "game.proxy.imperial";

    // For simplicity (of testing) we allow constructing a ImperialStatsProxy even without a root;
    // many usages will fail, though.
    util::NumberFormatter getNumberFormatter(game::Root* r)
    {
        return r != 0 ? r->userConfiguration().getNumberFormatter() : util::NumberFormatter(false, false);
    }

    String_t getTitle(const afl::io::xml::Nodes_t& nodes)
    {
        if (!nodes.empty()) {
            if (const afl::io::xml::TagNode* tag = dynamic_cast<afl::io::xml::TagNode*>(nodes[0])) {
                if (tag->getName() == "h1") {
                    return tag->getTextContent();
                }
            }
        }

        // Does not happen; if it does, we generate HTML with empty <title> which is harmless
        return String_t();
    }
}

class game::proxy::ImperialStatsProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<ImperialStatsProxy> reply, std::auto_ptr<game::map::info::LinkBuilder> link)
        : m_reply(reply),
          m_session(session),
          m_link(link),
          m_browser(session, *m_link, getNumberFormatter(session.getRoot().get()))
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

    void savePageAsHTML(game::map::info::Page page, const String_t& fileName)
        {
            // Open file (exit early on failure)
            afl::base::Ref<afl::io::Stream> file = m_session.world().fileSystem().openFile(fileName, afl::io::FileSystem::Create);

            // Create a second browser with a NullLinkBuilder and a default NumberFormatter (to avoid i18n problems with exported numbers!)
            game::map::info::NullLinkBuilder linkBuilder;
            game::map::info::Browser localBrowser(m_session, linkBuilder, util::NumberFormatter(false, false));
            localBrowser.setPageOptions(page, m_browser.getPageOptions(page));

            // Render page into internal XML
            afl::io::xml::Nodes_t nodes;
            localBrowser.renderPage(page, nodes);

            // Transform to HTML
            util::doc::RenderOptions opts;
            String_t html = renderHTML(nodes, opts);

            // Write to file
            const char*const PFX = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><title>";
            const char*const PFX2 =
                "</title><style>"
                "table.normaltable { border: solid #ccc 1px; margin: 2px 0; }"      // Mark tables
                ".color-white { font-weight: bold; }"                               // We do not enforce dark background, highlight anyway
                ".color-green { color: #080; }"
                "</style></head><body>";
            const char*const SUF = "</body></html>\n";
            file->fullWrite(afl::string::toBytes(PFX));
            file->fullWrite(afl::string::toBytes(util::encodeHtml(getTitle(nodes), false)));
            file->fullWrite(afl::string::toBytes(PFX2));
            file->fullWrite(afl::string::toBytes(html));
            file->fullWrite(afl::string::toBytes(SUF));
            file->flush();
        }

 private:
    util::RequestSender<ImperialStatsProxy> m_reply;
    Session& m_session;
    std::auto_ptr<game::map::info::LinkBuilder> m_link;
    game::map::info::Browser m_browser;
};

class game::proxy::ImperialStatsProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<ImperialStatsProxy> reply, std::auto_ptr<game::map::info::LinkBuilder> link)
        : m_reply(reply), m_link(link)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_link); }
 private:
    util::RequestSender<ImperialStatsProxy> m_reply;
    std::auto_ptr<game::map::info::LinkBuilder> m_link;
};


game::proxy::ImperialStatsProxy::ImperialStatsProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, std::auto_ptr<game::map::info::LinkBuilder> link)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), link)))
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

bool
game::proxy::ImperialStatsProxy::savePageAsHTML(WaitIndicator& ind, game::map::info::Page page, String_t fileName, String_t& errorMessage)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::map::info::Page page, const String_t& fileName)
            : m_page(page), m_fileName(fileName), m_errorMessage(), m_ok(false)
            { }
        virtual void handle(Trampoline& tpl)
            {
                try {
                    tpl.savePageAsHTML(m_page, m_fileName);
                    m_ok = true;
                }
                catch (std::exception& e) {
                    m_errorMessage = e.what();
                    m_ok = false;
                }
            }
        const String_t& getErrorMessage() const
            { return m_errorMessage; }
        bool isOK() const
            { return m_ok; }
     private:
        game::map::info::Page m_page;
        String_t m_fileName;
        String_t m_errorMessage;
        bool m_ok;
    };

    Task t(page, fileName);
    ind.call(m_sender, t);
    if (t.isOK()) {
        return true;
    } else {
        errorMessage = t.getErrorMessage();
        return false;
    }
}
