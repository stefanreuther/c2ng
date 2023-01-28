/**
  *  \file ui/layoutablegroup.hpp
  *  \brief Class ui::LayoutableGroup
  */
#ifndef C2NG_UI_LAYOUTABLEGROUP_HPP
#define C2NG_UI_LAYOUTABLEGROUP_HPP

#include "ui/layout/manager.hpp"
#include "ui/widget.hpp"

namespace ui {

    /** Basic layoutable container

        This widget provides the basis for a layoutable container.
        It has a ui::layout::Manager responsible for actual layout,
        and implements add() and pack() methods using it.

        The actual child widget layout area is provided by a descendant's transformSize()
        method to allow for additional frames or other decoration by the descendant.

        Descendant should call defaultDrawChildren() from its draw().

        Descendant must implement event delivery, possibly by calling defaultHandleKey(), defaultHandleMouse(). */
    class LayoutableGroup : public Widget {
     public:
        /** Constructor.
            \param mgr Layout manager. Needs to live at least as long as the widget. */
        explicit LayoutableGroup(ui::layout::Manager& mgr) throw();

        // virtual void draw(gfx::Canvas& can);
        // virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        /** Type of transformation in transformSize. */
        enum Transformation {
            OuterToInner,         ///< Given size of container, determine available room for content.
            InnerToOuter          ///< Given size of content, determine required container size.
        };

        /** Transform widget position/size.
            \param size  Size
            \param kind  Direction of transformation */
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const = 0;

        /** Add widget as new last widget.
            This does NOT yet layout the child, use pack() or doLayout() for that.
            \param child Widget */
        void add(Widget& child);

        /** Compute and apply optimum layout.
            Sets the widget to its preferred size and positions all content;
            does not change the top/left position. */
        void pack();

        /** Perform layout on content without changing widget size.
            Use after you set the size manually, or when content has changed. */
        void doLayout();

     private:
        ui::layout::Manager& m_manager;

        gfx::Point transformPoint(gfx::Point pt, Transformation kind) const;
    };

}

#endif
