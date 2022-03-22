/**
  *  \file client/tiles/taskeditortile.hpp
  *  \brief Class client::tiles::TaskEditorTile
  */
#ifndef C2NG_CLIENT_TILES_TASKEDITORTILE_HPP
#define C2NG_CLIENT_TILES_TASKEDITORTILE_HPP

#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace tiles {

    /** Task editor tile.
        Displays the content of a task, accessed by a TaskEditorProxy.
        Scrolling controls the underlying TaskEditor's cursor. */
    class TaskEditorTile : public ui::Widget {
     public:
        /** Constructor.
         @param root    UI root
         @param pProxy  Proxy. If passed as null, the widget remains empty.
                        (Used when TaskEditorTile is instantiated on a non-task screen.) */
        TaskEditorTile(ui::Root& root, game::proxy::TaskEditorProxy* pProxy);
        ~TaskEditorTile();

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        void onChange(const game::proxy::TaskEditorProxy::Status& status);
        void onListSelectionChange();

        class ListWidget;

        afl::base::Deleter m_deleter;
        game::proxy::TaskEditorProxy* m_pProxy;
        ListWidget* m_listWidget;
        Widget* m_childWidget;
        afl::base::SignalConnection conn_change;
    };

} }

#endif
