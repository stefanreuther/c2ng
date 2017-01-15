/**
  *  \file client/si/widgetfunction.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETFUNCTION_HPP
#define C2NG_CLIENT_SI_WIDGETFUNCTION_HPP

#include "afl/data/value.hpp"
#include "interpreter/arguments.hpp"
#include "ui/layout/manager.hpp"
#include "game/session.hpp"
#include "interpreter/nametable.hpp"
#include "afl/base/memory.hpp"

namespace client { namespace si {

    class ScriptSide;
    class WidgetReference;

    enum WidgetFunction {
        wifNewButton,
        wifNewSpacer,
        wifNewHBox,
        wifNewVBox,
        wifNewFrame,
        wifNewInput,
        wifNewKeyboardFocus,
        wifNewCheckbox,
        wifNewRadiobutton,
        wifNewFlowBox,
        wifNewGridBox,
        wifNewLabel
    };

    /** Get name table for a dialog.
        This table is used for "With UI.Dialog(...)". */
    afl::base::Memory<const interpreter::NameTable> getDialogNameTable();

    /** Get name table for string list.
        This table is used for "With Listbox(...)". */
    afl::base::Memory<const interpreter::NameTable> getStringListDialogNameTable();

    /*
     *  Widget Factory Functions
     */

    afl::data::Value* IFWidgetNewButton(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewFrame(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewHVBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args, bool horiz);
    afl::data::Value* IFWidgetNewKeyboardFocus(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewInput(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewSpacer(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewCheckbox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewRadiobutton(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewFlowBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewGridBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);
    afl::data::Value* IFWidgetNewLabel(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);

    /** Call widget function.
        \param func     Which function to call
        \param session  Game session (provides access to World, atom table etc.)
        \param ss       ScriptSide (provides indirect access to UserSide)
        \param ref      Reference to invoking widget
        \param args     Argument list
        \return Result */
    afl::data::Value* callWidgetFunction(WidgetFunction func, game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args);

} }

#endif
