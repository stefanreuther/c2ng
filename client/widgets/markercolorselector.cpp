/**
  *  \file client/widgets/markercolorselector.cpp
  *  \brief Class client::widgets::MarkerColorSelector
  */

#include "client/widgets/markercolorselector.hpp"
#include "afl/base/staticassert.hpp"
#include "client/map/renderer.hpp"
#include "game/map/drawing.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    const int NUM_LINES = 3;
    const int NUM_COLUMNS = 10;

    static_assert(NUM_LINES * NUM_COLUMNS == game::map::Drawing::NUM_USER_COLORS, "NUM_LINES*NUM_COLUMNS = NUM_USER_COLORS");

    gfx::Point getCellSize(ui::Root& root)
    {
        // ex WColorSelector::getFrameSize (sort-of)
        int em = root.provider().getFont("+")->getLineHeight();
        return gfx::Point(em, em);
    }
}

client::widgets::MarkerColorSelector::MarkerColorSelector(ui::Root& root)
    : IconGrid(root.engine(), getCellSize(root), NUM_COLUMNS, NUM_LINES),
      m_root(root),
      m_deleter()
{
    gfx::Point cellSize = getCellSize(root);
    for (int i = 0; i < NUM_LINES*NUM_COLUMNS; ++i) {
        addIcon(&m_deleter.addNew(new ui::icons::ColorTile(root, cellSize, client::map::getUserColor(i+1))));
    }
    setPadding(1);
}

client::widgets::MarkerColorSelector::~MarkerColorSelector()
{ }

void
client::widgets::MarkerColorSelector::setColor(uint8_t color)
{
    if (color > 0 && color <= NUM_LINES*NUM_COLUMNS) {
        setCurrentItem(color-1);
    }
}

uint8_t
client::widgets::MarkerColorSelector::getColor() const
{
    return static_cast<uint8_t>(getCurrentItem()+1);
}

bool
client::widgets::MarkerColorSelector::doStandardDialog(String_t title, afl::string::Translator& tx, bool* adjacent)
{
    // ex WColorSelector::doStandardDialog, chartdlg.pas:SetObjColor
    const int CANCEL = 0, OK = 1, ADJACENT = 2;

    afl::base::Deleter del;
    ui::EventLoop loop(m_root);

    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::StaticText(tx("Choose color:"), util::SkinColor::Static, gfx::FontRequest("+"), m_root.provider())));
    win.add(*this);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));

    ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, m_root));
    btnOK.sig_fire.addNewClosure(loop.makeStop(OK));
    g.add(btnOK);

    if (adjacent != 0) {
        ui::widgets::Button& btnAdjacent = del.addNew(new ui::widgets::Button(tx("Adjacent"), 'a', m_root));
        btnAdjacent.sig_fire.addNewClosure(loop.makeStop(ADJACENT));
        g.add(btnAdjacent);
    }

    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx("Cancel"), util::Key_Escape, m_root));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(CANCEL));
    g.add(btnCancel);
    win.add(g);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.pack();

    setColor(getColor());

    m_root.centerWidget(win);
    m_root.add(win);

    int result = loop.run();
    if (adjacent) {
        *adjacent = (result == ADJACENT);
    }

    return (result != CANCEL);
}
