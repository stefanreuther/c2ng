/**
  *  \file ui/reshack/dialogs.cpp
  *  \brief Class ui::reshack::Dialogs
  */

#include "ui/reshack/dialogs.hpp"

#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/reshack/fontutil.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/charsetfactory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::string::Format;
using afl::string::Translator;
using gfx::BitmapFont;
using gfx::PalettizedPixmap;
using ui::EventLoop;
using ui::Root;
using ui::reshack::Info;
using ui::reshack::Session;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using ui::widgets::InputLine;
using ui::widgets::Quit;
using ui::widgets::StaticText;
using util::SkinColor;

/*
 *  showCoverage
 */

namespace {
    class FontCoverageList : public ui::widgets::AbstractListbox {
     public:
        FontCoverageList(Session& session, const std::vector<Info::Coverage>& data)
            : m_session(session), m_data(data)
            { }
        virtual size_t getNumItems() const
            { return m_data.size(); }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
        virtual int getItemHeight(size_t /*n*/) const
            { return getFont()->getLineHeight(); }
        virtual int getHeaderHeight() const
            { return 0; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
            {
                // RHFontConverageWidget::drawContent(GfxCanvas& can)
                afl::base::Deleter del;

                gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
                ctx.useFont(*getFont());
                prepareColorListItem(ctx, area, state, m_session.root().colorScheme(), del);

                if (item < m_data.size()) {
                    const Info::Coverage& it = m_data[item];

                    ctx.setColor(SkinColor::Static);
                    outTextF(ctx, area.splitX(area.getWidth()/2), Format(m_session.translator()("%s: %d missing"), it.charsetName, it.numMissingCharacters));

                    if (it.numMissingCharacters == 0) {
                        ctx.setColor(SkinColor::Green);
                        outTextF(ctx, area, m_session.translator()("complete"));
                    } else {
                        ctx.setColor(SkinColor::Red);
                        outTextF(ctx, area, m_session.characterNames().getCharacterName(it.firstMissingCharacter));
                    }
                }
            }
        virtual void handlePositionChange()
            { return defaultHandlePositionChange(); }
        virtual ui::layout::Info getLayoutInfo() const
            { return getFont()->getCellSize().scaledBy(30, int(m_data.size())); }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
     private:
        Session& m_session;
        const std::vector<Info::Coverage>& m_data;

        Ref<gfx::Font> getFont() const
            { return m_session.root().provider().getFont(gfx::FontRequest()); }
    };
}

afl::base::Optional<afl::charset::Unichar_t>
ui::reshack::Dialogs::showCoverage(Session& session, std::vector<Info::Coverage> data)
{
    // doFontCoverageDialog(const GfxBitmapFont& font, uint32_t& result)
    FontCoverageList list(session, data);
    if (ui::widgets::doStandardDialog(session.translator()("Character Coverage"), "", list, false, session.root(), session.translator())) {
        size_t idx = list.getCurrentItem();
        if (idx < data.size() && data[idx].numMissingCharacters != 0) {
            return data[idx].firstMissingCharacter;
        }
    }
    return afl::base::Nothing;
}

/*
 *  showPreview
 */

namespace {
    bool makeSentence(BitmapFont& font, uint32_t line, String_t& out)
    {
        afl::charset::Unichar_t chr = 16*line;
        bool ok = false;
        out.clear();
        for (uint32_t i = 0; i < 16; ++i) {
            if (font.getGlyph(chr + i) != 0) {
                ok = true;
                afl::charset::Utf8().append(out, chr + i);
            }
        }
        return ok;
    }

    class PreviewWidget : public ui::SimpleWidget {
     public:
        PreviewWidget(BitmapFont& font)
            : m_font(font), m_line(0), m_text()
            { onNextSentence(); }
        virtual void draw(gfx::Canvas& can)
            {
                // RHPreviewWidget::drawContent(GfxCanvas& can)
                gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
                ctx.useFont(m_font);
                ctx.setColor(SkinColor::Static);
                ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
                drawBackground(ctx, getExtent());
                outTextF(ctx, getExtent(), m_text);
            }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void handlePositionChange()
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return m_font.getCellSize().scaledBy(16, 1); }
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
        void onNextSentence()
            {
                // RHPreviewWidget::onNextSentence()
                String_t newText;
                while (m_line < 65536/16) {
                    if (makeSentence(m_font, m_line, newText) && newText != m_text) {
                        break;
                    }
                    ++m_line;
                }
                setText(newText);
            }
        void onPreviousSentence()
            {
                // RHPreviewWidget::onPreviousSentence()
                String_t newText;
                while (m_line > 0) {
                    if (makeSentence(m_font, m_line, newText) && newText != m_text) {
                        break;
                    }
                    --m_line;
                }
                setText(newText);
            }
        void setText(String_t text)
            {
                // RHPreviewWidget::setText(string_t text)
                m_text = text;
                requestRedraw();
            }
     private:
        BitmapFont& m_font;
        uint32_t m_line;
        String_t m_text;
    };

    class PreviewWindow {
     public:
        PreviewWindow(Session& session, BitmapFont& font)
            : m_session(session),
              m_preview(font),
              m_input(50, 30, m_session.root())
            { }

        void run()
            {
                // RHPreviewWindow::init()
                Translator& tx = m_session.translator();
                Root& root = m_session.root();
                afl::base::Deleter del;

                ui::Window& win = del.addNew(new ui::Window(tx("Preview"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(m_preview);
                win.add(m_input);
                m_input.setText(m_session.getPreviewText());

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                Button& btnNext  = del.addNew(new Button(tx("Next"),     util::KeyMod_Alt + 'n', root));
                Button& btnPrev  = del.addNew(new Button(tx("Previous"), util::KeyMod_Alt + 'p', root));
                Button& btnSet   = del.addNew(new Button(tx("Set"),      util::Key_Return, root));
                Button& btnClose = del.addNew(new Button(tx("Close"),    util::Key_Escape, root));
                g.add(btnNext);
                g.add(btnPrev);
                g.add(btnSet);
                g.add(del.addNew(new ui::Spacer()));
                g.add(btnClose);
                win.add(g);

                EventLoop loop(root);
                btnNext.sig_fire.add(&m_preview, &PreviewWidget::onNextSentence);
                btnPrev.sig_fire.add(&m_preview, &PreviewWidget::onPreviousSentence);
                btnSet.sig_fire.add(this, &PreviewWindow::onCopy);
                btnClose.sig_fire.addNewClosure(loop.makeStop(0));

                win.add(del.addNew(new Quit(root, loop)));

                win.pack();
                root.centerWidget(win);
                root.add(win);
                m_input.requestFocus();
                loop.run();
            }

        void onCopy()
            {
                // RHPreviewWindow::onCopy()
                m_preview.setText(m_input.getText());
                m_session.setPreviewText(m_input.getText());
            }

     private:
        Session& m_session;
        PreviewWidget m_preview;
        InputLine m_input;
    };
}

void
ui::reshack::Dialogs::showPreview(Session& session, gfx::BitmapFont& font)
{
    // showPreview(GfxBitmapFont& font)
    // Font cannot be const because it is used by BaseContext::useFont which uses non-const to avoid accidental passing of temporaries.
    PreviewWindow(session, font).run();
}

/*
 *  showCharacterInSystemFonts
 */

namespace {
    class SystemFontList : public ui::widgets::AbstractListbox {
     public:
        SystemFontList(Session& session, String_t text)
            : m_session(session), m_text(text)
            { }
        virtual size_t getNumItems() const
            { return m_session.fontList().getNumFonts(); }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
        virtual int getItemHeight(size_t n) const
            {
                int size = 10;
                if (gfx::Font* f = m_session.fontList().getFontByIndex(n).get()) {
                    size = std::max(size, f->getLineHeight());
                }
                return size;
            }
        virtual int getHeaderHeight() const
            { return 0; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
            {
                // RHSystemCharacterWidget::drawContent(GfxCanvas& can) (sort-of)
                afl::base::Deleter del;
                gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_session.root().colorScheme(), del);

                if (gfx::Font* f = m_session.fontList().getFontByIndex(item).get()) {
                    ctx.useFont(*f);
                    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
                    outTextF(ctx, area, m_text);
                }
            }
        virtual void handlePositionChange()
            { return defaultHandlePositionChange(); }
        virtual ui::layout::Info getLayoutInfo() const
            {
                int width = 10;
                int height = 0;
                for (size_t i = 0, n = m_session.fontList().getNumFonts(); i < n; ++i) {
                    gfx::Font* f = m_session.fontList().getFontByIndex(i).get();
                    width = std::max(width, 2 * f->getEmWidth() + f->getTextWidth(m_text));
                    height += std::max(10, f->getLineHeight());
                }
                return gfx::Point(width, height);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
     private:
        Session& m_session;
        String_t m_text;
    };
}

gfx::Font*
ui::reshack::Dialogs::showCharacterInSystemFonts(Session& session, Unichar_t ch)
{
    // showCharacterInSystemFonts(uint32_t ch)
    String_t text;
    afl::charset::Utf8().append(text, ch);

    SystemFontList list(session, text);
    if (ui::widgets::doStandardDialog(session.translator()("System Fonts"), "", list, false, session.root(), session.translator())) {
        return session.fontList().getFontByIndex(list.getCurrentItem()).get();
    }
    return 0;
}

/*
 *  changeFontAlignment
 */

namespace {
    class FontAlignWindow {
     public:
        FontAlignWindow(Session& session)
            : m_session(session),
              m_loop(session.root()),
              m_topInput(10, session.root()),
              m_bottomInput(10, session.root()),
              m_topResult(),
              m_bottomResult()
            {
                m_topInput.setHotkey('t' + util::KeyMod_Alt);
                m_bottomInput.setHotkey('b' + util::KeyMod_Alt);
            }

        bool run(std::pair<int,int> dim)
            {
                // RHFontAlignWindow::init(int top, int bot)
                // VBox
                //   Grid
                //     "Top:"    InputLine
                //     "Bottom:" InputLine
                //   DocumentView
                //   StandardDialogButtons
                Translator& tx = m_session.translator();
                Root& root = m_session.root();
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(tx("Vertical Alignment"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                ui::Group& g = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));
                g.add(del.addNew(new StaticText(tx("Top:"), SkinColor::Static, "+", root.provider())));
                g.add(m_topInput);
                g.add(del.addNew(new StaticText(tx("Bottom:"), SkinColor::Static, "+", root.provider())));
                g.add(m_bottomInput);
                win.add(g);

                gfx::Point cellSize = root.provider().getFont(gfx::FontRequest())->getCellSize();
                ui::rich::DocumentView& doc = del.addNew(new ui::rich::DocumentView(cellSize.scaledBy(25, 2), 0, root.provider()));
                doc.getDocument().setPageWidth(25 * cellSize.getX());
                doc.getDocument().add(Format(tx("Enter number of pixels to add at top/bottom. "
                                                "Negative numbers can be used to remove pixels. "
                                                "Maximum you can remove without clipping from the top is "
                                                "%d pixels on the top, %d pixels at the bottom."),
                                             dim.first, dim.second));
                doc.getDocument().finish();
                doc.adjustToDocumentSize();
                win.add(doc);

                ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
                btn.ok().sig_fire.add(this, &FontAlignWindow::onOK);
                btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
                win.add(btn);

                FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Tab | FocusIterator::Vertical));
                it.add(m_topInput);
                it.add(m_bottomInput);
                win.add(it);
                win.add(del.addNew(new Quit(root, m_loop)));

                win.pack();
                root.centerWidget(win);
                root.add(win);
                return m_loop.run() != 0;
            }

        int getTop() const
            { return m_topResult; }

        int getBottom() const
            { return m_bottomResult; }

     private:
        Session& m_session;
        EventLoop m_loop;
        InputLine m_topInput;
        InputLine m_bottomInput;
        int m_topResult;
        int m_bottomResult;

        void onOK()
            {
                // RHFontAlignWindow::onOK()
                if (!afl::string::strToInteger(m_topInput.getText(), m_topResult)) {
                    m_topInput.requestFocus();
                    return;
                }
                if (!afl::string::strToInteger(m_bottomInput.getText(), m_bottomResult)) {
                    m_bottomInput.requestFocus();
                    return;
                }
                m_loop.stop(1);
            }
    };
}

void
ui::reshack::Dialogs::changeFontAlignment(Session& session, gfx::BitmapFont& font)
{
    FontAlignWindow win(session);
    if (win.run(FontUtil::findFontMargins(font))) {
        if (!FontUtil::changeFontAlignment(font, win.getTop(), win.getBottom())) {
            Translator& tx = session.translator();
            ui::dialogs::MessageBox(tx("The values you entered make the new font have a height of 0 or less."), tx("Vertical Alignment"), session.root())
                .doOkDialog(tx);
        }
    }
}

/*
 *  synthesizeCharacter
 */

namespace {
    class CharacterGenerator : public util::CharacterNameList::Generator {
     public:
        CharacterGenerator(Session& session, const BitmapFont& font)
            : m_session(session), m_font(font), m_result()
            { }

        bool check(afl::base::Memory<const afl::charset::Unichar_t> set)
            {
                // Check whether we have the required characters, and generate prompt on the fly
                Translator& tx = m_session.translator();
                String_t msg = tx("Generate this character from the following?");
                for (size_t i = 0; i < set.size(); ++i) {
                    afl::charset::Unichar_t ch = *set.at(i);
                    if (m_font.getGlyph(ch) == 0) {
                        return false;
                    }
                    msg += "\n";
                    msg += m_session.characterNames().getCharacterName(ch);
                }

                // Generate
                Ptr<PalettizedPixmap> result = ui::reshack::FontUtil::synthesizeCombinedCharacter(m_font, set);
                if (result.get() == 0) {
                    return false;
                }

                // Ask user
                if (!ui::dialogs::MessageBox(msg, tx("Auto-Generate Character"), m_session.root()).doYesNoDialog(tx)) {
                    return false;
                }

                // Accept
                m_result = result;
                return true;
            }

        const Ptr<PalettizedPixmap> getResult() const
            { return m_result; }

     private:
        Session& m_session;
        const BitmapFont& m_font;
        Ptr<PalettizedPixmap> m_result;
    };
}


afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::Dialogs::synthesizeCharacter(Session& session, const gfx::BitmapFont& font, Unichar_t ch)
{
    // Try combined
    CharacterGenerator gen(session, font);
    if (session.characterNames().generateCharacter(ch, gen)) {
        return gen.getResult();
    }

    // Try fully synthetic
    Ptr<PalettizedPixmap> result = FontUtil::synthesizeGraphicCharacter(font, ch);
    if (result.get() == 0) {
        Translator& tx = session.translator();
        ui::dialogs::MessageBox(tx("I do not know how to generate this character."), tx("Auto-Generate Character"), session.root())
            .doOkDialog(tx);
    }
    return result;
}

void
ui::reshack::Dialogs::changeFontEncoding(Session& session, gfx::BitmapFont& font)
{
    Translator& tx = session.translator();
    afl::container::PtrVector<afl::charset::CodepageCharset> codepages;
    util::CharsetFactory f;
    ui::widgets::StringListbox box(session.root().provider(), session.root().colorScheme());
    for (util::CharsetFactory::Index_t i = 0; i < f.getNumCharsets(); ++i) {
        std::auto_ptr<afl::charset::Charset> p(f.createCharsetByIndex(i));
        if (afl::charset::CodepageCharset* cp = dynamic_cast<afl::charset::CodepageCharset*>(p.get())) {
            p.release();
            codepages.pushBackNew(cp);
            box.addItem(static_cast<int>(codepages.size()-1), Format(tx("%s to Unicode"), f.getCharsetName(i, tx)));
        }
    }

    if (box.doStandardDialog(tx("Change Encoding"), "", 0, session.root(), tx)) {
        size_t i = static_cast<size_t>(box.getCurrentKey().orElse(-1));
        if (i < codepages.size()) {
            FontUtil::changeFontEncoding(font, codepages[i]->get());
        }
    }
}
