/**
  *  \file client/tiles/taskeditortile.hpp
  */
#ifndef C2NG_CLIENT_TILES_TASKEDITORTILE_HPP
#define C2NG_CLIENT_TILES_TASKEDITORTILE_HPP

#include "afl/base/deleter.hpp"
#include "client/si/userside.hpp"
#include "game/proxy/objectobserver.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace tiles {

    class TaskEditorTile : public ui::Widget {
     public:
        TaskEditorTile(ui::Root& root,
                       client::si::UserSide& userSide,
                       interpreter::Process::ProcessKind kind);
        ~TaskEditorTile();

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

        void setId(game::Id_t id);
        void attach(game::proxy::ObjectObserver& oop);
        
     private:
        void onChange(const game::proxy::TaskEditorProxy::Status& status);
        void onListSelectionChange();

        class ListWidget;

        afl::base::Deleter m_deleter;
        game::proxy::TaskEditorProxy m_proxy;
        util::RequestReceiver<TaskEditorTile> m_receiver;
        interpreter::Process::ProcessKind m_kind;
        ListWidget* m_listWidget;
        Widget* m_childWidget;
    };

} }

#endif
