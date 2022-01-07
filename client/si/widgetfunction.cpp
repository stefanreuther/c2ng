/**
  *  \file client/si/widgetfunction.cpp
  *  \brief Implementation of Widget Functions
  *
  *  Implementing a new widget:
  *
  *  (a) Implement a factory function.
  *  - add it to enum WidgetFunction
  *  - add an IFWidgetNewXXX function
  *  - add it to callWidgetFunction
  *  - implement it using the Factory class as a base. That class implements the common pattern.
  *
  *  (b) Add it to the relevant XXX_MAP tables.
  *  Right now, a widget creatio function must be listed in the map tables of all container widgets
  *  (DIALOG_MAP, FRAMEGROUP_MAP, etc.)
  *
  *  (c) If the widget has custom properties and commands, implement these in widgetproperty.cpp, widgetcommand.cpp.
  *  Add its map table here.
  *  Typically, if a widget has a "FOO" property, it will also have a "SETFOO" command implemented as setWidgetProperty().
  *
  *  Widgets that operate as tiles do not need a factory function here.
  *  Their map tables are in tilefactory.cpp.
  */

#include "client/si/widgetfunction.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/staticassert.hpp"
#include "client/si/compoundwidget.hpp"
#include "client/si/control.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/values.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetholder.hpp"
#include "client/si/widgetproperty.hpp"
#include "client/si/widgetreference.hpp"
#include "interpreter/error.hpp"
#include "interpreter/typehint.hpp"
#include "ui/group.hpp"
#include "ui/layout/flow.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/key.hpp"

using interpreter::checkStringArg;
using interpreter::checkIntegerArg;
using interpreter::checkBooleanArg;
using interpreter::checkFlagArg;

namespace {
    /*
     *  Map Tables
     */

    static const interpreter::NameTable BUTTON_MAP[] = {
        { "DISABLE",               client::si::wicDisable,          client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLE",                client::si::wicEnable,           client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLED",               client::si::wipEnabled,          client::si::WidgetPropertyDomain,      interpreter::thBool },
    };

    static const interpreter::NameTable INPUT_MAP[] = {
        { "DISABLE",               client::si::wicDisable,          client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLE",                client::si::wicEnable,           client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLED",               client::si::wipEnabled,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "FOCUS",                 client::si::wicFocus,            client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "FOCUSED",               client::si::wipFocused,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "SETVALUE",              client::si::wicInputSetValue,    client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "VALUE",                 client::si::wipInputValue,       client::si::WidgetPropertyDomain,      interpreter::thString },
    };

    static const interpreter::NameTable KEYBOARDFOCUS_MAP[] = {
        { "ADD",                   client::si::wicKeyboardFocusAdd, client::si::WidgetCommandDomain,       interpreter::thProcedure },
    };

    static const interpreter::NameTable CHECKBOX_MAP[] = {
        { "DISABLE",               client::si::wicDisable,          client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLE",                client::si::wicEnable,           client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLED",               client::si::wipEnabled,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "FOCUS",                 client::si::wicFocus,            client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "FOCUSED",               client::si::wipFocused,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "SETVALUE",              client::si::wicCheckboxSetValue, client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "VALUE",                 client::si::wipCheckboxValue,    client::si::WidgetPropertyDomain,      interpreter::thInt },
    };

    static const interpreter::NameTable RADIOBUTTON_MAP[] = {
        { "DISABLE",               client::si::wicDisable,          client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLE",                client::si::wicEnable,           client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "ENABLED",               client::si::wipEnabled,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "FOCUS",                 client::si::wicFocus,            client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "FOCUSED",               client::si::wipFocused,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "SETVALUE",              client::si::wicRadiobuttonSetValue, client::si::WidgetCommandDomain,    interpreter::thProcedure },
        { "VALUE",                 client::si::wipRadiobuttonValue, client::si::WidgetPropertyDomain,      interpreter::thInt },
    };

    static const interpreter::NameTable NUMBERINPUT_MAP[] = {
        { "FOCUS",                 client::si::wicFocus,            client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "FOCUSED",               client::si::wipFocused,          client::si::WidgetPropertyDomain,      interpreter::thBool },
        { "SETVALUE",              client::si::wicNumberInputSetValue, client::si::WidgetCommandDomain,    interpreter::thProcedure },
        { "VALUE",                 client::si::wipNumberInputValue, client::si::WidgetPropertyDomain,      interpreter::thInt },
    };

    static const interpreter::NameTable GROUP_MAP[] = {
        { "NEWBUTTON",             client::si::wifNewButton,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWCHECKBOX",           client::si::wifNewCheckbox,      client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFLOWBOX",            client::si::wifNewFlowBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFRAME",              client::si::wifNewFrame,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWGRIDBOX",            client::si::wifNewGridBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWHBOX",               client::si::wifNewHBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWINPUT",              client::si::wifNewInput,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWKEYBOARDFOCUS",      client::si::wifNewKeyboardFocus, client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWLABEL",              client::si::wifNewLabel,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWNUMBERINPUT",        client::si::wifNewNumberInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWPSEUDOINPUT",        client::si::wifNewPseudoInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWRADIOBUTTON",        client::si::wifNewRadiobutton,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWSPACER",             client::si::wifNewSpacer,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWVBOX",               client::si::wifNewVBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
    };

    static const interpreter::NameTable FRAMEGROUP_MAP[] = {
        { "COLOR",                 client::si::wipFrameColor,       client::si::WidgetPropertyDomain,      interpreter::thString },
        { "NEWBUTTON",             client::si::wifNewButton,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWCHECKBOX",           client::si::wifNewCheckbox,      client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFLOWBOX",            client::si::wifNewFlowBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFRAME",              client::si::wifNewFrame,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWGRIDBOX",            client::si::wifNewGridBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWHBOX",               client::si::wifNewHBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWINPUT",              client::si::wifNewInput,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWKEYBOARDFOCUS",      client::si::wifNewKeyboardFocus, client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWLABEL",              client::si::wifNewLabel,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWNUMBERINPUT",        client::si::wifNewNumberInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWPSEUDOINPUT",        client::si::wifNewPseudoInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWRADIOBUTTON",        client::si::wifNewRadiobutton,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWSPACER",             client::si::wifNewSpacer,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWVBOX",               client::si::wifNewVBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "SETCOLOR",              client::si::wicFrameSetColor,    client::si::WidgetCommandDomain,       interpreter::thProcedure },
    };

    static const interpreter::NameTable DIALOG_MAP[] = {
        { "NEWBUTTON",             client::si::wifNewButton,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWCHECKBOX",           client::si::wifNewCheckbox,      client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFLOWBOX",            client::si::wifNewFlowBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWFRAME",              client::si::wifNewFrame,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWGRIDBOX",            client::si::wifNewGridBox,       client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWHBOX",               client::si::wifNewHBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWINPUT",              client::si::wifNewInput,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWKEYBOARDFOCUS",      client::si::wifNewKeyboardFocus, client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWLABEL",              client::si::wifNewLabel,         client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWNUMBERINPUT",        client::si::wifNewNumberInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWPSEUDOINPUT",        client::si::wifNewPseudoInput,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWRADIOBUTTON",        client::si::wifNewRadiobutton,   client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWSPACER",             client::si::wifNewSpacer,        client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "NEWVBOX",               client::si::wifNewVBox,          client::si::WidgetFunctionDomain,      interpreter::thFunction },
        { "RUN",                   client::si::wicRun,              client::si::WidgetCommandDomain,       interpreter::thProcedure },
    };

    static const interpreter::NameTable STRINGLIST_DIALOG_MAP[] = {
        { "ADDITEM",               client::si::wicListboxAddItem,   client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "RUN",                   client::si::wicListboxDialogRun, client::si::WidgetCommandDomain,       interpreter::thProcedure },
        { "RUNMENU",               client::si::wicListboxDialogRunMenu, client::si::WidgetCommandDomain,   interpreter::thProcedure },
    };


    bool checkOptionalKeyArg(util::Key_t& key, afl::data::Value* p)
    {
        String_t s;
        if (!checkStringArg(s, p)) {
            return false;
        }

        if (s.empty()) {
            key = 0;
            return true;
        } else {
            if (!util::parseKey(s, key)) {
                throw interpreter::Error("Invalid key name");
            }
            return true;
        }
    }

    int convertWidth(int width, int flags, ui::Root& root, const gfx::FontRequest& font)
    {
        int result;
        if (width == 0) {
            result = root.getExtent().getWidth() / 2;
        } else {
            result = width;
        }
        if (width == 0 || (flags & 32) == 0) {
            if (int em = root.provider().getFont(font)->getEmWidth()) {
                result /= em;
            }
        }
        return result;
    }

    void applyFlags(ui::widgets::InputLine& widget, int flags)
    {
        // Convert flags
        //   N = numeric
        //   H = on high ASCII
        //   P = password masking
        //   F = frame
        //   G = game charset
        //   M = width is in ems
        if ((flags & 1) != 0) {
            widget.setFlag(widget.NumbersOnly, true);
        }
        if ((flags & 2) != 0) {
            widget.setFlag(widget.NoHi, true);
        }
        if ((flags & 4) != 0) {
            widget.setFlag(widget.Hidden, true);
        }
        // FIXME: flag 'F' (framed) is missing here: if ((flags & 8) != 0)
        if ((flags & 16) != 0) {
            widget.setFlag(widget.GameChars, true);
        }
    }
}

namespace client { namespace si { namespace {

    /** Utility class to implement a function-that-creates-a-widget-that-is-added-to-its-container.
        This implements the common pattern. */
    class Factory {
     public:
        virtual ~Factory()
            { }

        /** Parse arguments.
            This function is executed within the interpreter thread.
            \param session Session (for example, to access atoms)
            \param args User-specified args. This function should examine and consume the arguments.
            \retval true Arguments processed successfully
            \retval false Null argument encountered, make the return value of this function 0
            \throw interpreter::Error any error such as too few/too many args, wrong types, etc. */
        virtual bool parseArgs(game::Session& session, interpreter::Arguments& args) = 0;

        /** Create the widget.
            This function is executed within the GUI thread.
            \param ctl Control, for example, to obtain a ui::Root (Control::root()) and to authenticate against WidgetReference
            \param holder Dialog's WidgetHolder, for example, to access the Deleter (WidgetHolder::deleter())
            \return newly-allocated widget */
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder) = 0;

        WidgetValue* run(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args, afl::base::Memory<const interpreter::NameTable> names);
    };

} } }

/******************************** Factory ********************************/

client::si::WidgetValue*
client::si::Factory::run(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args, afl::base::Memory<const interpreter::NameTable> names)
{
    if (!parseArgs(session, args)) {
        return 0;
    }

    class Creator : public util::Request<Control> {
     public:
        Creator(const WidgetReference& container, Factory& parent, size_t& result)
            : m_container(container),
              m_parent(parent),
              m_result(result)
            { }
        virtual void handle(Control& ctl)
            {
                ui::Widget* p = m_parent.makeWidget(ctl, m_container.getHolder());
                m_result = m_container.getHolder().addNewWidget(ctl, p);
                if (ui::LayoutableGroup* g = dynamic_cast<ui::LayoutableGroup*>(m_container.get(ctl))) {
                    g->add(*p);
                }
            }
     private:
        const WidgetReference m_container;
        Factory& m_parent;
        size_t& m_result;
    };
    size_t newSlot = 0;
    Creator creator(ref, *this, newSlot);
    ss.call(creator);
    return new GenericWidgetValue(names, session, &ss, ref.makePeer(newSlot));
}


/*************************** Public Interfaces ***************************/

afl::base::Memory<const interpreter::NameTable>
client::si::getDialogNameTable()
{
    return afl::base::Memory<const interpreter::NameTable>(DIALOG_MAP);
}

afl::base::Memory<const interpreter::NameTable>
client::si::getStringListDialogNameTable()
{
    return afl::base::Memory<const interpreter::NameTable>(STRINGLIST_DIALOG_MAP);
}


/* @q NewButton(title:Str, key:Str, Optional command:Any):Widget (Widget Function)
   Creates a push-button widget.
   If the button is pushed, the specified command is executed.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewButton(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class ButtonFactory : public Factory {
     public:
        ButtonFactory()
            : m_title(),
              m_keyValue(0),
              m_commandAtom(0)
            { }
        virtual bool parseArgs(game::Session& session, interpreter::Arguments& args)
            {
                args.checkArgumentCount(2, 3);

                // Mandatory arguments
                if (!checkStringArg(m_title, args.getNext()) || !checkOptionalKeyArg(m_keyValue, args.getNext())) {
                    return false;
                }

                // Optional argument
                interpreter::checkCommandAtomArg(m_commandAtom, args.getNext(), session.world().atomTable());
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                std::auto_ptr<ui::widgets::Button> p(new ui::widgets::Button(m_title, m_keyValue, ctl.root()));
                if (m_commandAtom != 0) {
                    p->sig_fire.addNewClosure(holder.makeCommand(m_commandAtom));
                }
                return p.release();
            }
     private:
        String_t m_title;
        util::Key_t m_keyValue;
        util::Atom_t m_commandAtom;
    };

    return ButtonFactory().run(session, ss, ref, args, BUTTON_MAP);
}

/* @q NewFrame(type:Str, Optional width:Int, pad:Int):Widget (Widget Function)
   Creates a frame.
   The frame is a container which you can add new widgets to.
   By default, the frame behaves as a vertical box (NewVBox()).

   The frame type can be one of:
   - none
   - red
   - yellow
   - green
   - raised
   - lowered

   The %width parameter specifies the width of the frame and defaults to 2.
   If you use "raised" or "lowered", you may want to reduce that to 1.

   The %pad parameter specifies the additional padding between the frame and the contained widget(s).

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewFrame(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // Check parameters: NewFrame("type"[, width, pad])
    class FrameFactory : public Factory {
     public:
        FrameFactory()
            : m_type(ui::NoFrame),
              m_width(2),
              m_pad(0)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(1, 3);

                String_t typeString;
                if (!checkStringArg(typeString, args.getNext())) {
                    return false;
                }
                if (!parseFrameType(m_type, typeString)) {
                    throw interpreter::Error::rangeError();
                }
                checkIntegerArg(m_width, args.getNext(), 0, 1000);
                checkIntegerArg(m_pad, args.getNext(), 0, 1000);
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& /*holder*/)
            {
                std::auto_ptr<ui::widgets::FrameGroup> p(new ui::widgets::FrameGroup(ui::layout::VBox::instance0, ctl.root().colorScheme(), m_type));
                p->setFrameWidth(m_width);
                p->setPadding(m_pad);
                return p.release();
            }

     private:
        ui::FrameType m_type;
        int m_width;
        int m_pad;
    };
    return FrameFactory().run(session, ss, ref, args, FRAMEGROUP_MAP);
}

/* @q NewHBox(Optional space:Int, outer:Int):Widget (Widget Function), NewVBox(Optional space:Int, outer:Int):Widget (Widget Function)
   Creates a horizontal or vertical box.
   You can add new widgets to the box which will be aligned horizontally or vertically.

   The %space parameter specifies the distance between widgets in the box.

   The %outer paramter specifies the additional padding between the box and the contained widget(s).

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewHVBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args, bool horiz)
{
    class BoxFactory : public Factory {
     public:
        BoxFactory(bool horiz)
            : m_horizontal(horiz),
              m_space(5),
              m_outer(0)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(0, 2);
                checkIntegerArg(m_space, args.getNext(), 0, 1000);
                checkIntegerArg(m_outer, args.getNext(), 0, 1000);
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                ui::layout::Manager* mgr;
                if (m_horizontal) {
                    mgr = &holder.deleter(ctl).addNew(new ui::layout::HBox(m_space, m_outer));
                } else {
                    mgr = &holder.deleter(ctl).addNew(new ui::layout::VBox(m_space, m_outer));
                }
                return new ui::Group(*mgr);
            }

     private:
        bool m_horizontal;
        int m_space;
        int m_outer;
    };
    return BoxFactory(horiz).run(session, ss, ref, args, GROUP_MAP);
}

/* @q NewKeyboardFocus(flags:Str, Optional content():Widget):Widget (Widget Function)
   Creates a new keyboard focus handler.
   This enables users to change focus using the keyboard; by default, widgets only support focus handling via mouse.

   The flags contain a list of letters:
   - "h" (horizontal: left/right arrows)
   - "v" (vertical: up/down arrows)
   - "t" (tab key)
   - "p" (page up/down keys)
   - "e" (home/end keys)
   - "w" (enable wrap for arrows)

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewKeyboardFocus(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    using ui::widgets::FocusIterator;

    class FocusFactory : public Factory {
     public:
        FocusFactory()
            : m_flags(0)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                // Leave args pointing at remaining args for later.
                args.checkArgumentCountAtLeast(1);

                static_assert(FocusIterator::Horizontal == 1, "Horizontal/H");
                static_assert(FocusIterator::Vertical == 2, "Vertical/V");
                static_assert(FocusIterator::Tab == 4, "Tab/T");
                static_assert(FocusIterator::Page == 8, "Page/P");
                static_assert(FocusIterator::Home == 16, "Home/E");
                static_assert(FocusIterator::Wrap == 32, "Wrap/W");
                if (!checkFlagArg(m_flags, 0, args.getNext(), "HVTPEW")) {
                    return false;
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& /*ctl*/, WidgetHolder& /*holder*/)
            {
                return new FocusIterator(m_flags);
            }
     private:
        int32_t m_flags;
    };

    // Create the widget
    std::auto_ptr<WidgetValue> result(FocusFactory().run(session, ss, ref, args, KEYBOARDFOCUS_MAP));

    // Add widgets to it using the remaining args
    if (args.getNumArgs() > 0 && result.get() != 0) {
        IFKeyboardFocusAdd(ss, result->getValue(), args);
    }

    return result.release();
}

/* @q NewInput(Optional maxChars:Int, flags:Str, defaultText:Str, key:Str):Widget (Widget Function)
   Creates a text input field.
   See {UI.Input} for a description of the parameters.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewInput(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class InputFactory : public Factory {
     public:
        InputFactory()
            : m_maxChars(255),
              m_flags(0),
              m_width(0), /* FIXME: ui_root->getExtent().w / 2 */
              m_defaultText(),
              m_key(0)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(0, 4);
                // FIXME: same as in commands.cpp -- merge?
                checkIntegerArg(m_maxChars, args.getNext(), 0, 32000);
                checkFlagArg(m_flags, &m_width, args.getNext(), "NHPFGM");
                checkStringArg(m_defaultText, args.getNext());
                checkOptionalKeyArg(m_key, args.getNext());
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& /*holder*/)
            {
                // Font
                gfx::FontRequest font;
                font.addSize(1);

                // Convert length
                const int width = convertWidth(m_width, m_flags, ctl.root(), font);

                // Build a widget
                std::auto_ptr<ui::widgets::InputLine> widget(new ui::widgets::InputLine(m_maxChars, width, ctl.root()));
                widget->setFont(font);

                // Convert flags
                applyFlags(*widget, m_flags);

                widget->setText(m_defaultText);
                if (m_key != 0) {
                    widget->setHotkey(m_key);
                }

                return widget.release();
            }
     private:
        int32_t m_maxChars;
        int32_t m_flags;
        int32_t m_width;
        String_t m_defaultText;
        util::Key_t m_key;
    };

    return InputFactory().run(session, ss, ref, args, INPUT_MAP);
}

/* @q NewSpacer(Optional width:Int, height:Int):Widget (Widget Function)
   Creates a spacer.
   The spacer just occupies space and allows no specific user interaction.

   - no parameters: fills all remaining room, no matter how much/how little
   - one parameter: tries to be a square of the given size
   - two parameters: tries to be a rectangle of the given size

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewSpacer(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // NewSpacer([x,y])
    class SpacerFactory : public Factory {
     public:
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(0, 2);
                if (args.getNumArgs() > 0) {
                    // Single size
                    int width;
                    if (!checkIntegerArg(width, args.getNext(), 0, 1000)) {
                        return false;
                    }

                    int height;
                    if (args.getNumArgs() > 0) {
                        if (!checkIntegerArg(height, args.getNext(), 0, 1000)) {
                            return false;
                        }
                    } else {
                        height = width;
                    }

                    m_info = gfx::Point(width, height);
                } else {
                    // Growing
                    m_info = ui::layout::Info(gfx::Point(), gfx::Point(), ui::layout::Info::GrowBoth);
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& /*ctl*/, WidgetHolder& /*holder*/)
            {
                return new ui::Spacer(m_info);
            }

     private:
        ui::layout::Info m_info;
    };

    return SpacerFactory().run(session, ss, ref, args, afl::base::Nothing);
}

/* @q NewCheckbox(text:Str, key:Str):Widget (Widget Function)
   Creates a binary (on/off) checkbox.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewCheckbox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // NewCheckbox(text, key)
    class CheckboxFactory : public Factory {
     public:
        CheckboxFactory()
            : m_key(),
              m_text()
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(2);
                if (!checkStringArg(m_text, args.getNext())) {
                    return false;
                }
                if (!checkOptionalKeyArg(m_key, args.getNext())) {
                    return false;
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                std::auto_ptr<ui::widgets::Checkbox> p(new ui::widgets::Checkbox(ctl.root(), m_key, m_text, holder.createInteger(ctl)));
                p->addDefaultImages();
                return p.release();
            }
     private:
        util::Key_t m_key;
        String_t m_text;
    };
    return CheckboxFactory().run(session, ss, ref, args, CHECKBOX_MAP);
}

/* @q NewRadiobutton(text:Str, key:Str, value:Int, Optional master:Widget):Widget (Widget Function)
   Creates a radio button.
   Multiple radio buttons allow selection of a value;
   this instance has the value as given by the parameter.

   Without the %master argument, creates a new button group.

   With the %master argument, adds to the group containing %master.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewRadiobutton(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // NewRadiobutton(text, key, value[, master])
    class RadioFactory : public Factory {
     public:
        RadioFactory(WidgetHolder& dialogHolder)
            : m_dialogHolder(dialogHolder),
              m_key(),
              m_text(),
              m_value(),
              m_masterSlot()
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(3, 4);

                if (!checkStringArg(m_text, args.getNext())) {
                    return false;
                }
                if (!checkOptionalKeyArg(m_key, args.getNext())) {
                    return false;
                }
                if (!checkIntegerArg(m_value, args.getNext())) {
                    return false;
                }

                // "Master" argument
                if (afl::data::Value* masterArg = args.getNext()) {
                    WidgetValue* masterWidget = dynamic_cast<WidgetValue*>(masterArg);
                    if (masterWidget == 0) {
                        throw interpreter::Error("Type error, expecting widget");
                    }
                    if (&masterWidget->getValue().getHolder() != &m_dialogHolder) {
                        throw interpreter::Error("Attempt to use widget from different dialog");
                    }
                    m_masterSlot = masterWidget->getValue().getSlot();
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                // Obtain the value
                afl::base::Observable<int>* pValue;
                size_t master;
                if (m_masterSlot.get(master)) {
                    ui::widgets::RadioButton* p = dynamic_cast<ui::widgets::RadioButton*>(holder.get(ctl, master));
                    if (p == 0) {
                        throw interpreter::Error("Type error, expecting radio button widget");
                    }
                    pValue = &p->value();
                } else {
                    pValue = &holder.createInteger(ctl);
                }

                // Create widget
                return new ui::widgets::RadioButton(ctl.root(), m_key, m_text, *pValue, m_value);
            }
     private:
        WidgetHolder& m_dialogHolder;
        util::Key_t m_key;
        String_t m_text;
        int32_t m_value;
        afl::base::Optional<size_t> m_masterSlot;
    };
    return RadioFactory(ref.getHolder()).run(session, ss, ref, args, RADIOBUTTON_MAP);
}

/* @q NewFlowBox(numLines:Int, Optional rightJust:Bool, horizGap:Int, vertGap:Int):Widget (Widget Function)
   Creates a flow box.
   You can add widgets to the FlowBox, which will be placed on lines within the box like words on a page.
   The %numLines parameter specifies the number of lines you want.

   With %rightJust=False (default), widgets are allocated starting from top-left.
   With %rightJust=True, widgets are allocated starting from bottom-right.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewFlowBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class FlowFactory : public Factory {
     public:
        FlowFactory()
            : m_numLines(0),
              m_rightJustified(false),
              m_horizontalGap(5),
              m_verticalGap(5)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                // Parse args
                args.checkArgumentCount(1, 4);
                if (!checkIntegerArg(m_numLines, args.getNext(), 1, 100)) {
                    return false;
                } else {
                    checkBooleanArg(m_rightJustified, args.getNext());
                    checkIntegerArg(m_horizontalGap, args.getNext(), 0, 1000);
                    checkIntegerArg(m_verticalGap, args.getNext(), 0, 1000);
                    return true;
                }
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                ui::layout::Manager& mgr = holder.deleter(ctl).addNew(new ui::layout::Flow(m_numLines, m_rightJustified, m_horizontalGap, m_verticalGap));
                return new ui::Group(mgr);
            }
     private:
        int m_numLines;
        bool m_rightJustified;
        int m_horizontalGap;
        int m_verticalGap;
    };
    return FlowFactory().run(session, ss, ref, args, GROUP_MAP);
}

/* @q NewGridBox(numColumns:Int, Optional space:Int, outer:Int):Widget (Widget Function)
   Creates a grid box.
   You can add widgets to the GridBox, which will be placed in a grid with %numColumns columns.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewGridBox(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class GridFactory : public Factory {
     public:
        GridFactory()
            : m_numColumns(0),
              m_space(5),
              m_outer(5)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(1, 3);
                if (!checkIntegerArg(m_numColumns, args.getNext(), 1, 100)) {
                    return false;
                } else {
                    checkIntegerArg(m_space, args.getNext(), 0, 1000);
                    checkIntegerArg(m_outer, args.getNext(), 0, 1000);
                    return true;
                }
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                ui::layout::Manager& mgr = holder.deleter(ctl).addNew(new ui::layout::Grid(m_numColumns, m_space, m_outer));
                return new ui::Group(mgr);
            }
     private:
        int32_t m_numColumns;
        int32_t m_space;
        int32_t m_outer;
    };
    return GridFactory().run(session, ss, ref, args, GROUP_MAP);
}

/* @q NewLabel(text:Str, Optional style:Str):Widget (Widget Function)
   Creates a simple static label.

   @temporary

   @since PCC2 2.40.1 */
afl::data::Value*
client::si::IFWidgetNewLabel(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    // NewLabel(Text)
    // FIXME: this is probably incomplete
    class LabelFactory : public Factory {
     public:
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(1, 2);
                if (!checkStringArg(m_text, args.getNext())) {
                    return false;
                }

                String_t style;
                m_font = gfx::FontRequest().addSize(1);
                if (checkStringArg(style, args.getNext())) {
                    m_font.parse(style);
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& /*holder*/)
            {
                return new ui::widgets::StaticText(m_text, util::SkinColor::Static, m_font, ctl.root().provider());
            }
     private:
        String_t m_text;
        gfx::FontRequest m_font;
    };
    return LabelFactory().run(session, ss, ref, args, afl::base::Nothing);
}

/* @q NewNumberInput(Optional min:Int, max:Int, current:Int, step:Int):Widget (Widget Function)
   Creates a number input field.

   @temporary

   @see UI.InputNumber
   @since PCC2 2.40.8 */
afl::data::Value*
client::si::IFWidgetNewNumberInput(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class NumberInputFactory : public Factory {
     public:
        NumberInputFactory()
            : m_min(0), m_max(10000), m_current(0), m_step(10)
            { }
        virtual bool parseArgs(game::Session& /*session*/, interpreter::Arguments& args)
            {
                args.checkArgumentCount(0, 4);
                interpreter::checkIntegerArg(m_min, args.getNext());
                interpreter::checkIntegerArg(m_max, args.getNext());
                interpreter::checkIntegerArg(m_current, args.getNext());
                interpreter::checkIntegerArg(m_step, args.getNext());
                if (m_max < m_min) {
                    std::swap(m_min, m_max);
                }
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                ui::widgets::DecimalSelector& inner = holder.deleter(ctl)
                    .addNew(new ui::widgets::DecimalSelector(ctl.root(), ctl.translator(), holder.createInteger(ctl), m_min, m_max, m_step));
                ui::Widget& outer = inner.addButtons(holder.deleter(ctl), ctl.root());
                return new CompoundWidget<ui::widgets::DecimalSelector>(inner, outer);
            }
     private:
        int32_t m_min, m_max, m_current, m_step;
    };
    return NumberInputFactory().run(session, ss, ref, args, NUMBERINPUT_MAP);
}

/* @q NewPseudoInput(Optional content:Str, key:Str, command:Any, flags:Str):Widget (Widget Function)
   Creates a pseudo-input field.
   A pseudo-input field looks like a regular input field, but does not actually accept input.
   Instead, it will trigger a command when clicked.

   @temporary

   @see NewInput
   @since PCC2 2.40.8 */
afl::data::Value*
client::si::IFWidgetNewPseudoInput(game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    class CommandWrap : public afl::base::Closure<void()> {
     public:
        CommandWrap(afl::base::Closure<void(int)>* c)
            : m_closure(c)
            { }
        virtual void call()
            { m_closure->call(0); }
     private:
        afl::base::Closure<void(int)>* m_closure;
    };

    class InputFactory : public Factory {
     public:
        InputFactory()
            : m_defaultText(),
              m_key(0),
              m_commandAtom(0),
              m_flags(0),
              m_width(0)  /* FIXME: ui_root->getExtent().w / 2 */
            { }
        virtual bool parseArgs(game::Session& session, interpreter::Arguments& args)
            {
                args.checkArgumentCount(0, 4);
                checkStringArg(m_defaultText, args.getNext());
                checkOptionalKeyArg(m_key, args.getNext());
                interpreter::checkCommandAtomArg(m_commandAtom, args.getNext(), session.world().atomTable());
                checkFlagArg(m_flags, &m_width, args.getNext(), "NHPFGM");
                return true;
            }
        virtual ui::Widget* makeWidget(Control& ctl, WidgetHolder& holder)
            {
                // Font
                gfx::FontRequest font;
                font.addSize(1);

                // Convert length
                const int width = convertWidth(m_width, m_flags, ctl.root(), font);

                // Build a widget
                std::auto_ptr<ui::widgets::InputLine> widget(new ui::widgets::InputLine(10000, width, ctl.root()));
                widget->setFont(font);

                // Convert flags
                applyFlags(*widget, m_flags);
                widget->setText(m_defaultText);
                if (m_key != 0) {
                    widget->setHotkey(m_key);
                }

                // Make it pseudo
                widget->setFlag(widget->NonEditable, true);
                widget->sig_activate.addNewClosure(new CommandWrap(holder.makeCommand(m_commandAtom)));

                return widget.release();
            }
     private:
        String_t m_defaultText;
        util::Key_t m_key;
        util::Atom_t m_commandAtom;
        int32_t m_flags;
        int32_t m_width;
    };

    return InputFactory().run(session, ss, ref, args, INPUT_MAP);
}

// Call widget function.
afl::data::Value*
client::si::callWidgetFunction(WidgetFunction func, game::Session& session, ScriptSide& ss, const WidgetReference& ref, interpreter::Arguments& args)
{
    switch (func) {
     case wifNewButton:
        return IFWidgetNewButton(session, ss, ref, args);
     case wifNewSpacer:
        return IFWidgetNewSpacer(session, ss, ref, args);
     case wifNewHBox:
        return IFWidgetNewHVBox(session, ss, ref, args, true);
     case wifNewVBox:
        return IFWidgetNewHVBox(session, ss, ref, args, false);
     case wifNewFrame:
        return IFWidgetNewFrame(session, ss, ref, args);
     case wifNewInput:
        return IFWidgetNewInput(session, ss, ref, args);
     case wifNewKeyboardFocus:
        return IFWidgetNewKeyboardFocus(session, ss, ref, args);
     case wifNewCheckbox:
        return IFWidgetNewCheckbox(session, ss, ref, args);
     case wifNewRadiobutton:
        return IFWidgetNewRadiobutton(session, ss, ref, args);
     case wifNewFlowBox:
        return IFWidgetNewFlowBox(session, ss, ref, args);
     case wifNewGridBox:
        return IFWidgetNewGridBox(session, ss, ref, args);
     case wifNewLabel:
        return IFWidgetNewLabel(session, ss, ref, args);
     case wifNewNumberInput:
        return IFWidgetNewNumberInput(session, ss, ref, args);
     case wifNewPseudoInput:
        return IFWidgetNewPseudoInput(session, ss, ref, args);
    }
    return 0;
}
