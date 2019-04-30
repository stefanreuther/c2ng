/**
  *  \file client/widgets/scanresult.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SCANRESULT_HPP
#define C2NG_CLIENT_WIDGETS_SCANRESULT_HPP

#include "ui/widget.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "util/requestreceiver.hpp"
#include "game/session.hpp"
#include "ui/widgets/simpletable.hpp"

namespace client { namespace widgets {

    /** Scanner result widget.

        @change In PCC2, WScanResult was a listener to a chart widget, controlled rendering, and some buttons.
        In c2ng, ScanResult is a compound widget with its own proxy.
        It has no own rendering, hence there is no need for a proxy-less version. */
    class ScanResult : public ui::Widget {
     public:
        ScanResult(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);
        ~ScanResult();

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

        void addButton(Widget& w);

        void setPositions(game::map::Point origin, game::map::Point target);
        void clearPositions();

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<ScanResult> m_reply;

        ui::widgets::SimpleTable m_table;

        bool m_valid;
        game::map::Point m_origin;
        game::map::Point m_target;

        void doLayout();
        void setScanResult(game::map::Point origin, game::map::Point target, String_t resultText, String_t distanceText);
    };

} }

#endif
