/**
  *  \file ui/reshack/pictureeditor.cpp
  *  \brief Class ui::reshack::PictureEditor
  */

#include "ui/reshack/pictureeditor.hpp"

#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "gfx/codec/bmp.hpp"
#include "gfx/codec/codec.hpp"
#include "gfx/codec/custom.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/reshack/brushtool.hpp"
#include "ui/reshack/circletool.hpp"
#include "ui/reshack/colorselector.hpp"
#include "ui/reshack/linetool.hpp"
#include "ui/reshack/painter.hpp"
#include "ui/reshack/penciltool.hpp"
#include "ui/reshack/pipettetool.hpp"
#include "ui/reshack/rectangletool.hpp"
#include "ui/reshack/session.hpp"
#include "ui/reshack/toolselector.hpp"
#include "ui/reshack/zoomselector.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"

using afl::string::Format;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using ui::widgets::FrameGroup;
using ui::widgets::InputLine;
using ui::widgets::Quit;
using ui::widgets::StaticText;

namespace {
    const char*const EXTENSIONS[] = { ".gfx", ".cc", ".cd", ".bmp" };
    const char KEYS[] = { 'g', 'c', 'd', 'b' };

    enum {
        RawFormat,
        FourBitFormat,
        EightBitFormat,
        BitmapFormat
    };

    int getFormatFromFileName(String_t fn)
    {
        String_t::size_type n = fn.rfind('.');
        if (n != fn.npos) {
            fn.erase(0, n);
            for (int i = 0; i < int(countof(EXTENSIONS)); ++i) {
                if (fn == EXTENSIONS[i]) {
                    return i;
                }
            }
        }
        return EightBitFormat;
    }
}

class ui::reshack::PictureEditor::SaveDialog {
 public:
    SaveDialog(Session& session, String_t fileName)
        : m_session(session),
          m_format(getFormatFromFileName(fileName)),
          m_fileName(1000, 20, m_session.root())
        {
            m_fileName.setHotkey(util::KeyMod_Alt + 'n');
            m_fileName.setText(fileName);
            m_format.sig_change.add(this, &SaveDialog::onFormatChange);
        }

    String_t getFileName() const
        { return m_fileName.getText(); }

    int getFormat() const
        { return m_format.get(); }

    bool run()
        {
            // RHSaveImageDialog::init()
            afl::string::Translator& tx = m_session.translator();
            Root& root = m_session.root();

            // VBox
            //   HBox
            //     "File Name" [InputLine]
            //   HBox
            //     "File Type"
            //     VBox [RadioButton]
            //   StandardDialogButtons
            Window& win = m_deleter.addNew(new Window(tx("Save Image"), root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));
            Group& g1  = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
            Group& g2  = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
            Group& g22 = m_deleter.addNew(new Group(ui::layout::VBox::instance5));

            FocusIterator& it = m_deleter.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Tab));
            it.add(m_fileName);

            g1.add(m_deleter.addNew(new StaticText(tx("File Name:"), util::SkinColor::Static, "+", root.provider())));
            g1.add(FrameGroup::wrapWidget(m_deleter, root.colorScheme(), LoweredFrame, m_fileName));

            g2.add(m_deleter.addNew(new StaticText(tx("File Format:"), util::SkinColor::Static, "+", root.provider())));
            g2.add(g22);

            for (int i = 0; i < int(countof(EXTENSIONS)); ++i) {
                Widget& w = m_deleter.addNew(new ui::widgets::RadioButton(root, KEYS[i] + util::KeyMod_Alt, EXTENSIONS[i], m_format, i));
                it.add(w);
                g22.add(w);
            }

            EventLoop loop(root);
            ui::widgets::StandardDialogButtons& btn = m_deleter.addNew(new ui::widgets::StandardDialogButtons(root, tx));
            btn.addStop(loop);

            win.add(g1);
            win.add(g2);
            win.add(btn);
            win.add(it);
            win.add(m_deleter.addNew(new Quit(root, loop)));
            win.pack();

            root.centerWidget(win);
            root.add(win);
            return loop.run() != 0;
        }

 private:
    Session& m_session;
    afl::base::Observable<int32_t> m_format;
    InputLine m_fileName;
    afl::base::Deleter m_deleter;

    void onFormatChange()
        {
            // RHSaveImageDialog::onFileFormatChange(int n)
            int n = m_format.get();
            if (n >= 0 && n < static_cast<int>(countof(EXTENSIONS)) && n != getFormatFromFileName(m_fileName.getText())) {
                String_t dir = m_session.fileSystem().getDirectoryName(m_fileName.getText());
                String_t fn  = m_session.fileSystem().getFileName(m_fileName.getText());
                String_t::size_type dot = fn.rfind('.');
                if (dot != fn.npos) {
                    fn.erase(dot);
                }
                m_fileName.setText(m_session.fileSystem().makePathName(dir, fn + EXTENSIONS[n]));
            }
        }
};

class ui::reshack::PictureEditor::Dialog {
 public:
    Dialog(Session& session, afl::base::Ptr<gfx::PalettizedPixmap> pix, String_t fileName)
        : m_session(session), m_painter(pix, Palette::StandardPaletteColor, session.root()), m_fileName(fileName),
          m_modifyButton(session.translator()("Modify"), 'm', session.root())
        {
            m_modifyButton.sig_fire.add(this, &Dialog::onModify);
        }

    void run()
        {
            // RHPictureEditorWindow::init(Ptr<GfxPixmap> pixmap, ColorMode color_mode)
            afl::base::Deleter del;
            afl::string::Translator& tx = m_session.translator();
            Root& root = m_session.root();
            EventLoop loop(root);

            Window& win = del.addNew(new Window(m_fileName, root.provider(), root.colorScheme(), BLUE_BLACK_WINDOW, ui::layout::HBox::instance5));

            Group& group1 = del.addNew(new Group(ui::layout::VBox::instance5));
            Group& group3 = del.addNew(new Group(ui::layout::VBox::instance5));

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
            sel.addNewTool(util::Key_F10, new PipetteTool(tx, m_painter));

            win.add(group1);
            win.add(m_painter);
            win.add(group3);
            win.add(del.addNew(new Quit(root, loop)));

            group1.add(sel);
            group1.add(del.addNew(new ZoomSelector(root, m_painter, 4)));
            group1.add(del.addNew(new Spacer()));

            Button& btnClose = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
            btnClose.sig_fire.addNewClosure(loop.makeStop(0));

            group3.add(del.addNew(new ColorSelector(m_session, m_painter)));
            group3.add(del.addNew(new Spacer()));
            group3.add(m_modifyButton);
            group3.add(btnClose);

            afl::base::SignalConnection conn_clipboardChange(m_session.clipboard().sig_change.add(&sel, (void (Widget::*)()) &ToolSelector::requestRedraw));

            win.setExtent(root.getExtent());
            root.add(win);
            loop.run();
        }

    afl::base::Ptr<gfx::PalettizedPixmap> getPixmap() const
        { return m_painter.getPixmap(); }

    void onModify()
        {
            // RHPictureEditorWindow::onModify()
            afl::string::Translator& tx = m_session.translator();
            Root& root = m_session.root();

            ui::widgets::StringListbox box(root.provider(), root.colorScheme());
            box.addItem(1, tx("Set width"));
            box.addItem(2, tx("Set height"));
            box.addItem(3, tx("Add X aux line"));
            box.addItem(4, tx("Add Y aux line"));
            box.addItem(5, tx("Remove X aux line"));
            box.addItem(6, tx("Remove Y aux line"));

            EventLoop loop(root);
            if (!ui::widgets::MenuFrame(ui::layout::VBox::instance5, root, loop).doMenu(box, m_modifyButton.getExtent().getBottomLeft())) {
                return;
            }

            String_t title = "?";
            int32_t key = -1;
            box.getStringList().get(box.getCurrentItem(), key, title);

            afl::base::Observable<int32_t> value;
            ui::widgets::DecimalSelector nsel(root, tx, value, 0, 1024, 16);
            if (!ui::widgets::doStandardDialog(title, title, nsel, false, 0, root, tx)) {
                return;
            }

            switch (key) {
             case 1:
                m_painter.setSize(gfx::Point(value.get(), m_painter.getSize().getY()));
                break;
             case 2:
                m_painter.setSize(gfx::Point(m_painter.getSize().getX(), value.get()));
                break;
             case 3:
                m_painter.addAuxLine(value.get(), true);
                break;
             case 4:
                m_painter.addAuxLine(value.get(), false);
                break;
             case 5:
                m_painter.removeAuxLine(value.get(), true);
                break;
             case 6:
                m_painter.removeAuxLine(value.get(), false);
                break;
            }
        }

 private:
    Session& m_session;
    Painter m_painter;
    String_t m_fileName;
    Button m_modifyButton;
};

ui::reshack::PictureEditor::PictureEditor(afl::base::Ref<gfx::PalettizedPixmap> pix, String_t fileName)
    : m_pixmap(pix.asPtr()),
      m_fileName(fileName),
      m_dirty(false)
{ }

String_t
ui::reshack::PictureEditor::getName(afl::string::Translator& tx)
{
    // RHPictureEditor::getName()
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
    name += Format(" (%dx%d)", m_pixmap->getSize().getX(), m_pixmap->getSize().getY());
    return name;
}

void
ui::reshack::PictureEditor::edit(Session& session)
{
    // RHPictureEditor::edit()
    // @change Palette is set by Painter
    Dialog win(session, m_pixmap, m_fileName);
    win.run();
    m_pixmap = win.getPixmap();
    m_dirty = true;
}

void
ui::reshack::PictureEditor::save(Session& session)
{
    // RHPictureEditor::save()

    // Create dialog
    SaveDialog dlg(session, m_fileName);
    if (dlg.run()) {
        // OK
        afl::string::Translator& tx = session.translator();
        try {
            saveImage(session, dlg.getFormat(), dlg.getFileName());
            m_fileName = dlg.getFileName();
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
ui::reshack::PictureEditor::hasUnsavedChanges()
{
    // RHPictureEditor::hasUnsavedChanges()
    return m_dirty;
}

void
ui::reshack::PictureEditor::saveImage(Session& session, int fileFormat, String_t fileName)
{
    // RHPictureEditor::saveImage(string_t name, int format)
    std::auto_ptr<gfx::codec::Codec> codec;
    switch (fileFormat) {
     case RawFormat:      codec.reset(new gfx::codec::Custom(gfx::codec::Custom::Raw,      false)); break;
     case FourBitFormat:  codec.reset(new gfx::codec::Custom(gfx::codec::Custom::FourBit,  false)); break;
     case EightBitFormat: codec.reset(new gfx::codec::Custom(gfx::codec::Custom::EightBit, false)); break;
     case BitmapFormat:   codec.reset(new gfx::codec::BMP());                                       break;
    }
    if (codec.get() != 0) {
        codec->save(*m_pixmap->makeCanvas(), *session.fileSystem().openFile(fileName, afl::io::FileSystem::Create));
    }
}
