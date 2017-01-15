/**
  *  \file client/si/dialogfunction.cpp
  */

#include "client/si/dialogfunction.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/widgetholder.hpp"
#include "client/si/usercall.hpp"
#include "ui/window.hpp"
#include "client/si/control.hpp"
#include "ui/layout/vbox.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/widgetfunction.hpp"
#include "client/si/widgetcommand.hpp"
#include "interpreter/typehint.hpp"

client::si::DialogFunction::DialogFunction(game::Session& session, ScriptSide* pScriptSide)
    : m_session(session),
      m_pScriptSide(pScriptSide)
{ }

// BaseValue:
String_t
client::si::DialogFunction::toString(bool /*readable*/) const
{
    return "#<function>";
}

void
client::si::DialogFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

// IndexableValue:
afl::data::Value*
client::si::DialogFunction::get(interpreter::Arguments& args)
{
    /* @q UI.Dialog():Any (Function)
       @noproto
       | With UI.Dialog(title:Str) Do
       |   ....
       |   Run
       | EndWith

       This function creates a dialog box.

       FIXME: extend this doc

       @since PCC2 2.40.1 */

    args.checkArgumentCount(1);
    String_t title;
    if (!interpreter::checkStringArg(title, args.getNext())) {
        return 0;
    }
    if (ScriptSide* ss = m_pScriptSide.get()) {
        // OK
        afl::base::Ref<WidgetHolder> wh(*new WidgetHolder(ss->sender()));

        // Create a window object
        class Creator : public UserCall {
         public:
            Creator(afl::base::Ref<WidgetHolder> wh, String_t title, size_t& result)
                : m_wh(wh),
                  m_title(title),
                  m_result(result)
                { }
            void handle(UserSide& ui, Control& ctl)
                { m_result = m_wh->addNewWidget(ui, new ui::Window(m_title, ctl.root().provider(), ctl.root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)); }
         private:
            afl::base::Ref<WidgetHolder> m_wh;
            String_t m_title;
            size_t& m_result;
        };
        size_t slot;
        Creator creator(wh, title, slot);
        ss->call(creator);
        return new GenericWidgetValue(getDialogNameTable(), m_session, ss, WidgetReference(wh, slot));
    } else {
        return 0;
    }
}
void
client::si::DialogFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

int32_t
client::si::DialogFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
client::si::DialogFunction::makeFirstContext()
{
    throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
}

client::si::DialogFunction*
client::si::DialogFunction::clone() const
{
    return new DialogFunction(m_session, m_pScriptSide.get());
}
