/**
  *  \file client/dialogs/export.cpp
  *  \brief Export Dialog
  */

#include "client/dialogs/export.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/exportfieldlist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/exportproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/string.hpp"

using afl::base::Deleter;
using afl::string::Format;
using client::dialogs::SessionFileSelectionDialog;
using client::widgets::ExportFieldList;
using game::proxy::ExportProxy;
using interpreter::exporter::Configuration;
using ui::dialogs::MessageBox;
using ui::widgets::OptionGrid;
using ui::widgets::StandardDialogButtons;
using ui::widgets::StringListbox;
using util::CharsetFactory;
using util::FileNamePattern;

namespace {
    // Ids for OptionGrid
    enum {
        IdFileFormat,
        IdCharacterSet
    };

    /*
     *  Main Dialog Class
     *
     *  This trivially listens to the ExportProxy and displays updates from it:
     *  - format options in an OptionGrid
     *  - field list in ExportFieldList
     */

    class ExportDialog : private gfx::KeyEventConsumer {
     public:
        ExportDialog(ui::Root& root, ExportProxy& proxy, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        void init();
        void run();

     private:
        // Event handlers
        bool handleKey(util::Key_t key, int prefix);
        void onOK();
        void onFormatAction(int id);
        void onChange(const Configuration& config);

        // User actions
        void editField();
        void insertField();
        bool chooseField(String_t& fieldName, String_t title);
        void deleteField();
        void deleteAll();
        void swapFields(bool up);
        void changeFormat();
        void changeCharset();
        void saveSettings();
        void loadSettings();

        // Details
        void render();
        bool isAtField() const;
        ui::Widget& makeButton(Deleter& del, String_t label, util::Key_t key);
        ui::Widget& makeLabel(Deleter& del, String_t text);

        // Main bureaucracy
        ui::Root& m_root;
        ExportProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;

        // State
        Configuration m_config;

        // Widgets
        OptionGrid m_options;  // ex format_control
        ExportFieldList m_fieldList;
    };
}

ExportDialog::ExportDialog(ui::Root& root, ExportProxy& proxy, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_proxy(proxy),
      m_gameSender(gameSender),
      m_translator(tx),
      m_loop(root),
      m_config(),
      m_options(0, 12*root.provider().getFont(gfx::FontRequest())->getEmWidth(), root),
      m_fieldList(root, tx)
{
    // ex WExportFormatControl::init() (sort-of)
    m_options.addItem(IdFileFormat,   'f', tx("File type"));
    m_options.addItem(IdCharacterSet, 'c', tx("Character set"));
    m_options.sig_click.add(this, &ExportDialog::onFormatAction);
    m_fieldList.sig_itemDoubleClick.add(this, &ExportDialog::editField);
}

void
ExportDialog::init()
{
    client::Downlink link(m_root, m_translator);
    m_proxy.getStatus(link, m_config);
    render();
    m_proxy.sig_change.add(this, &ExportDialog::onChange);
}

void
ExportDialog::run()
{
    // WExportParameterDialog::init()
    // VBox
    //   FrameGroup/ScrollbarContainer/ExportFieldList
    //   HBox ["Ins", "Del", Spacer]
    //   HBox ["Add", "Remove", Spacer, "+", "-"]
    //   OptionGrid
    //   StandardDialogButtons
    Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Export"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Field list
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_fieldList, m_root))));

    // Field list buttons
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(makeButton(del, m_translator("Ins"), util::Key_Insert));
    g2.add(makeLabel(del, m_translator("Add...")));
    g2.add(makeButton(del, m_translator("Del"), util::Key_Delete));
    g2.add(makeLabel(del, m_translator("Remove")));
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(makeLabel(del, m_translator("Width")));
    g2.add(makeButton(del, "-", '-'));
    g2.add(makeButton(del, "+", '+'));
    win.add(g2);

    // Options
    win.add(m_options);

    // Save/load
    ui::Group& g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g3.add(makeButton(del, m_translator("Ctrl-R"), util::KeyMod_Ctrl+'r'));
    g3.add(makeLabel(del, m_translator("Load")));
    g3.add(makeButton(del, m_translator("Ctrl-S"), util::KeyMod_Ctrl+'s'));
    g3.add(makeLabel(del, m_translator("Save Settings")));
    g3.add(del.addNew(new ui::Spacer()));
    win.add(g3);

    // Dialog buttons
    StandardDialogButtons& btn = del.addNew(new StandardDialogButtons(m_root, m_translator));
    btn.ok().sig_fire.add(this, &ExportDialog::onOK);
    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    win.add(btn);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:export"));
    win.add(help);
    btn.addHelp(help);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));
    win.add(del.addNew(new ui::PrefixArgument(m_root)));
    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

/* Event handler: keyboard */
bool
ExportDialog::handleKey(util::Key_t key, int prefix)
{
    // WExportFieldList::handleEvent, CUsedFieldList.Handle
    switch (key) {
     case util::Key_Delete:
        deleteField();
        return true;

     case util::KeyMod_Ctrl + util::Key_Delete:
        deleteAll();
        return true;

     case util::Key_Insert:
        insertField();
        return true;

     case ' ':
        editField();
        return true;

     case '*':
        if (isAtField()) {
            m_proxy.toggleFieldAlignment(m_fieldList.getCurrentItem());
        }
        return true;

     case util::KeyMod_Shift + '-':
     case util::KeyMod_Shift + util::Key_Left:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), -1);
        }
        return true;

     case '-':
     case util::Key_Left:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), prefix != 0 ? -prefix : -10);
        }
        return true;

     case util::KeyMod_Ctrl + '-':
     case util::KeyMod_Ctrl + util::Key_Left:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), -100);
        }
        return true;

     case util::KeyMod_Shift + '+':
     case util::KeyMod_Shift + util::Key_Right:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), +1);
        }
        return true;

     case '+':
     case util::Key_Right:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), prefix != 0 ? prefix : +10);
        }
        return true;

     case util::KeyMod_Ctrl + '+':
     case util::KeyMod_Ctrl + util::Key_Right:
        if (isAtField()) {
            m_proxy.changeFieldWidth(m_fieldList.getCurrentItem(), +100);
        }
        return true;

     case util::KeyMod_Ctrl + util::Key_Up:
        swapFields(true);
        return true;

     case util::KeyMod_Ctrl + util::Key_Down:
        swapFields(false);
        return true;

     case 's':
     case 's' + util::KeyMod_Ctrl:
        saveSettings();
        return true;

     case 'r':
     case 'r' + util::KeyMod_Ctrl:
        loadSettings();
        return true;
    }
    return false;
}

/* Event handler: OK button */
void
ExportDialog::onOK()
{
    // WExportParameterDialog::onOK()
    // Must have some fields
    if (m_config.fieldList().size() == 0) {
        MessageBox(m_translator("Please select some fields to export."), m_translator("Export"), m_root)
            .doOkDialog(m_translator);
        return;
    }

    // Default file name extension
    const String_t ext = getFileNameExtension(m_config.getFormat());

    // Select output file name
    client::Downlink link(m_root, m_translator);
    SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Export"));
    dlg.setPattern(FileNamePattern::getAllFilesWithExtensionPattern(ext));
    dlg.setDefaultExtension(ext);
    if (dlg.runDefault(link)) {
        String_t name = dlg.getResult();
        String_t err;
        if (!m_proxy.exportFile(link, name, err)) {
            MessageBox(Format(m_translator("Error during export: %s"), err), m_translator("Export"), m_root)
                .doOkDialog(m_translator);
        } else {
            MessageBox(m_translator("Export succeeded."), m_translator("Export"), m_root)
                .doOkDialog(m_translator);
            m_loop.stop(0);
        }
    }
}

/* Event handler: button on format OptionGrid */
void
ExportDialog::onFormatAction(int id)
{
    // ex WExportFormatControl::handleEvent
    switch (id) {
     case IdFileFormat:
        changeFormat();
        break;
     case IdCharacterSet:
        changeCharset();
        break;
    }
}

/* Event handler: data update from proxy */
void
ExportDialog::onChange(const Configuration& config)
{
    m_config = config;
    render();
}

/* User action: edit current field */
void
ExportDialog::editField()
{
    if (isAtField()) {
        size_t pos = m_fieldList.getCurrentItem();
        String_t fieldName = m_config.fieldList().getFieldName(pos);
        if (chooseField(fieldName, m_translator("Change field"))) {
            m_proxy.setFieldName(pos, fieldName);
        }
    } else {
        insertField();
    }
}

/* User action: insert field. */
void
ExportDialog::insertField()
{
    // WExportFieldList::insertField()
    String_t fieldName;
    size_t pos = m_fieldList.getCurrentItem();

    // Initialize to previous line's position to simplify setting up a totally new export
    if (pos > 0) {
        fieldName = m_config.fieldList().getFieldName(pos-1);
    }

    if (chooseField(fieldName, m_translator("Add field"))) {
        // Update local copy first.
        // When at the last item, only this will allow the cursor to be moved down one.
        // (If game side is lagging, this will display garbage, but fix itself up.)
        m_config.fieldList().add(pos, fieldName, 0);
        render();

        // Add to proxy
        m_proxy.add(pos, fieldName, 0);

        // Update cursor
        m_fieldList.setCurrentItem(pos+1);
    }
}

/* User action (part): choose a field name */
bool
ExportDialog::chooseField(String_t& fieldName, String_t title)
{
    // ex export.pas:AskFieldName
    // Determine available fields
    client::Downlink link(m_root, m_translator);
    afl::data::StringList_t list;
    m_proxy.enumProperties(link, list);
    if (list.empty()) {
        return false;
    }

    // Build list box, determine initial focus
    StringListbox listBox(m_root.provider(), m_root.colorScheme());
    int32_t key = -1;
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        listBox.addItem(int32_t(i), util::formatName(list[i]));
        if (list[i] == fieldName) {
            key = int32_t(i);
        }
    }
    listBox.sortItemsAlphabetically();
    if (key >= 0) {
        listBox.setCurrentKey(key);
    }

    // Dialog
    if (listBox.doStandardDialog(title, String_t(), 0, m_root, m_translator)) {
        int32_t chosenKey;
        if (listBox.getCurrentKey().get(chosenKey) && chosenKey >= 0 && chosenKey < int32_t(list.size())) {
            fieldName = list[size_t(chosenKey)];
        }
        return true;
    } else {
        return false;
    }
}

/* User action: delete field. */
void
ExportDialog::deleteField()
{
    // WExportFieldList::deleteField()
    m_proxy.remove(m_fieldList.getCurrentItem());
}

/* User action: delete all. */
void
ExportDialog::deleteAll()
{
    if (m_config.fieldList().size() > 0
        && MessageBox(m_translator("Clear this configuration?"), m_translator("Export"), m_root).doYesNoDialog(m_translator))
    {
        m_proxy.clear();
    }
}

/* User action: swap up/down. */
void
ExportDialog::swapFields(bool up)
{
    // WExportFieldList::swapFields(bool up)
    size_t pos = m_fieldList.getCurrentItem();
    if (up) {
        if (pos > 0) {
            m_fieldList.setCurrentItem(pos-1);
            m_proxy.swap(pos, pos-1);
        }
    } else {
        if (pos+1 <= m_config.fieldList().size()) {
            m_fieldList.setCurrentItem(pos+1);
            m_proxy.swap(pos, pos+1);
        }
    }
}

/* User action: change format. */
void
ExportDialog::changeFormat()
{
    // WExportFormatControl::changeType()
    StringListbox listBox(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0; i < interpreter::exporter::NUM_FORMATS; ++i) {
        listBox.addItem(int(i), getFormatDescription(interpreter::exporter::Format(i), m_translator));
    }
    listBox.setCurrentKey(int(m_config.getFormat()));
    if (listBox.doStandardDialog(m_translator("Change File Type"), String_t(), 0, m_root, m_translator)) {
        int32_t key;
        if (listBox.getCurrentKey().get(key)) {
            m_proxy.setFormat(interpreter::exporter::Format(key));
        }
    }
}

/* User action: change charset. */
void
ExportDialog::changeCharset()
{
    // WExportFormatControl::changeCharset()
    CharsetFactory f;
    StringListbox listBox(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0, n = f.getNumCharsets(); i < n; ++i) {
        listBox.addItem(int(i), f.getCharsetDescription(i, m_translator));
    }
    listBox.setCurrentKey(int(m_config.getCharsetIndex()));
    if (listBox.doStandardDialog(m_translator("Change Character Set"), String_t(), 0, m_root, m_translator)) {
        int32_t key;
        if (listBox.getCurrentKey().get(key)) {
            m_proxy.setCharsetIndex(CharsetFactory::Index_t(key));
        }
    }
}

/* User action: save settings */
void
ExportDialog::saveSettings()
{
    // WExportFieldList::saveSettings()
    client::Downlink link(m_root, m_translator);
    SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Save Settings"));
    dlg.setPattern(FileNamePattern::getAllFilesWithExtensionPattern("ccx"));
    dlg.setDefaultExtension("ccx");
    if (dlg.runDefault(link)) {
        String_t name = dlg.getResult();
        String_t err;
        if (!m_proxy.save(link, name, err)) {
            MessageBox(Format(m_translator("Unable to save: %s"), err), m_translator("Save Settings"), m_root)
                .doOkDialog(m_translator);
        }
    }
}

/* User action: load settings */
void
ExportDialog::loadSettings()
{
    // WExportFieldList::loadSettings()
    client::Downlink link(m_root, m_translator);
    SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Load Settings"));
    dlg.setPattern(FileNamePattern::getAllFilesWithExtensionPattern("ccx"));
    dlg.setDefaultExtension("ccx");
    if (dlg.runDefault(link)) {
        String_t name = dlg.getResult();
        String_t err;
        if (!m_proxy.load(link, name, err)) {
            MessageBox(Format(m_translator("Unable to load: %s"), err), m_translator("Load Settings"), m_root)
                .doOkDialog(m_translator);
        }
    }
}

/* Render current status */
void
ExportDialog::render()
{
    // ex WExportFormatControl::drawData (sort-of. hmm.)
    m_options.findItem(IdFileFormat).setValue(getFormatDescription(m_config.getFormat(), m_translator));
    m_options.findItem(IdCharacterSet).setValue(CharsetFactory().getCharsetName(m_config.getCharsetIndex(), m_translator));
    m_fieldList.setContent(m_config.fieldList());
}

/* Check whether cursor is at an actual field (not the placeholder) */
bool
ExportDialog::isAtField() const
{
    return m_fieldList.getCurrentItem() < m_config.fieldList().size();
}

ui::Widget&
ExportDialog::makeButton(Deleter& del, String_t label, util::Key_t key)
{
    ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(label, key, m_root));
    btn.setFont(gfx::FontRequest());
    btn.dispatchKeyTo(*this);
    return btn;
}

ui::Widget&
ExportDialog::makeLabel(Deleter& del, String_t text)
{
    return del.addNew(new ui::widgets::StaticText(text, util::SkinColor::Static, gfx::FontRequest(), m_root.provider()));
}



/*
 *  Main Entry Point
 */

void
client::dialogs::doExport(ui::Root& root,
                          util::RequestSender<game::proxy::ExportAdaptor> adaptorSender,
                          util::RequestSender<game::Session> gameSender,
                          afl::string::Translator& tx)
{
    // ex EditExportFieldList
    ExportProxy proxy(adaptorSender, root.engine().dispatcher());
    ExportDialog dlg(root, proxy, gameSender, tx);
    dlg.init();
    dlg.run();
}
