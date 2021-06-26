/**
  *  \file ui/widgets/editor.cpp
  *  \brief Class ui::widgets::Editor
  */

#include "ui/widgets/editor.hpp"
#include "afl/charset/utf8.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"
#include "util/math.hpp"
#include "util/updater.hpp"

using util::Updater;

namespace {
    /*
     *  Color Configuration
     */
    const uint8_t Color_Background = ui::Color_Black;
    const uint8_t Color_Text = ui::Color_White;
    const uint8_t Color_Cursor = ui::Color_Yellow;
}


ui::widgets::Editor::Editor(util::editor::Editor& ed, Root& root)
    : m_editor(ed),
      m_editorFlags(),
      m_preferredSize(100, 100),
      m_root(root),
      m_firstColumn(0),
      m_firstLine(0),
      m_allowScrolling(true),
      conn_editorChange(ed.sig_change.add(this, &Editor::onEditorChange))
{ }

ui::widgets::Editor::~Editor()
{ }

void
ui::widgets::Editor::setPreferredSize(gfx::Point size)
{
    m_preferredSize = size;
}

void
ui::widgets::Editor::setPreferredSizeInCells(size_t columns, size_t lines)
{
    m_preferredSize = getFont()->getCellSize().scaledBy(int(columns)+1, int(lines));
}

void
ui::widgets::Editor::setFirstColumn(size_t fc)
{
    if (Updater().set(m_firstColumn, fc)) {
        requestRedraw();
    }
}

void
ui::widgets::Editor::setFirstLine(size_t fl)
{
    if (Updater().set(m_firstLine, fl)) {
        requestRedraw();
    }
}

void
ui::widgets::Editor::setAllowScrolling(bool flag)
{
    m_allowScrolling = flag;
}

void
ui::widgets::Editor::setFlag(util::editor::Flag flag, bool enable)
{
    m_editorFlags.set(flag, enable);
    if (flag == util::editor::Overwrite) {
        size_t line = m_editor.getCurrentLine();
        onEditorChange(line, line);
    }
}

void
ui::widgets::Editor::toggleFlag(util::editor::Flag flag)
{
    setFlag(flag, !m_editorFlags.contains(flag));
}

void
ui::widgets::Editor::draw(gfx::Canvas& can)
{
    gfx::Rectangle area = getExtent();
    gfx::ClipFilter clip(can, area);
    gfx::Context<uint8_t> ctx(clip, m_root.colorScheme());
    afl::base::Ref<gfx::Font> font(getFont());
    const gfx::Point cellSize = getFont()->getCellSize();
    if (cellSize.getX() <= 0 || cellSize.getY() <= 0) {
        return;
    }

    const afl::charset::Utf8 u8(0);

    ctx.useFont(*font);
    const size_t numLines = util::divideAndRoundUp(area.getHeight(), cellSize.getY());
    for (size_t i = 0; i < numLines; ++i) {
        gfx::Rectangle lineArea = area.splitY(cellSize.getY());
        if (clip.isVisible(lineArea)) {
            // Background
            drawSolidBar(ctx, lineArea, Color_Background);

            // Text
            ctx.setColor(Color_Text);
            String_t text = m_editor.getLineText(m_firstLine + i);
            text.erase(0, u8.charToBytePos(text, m_firstColumn));
            outText(ctx, lineArea.getTopLeft(), text);

            // Cursor
            if (m_firstLine + i == m_editor.getCurrentLine()) {
                size_t x = m_editor.getCurrentColumn();
                if (x >= m_firstColumn) {
                    x -= m_firstColumn;
                    ctx.setColor(Color_Cursor);

                    const int cw = cellSize.getX(), ch = cellSize.getY();
                    if (m_editorFlags.contains(util::editor::Overwrite)) {
                        drawRectangle(ctx, gfx::Rectangle(lineArea.getLeftX() + cw*int(x), lineArea.getTopY() + ch/2,     cw-1, 5*ch/16));
                    } else {
                        drawRectangle(ctx, gfx::Rectangle(lineArea.getLeftX() + cw*int(x), lineArea.getTopY() + 11*ch/16, cw-1, 2*ch/16));
                    }
                }
            }
        }
    }
}

void
ui::widgets::Editor::handleStateChange(State st, bool enable)
{
    (void) st;
    (void) enable;
}

void
ui::widgets::Editor::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
ui::widgets::Editor::getLayoutInfo() const
{
    return ui::layout::Info(m_preferredSize, m_preferredSize, ui::layout::Info::GrowBoth);
}

bool
ui::widgets::Editor::handleKey(util::Key_t key, int prefix)
{
    // Editor command?
    util::editor::Command cmd;
    if (lookupKey(key, cmd)) {
        switch (cmd) {
         case util::editor::cToggleInsert:
            requestActive();
            toggleFlag(util::editor::Overwrite);
            return true;

         case util::editor::cToggleWrap:
            requestActive();
            toggleFlag(util::editor::WordWrap);
            return true;

         default:
            if (m_editor.handleCommand(m_editorFlags, cmd)) {
                requestActive();
                return true;
            }
            break;
        }
    }

    if ((key & util::KeyMod_Mask) == 0 && (key < util::Key_FirstSpecial) /*&& acceptUnicode(key)*/) {
        /* Self-insert */
        String_t n;
        afl::charset::Utf8(0).append(n, key);
        requestActive();
        m_editor.handleInsert(m_editorFlags, n);
        return true;
    }

    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::Editor::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestActive();
        gfx::Point cellSize = getFont()->getCellSize();
        if (cellSize.getX() > 0 && cellSize.getY() > 0) {
            m_editor.setCursor(m_firstLine   + (pt.getY() - getExtent().getTopY())  / cellSize.getY(),
                               m_firstColumn + (pt.getX() - getExtent().getLeftX()) / cellSize.getX());
        }
        return true;
    } else {
        return defaultHandleMouse(pt, pressedButtons);
    }
}

void
ui::widgets::Editor::onEditorChange(size_t firstLine, size_t lastLine)
{
    gfx::Point cellSize = getFont()->getCellSize();
    if (cellSize.getX() > 0 && cellSize.getY() > 0) {
        // Dimensions
        const size_t numLines   = size_t(util::divideAndRoundUp(getExtent().getHeight(), cellSize.getY()));
        const size_t numColumns = size_t(util::divideAndRoundUp(getExtent().getWidth(), cellSize.getX()));

        // Do we need to scroll?
        if (m_allowScrolling) {
            const size_t x = m_editor.getCurrentColumn();
            if (x < m_firstColumn) {
                setFirstColumn(x);
            } else if (x - m_firstColumn >= numColumns) {
                setFirstColumn(x - numColumns - 1);
            }

            const size_t y = m_editor.getCurrentLine();
            if (y < m_firstLine) {
                setFirstLine(y);
            } else if (y - m_firstLine >= numLines) {
                setFirstLine(y - numLines + 1);
            }
        }

        // Redraw updated range
        lastLine = std::min(numLines + m_firstLine, lastLine);
        firstLine = std::max(firstLine, m_firstLine);
        if (lastLine >= firstLine) {
            gfx::Rectangle area = getExtent();
            area.intersect(gfx::Rectangle(area.getLeftX(),
                                          area.getTopY() + cellSize.getY() * int(firstLine - m_firstLine),
                                          area.getWidth(),
                                          cellSize.getY() * int(lastLine - firstLine + 1)));
            requestRedraw(area);
        }
    }
}

afl::base::Ref<gfx::Font>
ui::widgets::Editor::getFont()
{
    return m_root.provider().getFont(gfx::FontRequest().setStyle(FixedFont));
}
