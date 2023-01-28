/**
  *  \file client/si/widgetwrapper.hpp
  *  \brief Class client::si::WidgetWrapper
  */
#ifndef C2NG_CLIENT_SI_WIDGETWRAPPER_HPP
#define C2NG_CLIENT_SI_WIDGETWRAPPER_HPP

#include <memory>
#include "game/proxy/objectobserver.hpp"
#include "ui/widget.hpp"
#include "afl/base/ref.hpp"
#include "interpreter/nametable.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace si {

    class UserSide;
    class WidgetHolder;

    /** Wrapper to make a widget scriptable.
        This wraps a widget that has been created in the C++ side but should be accessible in the script side.
        The main use-case is information tiles that are updated by scripts.

        To make this possible, the widget must be stored in a WidgetHolder and therefore must be dynamically allocated.
        The WidgetWrapper provides a single WidgetHolder for each widget.
        All WidgetWrapper widgets will therefore be not related as far as scripts are concerned;
        related widgets can only be created if the widget's properties include methods to create new widgets. */
    class WidgetWrapper : public ui::Widget {
     public:
        /** Constructor.
            \param user User side
            \param theWidget Newly-allocated widget
            \param properties Definition of properties for this widget; see GenericWidgetValue. */
        WidgetWrapper(client::si::UserSide& user,
                      std::auto_ptr<ui::Widget> theWidget,
                      afl::base::Memory<const interpreter::NameTable> properties);

        /** Destructor. */
        ~WidgetWrapper();

        // Widget
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Attach to ObjectObserver.
            Whenever the underlying object reports a change, the given script command will be executed.
            \param oop ObjectObserver
            \param command Script command */
        void attach(game::proxy::ObjectObserver& oop, String_t command);

     private:
        const afl::base::Ref<client::si::WidgetHolder> m_holder;
        const size_t m_slot;
        util::RequestSender<game::Session> m_gameSender;
        const afl::base::Memory<const interpreter::NameTable> m_properties;
    };

} }

#endif
