/**
  *  \file client/usercallback.cpp
  *  \brief Class client::UserCallback
  */

#include "client/usercallback.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using ui::widgets::StaticText;

client::UserCallback::UserCallback(client::si::UserSide& us)
    : m_userSide(us)
{ }

client::UserCallback::~UserCallback()
{ }

void
client::UserCallback::askPassword(const PasswordRequest& req)
{
    // Wrap the operation into processInteraction(),
    // so it even works if the UI is blocked waiting for the browser operation to complete.
    class Task : public util::Request<client::si::UserSide> {
     public:
        Task(const PasswordRequest& req, PasswordResponse& resp)
            : m_req(req), m_resp(resp)
            { }

        virtual void handle(client::si::UserSide& us)
            {
                // Environment
                afl::string::Translator& tx = us.translator();
                ui::Root& root = us.root();

                // Build a dialog
                afl::base::Deleter del;
                ui::Window window(tx("Enter Password"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

                if (m_req.hasFailed) {
                    window.add(del.addNew(new StaticText(tx("User name or password not accepted!"), ui::SkinColor::Red, gfx::FontRequest(), root.provider())));
                }

                // TODO: embolden the account name?
                window.add(del.addNew(new StaticText(afl::string::Format(tx("Enter password for %s:"), m_req.accountName), ui::SkinColor::Static, gfx::FontRequest(), root.provider())));

                ui::widgets::InputLine& input = del.addNew(new ui::widgets::InputLine(1000, 20, root));
                input.setFlag(input.Hidden, true);
                input.setFont("+");
                window.add(input);

                // OK/Cancel buttons
                ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
                window.add(btn);

                ui::EventLoop loop(root);
                btn.addStop(loop);

                window.add(del.addNew(new ui::widgets::Quit(root, loop)));

                window.pack();
                root.centerWidget(window);
                root.addChild(window, 0);

                bool ok = (loop.run() != 0);

                // Send response
                m_resp.canceled = !ok;
                m_resp.password = input.getText();
            }
     private:
        const PasswordRequest& m_req;
        PasswordResponse& m_resp;
    };

    PasswordResponse resp;
    Task t(req, resp);
    m_userSide.processInteraction(t);
    sig_passwordResult.raise(resp);
}

