/**
  *  \file ui/reshack/fonteditor.cpp
  */

#include "ui/reshack/fonteditor.hpp"

#include "afl/base/deleter.hpp"
#include "afl/base/observable.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "gfx/bitmapglyph.hpp"
#include "gfx/codec/custom.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/reshack/brushtool.hpp"
#include "ui/reshack/charactergrid.hpp"
#include "ui/reshack/characternamelistbox.hpp"
#include "ui/reshack/characternamewidget.hpp"
#include "ui/reshack/circletool.hpp"
#include "ui/reshack/clipboard.hpp"
#include "ui/reshack/dialogs.hpp"
#include "ui/reshack/fontcolorselector.hpp"
#include "ui/reshack/fontutil.hpp"
#include "ui/reshack/info.hpp"
#include "ui/reshack/linetool.hpp"
#include "ui/reshack/painter.hpp"
#include "ui/reshack/penciltool.hpp"
#include "ui/reshack/rectangletool.hpp"
#include "ui/reshack/session.hpp"
#include "ui/reshack/toolselector.hpp"
#include "ui/reshack/zoomselector.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::string::Format;
using gfx::BitmapFont;
using gfx::BitmapGlyph;
using gfx::PalettizedPixmap;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using ui::widgets::FrameGroup;

/*
 *  CharacterListDialog - show list of characters by name and let user choose one
 */

class ui::reshack::FontEditor::CharacterListDialog {
 public:
    CharacterListDialog(Session& session)
        : m_session(session),
          m_input(1000, 20, session.root()),
          m_buttons(session.root(), session.translator()),
          m_list(session.root(), session.characterNames())
        {
            m_input.sig_change.add(this, &CharacterListDialog::onType);
        }

    afl::base::Optional<afl::charset::Unichar_t> run(String_t title);

 private:
    void onType();

    Session& m_session;
    ui::widgets::InputLine m_input;
    ui::widgets::StandardDialogButtons m_buttons;
    CharacterNameListbox m_list;
};


afl::base::Optional<afl::charset::Unichar_t>
ui::reshack::FontEditor::CharacterListDialog::run(String_t title)
{
    // RHPickCharacterDialog::init()
    Root& root = m_session.root();
    afl::base::Deleter del;

    Window& win = del.addNew(new Window(title, root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));

    win.add(FrameGroup::wrapWidget(del, root.colorScheme(), LoweredFrame, m_input));
    win.add(FrameGroup::wrapWidget(del, root.colorScheme(), LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_list, root))));
    win.add(m_buttons);

    FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Tab));
    it.add(m_input);
    it.add(m_list);
    win.add(it);

    EventLoop loop(root);
    m_buttons.addStop(loop);

    onType();

    win.add(del.addNew(new ui::widgets::Quit(root, loop)));
    win.pack();
    root.centerWidget(win);
    root.add(win);

    int result = loop.run();
    if (result != 0 && m_list.getNumItems() != 0) {
        return m_list.getCurrentCharacter();
    } else {
        return afl::base::Nothing;
    }
}

void
ui::reshack::FontEditor::CharacterListDialog::onType()
{
    // RHPickCharacterDialog::onType()
    m_list.setCharacters(m_session.characterNames().searchCharactersByName(m_input.getText(), 0xFFFF));
    m_buttons.ok().setState(Widget::DisabledState, m_list.getNumItems() == 0);
}


/*
 *  FontEditorWindow - main window
 */

class ui::reshack::FontEditor::FontEditorWindow : private gfx::KeyEventConsumer {
 public:
    FontEditorWindow(Session& session, Ptr<BitmapFont> font, String_t fileName)
        : m_session(session), m_font(font), m_fileName(fileName), m_currentCharacter(),
          m_painter(FontUtil::createPixmapFromGlyph(*m_font, m_currentCharacter), Palette::GrayscaleColor, session.root()),
          m_characterName(session.root(), session.characterNames())
        {
            loadCurrentCharacter();
        }

    void run();

 private:
    virtual bool handleKey(util::Key_t key, int prefix);

    void setCurrentCharacter(afl::charset::Unichar_t ch);
    void loadCurrentCharacter();
    void storeCurrentCharacter();

    afl::base::Optional<afl::charset::Unichar_t> pickCharacter(String_t title, bool withGrid);

    Session& m_session;
    Ptr<BitmapFont> m_font;
    String_t m_fileName;
    afl::charset::Unichar_t m_currentCharacter;

    Painter m_painter;
    CharacterNameWidget m_characterName;
};

void
ui::reshack::FontEditor::FontEditorWindow::run()
{
    // RHFontEditorWindow::init()
    // HBox
    //   VBox
    //     ToolSelector
    //     ZoomSelector
    //     Spacer
    //   VBox
    //     HBox
    //       Button <
    //       CharacterNameWidget
    //       Button "G"
    //       Button >
    //     Painter
    //   VBox
    //     FontColorSelector
    //     UISpacer
    //     UIButton "close"

    afl::base::Deleter del;
    Root& root = m_session.root();
    afl::string::Translator& tx = m_session.translator();

    Window& win = del.addNew(new Window(m_fileName, root.provider(), root.colorScheme(), BLUE_BLACK_WINDOW, ui::layout::HBox::instance5));
    EventLoop loop(root);

    Group& g1  = del.addNew(new Group(ui::layout::VBox::instance5));
    Group& g2  = del.addNew(new Group(ui::layout::VBox::instance5));
    Group& g21 = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& g3  = del.addNew(new Group(ui::layout::VBox::instance5));

    ToolSelector& sel = del.addNew(new ToolSelector(root, m_painter));
    sel.addNewTool(util::Key_F1, new PencilTool(tx));
    sel.addNewTool(util::Key_F2, new BrushTool(tx));
    sel.addNewTool(util::Key_F3, new LineTool(tx));
    sel.addNewTool(util::Key_F4, new RectangleTool(tx, false));
    sel.addNewTool(util::Key_F5, new RectangleTool(tx, true));
    sel.addNewTool(util::Key_F6, new CircleTool(tx));
    sel.addNewTool(util::Key_F7, new Clipboard::CopyTool(m_session.clipboard(), tx));
    sel.addNewTool(util::Key_F8, new Clipboard::PasteTool(m_session.clipboard(), true, tx));
    sel.addNewTool(util::Key_F9, new Clipboard::PasteTool(m_session.clipboard(), false, tx));
    m_painter.setZoom(2);

    win.add(g1);
    win.add(g2);
    win.add(g3);
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    g1.add(sel);
    g1.add(del.addNew(new ZoomSelector(root, m_painter, 4)));
    g1.add(del.addNew(new Spacer()));

    g2.add(g21);
    g2.add(m_painter);

    Button& btnClose = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
    Button& btnPrev  = del.addNew(new Button("<", util::Key_PgUp, root));
    Button& btnNext  = del.addNew(new Button(">", util::Key_PgDn, root));
    Button& btnGo    = del.addNew(new Button("G", 'g', root));

    g21.add(btnPrev);
    g21.add(m_characterName);
    g21.add(btnGo);
    g21.add(btnNext);

    g3.add(del.addNew(new FontColorSelector(m_session, m_painter)));
    g3.add(del.addNew(new Spacer()));
    g3.add(btnClose);

    afl::base::SignalConnection conn_clipboardChange(m_session.clipboard().sig_change.add(&sel, (void (Widget::*)()) &ToolSelector::requestRedraw));

    btnClose.sig_fire.addNewClosure(loop.makeStop(0));
    btnPrev.dispatchKeyTo(*this);
    btnNext.dispatchKeyTo(*this);
    btnGo.dispatchKeyTo(*this);

    /* Figure out horizontal aux lines: common heights */
    // regular character top
    int n;
    if (const BitmapGlyph* g = m_font->getGlyph('B')) {
        if ((n = FontUtil::findTop(*g)) >= 0) {
            m_painter.addAuxLine(n, false);
        }
    }

    // Lower-case top
    if (const BitmapGlyph* g = m_font->getGlyph('o')) {
        if ((n = FontUtil::findTop(*g)) >= 0) {
            m_painter.addAuxLine(n, false);
        }
    }

    // Bottom
    if (const BitmapGlyph* g = m_font->getGlyph('o')) {
        if ((n = FontUtil::findBottom(*g)) >= 0) {
            m_painter.addAuxLine(n, false);
        }
    }

    // Descender
    if (const BitmapGlyph* g = m_font->getGlyph('g')) {
        if ((n = FontUtil::findBottom(*g)) >= 0) {
            m_painter.addAuxLine(n, false);
        }
    }

    /* Figure out vertical aux lines: common widths. This is to
       support making fixed-width fonts */
    enum { MAXWIDTHS = 3 };
    int nwidths = 0;
    int widths[MAXWIDTHS];
    bool ok = true;
    for (uint32_t i = 0, e = m_font->getCurrentCharacterLimit(); i < e; ++i) {
        if (const BitmapGlyph* g = m_font->getGlyph(i)) {
            bool found = false;
            for (int j = 0; j < nwidths; ++j) {
                if (widths[j] == g->getWidth()) {
                    found = true;
                }
            }
            if (!found) {
                if (nwidths < MAXWIDTHS) {
                    widths[nwidths++] = g->getWidth();
                } else {
                    ok = false;
                    break;
                }
            }
        }
    }
    if (ok) {
        for (int i = 0; i < nwidths; ++i) {
            m_painter.addAuxLine(widths[i], true);
        }
    }

    // Run
    win.setExtent(root.getExtent());
    root.add(win);
    loop.run();

    storeCurrentCharacter();
}

bool
ui::reshack::FontEditor::FontEditorWindow::handleKey(util::Key_t key, int /*prefix*/)
{
    // RHFontEditorWindow::handleEvent(const UIEvent& event, bool second_pass)
    afl::string::Translator& tx = m_session.translator();
    afl::charset::Unichar_t ch;
    switch (key) {
     case util::Key_PgUp:
        if (m_currentCharacter > 0) {
            setCurrentCharacter(m_currentCharacter - 1);
        }
        return true;
     case util::Key_PgDn:
        if (m_currentCharacter < 0xFFFF) {
            setCurrentCharacter(m_currentCharacter + 1);
        }
        return true;
     case util::Key_PgUp + util::KeyMod_Ctrl:
        setCurrentCharacter(FontUtil::findPreviousExistingCharacter(*m_font, m_currentCharacter));
        return true;
     case util::Key_PgDn + util::KeyMod_Ctrl:
        setCurrentCharacter(FontUtil::findNextExistingCharacter(*m_font, m_currentCharacter));
        return true;
     case '+':
        m_painter.setSize(m_painter.getSize() + gfx::Point(1, 0));
        return true;
     case '-':
        if (m_painter.getSize().getX() != 0) {
            m_painter.setSize(m_painter.getSize() + gfx::Point(-1, 0));
        }
        return true;
     case util::Key_Delete:
        if (m_painter.getSize().getX() != 0) {
            if (MessageBox(tx("Delete this character?"), tx("Delete Character"), m_session.root()).doYesNoDialog(tx)) {
                m_painter.setSize(gfx::Point(0, m_font->getHeight()));
            }
        }
        return true;
     case 'a': {
        Ptr<PalettizedPixmap> pix = Dialogs::synthesizeCharacter(m_session, *m_font, m_currentCharacter);
        if (pix.get() != 0) {
            m_painter.setPixmap(pix);
        }
        return true;
     }
     case 'C':
     case 'c':
        storeCurrentCharacter();
        if (pickCharacter(tx("Copy From Character"), key == 'C').get(ch)) {
            m_painter.setPixmap(FontUtil::createPixmapFromGlyph(*m_font, ch));
        }
        return true;
     case 'e':
        storeCurrentCharacter();
        if (Dialogs::showCoverage(m_session, Info::getFontCoverage(*m_font, m_session.translator())).get(ch)) {
            setCurrentCharacter(ch);
        }
        return true;
     case 'E':
        MessageBox(Info::getEncodingInfo(m_currentCharacter, m_session.translator()), m_session.translator()("Encoding Information"), m_session.root())
            .doOkDialog(m_session.translator());
        return true;
     case 'G':
     case 'g':
        storeCurrentCharacter();
        if (pickCharacter(tx("Go to Character"), key == 'G').get(ch)) {
            setCurrentCharacter(ch);
        }
        return true;
     case 'm':
        storeCurrentCharacter();
        Dialogs::changeFontAlignment(m_session, *m_font);
        loadCurrentCharacter();
        return true;
     case 'O':
     case 'o':
        storeCurrentCharacter();
        if (pickCharacter(tx("Overlay Character"), key == 'O').get(ch)) {
            if (const BitmapGlyph* g = m_font->getGlyph(ch)) {
                // FIXME: it would make sense to use "maximum" logic, i.e. never
                // draw a FC_HALF pixel above a FC_WHITE pixel.
                g->drawColored(*m_painter.getPixmap()->makeCanvas(), gfx::Point(0, 0), Palette::FC_White, Palette::FC_Half);
                m_painter.requestRedraw();
            }
        }
        return true;
     case 'p':
        storeCurrentCharacter();
        Dialogs::showPreview(m_session, *m_font);
        return true;
     case 'r':
        storeCurrentCharacter();
        Dialogs::changeFontEncoding(m_session, *m_font);
        loadCurrentCharacter();
        break;
     case 's':
        if (const gfx::Font* font = Dialogs::showCharacterInSystemFonts(m_session, m_currentCharacter)) {
            if (const BitmapFont* bmFont = dynamic_cast<const BitmapFont*>(font)) {
                if (const BitmapGlyph* bmGlyph = bmFont->getGlyph(m_currentCharacter)) {
                    // Glyph exists in other font. Make sure this glyph is wide enough, then copy it
                    gfx::Point pt = m_painter.getSize();
                    if (pt.getX() < bmGlyph->getWidth()) {
                        pt.setX(bmGlyph->getWidth());
                        m_painter.setSize(pt);
                    }
                    bmGlyph->drawColored(*m_painter.getPixmap()->makeCanvas(), gfx::Point(0, 0), Palette::FC_White, Palette::FC_Half);
                    m_painter.requestRedraw();
                }
            }
        }
        return true;
     case 'u':
        if (MessageBox(tx("Undo changes to this character and restore previous content?"), tx("Font Editor"), m_session.root()).doYesNoDialog(tx)) {
            loadCurrentCharacter();
        }
        return true;

    }
    return false;
}

void
ui::reshack::FontEditor::FontEditorWindow::setCurrentCharacter(afl::charset::Unichar_t ch)
{
    // RHFontEditorWindow::setCharacter(uint32_t ch)
    if (ch != m_currentCharacter) {
        storeCurrentCharacter();
        m_currentCharacter = ch;
        loadCurrentCharacter();
    }
}

/* Load current character from font */
void
ui::reshack::FontEditor::FontEditorWindow::loadCurrentCharacter()
{
    m_characterName.setCharacter(m_currentCharacter);
    m_painter.setPixmap(FontUtil::createPixmapFromGlyph(*m_font, m_currentCharacter));
}

/* Store current character in font */
void
ui::reshack::FontEditor::FontEditorWindow::storeCurrentCharacter()
{
    // RHFontEditorWindow::storeCurrentCharacter()
    m_font->addNewGlyph(m_currentCharacter, FontUtil::createGlyphFromPixmap(m_painter.getPixmap()));
}

/* Pick a character, from grid or list */
afl::base::Optional<afl::charset::Unichar_t>
ui::reshack::FontEditor::FontEditorWindow::pickCharacter(String_t title, bool withGrid)
{
    // ex RHFontEditorWindow::pickCharacter(string_t title, uint32_t& result, bool withGrid)
    if (withGrid) {
        return pickCharacterFromGrid(m_session, title, *m_font, m_currentCharacter);
    } else {
        return CharacterListDialog(m_session).run(title);
    }
}


/*
 *  SaveDialog - ask for save parameters, and save
 */

class ui::reshack::FontEditor::SaveDialog {
 public:
    SaveDialog(Session& session)
        : m_session(session),
          m_fileName(1000, 20, session.root()),
          m_comment(255, 20, session.root()),
          m_encoding(0)
        {
            m_fileName.setHotkey(util::KeyMod_Alt + 'n');
            m_comment.setHotkey(util::KeyMod_Alt + 'f');
        }

    bool run()
        {
            // RHSaveFontDialog::init()
            // VBox
            //   HBox ["File Name", InputLine]
            //   HBox ["Font Name", InputLine]
            //   HBox
            //     "Encoding"
            //     VBox [RadioButton * 3]
            //   StandardDialogButtons
            afl::base::Deleter del;
            Root& root = m_session.root();
            afl::string::Translator& tx = m_session.translator();

            Window& win = del.addNew(new Window(tx("Save Font"), root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));

            Group& g1  = del.addNew(new Group(ui::layout::HBox::instance5));
            Group& g2  = del.addNew(new Group(ui::layout::HBox::instance5));
            Group& g3  = del.addNew(new Group(ui::layout::HBox::instance5));
            Group& g32 = del.addNew(new Group(ui::layout::HBox::instance5));

            g1.add(del.addNew(new ui::widgets::StaticText(tx("File Name:"), util::SkinColor::Static, "+", root.provider())));
            g1.add(m_fileName);

            g2.add(del.addNew(new ui::widgets::StaticText(tx("Font Name:"), util::SkinColor::Static, "+", root.provider())));
            g2.add(m_comment);

            g3.add(del.addNew(new ui::widgets::StaticText(tx("Encoding:"), util::SkinColor::Static, "+", root.provider())));
            g3.add(g32);

            Widget& rb1 = del.addNew(new ui::widgets::RadioButton(root, util::KeyMod_Alt + 's', tx("System"),            m_encoding, 0));
            Widget& rb2 = del.addNew(new ui::widgets::RadioButton(root, util::KeyMod_Alt + 'c', tx("Cyrillic (CP866)"),  m_encoding, 1));
            Widget& rb3 = del.addNew(new ui::widgets::RadioButton(root, util::KeyMod_Alt + 'u', tx("Unicode"),           m_encoding, 2));
            g32.add(rb1);
            g32.add(rb2);
            g32.add(rb3);

            EventLoop loop(root);
            ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
            btn.addStop(loop);

            win.add(g1);
            win.add(g2);
            win.add(g3);
            win.add(btn);

            FocusIterator& it1 = del.addNew(new FocusIterator(FocusIterator::Tab | FocusIterator::Vertical));
            it1.add(m_fileName);
            it1.add(m_comment);
            it1.add(g32);
            win.add(it1);

            FocusIterator& it2 = del.addNew(new FocusIterator(FocusIterator::Horizontal));
            it2.add(rb1);
            it2.add(rb2);
            it2.add(rb3);
            win.add(it2);

            win.add(del.addNew(new ui::widgets::Quit(root, loop)));

            win.pack();
            root.centerWidget(win);
            root.add(win);
            return loop.run() != 0;
        }

    void setFileName(const String_t& fn)
        { m_fileName.setText(fn); }

    String_t getFileName() const
        { return m_fileName.getText(); }

    String_t getComment() const
        { return m_comment.getText(); }

    int getEncoding() const
        { return m_encoding.get(); }

 private:
    Session& m_session;
    ui::widgets::InputLine m_fileName;
    ui::widgets::InputLine m_comment;
    afl::base::Observable<int> m_encoding;
};


/*
 *  FontEditor
 */

ui::reshack::FontEditor::FontEditor(afl::base::Ptr<gfx::BitmapFont> font, String_t fileName)
    : m_font(font),
      m_fileName(fileName),
      m_dirty(false)
{ }

String_t
ui::reshack::FontEditor::getName(afl::string::Translator& tx)
{
    // RHFontEditor::getName()
    String_t name;
    if (m_fileName.empty()) {
        name = tx("Unnamed");
    } else {
        name = m_fileName;

        String_t::size_type sep = name.find_last_of("\\/:");
        if (sep != String_t::npos && sep != 0) {
            name.erase(0, sep+1);
        }
    }
    name += Format(tx(" (font, height %d pixels)"), m_font->getLineHeight());
    return name;
}

void
ui::reshack::FontEditor::edit(Session& session)
{
    // RHFontEditor::edit()
    FontEditorWindow(session, m_font, m_fileName).run();
    m_dirty = true;
}

void
ui::reshack::FontEditor::save(Session& session)
{
    // RHFontEditor::save()
    SaveDialog dlg(session);
    dlg.setFileName(m_fileName);

    if (dlg.run()) {
        afl::string::Translator& tx = session.translator();
        try {
            Ref<afl::io::Stream> out = session.fileSystem().openFile(dlg.getFileName(), FileSystem::Create);
            FontUtil::saveFont(*out, *m_font, dlg.getComment(), static_cast<uint8_t>(dlg.getEncoding()));
            m_dirty = false;
        }
        catch (afl::except::FileProblemException& e) {
            MessageBox(Format(tx("Unable to save file %s: %s"), e.getFileName(), e.what()), tx("Error"), session.root()).doOkDialog(tx);
        }
        catch (std::exception& e) {
            MessageBox(Format(tx("Unable to save file %s: %s"), dlg.getFileName(), e.what()), tx("Error"), session.root()).doOkDialog(tx);
        }
    }
}

bool
ui::reshack::FontEditor::hasUnsavedChanges()
{
    // RHFontEditor::hasUnsavedChanges()
    return m_dirty;
}
