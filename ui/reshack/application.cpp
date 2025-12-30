/**
  *  \file ui/reshack/application.cpp
  *  \brief Class ui::reshack::Application
  */

#include "ui/reshack/application.hpp"

#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/bitmapglyph.hpp"
#include "gfx/codec/custom.hpp"
#include "gfx/context.hpp"
#include "gfx/windowparameters.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/dialogs/fileselectiondialog.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/engineimageloader.hpp"
#include "ui/res/manager.hpp"
#include "ui/reshack/editor.hpp"
#include "ui/reshack/fonteditor.hpp"
#include "ui/reshack/fontutil.hpp"
#include "ui/reshack/palette.hpp"
#include "ui/reshack/pictureeditor.hpp"
#include "ui/reshack/session.hpp"
#include "ui/reshack/win32import.hpp"
#include "ui/root.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/consolelogger.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::string::Translator;
using gfx::BitmapFont;
using gfx::PalettizedPixmap;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using ui::widgets::InputLine;
using ui::widgets::Quit;
using ui::widgets::StandardDialogButtons;
using ui::widgets::StaticText;
using util::SkinColor;

namespace {
    const char*const LOG_NAME = "reshack";
}

struct ui::reshack::Application::Parameters {
    gfx::WindowParameters windowParameters;
    std::vector<String_t> filesToLoad;
    afl::base::Optional<String_t> characterNameFile;
    afl::base::Optional<String_t> characterAliasFile;

    Parameters()
        : windowParameters(),
          filesToLoad(),
          characterNameFile(),
          characterAliasFile()
        {
            windowParameters.size = gfx::Point(800, 600);
            windowParameters.bitsPerPixel = 32;
        }
};

/*
 *  EditorList
 */

class ui::reshack::Application::EditorList : public ui::widgets::AbstractListbox {
 public:
    EditorList(Root& root, Translator& tx, afl::container::PtrVector<Editor>& editors)
        : m_root(root),
          m_translator(tx),
          m_editors(editors)
        { }
    virtual size_t getNumItems() const
        { return m_editors.size(); }
    virtual bool isItemAccessible(size_t /*n*/) const
        { return true; }
    virtual int getItemHeight(size_t /*n*/) const
        { return m_root.provider().getFont(gfx::FontRequest())->getLineHeight(); }
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
            // ex RHEditorList::drawPart(GfxCanvas& can, int from, int to)
            afl::base::Deleter del;
            gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
            prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
            String_t line;
            if (item < m_editors.size()) {
                line += m_editors[item]->getName(m_translator);
                if (m_editors[item]->hasUnsavedChanges()) {
                    line += " *";
                }
            }
            outTextF(ctx, area, line);
        }
    virtual void handlePositionChange()
        { defaultHandlePositionChange(); }
    virtual ui::layout::Info getLayoutInfo() const
        { return ui::layout::Info(m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 30), ui::layout::Info::GrowBoth); }
    virtual bool handleKey(util::Key_t key, int prefix)
        { return defaultHandleKey(key, prefix); }
 private:
    Root& m_root;
    Translator& m_translator;
    afl::container::PtrVector<Editor>& m_editors;
};

class ui::reshack::Application::NewPictureDialog {
 public:
    NewPictureDialog(Session& session)
        : m_session(session),
          m_loop(session.root()),
          m_widthInput(10, session.root()),
          m_heightInput(10, session.root()),
          m_width(1),
          m_height(1)
        {
            m_widthInput.setHotkey(util::KeyMod_Alt + 'w');
            m_widthInput.setHotkey(util::KeyMod_Alt + 'h');
        }

    bool run()
        {
            // RHNewPictureDialog::init()
            Root& root = m_session.root();
            Translator& tx = m_session.translator();
            afl::base::Deleter del;

            Window& win = del.addNew(new Window(tx("New Picture"), root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));
            Group& g = del.addNew(new Group(del.addNew(new ui::layout::Grid(2))));
            g.add(del.addNew(new StaticText(tx("Width:"), SkinColor::Static, "+", root.provider())));
            g.add(m_widthInput);
            g.add(del.addNew(new StaticText(tx("Height:"), SkinColor::Static, "+", root.provider())));
            g.add(m_heightInput);
            win.add(g);

            StandardDialogButtons& btn = del.addNew(new StandardDialogButtons(root, tx));
            btn.ok().sig_fire.add(this, &NewPictureDialog::onOK);
            btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
            win.add(btn);

            FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Tab | FocusIterator::Vertical));
            it.add(m_widthInput);
            it.add(m_heightInput);
            win.add(it);
            win.add(del.addNew(new Quit(root, m_loop)));

            win.pack();
            root.centerWidget(win);
            root.add(win);
            return m_loop.run() != 0;
        }

    int getWidth() const
        { return m_width; }

    int getHeight() const
        { return m_height; }

 private:
    void onOK()
        {
            // RHNewPictureDialog::onOK()
            if (!afl::string::strToInteger(m_widthInput.getText(), m_width) || m_width <= 0 || m_width > 2500) {
                m_widthInput.requestFocus();
                return;
            }
            if (!afl::string::strToInteger(m_heightInput.getText(), m_height) || m_height <= 0 || m_height > 2000) {
                m_heightInput.requestFocus();
                return;
            }
            m_loop.stop(1);
        }

    Session& m_session;
    EventLoop m_loop;
    InputLine m_widthInput;
    InputLine m_heightInput;
    int m_width;
    int m_height;
};

/** Resource hacker main dialog.
    Displays a list of editors (EditorList) and lets the user add and manipulate them. */
class ui::reshack::Application::MainDialog {
 public:
    MainDialog(Session& session)
        : m_session(session),
          m_editors(),
          m_list(session.root(), session.translator(), m_editors),
          m_newButton(session.translator()("New"), 'n', session.root()),
          m_loop(session.root())
        {
            m_newButton.sig_fire.add(this, &MainDialog::onNew);
        }

    void doLoad(String_t fileName)
        {
            // RHMainDialog::doLoad(string_t filename)
            Translator& tx = m_session.translator();

            // Open file
            Ptr<Stream> s = m_session.fileSystem().openFileNT(fileName, FileSystem::OpenRead);
            if (s.get() == 0) {
                MessageBox(Format(tx("Unable to open file: %s"), fileName), tx("Load"), m_session.root()).doOkDialog(tx);
                return;
            }

            // Try to load picture
            Ptr<gfx::Canvas> pix = m_session.manager().loadImage(*s);

            // When picture successfully loaded, finish.
            if (pix.get() != 0) {
                addNewEditor(new PictureEditor(Palette::makeEditable(*pix), fileName));
                return;
            }

            // Try to load font
            Ptr<BitmapFont> font = new BitmapFont();
            try {
                s->setPos(0);
                font->load(*s, 0, m_session.translator());
                addNewEditor(new FontEditor(font, fileName));
                return;
            }
            catch (...) { }

            // Try to load as BDF
            s->setPos(0);
            try {
                Ptr<BitmapFont> font = FontUtil::loadBDF(*s);
                if (font.get() != 0) {
                    addNewEditor(new FontEditor(font, fileName));
                    return;
                }
            }
            catch (...) { }

            // Nothing worked
            MessageBox(Format(tx("Unable to load file: %s"), fileName), tx("Load"), m_session.root()).doOkDialog(tx);
        }

    void run()
        {
            // RHMainDialog::RHMainDialog(), RHMainDialog::init()
            Translator& tx = m_session.translator();
            Root& root = m_session.root();

            afl::base::Deleter del;
            Window& win = del.addNew(new Window(tx("Resource Editor"), root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::HBox::instance5));

            win.add(m_list);

            Button& btn_edit = del.addNew(new Button(tx("Edit"), 'e', root));
            Button& btn_load = del.addNew(new Button(tx("Load"), 'l', root));
            Button& btn_save = del.addNew(new Button(tx("Save"), 's', root));
            Button& btn_exit = del.addNew(new Button(tx("Exit"), util::Key_Escape, root));

            btn_edit.sig_fire.add(this, &MainDialog::onEdit);
            btn_load.sig_fire.add(this, &MainDialog::onLoad);
            btn_save.sig_fire.add(this, &MainDialog::onSave);
            btn_exit.sig_fire.add(this, &MainDialog::onExit);

            Group& v = del.addNew(new Group(ui::layout::VBox::instance5));
            v.add(btn_edit);
            v.add(btn_load);
            v.add(btn_save);
            v.add(m_newButton);
            v.add(del.addNew(new Spacer()));
            v.add(btn_exit);
            win.add(v);

            ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
            disp.add(util::Key_Quit, this, &MainDialog::onExit);
            win.add(disp);

            win.pack();

            root.centerWidget(win);
            root.add(win);
            m_loop.run();
        }

 private:
    /** "Edit" function. Calls the current editor's edit function. */
    void onEdit()
        {
            // RHMainDialog::onEdit()
            size_t pos = m_list.getCurrentItem();
            if (pos < m_editors.size()) {
                m_editors[pos]->edit(m_session);
                m_list.updateCurrentItem();
            }
        }

    /** "Load" function. Asks for a file name and creates a new editor displaying that file. */
    void onLoad()
        {
            // RHMainDialog::onLoad()
            util::RequestReceiver<FileSystem> fsReceiver(m_session.root().engine().dispatcher(), m_session.fileSystem());
            ui::dialogs::FileSelectionDialog dlg(m_session.root(), m_session.translator(), fsReceiver.getSender(), m_session.translator()("Load"));
            dlg.setPattern("*");
            dlg.setFolder(".");

            if (!dlg.run()) {
                return;
            }

            // Load file
            doLoad(dlg.getResult());
        }

    /** "Save" function. Calls the current editor's save function. */
    void onSave()
        {
            // RHMainDialog::onSave()
            size_t pos = m_list.getCurrentItem();
            if (pos < m_editors.size()) {
                m_editors[pos]->save(m_session);
                m_list.updateCurrentItem();
            }
        }

    /** "Exit" function. */
    void onExit()
        {
            // RHMainDialog::onExit()
            // Any unsaved changes?
            bool unsavedChanges = false;
            for (afl::container::PtrVector<Editor>::iterator i = m_editors.begin(); i != m_editors.end(); ++i) {
                if ((*i)->hasUnsavedChanges()) {
                    unsavedChanges = true;
                }
            }

            // Prompt
            Translator& tx = m_session.translator();
            if (!unsavedChanges || MessageBox(tx("Do you want to exit? There are unsaved changes."), tx("Resource Editor"), m_session.root()).doYesNoDialog(tx)) {
                m_loop.stop(1);
            }
        }

    /** "New" function. */
    void onNew()
        {
            Root& root = m_session.root();
            Translator& tx = m_session.translator();
            ui::widgets::StringListbox box(root.provider(), root.colorScheme());
            box.addItem(1, tx("New Picture"));
            if (m_session.clipboard().hasContent()) {
                box.addItem(2, tx("New Picture from Clipboard"));
            }
            box.addItem(3, tx("New Font"));
            if (Win32Import::isSupported()) {
                box.addItem(4, tx("Import system font"));
            }

            EventLoop loop(root);
            if (!ui::widgets::MenuFrame(ui::layout::VBox::instance5, root, loop).doMenu(box, m_newButton.getExtent().getBottomLeft())) {
                return;
            }

            switch (box.getCurrentKey().orElse(0)) {
             case 1: {
                NewPictureDialog dlg(m_session);
                if (dlg.run()) {
                    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(dlg.getWidth(), dlg.getHeight());
                    pix->setPalette(0, gfx::codec::Custom::getPalette());
                    addNewEditor(new PictureEditor(pix, ""));
                }
                break;
             }

             case 2:
                if (m_session.clipboard().hasContent()) {
                    Ptr<PalettizedPixmap> orig = m_session.clipboard().getPixmap();
                    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(orig->getWidth(), orig->getHeight());
                    pix->pixels().copyFrom(orig->pixels());
                    pix->copyPalette(*orig);
                    addNewEditor(new PictureEditor(pix, ""));
                }
                break;

             case 3: {
                afl::base::Observable<int32_t> height(10);
                ui::widgets::DecimalSelector sel(root, tx, height, 1, 100, 10);
                if (ui::widgets::doStandardDialog(tx("New Font"), tx("Font Height"), sel, false, root, tx)) {
                    Ptr<BitmapFont> font = new BitmapFont();
                    font->addNewGlyph(0, new gfx::BitmapGlyph(1, static_cast<uint16_t>(height.get())));
                    font->addNewGlyph(0, 0);
                    addNewEditor(new FontEditor(font, ""));
                }
                break;
             }

             case 4: {
                Ptr<BitmapFont> font = Win32Import::importFont(m_session);
                if (font.get() != 0) {
                    addNewEditor(new FontEditor(font, ""));
                }
                break;
             }
            }
        }

    /** Add a new editor. Takes ownership of the editor.
        @param ed the editor */
    void addNewEditor(Editor* ed)
        {
            // RHMainDialog::addNewEditor(RHEditor* ed)
            m_editors.pushBackNew(ed);
            m_list.handleModelChange();
            m_list.setCurrentItem(m_editors.size()-1);
        }

    Session& m_session;
    afl::container::PtrVector<Editor> m_editors;
    EditorList m_list;
    Button m_newButton;
    EventLoop m_loop;
};


ui::reshack::Application::Application(afl::sys::Dialog& dialog,
                                      afl::string::Translator& tx,
                                      afl::sys::Environment& env,
                                      afl::io::FileSystem& fs)
    : gfx::Application(dialog, tx, tx("Resource Editor")),
      m_environment(env),
      m_fileSystem(fs)
{ }

void
ui::reshack::Application::appMain(gfx::Engine& engine)
{
    // Infrastructure
    util::ConsoleLogger console;
    console.attachWriter(true, m_environment.attachTextWriterNT(m_environment.Error));
    console.attachWriter(false, m_environment.attachTextWriterNT(m_environment.Output));
    log().addListener(console);
    util::ProfileDirectory profile(m_environment, m_fileSystem);

    // Parse parameters
    Parameters p;
    parseParameters(p);
    log().write(log().Info, LOG_NAME, translator()("Resource Editor"));

    // Derived environment
    Ref<Directory> resourceDirectory    = m_fileSystem.openDirectory(m_fileSystem.makePathName(m_fileSystem.makePathName(m_environment.getInstallationDirectoryName(), "share"), "resource"));
    Ref<Directory> defaultSpecDirectory = m_fileSystem.openDirectory(m_fileSystem.makePathName(m_fileSystem.makePathName(m_environment.getInstallationDirectoryName(), "share"), "specs"));

    // Resources
    ui::res::Manager mgr;
    mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
    mgr.addNewImageLoader(new ui::res::CCImageLoader());
    mgr.addNewProvider(new ui::res::DirectoryProvider(resourceDirectory, m_fileSystem, log(), translator()), "(MAIN)");

    // Window
    DefaultResourceProvider provider(mgr, resourceDirectory, engine.dispatcher(), translator(), log());
    Root root(engine, provider, p.windowParameters);
    mgr.setScreenSize(root.getExtent().getSize());

    // Run
    Session session(root, translator(), m_fileSystem, mgr, provider.fontList());
    MainDialog dlg(session);
    for (size_t i = 0, n = p.filesToLoad.size(); i < n; ++i) {
        dlg.doLoad(p.filesToLoad[i]);
    }
    loadCharacterNames(session, p);
    dlg.run();
}

void
ui::reshack::Application::parseParameters(Parameters& p)
{
    Translator& tx = translator();
    afl::sys::LongCommandLineParser parser(m_environment.getCommandLine());

    bool option;
    String_t text;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleWindowParameterOption(p.windowParameters, text, parser, tx)) {
                // ok
            } else if (text == "names") {
                p.characterNameFile = parser.getRequiredParameter(text);
            } else if (text == "alias") {
                p.characterAliasFile = parser.getRequiredParameter(text);
            } else {
                throw afl::except::CommandLineException(Format(tx("Unknown command line parameter \"-%s\""), text));
            }
        } else {
            p.filesToLoad.push_back(text);
        }
    }
}

void
ui::reshack::Application::loadCharacterNames(Session& s, Parameters& p)
{
    s.characterNames().addDefault();
    if (const String_t* name = p.characterNameFile.get()) {
        s.characterNames().loadNames(*s.fileSystem().openFile(*name, FileSystem::OpenRead));
    }
    if (const String_t* name = p.characterAliasFile.get()) {
        s.characterNames().loadAliases(*s.fileSystem().openFile(*name, FileSystem::OpenRead));
    }
}
