/**
  *  \file game/map/info/browser.hpp
  *  \brief Class game::map::info::Browser
  */
#ifndef C2NG_GAME_MAP_INFO_BROWSER_HPP
#define C2NG_GAME_MAP_INFO_BROWSER_HPP

#include "game/map/info/types.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "util/stringlist.hpp"

namespace game { namespace map { namespace info {

    class LinkBuilder;

    /** Information browser (Imperial Statistics).
        Ties together the functions from game/map/info/info.hpp into a uniform interface.

        Each page has an optional associated option value.
        Option values are are stored in the Browser object for all pages.

        TODO: persist the option values somehow */
    class Browser {
     public:
        /** Constructor.
            @param session Session // FIXME: can we use something smaller than a Session?
            @param link LinkBuilder */
        Browser(Session& session, const LinkBuilder& link);

        /** Set options for a page.

            @param page Page
            @param opts Options value */
        void setPageOptions(Page page, PageOptions_t opts);

        /** Get options for a page.

            @param page Page
            @return Options value */
        PageOptions_t getPageOptions(Page page) const;

        /** Render a page.

            @param [in]  page Page
            @param [out] out  Output */
        void renderPage(Page page, Nodes_t& out);

        /** Render page options.
            Produces a list of value/label pairs where each value corresponds to a set of options.
            This does not produce a list of all option combinations, but just the combinations
            that can be reached from the current value, using zero or one change.

            @param [in]  page Page
            @param [out] out  Output */
        void renderPageOptions(Page page, util::StringList& out);

     private:
        Session& m_session;
        const LinkBuilder& m_link;
        PageOptions_t m_options[NUM_PAGES];

        void renderTotalsPage(Nodes_t& out);
        void renderMineralsPage(Nodes_t& out, PageOptions_t opts);
        void renderPlanetsPage(Nodes_t& out, PageOptions_t opts);
        void renderColonyPage(Nodes_t& out, PageOptions_t opts);
        void renderStarbasePage(Nodes_t& out, PageOptions_t opts);
        void renderStarshipPage(Nodes_t& out, PageOptions_t opts, bool withFreighters);
        void renderStarchartPage(Nodes_t& out);
        void renderWeaponsPage(Nodes_t& out, PageOptions_t opts);

        void addSortOrders(util::StringList& out, uint8_t hi);

        Universe& universe();
    };

} } }

#endif
