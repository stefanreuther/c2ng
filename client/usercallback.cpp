/**
  *  \file client/usercallback.cpp
  */

#include "client/usercallback.hpp"
#include "afl/base/deleter.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringvalue.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    class RegisterTask : public util::Request<game::browser::Session> {
     public:
        RegisterTask(util::RequestSender<game::browser::UserCallback> p)
            : m_p(p)
            { }
        void handle(game::browser::Session& t)
            { t.userCallbackProxy().setInstance(m_p); }
     private:
        util::RequestSender<game::browser::UserCallback> m_p;
    };

    class Handler : public afl::base::Deletable {
     public:
        virtual void commit(afl::data::Segment& values) = 0;
    };

    class InputLineHandler : public Handler {
     public:
        InputLineHandler(ui::widgets::InputLine& line, size_t index)
            : m_line(line),
              m_index(index)
            { }
        virtual void commit(afl::data::Segment& values)
            { values.setNew(m_index, new afl::data::StringValue(m_line.getText())); }
     private:
        ui::widgets::InputLine& m_line;
        size_t m_index;
    };
}


client::UserCallback::UserCallback(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::browser::Session> sender)
    : m_receiver(root.engine().dispatcher(), *this),
      m_root(root),
      m_translator(tx),
      m_sender(sender)
{
    m_sender.postNewRequest(new RegisterTask(m_receiver.getSender()));
}

client::UserCallback::~UserCallback()
{
    m_sender.postNewRequest(new RegisterTask(util::RequestSender<game::browser::UserCallback>()));
}

bool
client::UserCallback::askInput(String_t title,
                               const std::vector<Element>& question,
                               afl::data::Segment& values)
{
    // Build a dialog
    afl::base::Deleter del;
    afl::container::PtrVector<Handler> handlers;
    ui::Window window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    size_t valueIndex = 0;
    for (size_t i = 0; i < question.size(); ++i) {
        switch (question[i].type) {
         case AskString: {
            window.add(del.addNew(new ui::widgets::StaticText(question[i].prompt,
                                                              ui::SkinColor::Static,
                                                              gfx::FontRequest(),
                                                              m_root.provider())));
            ui::widgets::InputLine& input = del.addNew(new ui::widgets::InputLine(1000, 40, m_root));
            input.setText(afl::data::Access(values[valueIndex]).toString());
            window.add(input);
            handlers.pushBackNew(new InputLineHandler(input, valueIndex));
            ++valueIndex;
            break;
         }
         case AskPassword: {
            window.add(del.addNew(new ui::widgets::StaticText(question[i].prompt,
                                                              ui::SkinColor::Static,
                                                              gfx::FontRequest(),
                                                              m_root.provider())));
            ui::widgets::InputLine& input = del.addNew(new ui::widgets::InputLine(1000, 40, m_root));
            input.setText(afl::data::Access(values[valueIndex]).toString());
            input.setFlag(input.Hidden, true);
            window.add(input);
            handlers.pushBackNew(new InputLineHandler(input, valueIndex));
            ++valueIndex;
            break;
         }
         case ShowInfo:
            window.add(del.addNew(new ui::widgets::StaticText(question[i].prompt,
                                                              ui::SkinColor::Static,
                                                              gfx::FontRequest(),
                                                              m_root.provider())));
            break;
        }
    }

    // OK/Cancel buttons
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    window.add(btn);

    ui::EventLoop loop(m_root);
    btn.addStop(loop);

    window.pack();
    m_root.centerWidget(window);
    m_root.addChild(window, 0);
    if (loop.run() != 0) {
        for (size_t i = 0; i < handlers.size(); ++i) {
            handlers[i]->commit(values);
        }
        return true;
    } else {
        return false;
    }
}
