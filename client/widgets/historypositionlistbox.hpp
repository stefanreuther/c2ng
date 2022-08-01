/**
  *  \file client/widgets/historypositionlistbox.hpp
  *  \brief Class client::widgets::HistoryPositionListbox
  */
#ifndef C2NG_CLIENT_WIDGETS_HISTORYPOSITIONLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_HISTORYPOSITIONLISTBOX_HPP

#include "afl/string/translator.hpp"
#include "game/map/shipinfo.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** Display position list for a history ship.
        Displays a game::map::ShipLocationInfos_t,
        in reverse order (current turn at bottom), aligned to the bottom of the widget. */
    class HistoryPositionListbox : public ui::widgets::AbstractListbox {
     public:
        typedef game::map::ShipLocationInfos_t Infos_t;
        typedef game::map::ShipLocationInfo Info_t;

        /** Constructor.
            @param root User-interface root
            @param tx Translator */
        HistoryPositionListbox(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~HistoryPositionListbox();

        /** Set number of lines.
            This is used to determine the preferred layout size and needs not correlate with the actual content.
            @param n Number of lines */
        void setNumLines(int n);

        /** Set width.
            This is used to determine the preferred layout size and needs not correlate with the actual content.
            @param width Width in pixels */
        void setWidth(int width);

        /** Set content.
            @param content New content */
        void setContent(const Infos_t& content);

        /** Set current turn number.
            Scroll to approriate position.
            @param turnNumber Turn number */
        void setCurrentTurnNumber(int turnNumber);

        /** Get current turn number.
            If cursor is on an item, returns its turn. Otherwise, return 0.
            @return turn number */
        int getCurrentTurnNumber() const;

        // AbstractListbox / Widget:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Infos_t m_content;

        int m_numLines;
        int m_width;

        const Info_t* getItem(size_t index) const;
    };

} }

#endif
