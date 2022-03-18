/**
  *  \file client/si/widgetholder.hpp
  *  \brief Class client::si::WidgetHolder
  */
#ifndef C2NG_CLIENT_SI_WIDGETHOLDER_HPP
#define C2NG_CLIENT_SI_WIDGETHOLDER_HPP

#include <memory>
#include "afl/base/refcounted.hpp"
#include "ui/widget.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/weaktarget.hpp"
#include "util/atomtable.hpp"
#include "util/requestsender.hpp"
#include "afl/base/observable.hpp"

namespace client { namespace si {

    class ScriptSide;
    class UserSide;
    class Control;

    /** Container for widgets created by/for scripts.
        The script side is not allowed to manipulate user-interface-side objects.
        It can therefore never use direct widget pointers.
        Instead, we put all objects into a WidgetHolder, which provides an integer->widget mapping.
        Each script-side widget reference therefore stores a reference to a WidgetHolder and an integer.
        Application code typically uses WidgetReference to store these WidgetHolder/integer pairs.

        Manipulation of the widgets can only be done by the user-interface thread.
        WidgetHolder's methods therefore take a UserSide parameter to let you prove you're the correct thread.
        Use ScriptSide::call() to get into that thread.

        Widgets are stateful in the sense that a widget tree they can be part of the global widget tree (ui::Root) only once.
        In addition, we need a way to access a client::si::Control in order to produce callbacks into the script side.
        We thereful allow a WidgetHolder to be associated with a Control,
        and use that as an indicator whether this widget tree is active. */
    class WidgetHolder : public afl::base::RefCounted, public afl::base::WeakTarget {
     public:
        /** Constructor.
            \param userSender Access to UserSide */
        explicit WidgetHolder(util::RequestSender<UserSide> userSender);

        /** Destructor. */
        ~WidgetHolder();

        /** Add new widget.
            The WidgetHolder will become owner of this object.
            \param user User Side to prove you're the correct thread.
            \param w Newly-allocated widget.
            \return index such that get(user,index) == w */
        size_t addNewWidget(UserSide& user, ui::Widget* w);
        size_t addNewWidget(Control& ctl, ui::Widget* w);

        /** Get widget.
            \param user User Side to prove you're the correct thread.
            \param n Index
            \return widget; null if n is out-of-range or the widget was destroyed for some reason */
        ui::Widget* get(UserSide& user, size_t n) const;
        ui::Widget* get(Control& user, size_t n) const;

        /** Get deleter.
            Use the deleter if you have anything to store which is not a widget.
            \param ctl Control to prove you're the correct thread.
            \return deleter */
        afl::base::Deleter& deleter(Control& ctl);

        /** Create integer value.
            \param ctl Control to prove you're the correct thread.
            \return newly-created integer value. */
        afl::base::Observable<int>& createInteger(Control& ctl);

        /** Attach Control.
            Only one Control can be attached at a time; see class description.
            \param ctl Control to attach
            \retval true Control attached successfully
            \retval false Control not attached because another one is active */
        bool attachControl(Control& ctl);

        /** Detach Control.
            Undoes a previous successful attachControl().
            \param ctl Control to detach */
        void detachControl(Control& ctl);

        /** Get attached Control.
            \return attached control; null if none */
        Control* getControl();

        /** Make command event.
            The resulting closure can be attached to events such as Button::sig_fire.
            When fired, it will call the Control::executeCommandWait() on the attached control.
            \param cmd Script command as an atom
            \return newly-allocated closure */
        afl::base::Closure<void(int)>* makeCommand(util::Atom_t cmd);

     private:
        class Impl;

        std::auto_ptr<Impl> m_impl;
        util::RequestSender<UserSide> m_userSender;
        Control* m_pControl;
    };

} }

#endif
