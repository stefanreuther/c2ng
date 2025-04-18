/**
  *  \file client/dialogs/preferences.cpp
  *  \brief Preferences Dialog
  */

#include "client/dialogs/preferences.hpp"
#include "afl/container/ptrvector.hpp"
#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/widgets/configstoragecontrol.hpp"
#include "client/widgets/configvaluelist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/actions/preconditions.hpp"
#include "game/interface/configurationeditorcontext.hpp"
#include "game/proxy/configurationeditoradaptor.hpp"
#include "game/proxy/configurationeditorproxy.hpp"
#include "game/root.hpp"
#include "interpreter/error.hpp"
#include "ui/cardgroup.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/treelistbox.hpp"
#include "ui/window.hpp"

using client::si::OutputState;
using client::si::RequestLink2;
using client::si::UserSide;
using game::config::ConfigurationEditor;
using game::interface::ConfigurationEditorContext;
using game::proxy::ConfigurationEditorAdaptor;
using game::proxy::ConfigurationEditorProxy;
using interpreter::VariableReference;
using ui::widgets::FrameGroup;
using ui::widgets::ScrollbarContainer;

namespace {
    /*
     *  ScriptAdaptor - Adaptor for a ConfigurationEditor scripted using ConfigurationEditorContext
     */

    class ScriptAdaptor : public ConfigurationEditorAdaptor {
     public:
        ScriptAdaptor(const ConfigurationEditorContext::DataRef& data)
            : m_data(data),
              m_root(game::actions::mustHaveRoot(data.ref->session))
            { }
        virtual game::config::Configuration& config()
            { return m_root->userConfiguration(); }
        virtual ConfigurationEditor& editor()
            { return m_data.ref->editor; }
        virtual afl::string::Translator& translator()
            { return m_data.ref->session.translator(); }
        virtual void notifyListeners()
            { m_root->userConfiguration().notifyListeners(); }

     private:
        // Keep both the DataRef_t and the Root alive
        ConfigurationEditorContext::DataRef m_data;
        afl::base::Ref<game::Root> m_root;
    };

    class ScriptAdaptorFromSession : public afl::base::Closure<ConfigurationEditorAdaptor*(game::Session&)> {
     public:
        ScriptAdaptorFromSession(const VariableReference& ref)
            : m_reference(ref)
            { }
        virtual ConfigurationEditorAdaptor* call(game::Session& session)
            {
                std::auto_ptr<afl::data::Value> value(m_reference.get(session.processList()));
                ConfigurationEditorContext* ctx = dynamic_cast<ConfigurationEditorContext*>(value.get());
                if (ctx == 0) {
                    throw interpreter::Error::typeError();
                }
                return new ScriptAdaptor(ctx->data());
            }

     private:
        VariableReference m_reference;
    };


    /*
     *  WholeConfigAdaptor - Adaptor for a ConfigurationEditor editing the entire UserConfiguration
     */

    class WholeConfigAdaptor : public ConfigurationEditorAdaptor {
     public:
        WholeConfigAdaptor(game::Session& session)
            : m_session(session),
              m_root(game::actions::mustHaveRoot(session))
            {
                m_editor.addAll(0, ConfigurationEditor::DefaultEditor, m_root->userConfiguration());
            }
        virtual game::config::Configuration& config()
            { return m_root->userConfiguration(); }
        virtual ConfigurationEditor& editor()
            { return m_editor; }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual void notifyListeners()
            { m_root->userConfiguration().notifyListeners(); }

     private:
        game::Session& m_session;
        afl::base::Ref<game::Root> m_root;
        ConfigurationEditor m_editor;
    };

    class WholeConfigAdaptorFromSession : public afl::base::Closure<ConfigurationEditorAdaptor*(game::Session&)> {
     public:
        ConfigurationEditorAdaptor* call(game::Session& session)
            { return new WholeConfigAdaptor(session); }
    };

    /** Load tree structure from a VariableReference.
        ConfigurationEditorContext maintains a tree structure that is not visible on the ConfigurationEditorProxy,
        so this is an ad-hoc function to retrieve it.
        @param gameSender   Game sender
        @param ind          WaitIndicator for UI synchronisation
        @param ref          Reference to variable containing ConfigurationEditorProxy
        @return TreeList */
    util::TreeList loadTree(util::RequestSender<game::Session> gameSender, game::proxy::WaitIndicator& ind, const VariableReference& ref)
    {
        class Task : public util::Request<game::Session> {
         public:
            Task(const VariableReference& ref, util::TreeList& out)
                : m_reference(ref), m_out(out)
                { }
            virtual void handle(game::Session& session)
                {
                    std::auto_ptr<afl::data::Value> value(m_reference.get(session.processList()));
                    if (ConfigurationEditorContext* ctx = dynamic_cast<ConfigurationEditorContext*>(value.get())) {
                        m_out = ctx->data().ref->optionNames;
                    }
                }
         private:
            VariableReference m_reference;
            util::TreeList& m_out;
        };
        util::TreeList out;
        Task t(ref, out);
        ind.call(gameSender, t);
        return out;
    }


    /*
     *  PreferenceValueDisplay - compound widget to display the value of an option
     */

    class PreferenceValueDisplay {
     public:
        PreferenceValueDisplay(ui::Root& root, afl::string::Translator& tx);
        ui::Widget& createWidget(afl::base::Deleter& del);
        void setContent(const String_t& title, const String_t& value, bool isEditable);

        afl::base::Signal<void()> sig_edit;

     private:
        afl::string::Translator& m_translator;
        ui::widgets::StaticText m_title;
        ui::widgets::StaticText m_value;
        ui::widgets::Button m_editButton;
    };


    /*
     *  BasePage - Base class for compound widget of a dialog page
     */

    class BasePage {
     public:
        BasePage(UserSide& us, ConfigurationEditorProxy& proxy, const util::KeyString& name);

        // Virtuals:
        virtual ~BasePage()
            { }

        /** Access underlying list widget.
            @return list widget */
        virtual ui::ScrollableWidget& getListWidget() = 0;

        /** Get current index into ConfigurationEditor.
            @return index */
        virtual size_t getCurrentIndex() const = 0;

        /** User action: edit value.
            Must perform all necessary user interactions. */
        virtual void onEdit() = 0;

        // Accessors:
        const util::KeyString& getName() const
            { return m_name; }
        UserSide& userSide()
            { return m_userSide; }
        ConfigurationEditorProxy& proxy()
            { return m_proxy; }

        /** Create the widget structure.
            Can be called once.
            @param del Deleter
            @return Newly-created widget tree */
        ui::Widget& createWidget(afl::base::Deleter& del);

        /** Set current value for display.
            @param name Name to use (may be different from p->name)
            @param p    Current value information. Can be null. */
        void setValue(const String_t& name, const ConfigurationEditor::Info* p);

     private:
        void onStorageChange(game::config::ConfigurationOption::Source source);

        UserSide& m_userSide;
        ConfigurationEditorProxy& m_proxy;
        PreferenceValueDisplay m_valueDisplay;
        client::widgets::ConfigStorageControl m_storageDisplay;
        util::KeyString m_name;
    };


    /*
     *  TreePage - compound widget to display the tree structure prepared in a ConfigurationEditorContext
     */

    class TreePage : public BasePage {
     public:
        TreePage(UserSide& us, ConfigurationEditorProxy& proxy, const util::TreeList& tree, size_t rootNode, const VariableReference& ref, const util::KeyString& name);

        ui::ScrollableWidget& getListWidget()
            { return m_list; }
        size_t getCurrentIndex() const;
        void onEdit();

     private:
        void onUpdate();

        ui::widgets::TreeListbox m_list;
        VariableReference m_reference;
        afl::base::SignalConnection conn_itemChange;
    };



    /*
     *  WholePage - compound widget to display the entire configuration
     */

    class WholePage : public BasePage {
     public:
        WholePage(UserSide& us, ConfigurationEditorProxy& proxy);

        ui::ScrollableWidget& getListWidget()
            { return m_list; }
        size_t getCurrentIndex() const;
        void onEdit();

     private:
        void onUpdate(size_t index, const game::config::ConfigurationEditor::Info& info);
        void onMove();

        client::widgets::ConfigValueList m_list;
        afl::base::SignalConnection conn_itemChange;
    };


    // Dialog class
    class Dialog : public client::si::Control {
     public:
        Dialog(UserSide& us, const afl::container::PtrVector<BasePage>& pages, client::si::OutputState& out);
        void run();

        // Control:
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target)
            { return dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
        virtual void handleEndDialog(RequestLink2 link, int code)
            { return dialogHandleEndDialog(link, code, m_outputState, m_loop, 0); }
        virtual void handlePopupConsole(RequestLink2 link)
            { return defaultHandlePopupConsole(link); }
        virtual void handleScanKeyboardMode(RequestLink2 link)
            { return defaultHandleScanKeyboardMode(link); }
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap)
            { return defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix)
            { return defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(RequestLink2 link, String_t text)
            { return defaultHandleOverlayMessage(link, text); }
        afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const
            { return defaultGetFocusedObjectId(type); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }

     private:
        ui::EventLoop m_loop;
        client::si::OutputState& m_outputState;
        const afl::container::PtrVector<BasePage>& m_pages;
    };
}


/*
 *  PreferenceValueDisplay
 */

PreferenceValueDisplay::PreferenceValueDisplay(ui::Root& root, afl::string::Translator& tx)
    : m_translator(tx),
      m_title(String_t(), util::SkinColor::Static, "b", root.provider()),
      m_value(String_t(), util::SkinColor::Static, "", root.provider()),
      m_editButton(tx("Enter - Change"), util::Key_Return, root)
{
    // ex WConfigEditor::getLayoutInfo [sort-of]
    const int w = root.provider().getFont(gfx::FontRequest())->getEmWidth() * 20;
    m_title.setForcedWidth(w);
    m_value.setForcedWidth(w);
    m_editButton.sig_fire.add(&sig_edit, &afl::base::Signal<void()>::raise);
}

ui::Widget&
PreferenceValueDisplay::createWidget(afl::base::Deleter& del)
{
    ui::Group& g = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g.add(m_title);
    g.add(m_value);

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(m_editButton);
    g.add(g2);

    return g;
}

void
PreferenceValueDisplay::setContent(const String_t& title, const String_t& value, bool isEditable)
{
    // ex WConfigEditor::drawContent
    m_title.setText(title);
    if (isEditable && value.empty()) {
        m_value.setText(m_translator("(empty)"));
        m_value.setColor(util::SkinColor::Faded);
    } else {
        m_value.setText(value);
        m_value.setColor(util::SkinColor::Static);
    }
    m_editButton.setState(ui::Widget::DisabledState, !isEditable);
}


/*
 *  BasePage
 */


BasePage::BasePage(UserSide& us, ConfigurationEditorProxy& proxy, const util::KeyString& name)
    : m_userSide(us),
      m_proxy(proxy),
      m_valueDisplay(us.root(), us.translator()),
      m_storageDisplay(us.root(), us.translator()),
      m_name(name)
{
    m_storageDisplay.sig_change.add(this, &BasePage::onStorageChange);
    m_valueDisplay.sig_edit.add(this, &BasePage::onEdit);
}

ui::Widget&
BasePage::createWidget(afl::base::Deleter& del)
{
    // HBox
    //   Framed ConfigValueList
    //   VBox
    //     Editor
    //     Spacer
    //     Storage
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(FrameGroup::wrapWidget(del, m_userSide.root().colorScheme(), ui::LoweredFrame, del.addNew(new ScrollbarContainer(getListWidget(), m_userSide.root()))));

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g2.add(m_valueDisplay.createWidget(del));
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(m_storageDisplay);
    g.add(g2);

    return g;
}

void
BasePage::setValue(const String_t& name, const ConfigurationEditor::Info* p)
{
    if (p != 0 && p->type != 0) {
        m_valueDisplay.setContent(name, p->value, true);
        m_storageDisplay.setSource(p->source);
    } else {
        m_valueDisplay.setContent(String_t(), String_t(), false);
        m_storageDisplay.setSource(ConfigurationEditor::NotStored);
    }
}

void
BasePage::onStorageChange(game::config::ConfigurationOption::Source source)
{
    m_proxy.setSource(getCurrentIndex(), source);
}


/*
 *  TreePage
 */

TreePage::TreePage(UserSide& us, ConfigurationEditorProxy& proxy, const util::TreeList& tree, size_t rootNode, const VariableReference& ref, const util::KeyString& name)
    : BasePage(us, proxy, name),
      m_list(us.root(), 20, 25*us.root().provider().getFont(gfx::FontRequest())->getEmWidth()),
      m_reference(ref),
      conn_itemChange(proxy.sig_itemChange.add(this, &TreePage::onUpdate))
{
    m_list.addTree(0, tree, rootNode);
    m_list.sig_change.add(this, &TreePage::onUpdate);

    onUpdate();
}

size_t
TreePage::getCurrentIndex() const
{
    int32_t currentId = m_list.getIdFromNode(m_list.getCurrentNode());
    return ConfigurationEditorContext::getEditorIndexFromTreeId(currentId);
}

void
TreePage::onEdit()
{
    // ex WConfigEditor::onEdit (sort-of)
    class Task : public client::si::ScriptTask {
     public:
        Task(const VariableReference& ref, size_t index)
            : m_reference(ref), m_index(index)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                // Compile
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                std::auto_ptr<afl::data::Value> value(m_reference.get(session.processList()));
                if (ConfigurationEditorContext* ctx = dynamic_cast<ConfigurationEditorContext*>(value.get())) {
                    ctx->compileEditor(*bco, m_index);
                }

                // Create process
                interpreter::Process& proc = session.processList().create(session.world(), "(Preferences)");
                proc.pushFrame(bco, false);
                session.processList().resumeProcess(proc, pgid);
            }
     private:
        VariableReference m_reference;
        size_t m_index;
    };

    int32_t currentId = m_list.getIdFromNode(m_list.getCurrentNode());
    size_t currentIndex = ConfigurationEditorContext::getEditorIndexFromTreeId(currentId);
    const ConfigurationEditorProxy::Infos_t& infos = proxy().getValues();
    if (currentIndex < infos.size() && infos[currentIndex].type == ConfigurationEditorContext::ScriptEditor) {
        if (client::si::Control* ctl = userSide().getControl()) {
            ctl->executeTaskWait(std::auto_ptr<client::si::ScriptTask>(new Task(m_reference, currentIndex)));
        }
    }
}

void
TreePage::onUpdate()
{
    const ui::widgets::TreeListbox::Node* n = m_list.getCurrentNode();
    int32_t currentId = m_list.getIdFromNode(n);
    size_t currentIndex = ConfigurationEditorContext::getEditorIndexFromTreeId(currentId);

    const ConfigurationEditorProxy::Infos_t& infos = proxy().getValues();
    const ConfigurationEditor::Info* p = (currentIndex < infos.size() ? &infos[currentIndex] : 0);

    setValue(m_list.getLabelFromNode(n), p);
}




/*
 *  WholePage
 */

WholePage::WholePage(UserSide& us, ConfigurationEditorProxy& proxy)
    : BasePage(us, proxy, util::KeyString(us.translator()("Configuration File"))),
      m_list(us.root()),
      conn_itemChange(proxy.sig_itemChange.add(this, &WholePage::onUpdate))
{
    m_list.setContent(proxy.getValues());
    m_list.setHighlightedSource(ConfigurationEditor::User);
    m_list.setNameColumnWidth(13);
    m_list.setValueColumnWidth(13);
    m_list.sig_change.add(this, &WholePage::onMove);

    onMove();
}

size_t
WholePage::getCurrentIndex() const
{
    return m_list.getCurrentItem();
}

void
WholePage::onEdit()
{
    // ex WConfigOptionEditor::edit (sort-of)
    if (const ConfigurationEditor::Info* p = m_list.getCurrentOption()) {
        afl::string::Translator& tx = userSide().translator();
        ui::widgets::InputLine input(10000, 20, userSide().root());
        input.setText(p->value);
        if (input.doStandardDialog(tx("Edit Option"), tx("New Value:"), tx)) {
            // FIXME: PCC2 loops if setting fails
            proxy().setValue(m_list.getCurrentItem(), input.getText());
        }
    }
}

void
WholePage::onUpdate(size_t index, const game::config::ConfigurationEditor::Info& info)
{
    m_list.setItemContent(index, info);
    onMove();
}

void
WholePage::onMove()
{
    const ConfigurationEditor::Info* p = m_list.getCurrentOption();
    setValue(p ? p->name : String_t(), p);
}


/*
 *  Dialog
 */

Dialog::Dialog(UserSide& us, const afl::container::PtrVector<BasePage>& pages, client::si::OutputState& out)
    : Control(us),
      m_loop(us.root()),
      m_outputState(out),
      m_pages(pages)
{ }

void
Dialog::run()
{
    // ex WPreferencesDialog::WPreferencesDialog, WPreferencesDialog::init (sort-of)
    // VBox
    //   CardTabBar
    //   CardGroup [pages...]
    //   HBox
    //     Button "Close"
    //     Spacer
    //     Button "Help"
    afl::string::Translator& tx = translator();
    afl::base::Deleter del;

    // Window
    ui::Window& win = del.addNew(new ui::Window(tx("Settings"), root().provider(), root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Cards
    ui::CardGroup cards;
    ui::widgets::CardTabBar tabs(root(), cards);
    for (size_t i = 0, n = m_pages.size(); i < n; ++i) {
        ui::Widget& widget = m_pages[i]->createWidget(del);
        cards.add(widget);
        tabs.addPage(m_pages[i]->getName(), widget);
    }
    win.add(tabs);
    win.add(cards);

    // Buttons
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(tx("Close"), util::Key_Escape, root()));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(tx("Help"), 'h', root()));
    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    buttonGroup.add(btnClose);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnHelp);
    win.add(buttonGroup);

    // Internals
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root(), translator(), interface().gameSender(), "pcc2:settings"));
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(root(), m_loop)));

    // Connect events
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    // Run it
    win.pack();
    root().centerWidget(win);
    root().add(win);
    m_loop.run();
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doPreferencesDialog(client::si::UserSide& us, const interpreter::VariableReference& options, client::si::OutputState& out)
{
    // ex client/dialogs/prefdlg.cc:doPreferencesDialog
    // Prepare everything
    Downlink link(us);
    ConfigurationEditorProxy treeProxy(us.gameSender().makeTemporary(new ScriptAdaptorFromSession(options)), us.root().engine().dispatcher());
    ConfigurationEditorProxy wholeProxy(us.gameSender().makeTemporary(new WholeConfigAdaptorFromSession()), us.root().engine().dispatcher());
    treeProxy.loadValues(link);
    wholeProxy.loadValues(link);

    // Dialog parts
    afl::container::PtrVector<BasePage> pages;
    util::TreeList tree = loadTree(us.gameSender(), link, options);
    for (size_t i = tree.getFirstChild(util::TreeList::root); i != util::TreeList::nil; i = tree.getNextSibling(i)) {
        String_t label;
        int32_t key;
        if (tree.hasChildren(i) && tree.get(i, key, label)) {
            pages.pushBackNew(new TreePage(us, treeProxy, tree, i, options, util::KeyString(label)));
        }
    }
    pages.pushBackNew(new WholePage(us, wholeProxy));

    // Run the dialog
    Dialog(us, pages, out).run();
}
