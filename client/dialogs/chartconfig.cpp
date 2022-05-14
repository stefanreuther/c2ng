/**
  *  \file client/dialogs/chartconfig.cpp
  */

#include "client/dialogs/chartconfig.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/observable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "client/downlink.hpp"
#include "client/widgets/chartdisplayconfig.hpp"
#include "client/widgets/chartmouseconfig.hpp"
#include "client/widgets/configstoragecontrol.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/markercolorselector.hpp"
#include "client/widgets/markerkindselector.hpp"
#include "client/widgets/markertemplatelist.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/configurationeditor.hpp"
#include "game/map/configuration.hpp"
#include "game/map/renderoptions.hpp"
#include "game/proxy/configurationeditoradaptor.hpp"
#include "game/proxy/configurationeditorproxy.hpp"
#include "game/proxy/mapconfigurationproxy.hpp"
#include "game/root.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/scrollbar.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using afl::string::Format;
using client::widgets::ChartDisplayConfig;
using client::widgets::ChartMouseConfig;
using client::widgets::ConfigStorageControl;
using client::widgets::MarkerTemplateList;
using game::config::ConfigurationEditor;
using game::config::ConfigurationOption;
using game::config::MarkerOption;
using game::config::UserConfiguration;
using game::map::Configuration;
using game::map::Point;
using game::map::RenderOptions;
using game::proxy::ConfigurationEditorProxy;
using game::proxy::MapConfigurationProxy;
using ui::Widget;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::FrameGroup;
using ui::widgets::InputLine;
using ui::widgets::RadioButton;
using ui::widgets::StaticText;

namespace {
    /* Indexes into ConfigurationEditor */
    enum {
        IdxDisplayOptions,
        IdxMarkerOptions,
        IdxMouseOptions,
        NumOptionGroups
    };

    /* Values for Map Geometry radio button */
    enum {
        ValFlat,
        ValWrapped,
        ValCircular
    };

    /*
     *  ConfigurationEditorAdaptor Implementation
     */

    class Adaptor : public game::proxy::ConfigurationEditorAdaptor {
     public:
        Adaptor(game::Session& session)
            : m_session(session),
              m_editor()
            {
                // Display options
                ConfigurationEditor::GenericNode& n1 = m_editor.addGeneric(0, "", 0, "");
                for (size_t i = 0; i < countof(UserConfiguration::ChartRenderOptions); ++i) {
                    for (size_t j = 0; j < countof(UserConfiguration::ChartRenderOptions[0]); ++j) {
                        n1.addOption(UserConfiguration::ChartRenderOptions[i][j]);
                    }
                }

                // Marker options
                ConfigurationEditor::GenericNode& n2 = m_editor.addGeneric(0, "", 0, "");
                for (int i = 0; i < UserConfiguration::NUM_CANNED_MARKERS; ++i) {
                    n2.addOption(*UserConfiguration::getCannedMarker(i));
                }

                // Mouse options
                ConfigurationEditor::GenericNode& n3 = m_editor.addGeneric(0, "", 0, "");
                n3.addOption(UserConfiguration::Lock_Left);
                n3.addOption(UserConfiguration::Lock_Right);
                n3.addOption(UserConfiguration::ChartWheel);
            }
        virtual game::config::Configuration& config()
            { return game::actions::mustHaveRoot(m_session).userConfiguration(); }
        virtual ConfigurationEditor& editor()
            { return m_editor; }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual void notifyListeners()
            { /* We explicitly flush all at once */ }
     private:
        game::Session& m_session;
        ConfigurationEditor m_editor;
    };

    class AdaptorFromSession : public afl::base::Closure<game::proxy::ConfigurationEditorAdaptor*(game::Session&)> {
     public:
        virtual game::proxy::ConfigurationEditorAdaptor* call(game::Session& session)
            { return new Adaptor(session); }
    };


    /*
     *  ChartConfigDialog
     *
     *  An information is committed to the game whenever it is confirmed with "OK", that is:
     *  - changes to markers are immediately committed
     *  - changes to display, geometry, mouse options and storage locations
     *    are committed when the dialog is confirmed
     *
     *  The dialog is otherwise passive and doesn't take updates from the game.
     *  If the game changes underneath, that will be overwritten when the dialog is confirmed.
     */

    class ChartConfigDialog {
     public:
        ChartConfigDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        // Main entry points
        void load(client::Downlink& link);
        void run();

     private:
        enum SaveMapResult {
            Success,
            BadCenter,
            BadSize
        };

        // Event handlers
        void onOK();
        void onEditMarkerType();
        void onEditMarkerName();
        void onGeometryPageFocused();

        SaveMapResult saveMapConfig();

        // Connections
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;

        // Proxies
        ConfigurationEditorProxy m_ceProxy;
        MapConfigurationProxy m_mcProxy;

        // Display page
        ConfigStorageControl m_displayStorage;
        ChartDisplayConfig m_displayConfig;

        // Geometry page
        afl::base::Observable<int32_t> m_geoKind;
        InputLine m_geoCenter;
        InputLine m_geoSize;
        bool m_geoNeedWarning;

        // Marker page
        ConfigStorageControl m_markerStorage;
        MarkerTemplateList m_markerList;
        std::vector<MarkerOption::Data> m_markerData;

        // Mouse page
        ConfigStorageControl m_mouseStorage;
        ChartMouseConfig m_mouseConfig;

        afl::base::Optional<ConfigurationOption::Source> m_newSources[NumOptionGroups];
    };
}

ChartConfigDialog::ChartConfigDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_loop(root),
      m_ceProxy(gameSender.makeTemporary(new AdaptorFromSession()), root.engine().dispatcher()),
      m_mcProxy(gameSender),
      m_displayStorage(root, tx),
      m_displayConfig(root, tx),
      m_geoKind(),
      m_geoCenter(20, 9, root),
      m_geoSize(20, 9, root),
      m_markerStorage(root, tx),
      m_markerList(root, tx),
      m_markerData(),
      m_mouseStorage(root, tx),
      m_mouseConfig(root, tx)
{
    m_geoCenter.setFlag(InputLine::NoHi, true);
    m_geoCenter.setHotkey('c');
    m_geoSize.setFlag(InputLine::NoHi, true);
    m_geoSize.setHotkey('s');
}

/* Load content from game and populate all widgets */
void
ChartConfigDialog::load(client::Downlink& link)
{
    // Display
    // ex WChartDisplayConfig::load
    for (size_t i = 0; i < RenderOptions::NUM_AREAS; ++i) {
        RenderOptions::Area a = RenderOptions::Area(i);
        m_displayConfig.set(a, m_mcProxy.getRenderOptions(link, a));
    }

    // Geometry
    Configuration config;
    m_mcProxy.getMapConfiguration(link, config);
    switch (config.getMode()) {
     case Configuration::Flat:     m_geoKind.set(ValFlat);     break;
     case Configuration::Wrapped:  m_geoKind.set(ValWrapped);  break;
     case Configuration::Circular: m_geoKind.set(ValCircular); break;
    }
    Point geoCenter = config.getCenter();
    Point geoSize = config.getSize();
    m_geoCenter.setText(Format("%d,%d", geoCenter.getX(), geoCenter.getY()));
    if (geoSize.getX() == geoSize.getY()) {
        m_geoSize.setText(Format("%d", geoSize.getX()));
    } else {
        m_geoSize.setText(Format("%d,%d", geoSize.getX(), geoSize.getY()));
    }
    m_geoNeedWarning = config.isSetFromHostConfiguration();

    // Markers
    m_mcProxy.getMarkerConfiguration(link, m_markerData);
    m_markerList.setContent(m_markerData);

    // Storage
    m_ceProxy.loadValues(link);
    const ConfigurationEditorProxy::Infos_t& infos = m_ceProxy.getValues();
    if (infos.size() == NumOptionGroups) {
        m_displayStorage.setSource(infos[IdxDisplayOptions].source);
        m_markerStorage.setSource(infos[IdxMarkerOptions].source);
        m_mouseStorage.setSource(infos[IdxMouseOptions].source);
    }

    // Mouse
    // ex WChartLockConfig::load
    m_mouseConfig.set(m_mcProxy.getOption(link, UserConfiguration::Lock_Left),
                      m_mcProxy.getOption(link, UserConfiguration::Lock_Right),
                      m_mcProxy.getOption(link, UserConfiguration::ChartWheel));
}

/* Run dialog */
void
ChartConfigDialog::run()
{
    // ex WChartConfigDialog::init()
    // VBox
    //   CardTabBar
    //   CardGroup [4 Pages]
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Starchart Configuration"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::CardGroup& cards = del.addNew(new ui::CardGroup());
    ui::widgets::CardTabBar& tabs = del.addNew(new ui::widgets::CardTabBar(m_root, cards));

    // "Display" page
    //     VBox "Display"
    //       HBox
    //         FrameGroup > ChartDisplayConfig
    //         Spacer
    //       Spacer
    //       ConfigStorageControl
    ui::Group& displayGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& displayGroup1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::LayoutableGroup& displayFrame = del.addNew(new FrameGroup(ui::layout::HBox::instance0, m_root.colorScheme(), ui::LoweredFrame));
    displayFrame.add(m_displayConfig);
    displayFrame.add(del.addNew(new ui::widgets::Scrollbar(m_displayConfig, m_root)));
    displayGroup1.add(displayFrame);
    displayGroup1.add(del.addNew(new ui::Spacer()));
    displayGroup.add(displayGroup1);
    displayGroup.add(del.addNew(new ui::Spacer()));
    displayGroup.add(m_displayStorage);

    // "Geometry" page
    //     VBox "Geometry"
    //       StaticText "Map type"
    //       HBox
    //         Spacer
    //         VBox
    //           RadioButton (3x)
    //       Grid
    //         "Center"  InputLine
    //         "Size"  InputLine
    //       Spacer
    ui::Group& geoGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& geoGroup1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& geoGroup11 = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    ui::Group& geoGroup2 = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));

    geoGroup.add(del.addNew(new StaticText(m_translator("Map type:"), util::SkinColor::Static, "+", m_root.provider())));
    geoGroup.add(geoGroup1);
    geoGroup1.add(del.addNew(new StaticText("  ", ui::SkinColor::Static, "+", m_root.provider())));  // Spacer
    geoGroup1.add(geoGroup11);

    Widget& radio1 = del.addNew(new RadioButton(m_root, 'n', m_translator("Normal (plane)"),                m_geoKind, ValFlat));
    Widget& radio2 = del.addNew(new RadioButton(m_root, 'w', m_translator("Wrapped (rectangular, Sphere)"), m_geoKind, ValWrapped));
    Widget& radio3 = del.addNew(new RadioButton(m_root, 'r', m_translator("Round wrap (circular, PWrap)"),  m_geoKind, ValCircular));
    geoGroup11.add(radio1);
    geoGroup11.add(radio2);
    geoGroup11.add(radio3);
    geoGroup1.add(del.addNew(new ui::Spacer()));
    geoGroup.add(geoGroup2);
    geoGroup2.add(del.addNew(new StaticText(m_translator("Center:"), util::SkinColor::Static, "+", m_root.provider())));
    geoGroup2.add(FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_geoCenter));
    geoGroup2.add(del.addNew(new StaticText(m_translator("Size:"), util::SkinColor::Static, "+", m_root.provider())));
    geoGroup2.add(FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_geoSize));
    geoGroup.add(del.addNew(new ui::Spacer()));

    ui::widgets::FocusIterator& geoIt = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical));
    geoIt.add(radio1);
    geoIt.add(radio2);
    geoIt.add(radio3);
    geoIt.add(m_geoCenter);
    geoIt.add(m_geoSize);
    geoGroup.add(geoIt);

    // "Markers" page
    //     VBox "Markers"
    //       HBox
    //         MarkerList
    //         VBox
    //           Button "Edit"
    //           Button "Name"
    //           Spacer
    //       Spacer
    //       ConfigStorageControl
    ui::Group& markerGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& markerGroup1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& markerButtons = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    markerGroup.add(markerGroup1);
    markerGroup1.add(m_markerList);
    markerGroup1.add(markerButtons);
    markerGroup1.add(del.addNew(new ui::Spacer()));
    markerGroup.add(del.addNew(new ui::Spacer()));
    markerGroup.add(m_markerStorage);

    Button& markerEditButton = del.addNew(new Button(m_translator("Space - Edit"), ' ', m_root));
    Button& markerNameButton = del.addNew(new Button(m_translator("N - Name"), 'n', m_root));
    markerButtons.add(markerEditButton);
    markerButtons.add(markerNameButton);
    markerButtons.add(del.addNew(new ui::Spacer()));
    markerEditButton.sig_fire.add(this, &ChartConfigDialog::onEditMarkerType);
    markerNameButton.sig_fire.add(this, &ChartConfigDialog::onEditMarkerName);

    // "Mouse" page
    //     VBox "Mouse"
    //       HBox
    //         FrameGroup > ChartLockConfig
    //         Spacer
    //       Spacer
    //       ConfigStorageControl
    ui::Group& mouseGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& mouseGroup1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::LayoutableGroup& mouseFrame = del.addNew(new FrameGroup(ui::layout::HBox::instance0, m_root.colorScheme(), ui::LoweredFrame));
    mouseFrame.add(m_mouseConfig);
    mouseFrame.add(del.addNew(new ui::widgets::Scrollbar(m_mouseConfig, m_root)));
    mouseGroup1.add(mouseFrame);
    mouseGroup1.add(del.addNew(new ui::Spacer()));
    mouseGroup.add(mouseGroup1);
    mouseGroup.add(del.addNew(new ui::Spacer()));
    mouseGroup.add(m_mouseStorage);

    // Create pages
    cards.add(displayGroup);
    cards.add(geoGroup);
    cards.add(markerGroup);
    cards.add(mouseGroup);
    tabs.addPage(util::KeyString(m_translator("Display")), displayGroup);
    tabs.addPage(util::KeyString(m_translator("Geometry")), geoGroup);
    tabs.addPage(util::KeyString(m_translator("Markers")), markerGroup);
    tabs.addPage(util::KeyString(m_translator("Mouse")), mouseGroup);
    win.add(tabs);
    win.add(cards);

    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    btn.ok().sig_fire.add(this, &ChartConfigDialog::onOK);
    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(1));
    win.add(btn);

    Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:starchartopts"));
    btn.addHelp(help);
    win.add(help);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    it.add(displayGroup);
    it.add(geoGroup);
    it.add(markerGroup);
    it.add(mouseGroup);
    win.add(it);
    m_displayConfig.requestFocus();

    // Events for Storage buttons
    class StorageChangeHandler : public afl::base::Closure<void(ConfigurationOption::Source)> {
     public:
        StorageChangeHandler(ChartConfigDialog& parent, size_t index, ConfigStorageControl& widget)
            : m_parent(parent), m_index(index), m_widget(widget)
            { }
        virtual void call(ConfigurationOption::Source src)
            {
                m_parent.m_newSources[m_index] = src;
                m_widget.setSource(ConfigurationEditor::convertSource(src));
            }
     private:
        ChartConfigDialog& m_parent;
        size_t m_index;
        ConfigStorageControl& m_widget;
    };
    m_displayStorage.sig_change.addNewClosure(new StorageChangeHandler(*this, IdxDisplayOptions, m_displayStorage));
    m_markerStorage.sig_change.addNewClosure(new StorageChangeHandler(*this, IdxMarkerOptions, m_markerStorage));
    m_mouseStorage.sig_change.addNewClosure(new StorageChangeHandler(*this, IdxMouseOptions, m_mouseStorage));

    // Focus change
    class FocusHandler : public afl::base::Closure<void()> {
     public:
        FocusHandler(ChartConfigDialog& parent, Widget& geoGroup)
            : m_parent(parent), m_geoGroup(geoGroup)
            { }
        virtual void call()
            {
                if (m_geoGroup.hasState(Widget::FocusedState)) {
                    m_parent.onGeometryPageFocused();
                }
            }
     private:
        ChartConfigDialog& m_parent;
        Widget& m_geoGroup;
    };
    cards.sig_handleFocusChange.addNewClosure(new FocusHandler(*this, geoGroup));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

/* Event handler: main "OK" button. Try to save everything; on success, stop dialog. */
void
ChartConfigDialog::onOK()
{
    // ex WChartConfigDialog::onOK()
    // Map config can block saving the rest
    switch (saveMapConfig()) {
     case BadSize:
        m_geoSize.requestFocus();
        break;

     case BadCenter:
        m_geoCenter.requestFocus();
        break;

     case Success:
        // Display Settings - ex WChartDisplayConfig::save()
        for (size_t i = 0; i < RenderOptions::NUM_AREAS; ++i) {
            RenderOptions::Area a = RenderOptions::Area(i);
            m_mcProxy.setRenderOptions(a, m_displayConfig.get(a));
        }

        // Mouse Settings - ex WChartLockConfig::save()
        m_mcProxy.setOption(UserConfiguration::Lock_Left, m_mouseConfig.getLeftLock());
        m_mcProxy.setOption(UserConfiguration::Lock_Right, m_mouseConfig.getRightLock());
        m_mcProxy.setOption(UserConfiguration::ChartWheel, m_mouseConfig.getWheelMode());

        // Storage Settings
        for (size_t i = 0; i < NumOptionGroups; ++i) {
            if (const ConfigurationOption::Source* p = m_newSources[i].get()) {
                m_ceProxy.setSource(i, *p);
            }
        }

        // Notify everyone
        m_gameSender.postRequest(&game::Session::notifyListeners);
        m_loop.stop(1);
        break;
    }
}

/* Event handler: "edit marker type" button */
void
ChartConfigDialog::onEditMarkerType()
{
    // ex WChartConfigDialog::onEditMarkerType()
    size_t pos = m_markerList.getCurrentItem();
    if (pos >= m_markerData.size()) {
        return;
    }

    // Edit type
    client::widgets::MarkerKindSelector mks(m_root);
    mks.setMarkerKind(m_markerData[pos].markerKind);
    if (!mks.doStandardDialog(m_translator("Edit Marker"), m_translator)) {
        return;
    }

    // Edit color
    client::widgets::MarkerColorSelector mcs(m_root);
    mcs.setColor(m_markerData[pos].color);
    if (!mcs.doStandardDialog(m_translator("Edit Marker"), m_translator, 0)) {
        return;
    }

    // Set it
    m_markerData[pos].markerKind = static_cast<uint8_t>(mks.getMarkerKind());
    m_markerData[pos].color = mcs.getColor();
    m_markerList.setContent(m_markerData);
    m_mcProxy.setMarkerConfiguration(pos, m_markerData[pos]);
}

/* Event handler: "edit marker name" button */
void
ChartConfigDialog::onEditMarkerName()
{
    // ex WChartConfigDialog::onEditMarkerName()
    size_t pos = m_markerList.getCurrentItem();
    if (pos >= m_markerData.size()) {
        return;
    }

    // Edit name
    InputLine input(255, m_root);
    input.setFlag(InputLine::GameChars, true);
    input.setText(m_markerData[pos].note);
    if (!input.doStandardDialog(m_translator("Edit Marker"), m_translator("Name:"), m_translator)) {
        return;
    }

    // Set it
    m_markerData[pos].note = input.getText();
    m_markerList.setContent(m_markerData);
    m_mcProxy.setMarkerConfiguration(pos, m_markerData[pos]);
}

/* Event handler: "Geometry" page got focus */
void
ChartConfigDialog::onGeometryPageFocused()
{
    // ex WChartConfigDialog::onFocusChange() [part/reworked]
    class Task : public afl::base::Runnable {
     public:
        Task(ui::Root& root, afl::string::Translator& tx)
            : m_root(root), m_translator(tx)
            { }
        virtual void run()
            {
                MessageBox(m_translator("These settings have been taking from the host configuration. "
                                        "Changes will only last until PCC2 reads the configuration again. "
                                        "Instead of manually changing Geometry settings, "
                                        "it's usually better to work with a current copy of the host configuration."),
                           m_translator("Starchart Configuration"),
                           m_root)
                    .doOkDialog(m_translator);
            }
     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
    };

    // Post warning dialog once, as task so it is executed after (not during) this callback.
    // The card group will redraw its content from its onFocusChange().
    // However, there is no guarantee that that has been called before this function.
    // PCC2 cheated by calling drawWidget() (which is what CardGroup would do).
    // Posting a task is the safe, guaranteed way.
    if (m_geoNeedWarning) {
        m_geoNeedWarning = false;
        m_root.engine().dispatcher().postNewRunnable(new Task(m_root, m_translator));
    }
}

/* Save map config. If map config is invalid, display a message box and return appropriate status. */
ChartConfigDialog::SaveMapResult
ChartConfigDialog::saveMapConfig()
{
    // ex WChartConfigDialog::saveMapConfig(), chartdlg.pas::CheckGeometryConf
    // Convert geometry selection
    Configuration::Mode m =
        m_geoKind.get() == ValWrapped ? Configuration::Wrapped
        : m_geoKind.get() == ValCircular ? Configuration::Circular : Configuration::Flat;

    // Parse center
    Point center;
    if (!center.parseCoordinates(m_geoCenter.getText())) {
        MessageBox(m_translator("The center coordinate specification is invalid. Please enter an "
                                "expression of the form \"2000,2000\" (X,Y)."),
                   m_translator("Starchart Configuration"),
                   m_root)
            .doOkDialog(m_translator);
        return BadCenter;
    }

    // Parse size
    Point size;
    int x;
    if (afl::string::strToInteger(m_geoSize.getText(), x)) {
        size = Point(x, x);
    } else {
        if (!size.parseCoordinates(m_geoSize.getText())) {
            MessageBox(m_translator("The map extent specification is invalid. Please enter an "
                                    "expression of the form \"1100,1100\" (sizeX,sizeY) or \"1100\" (size)."),
                       m_translator("Starchart Configuration"),
                       m_root)
                .doOkDialog(m_translator);
            return BadSize;
        }
    }

    // Validate ranges
    if (center.getX() < 500 || center.getY() < 500 || center.getX() > 4000 || center.getY() > 4000) {
        MessageBox(m_translator("The center coordinates must be between 500 to 4000."),
                   m_translator("Starchart Configuration"),
                   m_root)
            .doOkDialog(m_translator);
        return BadCenter;
    }

    // Validate size for circular
    if (m == Configuration::Circular) {
        if (size.getX() != size.getY()) {
            MessageBox(m_translator("X and Y size must be equal for circular wrap."),
                       m_translator("Starchart Configuration"),
                       m_root)
                .doOkDialog(m_translator);
            return BadSize;
        }
    }

    // Validate size
    int factor = (m == Configuration::Circular ? 1 : 2);
    if (size.getX() < 500 || size.getY() < 500 || size.getX() > 4000 || size.getY() > 4000 || size.getX() > center.getX()*factor || size.getY() > center.getY()*factor) {
        MessageBox(m_translator("The map size must be between 500 and 4000. "
                                "The values must be smaller than the center coordinate value "
                                "(the map must not contain negative coordinates)."),
                   m_translator("Starchart Configuration"),
                   m_root)
            .doOkDialog(m_translator);
        return BadSize;
    }

    // Is this actually a change?
    Configuration mapConfig;
    mapConfig.setConfiguration(m, center, size);
    // FIXME: must validate and reload planet XYs here [missing in PCC2 as well]
    m_mcProxy.setMapConfiguration(mapConfig);

    return Success;
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doChartConfigDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // ex doChartConfig(), chartdlg.pas:ChartConfig
    ChartConfigDialog dlg(root, gameSender, tx);
    Downlink link(root, tx);
    dlg.load(link);
    dlg.run();
}
