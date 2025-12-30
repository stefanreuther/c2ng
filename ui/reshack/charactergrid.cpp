/**
  *  \file ui/reshack/charactergrid.cpp
  *  \brief Character Grid Dialog
  */

#include "ui/reshack/charactergrid.hpp"
#include "gfx/bitmapglyph.hpp"
#include "gfx/complex.hpp"
#include "ui/icons/icon.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/reshack/characternamewidget.hpp"
#include "ui/widgets/icongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"

using gfx::BitmapFont;
using gfx::BitmapGlyph;
using afl::charset::Unichar_t;
using ui::reshack::Session;
using ui::widgets::IconGrid;

namespace {
    /*
     *  CharacterIcon - an Icon implementation that displays a single character
     */
    class CharacterIcon : public ui::icons::Icon {
     public:
        CharacterIcon(const BitmapFont& font, Unichar_t characterCode)
            : m_font(font),
              m_characterCode(characterCode)
            { }

        // Size; does not matter because it's overriden by containing widget
        virtual gfx::Point getSize() const
            { return gfx::Point(16, 16); }

        // Draw it
        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::ButtonFlags_t flags) const
            {
                // RHCharacterGrid::drawIcon(GfxCanvas& can, const GfxRect& pos, int x, int y, FocusState state)
                if (!flags.contains(ui::FocusedButton)) {
                    drawBackground(ctx, area);
                    ctx.setColor(util::SkinColor::Static);
                } else {
                    drawSolidBar(ctx, area, util::SkinColor::Static);
                    ctx.setColor(util::SkinColor::InvStatic);
                }

                if (const BitmapGlyph* g = m_font.getGlyph(m_characterCode)) {
                    g->draw(ctx, gfx::Point(area.getLeftX() + (area.getWidth() - g->getWidth()) / 2, area.getTopY() + 1));
                } else {
                    ctx.setColor(util::SkinColor::Faded);
                    drawLine(ctx, gfx::Point(area.getLeftX() + 1, area.getTopY() + 1),                    gfx::Point(area.getLeftX() + area.getWidth() - 1, area.getTopY() + area.getHeight() - 2));
                    drawLine(ctx, gfx::Point(area.getLeftX() + 1, area.getTopY() + area.getHeight() - 2), gfx::Point(area.getLeftX() + area.getWidth() - 1, area.getTopY() + 1));
                }
            }
     private:
        const BitmapFont& m_font;
        const Unichar_t m_characterCode;
    };

    /*
     *  Data - prepared dialog data
     */
    struct Data {
        int width;
        std::vector<Unichar_t> firsts;
        afl::base::Deleter del;
    };

    /*
     *  Dialog - hosts callbacks for running the dialog
     */
    class Dialog {
     public:
        Dialog(Session& session, IconGrid& grid, const Data& d)
            : m_session(session),
              m_grid(grid),
              m_characterName(session.root(), session.characterNames()),
              m_data(d)
            {
                // Note: for a re-usable component, we'd have to unregister this listener on destruction
                m_grid.sig_itemSelected.add(this, &Dialog::onItemSelected);
            }

        /** Get currently selected character number. */
        afl::base::Optional<Unichar_t> getCurrentCharacter() const
            {
                // RHCharacterGrid::getCurrentCharacter()
                int y = m_grid.getCurrentLine();
                if (y >= 0 && static_cast<size_t>(y) < m_data.firsts.size()) {
                    return m_data.firsts[y] + m_grid.getCurrentColumn();
                } else {
                    return afl::base::Nothing;
                }
            }

        /** Move cursor to a character.
            If that character is not visible because it has no glyph, goes to a character nearby. */
        void setCurrentCharacter(Unichar_t n)
            {
                // RHCharacterGrid::setCurrentCharacter(uint32_t n)
                if (!m_data.firsts.empty()) {
                    size_t line = 0;
                    int col = 0;
                    while (line < m_data.firsts.size()-1 && n >= m_data.firsts[line+1]) {
                        ++line;
                    }
                    if (n >= m_data.firsts[line] && n < m_data.firsts[line] + m_data.width) {
                        col = n - m_data.firsts[line];
                    }
                    m_grid.setCurrentItem(col, static_cast<int>(line));
                }
            }

        bool run(String_t title)
            {
                // RHCharacterGridDialog::init(const GfxBitmapFont& font)
                ui::Root& root = m_session.root();
                ui::Window win(title, root.provider(), root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
                win.add(m_grid);
                win.add(m_characterName);

                ui::EventLoop loop(root);
                ui::widgets::StandardDialogButtons btn(root, m_session.translator());
                btn.addStop(loop);
                win.add(btn);

                ui::widgets::Quit quit(root, loop);
                win.add(quit);

                onItemSelected();

                win.pack();
                root.centerWidget(win);
                root.add(win);
                return loop.run() != 0;
            }

        void onItemSelected()
            {
                // RHCharacterGridDialog::onMove()
                Unichar_t ch;
                if (getCurrentCharacter().get(ch)) {
                    m_characterName.setCharacter(ch);
                }
            }

     private:
        Session& m_session;
        IconGrid& m_grid;
        ui::reshack::CharacterNameWidget m_characterName;
        const Data& m_data;
    };
}

/*
 *  Main Entry Point
 */

afl::base::Optional<afl::charset::Unichar_t>
ui::reshack::pickCharacterFromGrid(Session& session, String_t title, const gfx::BitmapFont& font, afl::charset::Unichar_t current)
{
    // RHCharacterGrid::init() - part
    // RHCharacterGridDialog::doDialog(const GfxBitmapFont& font, string_t title, uint32_t& result)

    /* In PCC2, RHCharacterGrid was a re-usable component (used once).
       Due to the way our IconGrid works, this implementation must differ completely. */

    // Figure out cell size
    int minX = 10, minY = 10;
    for (Unichar_t i = 0, end = font.getCurrentCharacterLimit(); i < end; ++i) {
        if (const BitmapGlyph* g = font.getGlyph(i)) {
            if (g->getWidth() > minX) {
                minX = g->getWidth();
            }
            if (g->getHeight() > minY) {
                minY = g->getHeight();
            }
        }
    }
    minX += 2;
    minY += 2;

    // Figure out grid size
    Data d;
    d.width = 8;
    while (d.width < 32 && minX * d.width < session.root().getExtent().getWidth() / 3) {
        d.width *= 2;
    }

    // Populate "firsts" array
    bool prevLine = false;
    for (Unichar_t i = 0, end = font.getCurrentCharacterLimit(); i < end; i += d.width) {
        // Anything on this line?
        bool thisLine = false;
        for (int j = 0; j < d.width; ++j) {
            if (font.getGlyph(i + j)) {
                thisLine = true;
            }
        }

        // Show this line if anything is on it, or on the previous one (spacer)
        if (thisLine || prevLine) {
            d.firsts.push_back(i);
        }
        prevLine = thisLine;
    }
    if (d.firsts.empty()) {
        d.firsts.push_back(32);
    }

    // Create icon grid widget
    int height = std::min(session.root().getExtent().getHeight() * 3/4 / minY, static_cast<int>(d.firsts.size()));
    IconGrid grid(session.root().engine(), gfx::Point(minX, minY), d.width, height);

    // Populate icon grid widget
    for (size_t i = 0; i < d.firsts.size(); ++i) {
        for (int j = 0; j < d.width; ++j) {
            grid.addIcon(&d.del.addNew(new CharacterIcon(font, d.firsts[i]+j)));
        }
    }

    // Dialog
    Dialog dlg(session, grid, d);
    dlg.setCurrentCharacter(current);
    if (dlg.run(title)) {
        return dlg.getCurrentCharacter();
    } else {
        return afl::base::Nothing;
    }
}
