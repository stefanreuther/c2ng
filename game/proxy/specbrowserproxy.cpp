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

class game::proxy::SpecBrowserProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<SpecBrowserProxy> reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
        : m_session(session),
          m_reply(reply),
          m_picNamer(picNamer),
          m_browser(),
          m_filter(),
          m_page(),
          m_id(),
          m_sort(game::spec::info::Range_Id),
          m_withCost(true)
        {
            const Root* pRoot = session.getRoot().get();
            const game::spec::ShipList* pShipList = session.getShipList().get();
            const Game* pGame = session.getGame().get();
            if (pRoot != 0 && pShipList != 0) {
                m_browser.reset(new game::spec::info::Browser(*m_picNamer, *pRoot, *pShipList, pGame != 0 ? pGame->getViewpointPlayer() : 0, session.translator()));
            }
        }

    void setPage(game::spec::info::Page page);
    void setId(Id_t id);
    void setPageId(game::spec::info::Page page, Id_t id);
    void eraseFilter(size_t index);
    void addFilter(game::spec::info::FilterElement elem);
    void addCurrentAsFilter();
    void setFilter(size_t index, game::spec::info::FilterElement elem);
    void setNameFilter(String_t value);
    void setSortOrder(game::spec::info::FilterAttribute sort);
    void setWithCost(bool flag);

 private:
    void sendList();
    void sendPage();
    void sendFilter();
    void sendSortOrder();

    Session& m_session;
    util::RequestSender<SpecBrowserProxy> m_reply;
    std::auto_ptr<game::spec::info::PictureNamer> m_picNamer;
    std::auto_ptr<game::spec::info::Browser> m_browser;
    game::spec::info::Filter m_filter;
    game::spec::info::Page m_page;
    afl::base::Optional<Id_t> m_id;
    game::spec::info::FilterAttribute m_sort;
    bool m_withCost;
};


class game::proxy::SpecBrowserProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<SpecBrowserProxy> reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
        : m_reply(reply), m_picNamer(picNamer)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply, m_picNamer); }
 private:
    util::RequestSender<SpecBrowserProxy> m_reply;
    std::auto_ptr<game::spec::info::PictureNamer> m_picNamer;
};


/*
 *  Trampoline
 */

void
game::proxy::SpecBrowserProxy::Trampoline::setPage(game::spec::info::Page page)
{
    log(m_session, Log::Trace, Format("=> setPage(%d)", int(page)));
    if (m_page != page) {
        m_id.clear();
    }
    m_page = page;
    sendFilter();
    sendSortOrder();
    sendList();
}

void
game::proxy::SpecBrowserProxy::Trampoline::setId(Id_t id)
{
    log(m_session, Log::Trace, Format("=> setId(%d)", id));
    m_id = id;
    sendPage();
}

void
game::proxy::SpecBrowserProxy::Trampoline::setPageId(game::spec::info::Page page, Id_t id)
{
    log(m_session, Log::Trace, Format("=> setPageId(%d,%d)", int(page), id));
    m_page = page;
    m_id = id;
    sendFilter();
    sendSortOrder();
    sendList();  // implies sendPage
}

void
game::proxy::SpecBrowserProxy::Trampoline::eraseFilter(size_t index)
{
    log(m_session, Log::Trace, Format("=> eraseFilter(%d)", index));
    m_filter.erase(index);
    sendFilter();
    sendList();
}

void
game::proxy::SpecBrowserProxy::Trampoline::addFilter(game::spec::info::FilterElement elem)
{
    log(m_session, Log::Trace, "=> addFilter");
    m_filter.add(elem);
    sendFilter();
    sendList();
}

inline void
game::proxy::SpecBrowserProxy::Trampoline::addCurrentAsFilter()
{
    log(m_session, Log::Trace, "=> addCurrentAsFilter");
    Id_t id;
    if (m_browser.get() != 0 && m_id.get(id)) {
        m_browser->addItemFilter(m_filter, m_page, id);
        sendFilter();
        sendList();
    }
}

void
game::proxy::SpecBrowserProxy::Trampoline::setFilter(size_t index, game::spec::info::FilterElement elem)
{
    log(m_session, Log::Trace, Format("=> setFilter(%d)", index));
    m_filter.setRange(index, elem.range);
    m_filter.setValue(index, elem.value);
    sendFilter();
    sendList();
}

void
game::proxy::SpecBrowserProxy::Trampoline::setNameFilter(String_t value)
{
    log(m_session, Log::Trace, Format("=> setNameFilter('%s')", value));
    m_filter.setNameFilter(value);
    sendFilter();
    sendList();
}

inline void
game::proxy::SpecBrowserProxy::Trampoline::setSortOrder(game::spec::info::FilterAttribute sort)
{
    log(m_session, Log::Trace, "=> setSortOrder");
    m_sort = sort;
    sendSortOrder();
    sendList();
}

void
game::proxy::SpecBrowserProxy::Trampoline::setWithCost(bool flag)
{
    log(m_session, Log::Trace, afl::string::Format("=> setWithCost(%d)", int(flag)));
    m_withCost = flag;
    sendPage();
}

void
game::proxy::SpecBrowserProxy::Trampoline::sendList()
{
    if (!m_browser.get()) {
        log(m_session, Log::Warn, "<= sendList: no browser");
        return;
    }

    std::auto_ptr<game::spec::info::ListContent> content(m_browser->listItems(m_page, m_filter, m_sort));
    if (!content.get()) {
        log(m_session, Log::Trace, "<= sendList: no content");
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

    log(m_session, Log::Trace, Format("<= sendList(%d elems, slot %d, id %d)", content->content.size(), slot, m_id.orElse(-1)));

    class Response : public util::Request<SpecBrowserProxy> {
     public:
        Response(std::auto_ptr<game::spec::info::ListContent> p, size_t slot, game::spec::info::Page page)
            : m_p(p), m_slot(slot), m_page(page)
            { }
        virtual void handle(SpecBrowserProxy& proxy)
            { proxy.sig_listChange.raise(*m_p, m_slot, m_page); }
     private:
        std::auto_ptr<game::spec::info::ListContent> m_p;
        const size_t m_slot;
        const game::spec::info::Page m_page;
    };
    m_reply.postNewRequest(new Response(content, slot, m_page));

    sendPage();
}

void
game::proxy::SpecBrowserProxy::Trampoline::sendPage()
{
    if (!m_browser.get()) {
        log(m_session, Log::Warn, "<= sendPage: no browser");
        return;
    }

    Id_t id;
    if (!m_id.get(id)) {
        log(m_session, Log::Trace, "<= sendPage: no id");
        return;
    }

    std::auto_ptr<game::spec::info::PageContent> content(m_browser->describeItem(m_page, id, m_withCost, m_filter.getPlayerFilter()));
    if (!content.get()) {
        log(m_session, Log::Trace, "<= sendPage: no content");
        return;
    }

    log(m_session, Log::Trace, Format("<= sendPage(id=%d)", id));

    class Response : public util::Request<SpecBrowserProxy> {
     public:
        Response(std::auto_ptr<game::spec::info::PageContent> p, game::spec::info::Page page)
            : m_p(p), m_page(page)
            { }
        virtual void handle(SpecBrowserProxy& proxy)
            { proxy.sig_pageChange.raise(*m_p, m_page); }
     private:
        std::auto_ptr<game::spec::info::PageContent> m_p;
        game::spec::info::Page m_page;
    };
    m_reply.postNewRequest(new Response(content, m_page));
}

void
game::proxy::SpecBrowserProxy::Trampoline::sendFilter()
{
    if (!m_browser.get()) {
        log(m_session, Log::Warn, "<= sendFilter: no browser");
        return;
    }

    std::auto_ptr<game::spec::info::FilterInfos_t> existing = m_browser->describeFilters(m_page, m_filter);
    if (!existing.get()) {
        log(m_session, Log::Trace, "<= sendFilter: no existing filter");
        return;
    }

    std::auto_ptr<game::spec::info::FilterInfos_t> available = m_browser->getAvailableFilters(m_page, m_filter);
    if (!available.get()) {
        log(m_session, Log::Trace, "<= sendFilter: no available filter");
        return;
    }

    log(m_session, Log::Trace, Format("<= sendFilter(%d existing, %d available)", existing->size(), available->size()));

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

void
game::proxy::SpecBrowserProxy::Trampoline::sendSortOrder()
{
    if (!m_browser.get()) {
        log(m_session, Log::Warn, "<= sendFilter: no browser");
        return;
    }

    log(m_session, Log::Trace, "<= sendSortOrder");

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


/*
 *  SpecBrowserProxy
 */

game::proxy::SpecBrowserProxy::SpecBrowserProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender(), picNamer)))
{ }

game::proxy::SpecBrowserProxy::~SpecBrowserProxy()
{ }

void
game::proxy::SpecBrowserProxy::setPage(game::spec::info::Page p)
{
    m_sender.postRequest(&Trampoline::setPage, p);
}

void
game::proxy::SpecBrowserProxy::setId(Id_t id)
{
    m_sender.postRequest(&Trampoline::setId, id);
}

void
game::proxy::SpecBrowserProxy::setPageId(game::spec::info::Page p, Id_t id)
{
    m_sender.postRequest(&Trampoline::setPageId, p, id);
}

void
game::proxy::SpecBrowserProxy::eraseFilter(size_t index)
{
    m_sender.postRequest(&Trampoline::eraseFilter, index);
}

void
game::proxy::SpecBrowserProxy::addFilter(const game::spec::info::FilterElement& elem)
{
    m_sender.postRequest(&Trampoline::addFilter, elem);
}

void
game::proxy::SpecBrowserProxy::addCurrentAsFilter()
{
    m_sender.postRequest(&Trampoline::addCurrentAsFilter);
}

void
game::proxy::SpecBrowserProxy::setFilter(size_t index, const game::spec::info::FilterElement& elem)
{
    m_sender.postRequest(&Trampoline::setFilter, index, elem);
}

void
game::proxy::SpecBrowserProxy::setNameFilter(const String_t& value)
{
    m_sender.postRequest(&Trampoline::setNameFilter, value);
}

void
game::proxy::SpecBrowserProxy::setSortOrder(game::spec::info::FilterAttribute sort)
{
    m_sender.postRequest(&Trampoline::setSortOrder, sort);
}

void
game::proxy::SpecBrowserProxy::setWithCost(bool flag)
{
    m_sender.postRequest(&Trampoline::setWithCost, flag);
}
