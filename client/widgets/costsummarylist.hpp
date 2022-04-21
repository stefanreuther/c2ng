/**
  *  \file client/widgets/costsummarylist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COSTSUMMARYLIST_HPP
#define C2NG_CLIENT_WIDGETS_COSTSUMMARYLIST_HPP

#include "afl/base/closure.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/spec/costsummary.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/requestsender.hpp"

namespace client { namespace widgets {

    /** Display a CostSummary object.
        Shows
        - a header
        - a list of CostSummary items
        - an optional one- or two-line footer

        The list can be a proper scrollable list, or a static table. */
    class CostSummaryList : public ui::widgets::AbstractListbox {
     public:
        /** Footer style */
        enum FooterStyle {
            NoFooter,               ///< Do not show a footer.
            TotalsFooter,           ///< Show a single "totals" footer.
            ComparisonFooter        ///< Show "totals" footer and "amount remaining" after consumption.
        };

        /** Constructor.
            \param numLines Number of lines for this widget for layout purposes
            \param isList true to exhibit list behaviour, false to exhibit static behaviour
            \param footerStyle Footer style
            \param tx Translator */
        CostSummaryList(int numLines, bool isList, FooterStyle footerStyle, ui::Root& root, afl::string::Translator& tx);
        ~CostSummaryList();

        /** Set content.
            \param content Cost summary to display */
        void setContent(const game::spec::CostSummary& content);

        /** Set available amount for ComparisonFooter.
            \param available Available amount */
        void setAvailableAmount(game::spec::Cost available);

        /** Perform export.
            \param gameSender Game Sender */
        void doExport(util::RequestSender<game::Session> gameSender);

        /** Convenience method to make a closure that calls doExport().
            \param gameSender Game Sender
            \return newly-allocated closure */
        afl::base::Closure<void(int)>* makeExporter(util::RequestSender<game::Session> gameSender);

        // AbstractListbox:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        int m_numLines;
        FooterStyle m_footerStyle;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        game::spec::CostSummary m_content;
        game::spec::Cost m_available;

        int getLineHeight() const;
    };

} }

#endif
