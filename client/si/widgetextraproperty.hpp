/**
  *  \file client/si/widgetextraproperty.hpp
  *  \brief Extra Widget Properties
  */
#ifndef C2NG_CLIENT_SI_WIDGETEXTRAPROPERTY_HPP
#define C2NG_CLIENT_SI_WIDGETEXTRAPROPERTY_HPP

#include "afl/data/value.hpp"
#include "ui/widget.hpp"

namespace client { namespace si {

    class WidgetReference;
    class ScriptSide;

    /** Extra properties.
        These properties are write-only and mostly apply to control-screen data display widgets. */
    enum WidgetExtraProperty {
        wxpControlScreenHeaderHeading,
        wxpControlScreenHeaderSubtitle,
        wxpControlScreenHeaderImage,
        wxpRichDocumentContent,
        wxpDataViewContent
    };

    /** Set property of widget (UI side).
        \param p Property to get
        \param value New value
        \param w Widget to access
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetExtraProperty p, const afl::data::Value* value, ui::Widget* w);

    /** Set property of widget (script side).
        \param p Property to get
        \param value New value
        \param ss ScriptSide
        \param ref WidgetReference
        \throw interpreter::Error on errors (widget or value type mismatch) */
    void setWidgetProperty(WidgetExtraProperty p, const afl::data::Value* value, ScriptSide& ss, const WidgetReference& ref);

} }

#endif
