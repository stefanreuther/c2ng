/**
  *  \file game/proxy/specbrowserproxy.hpp
  *  \brief Class game::proxy::SpecBrowserProxy
  */
#ifndef C2NG_GAME_PROXY_SPECBROWSERPROXY_HPP
#define C2NG_GAME_PROXY_SPECBROWSERPROXY_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "game/session.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "game/spec/info/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"

namespace game { namespace proxy {

    /** Specification browser (Universe Almanac) proxy.

        Bidirectional, asynchronous:
        - select page and identifier
        - modify filters
        - receive list of items for current page
        - receive list of filters
        - receive content of selected item

        Set up an object and receive updates asynchronously.
        For now, the sequence and order of responses (signals) is unspecified.

        \see game::spec::info::Browser */
    class SpecBrowserProxy {
     public:
        /** Constructor.
            \param gameSender Sender
            \param receiver RequestDispatcher to receive replies
            \param picNamer PictureNamer (will be transferred to game thread) */
        SpecBrowserProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver, std::auto_ptr<game::spec::info::PictureNamer> picNamer);

        /** Destructor. */
        ~SpecBrowserProxy();

        /** Set page.
            \param p Page to show */
        void setPage(game::spec::info::Page p);

        /** Set Id.
            \param id Id; needs to be taken from the current page's list (sig_listChange). */
        void setId(Id_t id);

        /** Set page and Id.
            Use if you know the Id beforehand.
            \param p Page to show
            \param id Id */
        void setPageId(game::spec::info::Page p, Id_t id);

        /** Erase element from filter.
            \param index Index into filter */
        void eraseFilter(size_t index);

        /** Add new filter element.
            \param elem New element, derived from sig_filterChange's second argument. */
        void addFilter(const game::spec::info::FilterElement& elem);

        /** Add current element as filter.
            \see game::spec::info::Browser::addItemFilter(). */
        void addCurrentAsFilter();

        /** Update filter element.
            \param index Index
            \param elem New element, derived from sig_filterChange's first argument. */
        void setFilter(size_t index, const game::spec::info::FilterElement& elem);

        /** Set name filter.
            \param value New filter (empty to remove filter) */
        void setNameFilter(const String_t& value);

        /** Set sort order.
            \param sort New sort order */
        void setSortOrder(game::spec::info::FilterAttribute sort);

        /** Configure whether costs are included in reports.
            \param flag true: costs are reported as textual output (default); false: costs are not reported */
        void setWithCost(bool flag);

        /** Signal: list changed.
            \param list Current list content
            \param index Desired index to select */
        afl::base::Signal<void(const game::spec::info::ListContent& list, size_t index)> sig_listChange;

        /** Signal: page content changed.
            \param content New page content */
        afl::base::Signal<void(const game::spec::info::PageContent& content)> sig_pageChange;

        /** Signal: filter changed.
            \param existing Existing filters (for eraseFilter, setFilter)
            \param available Available filters (for addFilter) */
        afl::base::Signal<void(const game::spec::info::FilterInfos_t& existing,
                               const game::spec::info::FilterInfos_t& available)> sig_filterChange;

        /** Signal: sort order changed.
            \param active Active sort order
            \param available Available sort orders (for setSortOrder) */
        afl::base::Signal<void(game::spec::info::FilterAttribute active,
                               game::spec::info::FilterAttributes_t available)> sig_sortChange;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<SpecBrowserProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
