/**
  *  \file client/si/widgetextraproperty.cpp
  *  \brief Widget Extra Properties
  */

#include "client/si/widgetextraproperty.hpp"
#include "client/si/scriptside.hpp"
#include "client/widgets/controlscreenheader.hpp"
#include "client/si/usercall.hpp"
#include "client/si/widgetreference.hpp"
#include "ui/rich/document.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/richtextvalue.hpp"
#include "ui/rich/documentview.hpp"
#include "client/widgets/standarddataview.hpp"

// Set property of widget (UI side).
void
client::si::setWidgetProperty(WidgetExtraProperty p, const afl::data::Value* value, ui::Widget* w)
{
    using client::widgets::ControlScreenHeader;
    switch (p) {
     case wxpControlScreenHeaderHeading:
        if (ControlScreenHeader* p = dynamic_cast<ControlScreenHeader*>(w)) {
            String_t stringValue;
            if (interpreter::checkStringArg(stringValue, value)) {
                p->setText(ControlScreenHeader::txtHeading, stringValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wxpControlScreenHeaderSubtitle:
        if (ControlScreenHeader* p = dynamic_cast<ControlScreenHeader*>(w)) {
            String_t stringValue;
            if (interpreter::checkStringArg(stringValue, value)) {
                p->setText(ControlScreenHeader::txtSubtitle, stringValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wxpControlScreenHeaderImage:
        if (ControlScreenHeader* p = dynamic_cast<ControlScreenHeader*>(w)) {
            String_t stringValue;
            if (interpreter::checkStringArg(stringValue, value)) {
                p->setImage(stringValue);
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wxpRichDocumentContent:
        if (ui::rich::DocumentView* p = dynamic_cast<ui::rich::DocumentView*>(w)) {
            game::interface::RichTextValue::Ptr_t richValue;
            if (game::interface::checkRichArg(richValue, value)) {
                if (richValue.get() != 0) {
                    p->getDocument().clear();
                    p->getDocument().add(*richValue);
                    p->getDocument().finish();
                    p->handleDocumentUpdate();
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wxpDataViewContent:
        if (client::widgets::StandardDataView* p = dynamic_cast<client::widgets::StandardDataView*>(w)) {
            game::interface::RichTextValue::Ptr_t richValue;
            if (game::interface::checkRichArg(richValue, value)) {
                if (richValue.get() != 0) {
                    p->setText(*richValue);
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;
    }
}

// Set property of widget (script side).
void
client::si::setWidgetProperty(WidgetExtraProperty p, const afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref)
{
    // UserCall for the thread transition.
    class Setter : public UserCall {
     public:
        Setter(WidgetExtraProperty p, const WidgetReference& ref, const afl::data::Value* value)
            : m_property(p), m_ref(ref), m_value(value)
            { }
        virtual void handle(Control& ctl)
            { setWidgetProperty(m_property, m_value, m_ref.get(ctl)); }
     private:
        const WidgetExtraProperty m_property;
        const WidgetReference m_ref;
        const afl::data::Value*const m_value;
    };

    // Call it. call() will proxy possible exceptions.
    Setter g(p, ref, value);
    ss.call(g);
}
