/**
  *  \file game/proxy/specbrowserproxy.cpp
  *  \brief Class game::proxy::SpecBrowserProxy
  */

#include "game/proxy/specbrowserproxy.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/spec/info/browser.hpp"
#include "game/spec/info/filter.hpp"
#include "util/slaveobject.hpp"

namespace {
    using afl::sys::Log;
    using afl::string::Format;

    const char*const LOG_NAME = "game.proxy.specbrowser";

    void log(game::Session& session, Log::Level level, const String_t& text)
    {
        session.log().write(level, LOG_NAME, text);
    }
}

/*
 *  FIXME: as of 20200520, this proxy will build up lag if requests come in faster than we reply to them.
 */

class game::proxy::SpecBrowserProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    Trampoline(util::RequestSender<SpecBrowserProxy> reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
        : m_reply(reply),
          m_picNamer(picNamer),
          m_browser(),
          m_filter(),
          m_page(),
          m_id(),
          m_sort(game::spec::info::Range_Id)
        { }

    virtual void init(Session& session)
        {
            const Root* pRoot = session.getRoot().get();
            const game::spec::ShipList* pShipList = session.getShipList().get();
            const Game* pGame = session.getGame().get();
            if (pRoot != 0 && pShipList != 0) {
                m_browser.reset(new game::spec::info::Browser(*m_picNamer, *pRoot, *pShipList, pGame != 0 ? pGame->getViewpointPlayer() : 0, session.translator()));
            }
        }

    virtual void done(Session& /*session*/)
        {
            m_browser.reset();
        }

    void setPage(Session& session, game::spec::info::Page page)
        {
            log(session, Log::Trace, Format("=> setPage(%d)", int(page)));
            if (m_page != page) {
                m_id.clear();
            }
            m_page = page;
            sendFilter(session);
            sendSortOrder(session);
            sendList(session);
        }

    void setId(Session& session, Id_t id)
        {
            log(session, Log::Trace, Format("=> setId(%d)", id));
            m_id = id;
            sendPage(session);
        }

    void eraseFilter(Session& session, size_t index)
        {
            log(session, Log::Trace, Format("=> eraseFilter(%d)", index));
            m_filter.erase(index);
            sendFilter(session);
            sendList(session);
        }

    void addFilter(Session& session, const game::spec::info::FilterElement& elem)
        {
            log(session, Log::Trace, "=> addFilter");
            m_filter.add(elem);
            sendFilter(session);
            sendList(session);
        }

    void addCurrentAsFilter(Session& session)
        {
            log(session, Log::Trace, "=> addCurrentAsFilter");
            Id_t id;
            if (m_browser.get() != 0 && m_id.get(id)) {
                m_browser->addItemFilter(m_filter, m_page, id);
                sendFilter(session);
                sendList(session);
            }
        }

    void setFilter(Session& session, size_t index, const game::spec::info::FilterElement& elem)
        {
            log(session, Log::Trace, Format("=> setFilter(%d)", index));
            m_filter.setRange(index, elem.range);
            m_filter.setValue(index, elem.value);
            sendFilter(session);
            sendList(session);
        }

    void setNameFilter(Session& session, const String_t& value)
        {
            log(session, Log::Trace, Format("=> setNameFilter('%s')", value));
            m_filter.setNameFilter(value);
            sendFilter(session);
            sendList(session);
        }

    void setSortOrder(Session& session, game::spec::info::FilterAttribute sort)
        {
            log(session, Log::Trace, "=> setSortOrder");
            m_sort = sort;
            sendSortOrder(session);
            sendList(session);
        }

 private:
    void sendList(Session& session)
        {
            if (!m_browser.get()) {
                log(session, Log::Error, "<= sendList: no browser");
                return;
            }

            std::auto_ptr<game::spec::info::ListContent> content(m_browser->listItems(m_page, m_filter, m_sort));
            if (!content.get()) {
                log(session, Log::Error, "<= sendList: no content");
                return;
            }

            size_t slot = 0;
            Id_t id;
            if (m_id.get(id)) {
                for (size_t i = 0, n = content->content.size(); i < n; ++i) {
                    if (content->content[i].id == id) {
                        slot = i;
                        break;
                    }
                }
            }

            m_id = (slot < content->content.size() ? content->content[slot].id : 0);

            log(session, Log::Trace, Format("<= sendList(%d elems, slot %d, id %d)", content->content.size(), slot, m_id.orElse(-1)));

            class Response : public util::Request<SpecBrowserProxy> {
             public:
                Response(std::auto_ptr<game::spec::info::ListContent> p, size_t slot)
                    : m_p(p), m_slot(slot)
                    { }
                virtual void handle(SpecBrowserProxy& proxy)
                    { proxy.sig_listChange.raise(*m_p, m_slot); }
             private:
                std::auto_ptr<game::spec::info::ListContent> m_p;
                size_t m_slot;
            };
            m_reply.postNewRequest(new Response(content, slot));

            sendPage(session);
        }

    void sendPage(Session& session)
        {
            if (!m_browser.get()) {
                log(session, Log::Error, "<= sendPage: no browser");
                return;
            }

            Id_t id;
            if (!m_id.get(id)) {
                log(session, Log::Error, "<= sendPage: no id");
                return;
            }

            std::auto_ptr<game::spec::info::PageContent> content(m_browser->describeItem(m_page, id));
            if (!content.get()) {
                log(session, Log::Error, "<= sendPage: no content");
                return;
            }

            log(session, Log::Trace, Format("<= sendPage(id=%d)", id));

            class Response : public util::Request<SpecBrowserProxy> {
             public:
                Response(std::auto_ptr<game::spec::info::PageContent> p)
                    : m_p(p)
                    { }
                virtual void handle(SpecBrowserProxy& proxy)
                    { proxy.sig_pageChange.raise(*m_p); }
             private:
                std::auto_ptr<game::spec::info::PageContent> m_p;
            };
            m_reply.postNewRequest(new Response(content));
        }

    void sendFilter(Session& session)
        {
            if (!m_browser.get()) {
                log(session, Log::Error, "<= sendFilter: no browser");
                return;
            }

            std::auto_ptr<game::spec::info::FilterInfos_t> existing = m_browser->describeFilters(m_page, m_filter);
            if (!existing.get()) {
                log(session, Log::Error, "<= sendFilter: no existing filter");
                return;
            }

            std::auto_ptr<game::spec::info::FilterInfos_t> available = m_browser->getAvailableFilters(m_page, m_filter);
            if (!available.get()) {
                log(session, Log::Error, "<= sendFilter: no available filter");
                return;
            }

            log(session, Log::Trace, Format("<= sendFilter(%d existing, %d available)", existing->size(), available->size()));

            class Response : public util::Request<SpecBrowserProxy> {
             public:
                Response(std::auto_ptr<game::spec::info::FilterInfos_t> existing, std::auto_ptr<game::spec::info::FilterInfos_t> available)
                    : m_existing(existing), m_available(available)
                    { }
                virtual void handle(SpecBrowserProxy& proxy)
                    { proxy.sig_filterChange.raise(*m_existing, *m_available); }
             private:
                std::auto_ptr<game::spec::info::FilterInfos_t> m_existing;
                std::auto_ptr<game::spec::info::FilterInfos_t> m_available;
            };
            m_reply.postNewRequest(new Response(existing, available));
        }

    void sendSortOrder(Session& session)
        {
            if (!m_browser.get()) {
                log(session, Log::Error, "<= sendFilter: no browser");
                return;
            }

            log(session, Log::Trace, "<= sendSortOrder");

            class Response : public util::Request<SpecBrowserProxy> {
             public:
                Response(game::spec::info::FilterAttribute active, game::spec::info::FilterAttributes_t available)
                    : m_active(active), m_available(available)
                    { }
                virtual void handle(SpecBrowserProxy& proxy)
                    { proxy.sig_sortChange.raise(m_active, m_available); }
             private:
                game::spec::info::FilterAttribute m_active;
                game::spec::info::FilterAttributes_t m_available;
            };
            m_reply.postNewRequest(new Response(m_sort, m_browser->getAvailableSortAttributes(m_page)));
        }

    util::RequestSender<SpecBrowserProxy> m_reply;
    std::auto_ptr<game::spec::info::PictureNamer> m_picNamer;
    std::auto_ptr<game::spec::info::Browser> m_browser;
    game::spec::info::Filter m_filter;
    game::spec::info::Page m_page;
    afl::base::Optional<Id_t> m_id;
    game::spec::info::FilterAttribute m_sort;
};


game::proxy::SpecBrowserProxy::SpecBrowserProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_receiver(receiver, *this),
      m_sender(gameSender, new Trampoline(m_receiver.getSender(), picNamer))
{ }

game::proxy::SpecBrowserProxy::~SpecBrowserProxy()
{ }

void
game::proxy::SpecBrowserProxy::setPage(game::spec::info::Page p)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(game::spec::info::Page p)
            : m_page(p)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setPage(session, m_page); }
     private:
        game::spec::info::Page m_page;
    };
    m_sender.postNewRequest(new Request(p));
}

void
game::proxy::SpecBrowserProxy::setId(Id_t id)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(Id_t id)
            : m_id(id)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setId(session, m_id); }
     private:
        Id_t m_id;
    };
    m_sender.postNewRequest(new Request(id));
}

void
game::proxy::SpecBrowserProxy::eraseFilter(size_t index)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(size_t index)
            : m_index(index)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.eraseFilter(session, m_index); }
     private:
        size_t m_index;
    };
    m_sender.postNewRequest(new Request(index));
}

void
game::proxy::SpecBrowserProxy::addFilter(const game::spec::info::FilterElement& elem)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(const game::spec::info::FilterElement& elem)
            : m_elem(elem)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.addFilter(session, m_elem); }
     private:
        game::spec::info::FilterElement m_elem;
    };
    m_sender.postNewRequest(new Request(elem));
}

void
game::proxy::SpecBrowserProxy::addCurrentAsFilter()
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.addCurrentAsFilter(session); }
    };
    m_sender.postNewRequest(new Request());
}

void
game::proxy::SpecBrowserProxy::setFilter(size_t index, const game::spec::info::FilterElement& elem)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(size_t index, const game::spec::info::FilterElement& elem)
            : m_index(index), m_elem(elem)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setFilter(session, m_index, m_elem); }
     private:
        size_t m_index;
        game::spec::info::FilterElement m_elem;
    };
    m_sender.postNewRequest(new Request(index, elem));
}

void
game::proxy::SpecBrowserProxy::setNameFilter(const String_t& value)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(const String_t& value)
            : m_value(value)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setNameFilter(session, m_value); }
     private:
        String_t m_value;
    };
    m_sender.postNewRequest(new Request(value));
}

void
game::proxy::SpecBrowserProxy::setSortOrder(game::spec::info::FilterAttribute sort)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(const game::spec::info::FilterAttribute sort)
            : m_sort(sort)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setSortOrder(session, m_sort); }
     private:
        game::spec::info::FilterAttribute m_sort;
    };
    m_sender.postNewRequest(new Request(sort));
}

