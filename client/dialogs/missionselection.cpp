/**
  *  \file client/dialogs/missionselection.cpp
  *  \brief Mission Selection Dialog
  */

#include "client/dialogs/missionselection.hpp"
#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/cardgroup.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/stringlist.hpp"

using afl::container::PtrVector;
using client::widgets::HelpWidget;
using game::spec::MissionList;
using ui::CardGroup;
using ui::widgets::FrameGroup;
using ui::widgets::IconBox;
using ui::widgets::ScrollbarContainer;
using ui::widgets::SimpleIconBox;
using ui::widgets::StandardDialogButtons;
using ui::widgets::StringListbox;

namespace {
    class Dialog {
     public:
        Dialog(ui::Root& root)
            : m_root(root),
              m_deleter(),
              m_loop(root),
              m_cards(),
              m_iconBox(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 1), root),
              m_lists(),
              m_currentPage(0)
            {
                m_iconBox.sig_change.add(this, &Dialog::onIconClick);
                m_iconBox.setKeys(IconBox::Tab | IconBox::Arrows);
                m_iconBox.setItemKeys(SimpleIconBox::UseAltKeys);
            }

        bool setData(MissionList::Grouped& choices, int currentValue, afl::string::Translator& tx);
        bool run(String_t title, String_t helpId, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

        afl::base::Optional<int32_t> getCurrentKey() const;

     private:
        void onIconClick(size_t n);
        void onItemDoubleClick(size_t index);

        ui::Root& m_root;
        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;
        CardGroup m_cards;
        SimpleIconBox m_iconBox;
        PtrVector<StringListbox> m_lists;
        size_t m_currentPage;
    };
}

bool
Dialog::setData(MissionList::Grouped& choices, int currentValue, afl::string::Translator& tx)
{
    SimpleIconBox::Items_t icons;
    for (std::map<String_t, util::StringList>::iterator it = choices.groups.begin(); it != choices.groups.end(); ++it) {
        // Find insert point. 'allName' always goes first
        size_t insertAt = 0;
        if (it->first != choices.allName) {
            while (insertAt < icons.size()
                   && (icons[insertAt].text == choices.allName
                       || afl::string::strCaseCompare(it->first, icons[insertAt].text) > 0))
            {
                ++insertAt;
            }
        }

        // Insert
        icons.insert(icons.begin() + insertAt, SimpleIconBox::Item(it->first));
        StringListbox& list = *m_lists.insertNew(m_lists.begin() + insertAt, new StringListbox(m_root.provider(), m_root.colorScheme()));
        list.swapItems(it->second);
        list.sig_itemDoubleClick.add(this, &Dialog::onItemDoubleClick);
        m_cards.add(FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::LoweredFrame,
                                           m_deleter.addNew(new ScrollbarContainer(list, m_root))));
    }

    // Set data
    m_iconBox.swapContent(icons, 0);
    if (m_lists.size() > 0) {
        m_lists[0]->addItem(-1, tx("# - Extended Mission"));
        m_lists[0]->setCurrentKey(currentValue);
        return true;
    } else {
        return false;
    }
}

bool
Dialog::run(String_t title, String_t helpId, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    ui::Window win(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    win.add(m_iconBox);
    win.add(m_cards);
    if (m_lists.size() > 0) {
        m_lists[0]->requestFocus();
    }

    StandardDialogButtons& btn = m_deleter.addNew(new StandardDialogButtons(m_root, tx));
    if (!helpId.empty()) {
        btn.addHelp(m_deleter.addNew(new HelpWidget(m_root, tx, gameSender, helpId)));
    }
    btn.addStop(m_loop);
    win.add(btn);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return m_loop.run() != 0;
}

afl::base::Optional<int32_t>
Dialog::getCurrentKey() const
{
    if (m_currentPage < m_lists.size()) {
        return m_lists[m_currentPage]->getCurrentKey();
    } else {
        return afl::base::Nothing;
    }
}

void
Dialog::onIconClick(size_t n)
{
    afl::base::Optional<int32_t> k = getCurrentKey();
    if (n < m_lists.size()) {
        if (const int32_t* p = k.get()) {
            m_lists[n]->setCurrentKey(*p);
        }
        m_lists[n]->requestFocus();
    }
    m_currentPage = n;
}

void
Dialog::onItemDoubleClick(size_t /*index*/)
{
    m_loop.stop(1);
}

/*
 *  Main Entry Point
 */

afl::base::Optional<int>
client::dialogs::chooseMission(game::spec::MissionList::Grouped& choices,
                               int currentValue,
                               String_t title,
                               String_t helpId,
                               ui::Root& root,
                               afl::string::Translator& tx,
                               util::RequestSender<game::Session> gameSender)
{
    Dialog dlg(root);
    if (!dlg.setData(choices, currentValue, tx)) {
        return afl::base::Nothing;
    }

    if (!dlg.run(title, helpId, tx, gameSender)) {
        return afl::base::Nothing;
    }

    return dlg.getCurrentKey();
}
