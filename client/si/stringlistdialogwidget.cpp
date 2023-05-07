/**
  *  \file client/si/stringlistdialogwidget.cpp
  *  \brief Class client::si::StringListDialogWidget
  */

#include "client/si/stringlistdialogwidget.hpp"
#include "afl/base/deleter.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbar.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"

namespace {
    ui::widgets::BaseButton* findKeyButton(ui::Widget& me, util::Key_t key)
    {
        // This mirrors the logic of defaultHandleKey() to emulate key dispatch.
        // In particular, if multiple sub-widgets define the same key, the one that has focus will be picked.
        // ex UIBaseComplexWidget::handleEvent, sort-of
        ui::widgets::BaseButton* btn = dynamic_cast<ui::widgets::BaseButton*>(&me);
        if (btn != 0 && btn->getKey() == key) {
            return btn;
        }

        if (ui::Widget* w = me.getFocusedChild()) {
            if (ui::widgets::BaseButton* btn = findKeyButton(*w, key)) {
                return btn;
            }
        }
        for (ui::Widget* w = me.getFirstChild(); w != 0; w = w->getNextSibling()) {
            // Focused child has already been processed above; do not process it again.
            if (w != me.getFocusedChild()) {
                if (ui::widgets::BaseButton* btn = findKeyButton(*w, key)) {
                    return btn;
                }
            }
            if (w->hasState(ui::Widget::ModalState)) {
                break;
            }
        }
        return 0;
    }

    gfx::Point parseAnchor(ui::Root& root, const String_t& anchor)
    {
        // Is it a key?
        util::Key_t key;
        if (util::parseKey(anchor, key)) {
            if (ui::widgets::BaseButton* btn = findKeyButton(root, key)) {
                return btn->getExtent().getBottomLeft();
            }
        }

        // Is it an absolute position?
        int x = 0, y = 0;
        util::StringParser p(anchor);
        if (p.parseCharacter('@')
            && p.parseInt(x)
            && p.parseCharacter(',')
            && p.parseInt(y)
            && p.parseEnd())
        {
            gfx::Point result(x, y);
            if (root.getExtent().contains(result)) {
                return result;
            }
        }

        return root.getExtent().getCenter();
    }
}

// Constructor.
client::si::StringListDialogWidget::StringListDialogWidget(gfx::ResourceProvider& provider, ui::ColorScheme& scheme,
                                                           String_t dialogTitle, int32_t current, int32_t width, int32_t height, String_t help)
    : StringListbox(provider, scheme),
      m_dialogTitle(dialogTitle),
      m_current(current),
      m_width(width),
      m_height(height),
      m_help(help)
{ }

// Execute standard dialog.
bool
client::si::StringListDialogWidget::run(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    // Configure
    setPreferredHeight(m_height <= 0 ? 0 : m_height <= 3 ? 3 : m_height);
    setPreferredWidth(m_width <= 0 ? 0 : m_width, true);
    setCurrentKey(m_current);

    // Standard dialog
    std::auto_ptr<ui::Widget> pHelp;
    if (!m_help.empty()) {
        pHelp.reset(new client::widgets::HelpWidget(root, tx, gameSender, m_help));
    }
    bool result = doStandardDialog(m_dialogTitle, String_t(), pHelp.get(), root, tx);

    // Update current
    if (result) {
        getCurrentKey().get(m_current);
    }
    return result;
}

bool
client::si::StringListDialogWidget::runMenu(ui::Root& root, const String_t& anchor)
{
    // Configure
    setPreferredHeight(static_cast<int>(getNumItems()));
    setPreferredWidth(0, true);
    setCurrentKey(m_current);

    // We cannot operate when we have a parent
    if (getParent() != 0) {
        return false;
    }

    gfx::Point anchorPoint = parseAnchor(root, anchor);

    ui::EventLoop loop(root);
    return ui::widgets::MenuFrame(ui::layout::VBox::instance0, root, loop).doMenu(*this, anchorPoint);
}
