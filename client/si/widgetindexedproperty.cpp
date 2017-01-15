/**
  *  \file client/si/widgetindexedproperty.cpp
  */

#include "client/si/widgetindexedproperty.hpp"
#include "client/si/usercall.hpp"
#include "client/si/widgetreference.hpp"
#include "client/widgets/controlscreenheader.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"
#include "ui/widgets/framegroup.hpp"
#include "client/si/values.hpp"
#include "client/si/scriptside.hpp"
#include "client/widgets/standarddataview.hpp"
#include "client/widgets/commanddataview.hpp"
#include "game/interface/richtextvalue.hpp"
#include "game/interface/richtextfunctions.hpp"

using afl::string::strCaseCompare;
using client::widgets::ControlScreenHeader;
using client::widgets::StandardDataView;
using client::widgets::CommandDataView;
using interpreter::checkStringArg;
using ui::widgets::FrameGroup;

namespace {
    bool parseButton(ControlScreenHeader::Button& result, const String_t& value)
    {
        if (strCaseCompare(value, "i") == 0) {
            result = ControlScreenHeader::btnX;
            return true;
        } else if (strCaseCompare(value, "auto") == 0) {
            result = ControlScreenHeader::btnAuto;
            return true;
        } else if (strCaseCompare(value, "cscr") == 0) {
            result = ControlScreenHeader::btnCScr;
            return true;
        } else if (strCaseCompare(value, "x") == 0) {
            result = ControlScreenHeader::btnX;
            return true;
        } else if (strCaseCompare(value, "add") == 0) {
            result = ControlScreenHeader::btnAdd;
            return true;
        } else if (strCaseCompare(value, "tab") == 0) {
            result = ControlScreenHeader::btnTab;
            return true;
        } else if (strCaseCompare(value, "j") == 0) {
            result = ControlScreenHeader::btnJoin;
            return true;
        } else if (strCaseCompare(value, "n") == 0) {
            result = ControlScreenHeader::btnName;
            return true;
        } else if (strCaseCompare(value, "image") == 0) {
            result = ControlScreenHeader::btnImage;
            return true;
        } else {
            return false;
        }
    }

    void setCommandViewText(bool left, afl::data::Value* index, afl::data::Value* value, ui::Widget* w)
    {
        if (CommandDataView* dv = dynamic_cast<CommandDataView*>(w)) {
            String_t indexString;
            game::interface::RichTextValue::Ptr_t richValue;
            if (checkStringArg(indexString, index) && game::interface::checkRichArg(richValue, value)) {
                util::Key_t key;
                bool ok = util::parseKey(indexString, key)
                    && richValue.get() != 0
                    && dv->setText(key, left, *richValue);
                if (!ok) {
                    throw interpreter::Error::rangeError();
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
    }
}


// Set property of widget (UI side).
void
client::si::setWidgetProperty(WidgetIndexedProperty p, afl::data::Value* index, afl::data::Value* value, ui::Widget* w)
{
    switch (p) {
     case wipControlScreenHeaderButton:
        if (ControlScreenHeader* csh = dynamic_cast<ControlScreenHeader*>(w)) {
            String_t indexString;
            String_t valueString;
            if (checkStringArg(indexString, index) && checkStringArg(valueString, value)) {
                // Check button
                ControlScreenHeader::Button btn;
                if (!parseButton(btn, indexString)) {
                    throw interpreter::Error::rangeError();
                }

                // Check value
                FrameGroup::Type type;
                if (valueString == "" || strCaseCompare(valueString, "hidden") == 0) {
                    csh->disableButton(btn);
                } else if (parseFrameType(type, valueString)) {
                    csh->enableButton(btn, type);
                } else {
                    throw interpreter::Error::rangeError();
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipDataViewButton:
        if (StandardDataView* dv = dynamic_cast<StandardDataView*>(w)) {
            String_t indexString;
            String_t valueString;
            if (checkStringArg(indexString, index) && checkStringArg(valueString, value)) {
                util::Key_t key;
                if (!util::parseKey(indexString, key)) {
                    throw interpreter::Error::rangeError();
                }

                FrameGroup::Type type;
                bool ok;
                if (valueString == "" || strCaseCompare(valueString, "hidden") == 0) {
                    ok = dv->disableButton(key);
                } else if (parseFrameType(type, valueString)) {
                    ok = dv->enableButton(key, type);
                } else {
                    ok = false;
                }

                if (!ok) {
                    throw interpreter::Error::rangeError();
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipCommandViewButton:
        if (CommandDataView* dv = dynamic_cast<CommandDataView*>(w)) {
            String_t indexString;
            String_t valueString;
            if (checkStringArg(indexString, index) && checkStringArg(valueString, value)) {
                util::Key_t key;
                FrameGroup::Type type;

                bool ok = util::parseKey(indexString, key)
                    && parseFrameType(type, valueString)
                    && dv->setFrame(key, type);

                if (!ok) {
                    throw interpreter::Error::rangeError();
                }
            }
        } else {
            throw interpreter::Error::notAssignable();
        }
        break;

     case wipCommandViewLeftText:
        setCommandViewText(true, index, value, w);
        break;

     case wipCommandViewRightText:
        setCommandViewText(false, index, value, w);
        break;
    }
}


// Set property of widget (script side).
void
client::si::setWidgetProperty(WidgetIndexedProperty p, afl::data::Value* index, afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref)
{
    // UserCall for the thread transition.
    class Setter : public UserCall {
     public:
        Setter(WidgetIndexedProperty p, const WidgetReference& ref, afl::data::Value* index, afl::data::Value* value)
            : m_property(p), m_ref(ref), m_index(index), m_value(value)
            { }
        virtual void handle(UserSide& ui, Control& /*ctl*/)
            { setWidgetProperty(m_property, m_index, m_value, m_ref.get(ui)); }
     private:
        const WidgetIndexedProperty m_property;
        const WidgetReference m_ref;
        afl::data::Value*const m_index;
        afl::data::Value*const m_value;
    };

    // Call it. call() will proxy possible exceptions.
    Setter g(p, ref, index, value);
    ss.call(g);
}
