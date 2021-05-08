/**
  *  \file client/map/markeroverlaybase.cpp
  */

#include "client/map/markeroverlaybase.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/map/screen.hpp"
#include "client/widgets/markercolorselector.hpp"

client::map::MarkerOverlayBase::MarkerOverlayBase(ui::Root& root, afl::string::Translator& tx, Screen& screen, const game::map::Drawing& drawing)
    : m_root(root),
      m_translator(tx),
      m_screen(screen),
      m_drawing(drawing),
      conn_drawingUpdate(screen.drawingProxy().sig_update.add(this, &MarkerOverlayBase::onDrawingUpdate))
{ }

const game::map::Drawing&
client::map::MarkerOverlayBase::drawing() const
{
    return m_drawing;
}

client::map::Screen&
client::map::MarkerOverlayBase::screen() const
{
    return m_screen;
}

ui::Root&
client::map::MarkerOverlayBase::root() const
{
    return m_root;
}

afl::string::Translator&
client::map::MarkerOverlayBase::translator() const
{
    return m_translator;
}

bool
client::map::MarkerOverlayBase::defaultHandleKey(util::Key_t key, int /*prefix*/, const Renderer& /*ren*/)
{
    // ex WMarkerChartMode::handleEvent
    switch (key) {
     case util::Key_Escape:
     case util::Key_Quit:       // FIXME: should be re-posted?
        // Exit move mode
        m_screen.removeOverlay(this);
        return true;

     case 'c':
        editColor();
        return true;

     case util::Key_Delete:
        // Delete marker
        m_screen.drawingProxy().erase(false);
        m_screen.removeOverlay(this);
        return true;

     case 'h':
     case util::KeyMod_Alt + 'h':
        client::dialogs::doHelpDialog(m_root, m_translator, m_screen.gameSender(), "pcc2:draw");
        return true;

     default:
        return false;
    }
}

void
client::map::MarkerOverlayBase::onDrawingUpdate(const Status_t& st)
{
    if (const game::map::Drawing* p = st.get()) {
        m_drawing = *p;
    } else {
        m_screen.removeOverlay(this);
    }
}

void
client::map::MarkerOverlayBase::editColor()
{
    client::widgets::MarkerColorSelector csel(m_root);
    csel.setColor(m_drawing.getColor());
    if (csel.doStandardDialog(m_translator("Drawing Color"), m_translator, 0)) {
        m_screen.drawingProxy().setColor(csel.getColor(), false);
    }
}
