/**
  *  \file client/dialogs/referencesortorder.cpp
  */

#include "client/dialogs/referencesortorder.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "game/ref/configuration.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/scrollbar.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

namespace gr = game::ref;
using afl::string::Format;

namespace {
    struct Item {
        const char* name;
        int id;
        bool isDefault;
    };

    const Item ITEMS[] = {
        { N_("Id"),            gr::ConfigSortById,          true },
        { N_("Name"),          gr::ConfigSortByName,        true },
        { N_("Owner"),         gr::ConfigSortByOwner,       true },
        { N_("Hull Type"),     gr::ConfigSortByHull,        true },
        { N_("Hull Mass"),     gr::ConfigSortByHullMass,    true },
        { N_("Mass"),          gr::ConfigSortByMass,        true },
        { N_("Damage"),        gr::ConfigSortByDamage,      true },
        { N_("Fleet"),         gr::ConfigSortByFleet,       true },
        { N_("Tow Group"),     gr::ConfigSortByTowGroup,    true },
        { N_("Position"),      gr::ConfigSortByLocation,    true },
        { N_("Next Position"), gr::ConfigSortByNextPosition, true },
        { N_("Battle Order"),  gr::ConfigSortByBattleOrder, true },
    };

    const Item* find(int id)
    {
        for (size_t i = 0; i < countof(ITEMS); ++i) {
            if (ITEMS[i].id == id) {
                return &ITEMS[i];
            }
        }
        return 0;
    }

    size_t findIndex(int id)
    {
        for (size_t i = 0; i < countof(ITEMS); ++i) {
            if (ITEMS[i].id == id) {
                return i;
            }
        }
        return 0;
    }

    int getId(size_t index)
    {
        if (index < countof(ITEMS)) {
            return ITEMS[index].id;
        } else {
            return 0;
        }
    }

    String_t getName(int id)
    {
        if (const Item* p = find(id)) {
            return p->name;
        } else {
            return Format("<%d>", id);
        }
    }

    String_t getActionName(int first, int second, afl::string::Translator& tx)
    {
        if (second == gr::ConfigSortById) {
            return Format(tx.translateString("Sort by %s").c_str(), getName(first));
        } else {
            return Format(tx.translateString("Sort by %s \xC2\xBB %s").c_str(), getName(first), getName(second));
        }
    }

    void initList(ui::widgets::RichListbox& box, bool second, afl::string::Translator& tx)
    {
        size_t start = 0;
        if (second) {
            // FIXME: use this?
            // box.addItem(tx.translateString("none"), 0, true);
            // start = 1;
        }
        for (size_t i = start; i < countof(ITEMS); ++i) {
            box.addItem(tx.translateString(ITEMS[i].name), 0, true);
        }

        box.setRenderFlag(ui::widgets::RichListbox::UseBackgroundColorScheme, true);
        box.setRenderFlag(ui::widgets::RichListbox::DisableWrap, true);
    }


    class ReferenceSortOrderDialog {
     public:
        ReferenceSortOrderDialog(ui::Root& root, afl::string::Translator& tx);

        bool run(game::ref::Configuration& order, ui::Root& root, afl::string::Translator& tx);
        void onFirstChange();

     private:
        ui::widgets::RichListbox m_firstList;
        ui::widgets::RichListbox m_secondList;
    };
}

ReferenceSortOrderDialog::ReferenceSortOrderDialog(ui::Root& root, afl::string::Translator& tx)
    : m_firstList(root.provider(), root.colorScheme()),
      m_secondList(root.provider(), root.colorScheme())
{
    initList(m_firstList, false, tx);
    initList(m_secondList, true, tx);

    int preferredWidth = 20 * root.provider().getFont(gfx::FontRequest())->getCellSize().getX();
    m_firstList.setPreferredWidth(preferredWidth);
    m_secondList.setPreferredWidth(preferredWidth);
}

bool
ReferenceSortOrderDialog::run(game::ref::Configuration& order, ui::Root& root, afl::string::Translator& tx)
{
    afl::base::Deleter del;

    // Window [VBox]
    //  HBox
    //    VBox
    //      "Sort by..."
    //      first list
    //    VBox
    //      "then by..."
    //      second list
    //  Buttons

    ui::Window& win(del.addNew(new ui::Window(tx.translateString("Sort order"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));
    ui::Group& g1(del.addNew(new ui::Group(ui::layout::HBox::instance5)));
    ui::Group& g11(del.addNew(new ui::Group(ui::layout::VBox::instance5)));
    ui::Group& g12(del.addNew(new ui::Group(ui::layout::VBox::instance5)));
    g11.add(del.addNew(new ui::widgets::StaticText(tx("Sort by..."), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider())));
    g11.add(ui::widgets::FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_firstList, root))));
    g1.add(g11);

    g12.add(del.addNew(new ui::widgets::StaticText(tx("then by..."), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider())));
    g12.add(ui::widgets::FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_secondList, root))));
    g1.add(g12);

    win.add(g1);

    ui::widgets::StandardDialogButtons& btn(del.addNew(new ui::widgets::StandardDialogButtons(root, tx)));
    win.add(btn);

    ui::widgets::FocusIterator& fi(del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Horizontal | ui::widgets::FocusIterator::Tab)));
    fi.add(m_firstList);
    fi.add(m_secondList);
    win.add(fi);

    ui::EventLoop loop(root);
    btn.addStop(loop);

    win.pack();
    root.centerWidget(win);
    root.add(win);

    m_firstList.requestFocus();
    m_firstList.setCurrentItem(findIndex(order.order.first));
    m_secondList.setCurrentItem(findIndex(order.order.second));
    onFirstChange();
    m_firstList.sig_change.add(this, &ReferenceSortOrderDialog::onFirstChange);

    bool ok = (loop.run() != 0);
    if (ok) {
        order.order.first  = getId(m_firstList.getCurrentItem());
        order.order.second = (order.order.first == ITEMS[0].id
                              ? order.order.first
                              : getId(m_secondList.getCurrentItem()));
    }
    return ok;
}

void
ReferenceSortOrderDialog::onFirstChange()
{
    size_t firstIndex = m_firstList.getCurrentItem();

    // First, enable right list entirely
    for (size_t i = 0, n = m_secondList.getNumItems(); i < n; ++i) {
        m_secondList.setItemAccessible(i, true);
    }

    // Now, configure according to left selection
    if (firstIndex == 0) {
        // "Id" is selected: block right selection entirely
        m_secondList.setState(ui::Widget::DisabledState, true);
    } else {
        // Not "Id", allow secondary selection but disallow selecting the same as on the left side
        if (m_secondList.getCurrentItem() == firstIndex) {
            m_secondList.setCurrentItem(0);
        }
        m_secondList.setState(ui::Widget::DisabledState, false);
        m_secondList.setItemAccessible(firstIndex, false);
    }
}

bool
client::dialogs::doReferenceSortOrderMenu(game::ref::Configuration& order, gfx::Point anchor, ui::Root& root, afl::string::Translator& tx)
{
    // ex WVisualScanListWindow::onMenu, sort.pas:ChooseSortOrder
    ui::widgets::StringListbox box(root.provider(), root.colorScheme());

    // FIXME: improve this...
    const int32_t NUM_ITEMS = int32_t(countof(ITEMS));
    int32_t current = 0;
    for (int32_t i = 0; i < NUM_ITEMS; ++i) {
        box.addItem(i, getActionName(ITEMS[i].id, 0, tx));
        if (order.order.first == ITEMS[i].id && order.order.second == gr::ConfigSortById) {
            current = i;
        }
    }
    box.addItem(999, tx.translateString("More..."));
    box.setCurrentKey(current);

    // Do the menu
    ui::EventLoop loop(root);
    if (!ui::widgets::MenuFrame(ui::layout::HBox::instance0, root, loop).doMenu(box, anchor)) {
        return false;
    }

    int32_t result;
    if (!box.getCurrentKey(result)) {
        return false;
    }
    if (result == 999) {
        return ReferenceSortOrderDialog(root, tx).run(order, root, tx);
    } else if (result >= 0 && result < NUM_ITEMS) {
        order.order.first = ITEMS[result].id;
        order.order.second = gr::ConfigSortById;
        return true;
    } else {
        return false;
    }
}
