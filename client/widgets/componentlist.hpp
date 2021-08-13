/**
  *  \file client/widgets/componentlist.hpp
  *  \brief Class client::widgets::ComponentList
  */
#ifndef C2NG_CLIENT_WIDGETS_COMPONENTLIST_HPP
#define C2NG_CLIENT_WIDGETS_COMPONENTLIST_HPP

#include <vector>
#include "game/types.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List of starship components.
        Displays a list of starship parts with appropriate coloring. */
    class ComponentList : public ui::widgets::AbstractListbox {
     public:
        /** Data for a list item (one part). */
        struct Part {
            String_t name;                      ///< Name of part.
            int id;                             ///< Id (can serve as primary key, e.g. part number).
            int numParts;                       ///< Number of parts.
            bool isAccessible;                  ///< true if part is accessible.
            game::TechStatus techStatus : 8;    ///< Tech status (determines color).

            Part(int id, String_t name, int numParts, bool isAccessible, game::TechStatus techStatus)
                : name(name), id(id), numParts(numParts), isAccessible(isAccessible), techStatus(techStatus)
                { }
        };
        typedef std::vector<Part> Parts_t;

        /** Constructor.
            \param root Root
            \param numLines Preferred widget height in lines
            \param widthInEms Preferred widget width in ems */
        ComponentList(ui::Root& root, int numLines, int widthInEms);
        ~ComponentList();

        /** Set content.
            Tries to preserve the current element as identified by its Id.
            \param parts New content */
        void setContent(const Parts_t& parts);

        /** Set current element by Id.
            If a Part with the given Id exists, moves cursor to it.
            \param id Target Id */
        void setCurrentId(int id);

        /** Get current element Id.
            \return Part::id of the Part corresponding to the current selection, 0 if none */
        int getCurrentId() const;

        /** Get current element amount.
            \return Part::numParts of the Part corresponding to the current selection, 0 if none. */
        int getCurrentAmount() const;

        // AbstractListbox virtuals:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget virtuals:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        int m_numLines;
        int m_widthInEms;

        Parts_t m_content;

        afl::base::Ref<gfx::Font> getFont() const;
    };

} }

#endif
