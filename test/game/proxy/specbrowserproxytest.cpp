/**
  *  \file test/game/proxy/specbrowserproxytest.cpp
  *  \brief Test for game::proxy::SpecBrowserProxy
  */

#include "game/proxy/specbrowserproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "util/simplerequestdispatcher.hpp"

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
        gsi::Page page;

        ListReceiver()
            : list(), page()
            { }
        void onListChange(const gsi::ListContent& list, size_t, gsi::Page page)
            { this->list = list; this->page = page; }
    };

    struct PageReceiver {
        gsi::PageContent content;
        gsi::Page page;

        PageReceiver()
            : content(), page()
            { }
        void onPageChange(const gsi::PageContent& content, gsi::Page page)
            { this->content = content; this->page = page; }
    };

    class NamedPageReceiver {
     public:
        NamedPageReceiver(afl::test::Assert a, String_t expectedName)
            : m_assert(a), m_expectedName(expectedName), m_count(0)
            { }

        void onPageChange(const gsi::PageContent& content, gsi::Page)
            {
                m_assert.checkEqual("onPageChange title", content.title, m_expectedName);
                ++m_count;
            }

        int getCount() const
            { return m_count; }

     private:
        afl::test::Assert m_assert;
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
AFL_TEST("game.proxy.SpecBrowserProxy:basics", a)
{
    // Environment
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
        a.check("01. wait", disp.wait(1000));
    }

    // Verify list content
    a.checkEqual("11. size", list.list.content.size(), 3U);
    a.checkEqual("12name",   list.list.content[0].name, "Two-speed");
    a.checkEqual("13id",     list.list.content[0].id, 2);
    a.checkEqual("14name",   list.list.content[1].name, "Three-speed");
    a.checkEqual("15id",     list.list.content[1].id, 3);
    a.checkEqual("16name",   list.list.content[2].name, "Four-speed");
    a.checkEqual("17id",     list.list.content[2].id, 4);
    a.checkEqual("18. page", list.page, gsi::EnginePage);

    // Select an entry
    PageReceiver page;
    testee.sig_pageChange.add(&page, &PageReceiver::onPageChange);
    testee.setId(3);
    while (page.content.title != "Three-speed") {
        a.check("21. wait", disp.wait(1000));
    }
    a.checkEqual("22. page", page.page, gsi::EnginePage);

    // Set some filters
    FilterReceiver filter;
    testee.sig_filterChange.add(&filter, &FilterReceiver::onFilterChange);
    testee.addFilter(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(3, 5)));
    testee.setNameFilter("ree");
    while (filter.existing.size() != 2U || list.list.content.size() != 1) {
        a.check("31. wait", disp.wait(1000));
    }

    // Verify filters
    a.checkEqual("41. name",  filter.existing[0].name, "Tech level");
    a.checkEqual("42. value", filter.existing[0].value, "3 to 5");
    a.checkEqual("43. name",  filter.existing[1].name, "Name");        // Name is always last
    a.checkEqual("44. value", filter.existing[1].value, "ree");

    // Verify filtered list
    a.checkEqual("51. size", list.list.content.size(), 1U);
    a.checkEqual("52name",   list.list.content[0].name, "Three-speed");
    a.checkEqual("53id",     list.list.content[0].id, 3);
}

/** Test filter modifications.
    A: add some filters.
    E: correct filter reported back */
AFL_TEST("game.proxy.SpecBrowserProxy:filter", a)
{
    // Environment
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
        a.check("01. wait", disp.wait(1000));
    }

    // Verify filter
    a.checkEqual("11. size", filter.existing.size(), 1U);
    a.checkEqual("12. name", filter.existing[0].name, "Tech level");
    a.checkEqual("13. value", filter.existing[0].value, "3 to 5");

    // Modify filter
    testee.setFilter(0, gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(4, 4)));
    while (filter.existing.size() != 1U || filter.existing[0].value != "4") {
        a.check("21. wait", disp.wait(1000));
    }

    // Remove filter
    testee.eraseFilter(0);
    while (!filter.existing.empty()) {
        a.check("31. wait", disp.wait(1000));
    }
}

/** Test sort modifications.
    A: change sort order.
    E: sort order reported back */
AFL_TEST("game.proxy.SpecBrowserProxy:sort", a)
{
    // Environment
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));
    SortReceiver sort;
    testee.sig_sortChange.add(&sort, &SortReceiver::onSortChange);
    testee.setPage(gsi::EnginePage);
    while (sort.available.empty()) {
        a.check("01. wait", disp.wait(1000));
    }

    // Verify initial sort
    a.checkEqual("11. active", sort.active, gsi::Range_Id);
    a.check("12. String_Name", sort.available.contains(gsi::String_Name));
    a.check("13. Range_Tech",  sort.available.contains(gsi::Range_Tech));

    // Sort
    testee.setSortOrder(gsi::Range_Tech);
    while (sort.active != gsi::Range_Tech) {
        a.check("21. wait", disp.wait(1000));
    }
}

/** Test setPageId.
    A: use setPageId.
    E: only one update received for that page. */
AFL_TEST("game.proxy.SpecBrowserProxy:setPageId", a)
{
    // Environment
    util::SimpleRequestDispatcher disp;
    SessionThread s;
    prepare(s);

    // Testee
    game::proxy::SpecBrowserProxy testee(s.gameSender(), disp, std::auto_ptr<gsi::PictureNamer>(new gsi::NullPictureNamer()));
    NamedPageReceiver recv(a, "Four-speed");
    testee.sig_pageChange.add(&recv, &NamedPageReceiver::onPageChange);
    testee.setPageId(gsi::EnginePage, 4);
    while (recv.getCount() == 0) {
        a.check("01. wait", disp.wait(1000));
    }
}
