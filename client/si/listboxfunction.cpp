/**
  *  \file client/si/listboxfunction.cpp
  */

#include "client/si/listboxfunction.hpp"
#include "client/si/widgetholder.hpp"
#include "interpreter/arguments.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usercall.hpp"
#include "client/si/control.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetcommandvalue.hpp"
#include "client/si/stringlistdialogwidget.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/widgetfunction.hpp"

namespace {
    // /** Listbox data. */
    // struct ListboxData : public afl::base::RefCounted {
    //     ListboxData(client::si::ScriptSide* ss, afl::base::Ref<client::si::WidgetHolder> holder)
    //         : title(),
    //           current(-1),
    //           width(320),
    //           height(-1),
    //           help(),
    //           m_pScriptSide(ss),
    //           holder(holder),
    //           widgetIndex(0)
    //         { }

    //     String_t title;
    //     int32_t current;
    //     int32_t width;
    //     int32_t height;
    //     String_t help;

    //     afl::base::WeakLink<client::si::ScriptSide> m_pScriptSide;

    //     afl::base::Ref<client::si::WidgetHolder> holder;
    //     size_t widgetIndex;
    // };

    // enum ListboxCommand {
    //     lbcAddItem,
    //     lbcRun
    // };

    // const interpreter::NameTable LISTBOX_TABLE[] = {
    //     { "ADDITEM",     lbcAddItem,   0,     interpreter::thProcedure },
    //     { "RUN",         lbcRun,       0,     interpreter::thProcedure },
    // };

    // class ListboxValue : public interpreter::SingleContext {
    //  public:
    //     ListboxValue(afl::base::Ref<ListboxData> data)
    //         : m_data(data)
    //         { }
    //     virtual ListboxValue* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
    //         { return lookupName(name, LISTBOX_TABLE, result) ? this : 0; }
    //     virtual void set(PropertyIndex_t /*index*/, afl::data::Value* /*value*/)
    //         { throw interpreter::Error::notAssignable(); }
    //     virtual afl::data::Value* get(PropertyIndex_t index)
    //         {
    //             switch (ListboxCommand(LISTBOX_TABLE[index].index)) {
    //              case lbcAddItem:
    //                 return new client::si::WidgetCommandValue(client::si::wicListboxAddItem, m_data->m_pScriptSide.get(), WidgetReference(m_data->holder, m_data->widgetIndex));
    //              case lbcRun:
    //                 return 0;
    //             }
    //             return 0;
    //         }
    //     virtual ListboxValue* clone() const
    //         { return new ListboxValue(m_data); }
    //     virtual void enumProperties(interpreter::PropertyAcceptor& acceptor)
    //         { acceptor.enumTable(LISTBOX_TABLE); }

    //     virtual game::map::Object* getObject()
    //         { return 0; }
    //     virtual String_t toString(bool /*readable*/) const
    //         { return "#<widget>"; }
    //     virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
    //         { throw interpreter::Error::notSerializable(); }

    //  private:
    //     afl::base::Ref<ListboxData> m_data;
    // };

    
}


client::si::ListboxFunction::ListboxFunction(game::Session& session, ScriptSide* pScriptSide)
    : m_session(session),
      m_pScriptSide(pScriptSide)
{ }

// BaseValue:
String_t
client::si::ListboxFunction::toString(bool /*readable*/) const
{
    return "#<function>";
}

void
client::si::ListboxFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

// IndexableValue:
/* @q Listbox():Any (Function)
   @noproto
   | With Listbox(title:Str, Optional current:Int, width:Int, height:Int, help:Str) Do
   |   AddItem id:Int, text:Str
   |   Run
   | EndWith

   This command sequence creates a standard list box.
   It consists of three parts:

   - the <tt>With Listbox()</tt> part creates a blank, still invisible list box template;
   - the <tt>{AddItem (Listbox Command)|AddItem}</tt> part adds items to the list box.
     You can use any number of these.
     Items are identified by their %id value which is an integer between 0 and 2^31-1
     (PCC 1.1.6 and below accept only values up to 32767);
   - the <tt>{Run (Listbox Command)|Run}</tt> part finally displays the list box
     and lets the user choose from it.
     You can repeat %Run as often as you wish.

   The parameters are as follows:
 
   - %title: a string that is displayed in the title bar of the list box.
     This is the only mandatory parameter for %Listbox();
   - %current: the %id value of the entry which will be selected by default
     when the list box opens. When there's no item with that identifier, the
     first one will be selected;
   - %width: the width of the list box in pixels. Must be between 200 and 1000,
     default is 320;
   - %height: the height of the list box in lines. Must be between 3 and 100,
     default is number of items in list box.
     Pass -1 here to choose that default;
   - %help: the help page associated with this list box. See {UI.Help}.
 
   The %Run command actually displays the list box and lets the user choose
   from it. It sets the {UI.Result} variable to the identifier (%id) of the
   item chosen by the user, or to EMPTY if she canceled.
 
   Example: this is a simplified version of the "Set Primary Enemy" command:
   |   Local i, UI.Result
   |   With Listbox("Primary Enemy", Enemy$, 260, 12, 10026) Do
   |     AddItem 0, "0 - none"
   |     For i:=1 To 11 Do AddItem i, Player(i).Race.Short
   |     Run
   |     SetEnemy UI.Result
   |   EndWith
 
   Note: scripts can not suspend while a <tt>With Listbox</tt> block is active.
   @since PCC 1.1.1, PCC2 1.99.25, PCC2 2.40.1 */
afl::data::Value*
client::si::ListboxFunction::get(interpreter::Arguments& args)
{
    // ex int/if/listif.h:IFListbox
    args.checkArgumentCount(1, 5);

    // Must have a ScriptSide
    ScriptSide* ss = m_pScriptSide.get();
    if (!ss) {
        return 0;
    }

    // Parse args and populate Creator
    class Creator : public UserCall {
     public:
        Creator(afl::base::Ref<WidgetHolder> wh, size_t& result)
            : m_wh(wh),
              m_result(result),
              m_dialogTitle(),
              m_current(0),
              m_width(320),
              m_height(0),
              m_help()
            { }

        bool processArguments(interpreter::Arguments& args)
            {
                if (!interpreter::checkStringArg(m_dialogTitle, args.getNext())) {
                    return false;
                }
                interpreter::checkIntegerArg(m_current, args.getNext());
                // \change: minimum width is 0 (=auto), was 200.
                interpreter::checkIntegerArg(m_width, args.getNext(), 0, 2000);
                interpreter::checkIntegerArg(m_height, args.getNext(), -1, 100);
                interpreter::checkStringArg(m_help, args.getNext());
                return true;
            }
        void handle(UserSide& ui, Control& ctl)
            {
                m_result = m_wh->addNewWidget(ui,
                                              new StringListDialogWidget(ctl.root().provider(), ctl.root().colorScheme(),
                                                                         m_dialogTitle, m_current, m_width, m_height, m_help));
            }
     private:
        const afl::base::Ref<WidgetHolder> m_wh;
        size_t& m_result;
        String_t m_dialogTitle;
        int32_t m_current;
        int32_t m_width;
        int32_t m_height;
        String_t m_help;
    };

    afl::base::Ref<WidgetHolder> wh = *new WidgetHolder(ss->sender());
    size_t result;
    Creator c(wh, result);
    if (!c.processArguments(args)) {
        return 0;
    }

    // Create listbox widget
    ss->call(c);

    // Produce result
    return new GenericWidgetValue(getStringListDialogNameTable(), m_session, ss, WidgetReference(wh, result));
}

void
client::si::ListboxFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

int32_t
client::si::ListboxFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
client::si::ListboxFunction::makeFirstContext()
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

client::si::ListboxFunction*
client::si::ListboxFunction::clone() const
{
    return new ListboxFunction(m_session, m_pScriptSide.get());
}
