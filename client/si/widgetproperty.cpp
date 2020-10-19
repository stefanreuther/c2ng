/**
  *  \file client/si/widgetproperty.cpp
  *  \brief Widget Properties
  */

#include "client/si/widgetproperty.hpp"
#include "client/si/compoundwidget.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usercall.hpp"
#include "client/si/values.hpp"
#include "client/si/widgetreference.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/radiobutton.hpp"

/*
 *  Direct Widget Access
 *
 *  These run directly in the UI thread.
 *  Normally, we'd avoid doing script things in the UI thread.
 *  In this case, we're only transferring values at a time when the script thread is waiting/blocked, so this is ok.
 *  Should we start transferring larger values and/or keeping them on the UI side, this must be reconsidered.
 */

// Get property of widget (UI side).
afl::data::Value*
client::si::getWidgetProperty(WidgetProperty p, ui::Widget* w)
{
    switch (p) {
     case wipFrameColor:
        if (ui::widgets::FrameGroup* g = dynamic_cast<ui::widgets::FrameGroup*>(w)) {
            return interpreter::makeStringValue(formatFrameType(g->getType()));
        } else {
            return 0;
        }

     case wipInputValue:
        if (ui::widgets::InputLine* il = dynamic_cast<ui::widgets::InputLine*>(w)) {
            return interpreter::makeStringValue(il->getText());
        } else {
            return 0;
        }

     case wipEnabled:
        if (w != 0) {
            return interpreter::makeBooleanValue(!w->hasState(ui::Widget::DisabledState));
        } else {
            return 0;
        }

     case wipFocused:
        if (w != 0) {
            return interpreter::makeBooleanValue(w->hasState(ui::Widget::FocusedState));
        } else {
            return 0;
        }

     case wipCheckboxValue:
        if (ui::widgets::Checkbox* b = dynamic_cast<ui::widgets::Checkbox*>(w)) {
            return interpreter::makeIntegerValue(b->value().get());
        } else {
            return 0;
        }

     case wipRadiobuttonValue:
        if (ui::widgets::RadioButton* b = dynamic_cast<ui::widgets::RadioButton*>(w)) {
            return interpreter::makeIntegerValue(b->value().get());
        } else {
            return 0;
        }

     case wipNumberInputValue:
        if (CompoundWidget<ui::widgets::DecimalSelector>* b = dynamic_cast<CompoundWidget<ui::widgets::DecimalSelector>*>(w)) {
            return interpreter::makeIntegerValue(b->widget().value().get());
        } else {
            return 0;
        }
    }
    return 0;
}

// Set property of widget (UI side).
void
client::si::setWidgetProperty(WidgetProperty p, afl::data::Value* value, ui::Widget* w)
{
    switch (p) {
     case wipFrameColor:
        if (ui::widgets::FrameGroup* g = dynamic_cast<ui::widgets::FrameGroup*>(w)) {
            String_t stringValue;
            if (interpreter::checkStringArg(stringValue, value)) {
                ui::FrameType type;
                if (!parseFrameType(type, stringValue)) {
                    throw interpreter::Error::rangeError();
                }
                g->setType(type);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipInputValue:
        if (ui::widgets::InputLine* il = dynamic_cast<ui::widgets::InputLine*>(w)) {
            String_t stringValue;
            if (interpreter::checkStringArg(stringValue, value)) {
                il->setText(stringValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipEnabled:
        if (w != 0) {
            bool boolValue;
            if (interpreter::checkBooleanArg(boolValue, value)) {
                w->setState(ui::Widget::DisabledState, !boolValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipFocused:
        throw interpreter::Error::notAssignable();

     case wipCheckboxValue:
        if (ui::widgets::Checkbox* b = dynamic_cast<ui::widgets::Checkbox*>(w)) {
            int32_t intValue;
            if (interpreter::checkIntegerArg(intValue, value)) {
                b->value().set(intValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipRadiobuttonValue:
        if (ui::widgets::RadioButton* b = dynamic_cast<ui::widgets::RadioButton*>(w)) {
            int32_t intValue;
            if (interpreter::checkIntegerArg(intValue, value)) {
                b->value().set(intValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipNumberInputValue:
        if (CompoundWidget<ui::widgets::DecimalSelector>* b = dynamic_cast<CompoundWidget<ui::widgets::DecimalSelector>*>(w)) {
            int32_t intValue;
            if (interpreter::checkIntegerArg(intValue, value)) {
                b->widget().value().set(intValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
    }
}


/*
 *  Widget property access from script side
 */

// Get property of widget (script side).
afl::data::Value*
client::si::getWidgetProperty(WidgetProperty p, ScriptSide& ss, const WidgetReference& ref)
{
    // UserCall for the thread transition
    class Getter : public UserCall {
     public:
        Getter(WidgetProperty p, const WidgetReference& ref, std::auto_ptr<afl::data::Value>& result)
            : m_property(p), m_ref(ref), m_result(result)
            { }
        virtual void handle(UserSide& ui, Control& /*ctl*/)
            { m_result.reset(getWidgetProperty(m_property, m_ref.get(ui))); }
     private:
        const WidgetProperty m_property;
        const WidgetReference m_ref;
        std::auto_ptr<afl::data::Value>& m_result;
    };

    // Do it
    std::auto_ptr<afl::data::Value> result;
    Getter g(p, ref, result);
    ss.call(g);
    return result.release();
}

// Set property of widget (script side).
void
client::si::setWidgetProperty(WidgetProperty p, afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref)
{
    // UserCall for the thread transition.
    class Setter : public UserCall {
     public:
        Setter(WidgetProperty p, const WidgetReference& ref, afl::data::Value* value)
            : m_property(p), m_ref(ref), m_value(value)
            { }
        virtual void handle(UserSide& ui, Control& /*ctl*/)
            { setWidgetProperty(m_property, m_value, m_ref.get(ui)); }
     private:
        const WidgetProperty m_property;
        const WidgetReference m_ref;
        afl::data::Value* m_value;
    };

    // Call it. call() will proxy possible exceptions.
    Setter g(p, ref, value);
    ss.call(g);
}
