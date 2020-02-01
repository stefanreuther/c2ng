/**
  *  \file client/si/widgetcommand.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETCOMMAND_HPP
#define C2NG_CLIENT_SI_WIDGETCOMMAND_HPP

#include "afl/base/types.hpp"
#include "interpreter/process.hpp"
#include "interpreter/arguments.hpp"
#include "game/session.hpp"

namespace client { namespace si {

    class ScriptSide;
    class WidgetReference;

    enum WidgetCommand {
        wicRun,
        wicEnable,
        wicDisable,
        wicFrameSetColor,
        wicFocus,
        wicInputSetValue,
        wicKeyboardFocusAdd,

        wicControlScreenHeaderSetHeading,
        wicControlScreenHeaderSetSubtitle,
        wicControlScreenHeaderSetImage,
        wicControlScreenHeaderSetButton,

        wicRichDocumentSetContent,

        wicListboxAddItem,
        wicListboxDialogRun,
        wicListboxDialogRunMenu,

        wicCheckboxSetValue,
        wicRadiobuttonSetValue,

        wicDataViewSetContent,
        wicDataViewSetButton,

        wicCommandViewSetButton,
        wicCommandViewSetLeftText,
        wicCommandViewSetRightText,

        wicNumberInputSetValue
    };

    void IFWidgetRun(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args);
    void IFWidgetFocus(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    void IFKeyboardFocusAdd(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    void IFListboxAddItem(ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    void IFListboxDialogRun(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args);
    void IFListboxDialogRunMenu(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args);

    void callWidgetCommand(WidgetCommand cmd, game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Process& proc, interpreter::Arguments& args);

} }

#endif
