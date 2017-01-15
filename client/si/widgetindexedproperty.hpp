/**
  *  \file client/si/widgetindexedproperty.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETINDEXEDPROPERTY_HPP
#define C2NG_CLIENT_SI_WIDGETINDEXEDPROPERTY_HPP

#include "afl/data/value.hpp"
#include "ui/widget.hpp"

namespace client { namespace si {

    class WidgetReference;
    class ScriptSide;

    /** Indexed properties.
        These properties take an additional index.
        They are write-only and mostly apply to control-screen data display widgets. */
    enum WidgetIndexedProperty {
        wipControlScreenHeaderButton,
        wipDataViewButton,
        wipCommandViewButton,
        wipCommandViewLeftText,
        wipCommandViewRightText
    };

    /** Set property of widget (UI side).
        \param p Property to get
        \param index Index
        \param value New value
        \param w Widget to access
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetIndexedProperty p, afl::data::Value* index, afl::data::Value* value, ui::Widget* w);

    /** Set property of widget (script side).
        \param p Property to get
        \param index Index
        \param value New value
        \param ss ScriptSide
        \param ref WidgetReference
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetIndexedProperty p, afl::data::Value* index, afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref);

} }


#endif
