/**
  *  \file client/si/widgetproperty.hpp
  *  \brief Widget Properties
  */
#ifndef C2NG_CLIENT_SI_WIDGETPROPERTY_HPP
#define C2NG_CLIENT_SI_WIDGETPROPERTY_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"
#include "ui/widget.hpp"

namespace client { namespace si {

    class WidgetReference;
    class ScriptSide;

    enum WidgetProperty {
        wipFrameColor,
        wipInputValue,
        wipEnabled,
        wipFocused,
        wipCheckboxValue,
        wipRadiobuttonValue
    };

    /** Get property of widget (UI side).
        \param p Property to get
        \param w Widget to access
        \return property value; null if mismatching widget/property */
    afl::data::Value* getWidgetProperty(WidgetProperty p, ui::Widget* w);

    /** Set property of widget (UI side).
        \param p Property to get
        \param value New value
        \param w Widget to access
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetProperty p, afl::data::Value* value, ui::Widget* w);


    /** Get property of widget (script side).
        \param p Property to set
        \param ss ScriptSide
        \param ref WidgetReference
        \return property value; null if mismatching widget/property */
    afl::data::Value* getWidgetProperty(WidgetProperty p, ScriptSide& ss, const WidgetReference& ref);

    /** Set property of widget (script side).
        \param p Property to get
        \param value New value
        \param ss ScriptSide
        \param ref WidgetReference
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetProperty p, afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref);

} }

#endif
