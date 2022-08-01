/**
  *  \file client/dialogs/changepassword.cpp
  *  \brief Password change dialog
  */

#include "client/dialogs/changepassword.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using ui::widgets::Button;
using ui::widgets::InputLine;

bool
client::dialogs::doChangePassword(ui::Root& root, afl::string::Translator& tx, String_t& result)
{
    // ex doPasswordChange, misc.pas:ChangePassword
    const int WIDTH_EM = 20;
    const int WIDTH_PX = WIDTH_EM * root.provider().getFont("+")->getEmWidth();
    enum {
        IdCancel     = 0,  // Must be 0 because used by Quit widget
        IdOK         = 1,
        IdNoPassword = 2
    };

    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Change password"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Help
    win.add(del.addNew(new ui::rich::StaticText(util::rich::Parser::parseXml(tx("<big>Enter new password:</big>\n\n"
                                                                                "With PCC, the new password will become effective immediately. "
                                                                                "Other utilities may require the old password until the next turn.\n\n"
                                                                                "Remember that a result file password does not protect "
                                                                                "against a determined attacker.")),
                                                WIDTH_PX, root.provider())));

    // Input
    InputLine& input = del.addNew(new InputLine(10, WIDTH_EM, root));
    input.setFlag(InputLine::NoHi, true);
    input.setFlag(InputLine::Hidden, true);
    win.add(ui::widgets::FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, input));

    // Buttons
    // "No password" used to be Alt-D, but that clashes with an editing key
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    Button& btnNoPassword = del.addNew(new Button(tx("Alt-N - No password"), util::KeyMod_Alt + 'n', root));
    Button& btnOK         = del.addNew(new Button(tx("OK"),                  util::Key_Return,       root));
    Button& btnCancel     = del.addNew(new Button(tx("Cancel"),              util::Key_Escape,       root));
    g.add(btnNoPassword);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(btnCancel);
    win.add(g);

    // Admin
    ui::EventLoop loop(root);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    btnNoPassword.sig_fire.addNewClosure(loop.makeStop(IdNoPassword));
    btnOK.sig_fire.addNewClosure(loop.makeStop(IdOK));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(IdCancel));

    win.pack();
    input.requestFocus();
    root.centerWidget(win);
    root.add(win);

    bool ok = false;
    switch (loop.run()) {
     case IdNoPassword: ok = true; result = "NOPASSWORD";    break;
     case IdOK:         ok = true; result = input.getText(); break;
     case IdCancel:                                          break;
    }
    return ok;
}
