/**
  *  \file u/t_game_proxy_specbrowserproxy.cpp
  *  \brief Test for game::proxy::SpecBrowserProxy
  */

#include "game/proxy/specbrowserproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/test/sessionthread.hpp"
#include "util/simplerequestdispatcher.hpp"
#include "game/test/root.hpp"

namespace gsi = game::spec::info;
using game::test::SessionThread;
using game::HostVersion;

namespace {
    /*
     *  Setup
     */

    void addRoot(SessionThread& s)
    {
        s.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,4,0))).asPtr());
    }

    void addShipList(SessionThread& s)
    {
        s.session().setShipList(new game::spec::ShipList());
    }

    void createEngine(SessionThread& s, int id, String_t name, int tech)
    {
        game::spec::Engine* e = s.session().getShipList()->engines().create(id);
        e->setName(name);
        e->setTechLevel(tech);
    }

    void prepare(SessionThread& s)
    {
        addRoot(s);
        addShipList(s);
        createEngine(s, 2, "Two-speed", 3);
        createEngine(s, 3, "Three-speed", 4);
        createEngine(s, 4, "Four-speed", 5);
    }

    /*
     *  Event Receivers
     */

    struct ListReceiver {
        gsi::ListContent list;

        void onListChange(const gsi::ListContent& list, size_t)
            { this->list = list; }
    };

    struct PageReceiver {
        gsi::PageContent content;

        void onPageChange(const gsi::PageContent& content)
            { this->content = content; }
    };

    class NamedPageReceiver {
     public:
        NamedPageReceiver(String_t expectedName)
            : m_expectedName(expectedName), m_count(0)
            { }

        void onPageChange(const gsi::PageContent& content)
            {
                TS_ASSERT_EQUALS(content.title, m_expectedName);
                ++m_count;
            }

        int getCount() const
            { return m_count; }

     private:
        String_t m_expectedName;
        int m_count;
    };

    struct FilterReceiver {
        gsi::FilterInfos_t existing;
        gsi::FilterInfos_t available;

        void onFilterChange(const game::spec::info::FilterInfos_t& existing, const game::spec::info::FilterInfos_t& available)
            {
                this->existing = existing;
                this->available = available;
            }
    };

    struct SortReceiver {
        gsi::FilterAttribute active;
        gsi::FilterAttributes_t available;
        void onSortChange(game::spec::info::FilterAttribute active, game::spec::info::FilterAttributes_t available)
            {
                this->active = active;
                this->available = available;
            }
    };
}

/** Simple test sequence.
    A: prepare a ship list. Request data, filter it.
    E: correct data produced */
void
TestGameProxySpecBrowserProxy::testIt()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));

    // Select a page
    ListReceiver list;
    testee.sig_listChange.add(&list, &ListReceiver::onListChange);
    testee.setPage(gsi::EnginePage);
    while (list.list.content.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify list content
    TS_ASSERT_EQUALS(list.list.content.size(), 3U);
    TS_ASSERT_EQUALS(list.list.content[0].name, "Two-speed");
    TS_ASSERT_EQUALS(list.list.content[0].id, 2);
    TS_ASSERT_EQUALS(list.list.content[1].name, "Three-speed");
    TS_ASSERT_EQUALS(list.list.content[1].id, 3);
    TS_ASSERT_EQUALS(list.list.content[2].name, "Four-speed");
    TS_ASSERT_EQUALS(list.list.content[2].id, 4);

    // Select an entry
    PageReceiver page;
    testee.sig_pageChange.add(&page, &PageReceiver::onPageChange);
    testee.setId(3);
    while (page.content.title != "Three-speed") {
        TS_ASSERT(disp.wait(1000));
    }

    // Set some filters
    FilterReceiver filter;
    testee.sig_filterChange.add(&filter, &FilterReceiver::onFilterChange);
    testee.addFilter(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(3, 5)));
    testee.setNameFilter("ree");
    while (filter.existing.size() != 2U || list.list.content.size() != 1) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify filters
    TS_ASSERT_EQUALS(filter.existing[0].name, "Tech level");
    TS_ASSERT_EQUALS(filter.existing[0].value, "3 to 5");
    TS_ASSERT_EQUALS(filter.existing[1].name, "Name");        // Name is always last
    TS_ASSERT_EQUALS(filter.existing[1].value, "ree");

    // Verify filtered list
    TS_ASSERT_EQUALS(list.list.content.size(), 1U);
    TS_ASSERT_EQUALS(list.list.content[0].name, "Three-speed");
    TS_ASSERT_EQUALS(list.list.content[0].id, 3);
}

/** Test filter modifications.
    A: add some filters.
    E: correct filter reported back */
void
TestGameProxySpecBrowserProxy::testFilter()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));
    testee.setPage(gsi::EnginePage);

    // Add a filter and wait for it to echo back
    FilterReceiver filter;
    testee.sig_filterChange.add(&filter, &FilterReceiver::onFilterChange);
    testee.addFilter(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(3, 5)));
    while (filter.existing.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify filter
    TS_ASSERT_EQUALS(filter.existing.size(), 1U);
    TS_ASSERT_EQUALS(filter.existing[0].name, "Tech level");
    TS_ASSERT_EQUALS(filter.existing[0].value, "3 to 5");

    // Modify filter
    testee.setFilter(0, gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(4, 4)));
    while (filter.existing.size() != 1U || filter.existing[0].value != "4") {
        TS_ASSERT(disp.wait(1000));
    }

    // Remove filter
    testee.eraseFilter(0);
    while (!filter.existing.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
}

/** Test sort modifications.
    A: change sort order.
    E: sort order reported back */
void
TestGameProxySpecBrowserProxy::testSort()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));
    SortReceiver sort;
    testee.sig_sortChange.add(&sort, &SortReceiver::onSortChange);
    testee.setPage(gsi::EnginePage);
    while (sort.available.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    // Verify initial sort
    TS_ASSERT_EQUALS(sort.active, gsi::Range_Id);
    TS_ASSERT(sort.available.contains(gsi::String_Name));
    TS_ASSERT(sort.available.contains(gsi::Range_Tech));

    // Sort
    testee.setSortOrder(gsi::Range_Tech);
    while (sort.active != gsi::Range_Tech) {
        TS_ASSERT(disp.wait(1000));
    }
}

/** Test setPageId.
    A: use setPageId.
    E: only one update received for that page. */
void
TestGameProxySpecBrowserProxy::testSetPageId()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));
    NamedPageReceiver recv("Four-speed");
    testee.sig_pageChange.add(&recv, &NamedPageReceiver::onPageChange);
    testee.setPageId(gsi::EnginePage, 4);
    while (recv.getCount() == 0) {
        TS_ASSERT(disp.wait(1000));
    }
}

