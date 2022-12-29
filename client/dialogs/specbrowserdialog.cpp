/**
  *  \file client/dialogs/specbrowserdialog.cpp
  *  \brief Specification Browser Dialog
  */

#include "client/dialogs/specbrowserdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/choosehull.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/filterdisplay.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/specbrowserproxy.hpp"
#include "game/spec/info/utils.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/icons/image.hpp"
#include "ui/icons/stylableicon.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/widgets/tabbar.hpp"
#include "ui/window.hpp"
#include "util/stringlist.hpp"
#include "util/unicodechars.hpp"

namespace {
    using game::proxy::SpecBrowserProxy;
    using ui::Group;
    using ui::widgets::Button;

    namespace gsi = game::spec::info;

    class EditRangeDialog {
     public:
        EditRangeDialog(ui::Root& root, const gsi::IntRange_t& range, const gsi::IntRange_t& maxRange)
            : m_root(root), m_loop(root), m_maxRange(maxRange),
              m_minValue(range.min()), m_maxValue(range.max())
            {
                m_minValue.sig_change.add(this, &EditRangeDialog::onMinEdit);
                m_maxValue.sig_change.add(this, &EditRangeDialog::onMaxEdit);
            }

        bool run(const String_t& title, afl::string::Translator& tx)
            {
                // VBox
                //   Grid
                //     "From" | DecimalSelector
                //     "To"   | DecimalSelector
                //   StandardDialogButtons
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                ui::Widget& minSel = del.addNew(new ui::widgets::DecimalSelector(m_root, tx, m_minValue, m_maxRange.min(), m_maxRange.max(), 1));
                ui::Widget& maxSel = del.addNew(new ui::widgets::DecimalSelector(m_root, tx, m_maxValue, m_maxRange.min(), m_maxRange.max(), 1));

                ui::Group& g = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));
                g.add(del.addNew(new ui::widgets::StaticText(tx("From"), util::SkinColor::Static, "+", m_root.provider())));
                g.add(minSel);
                g.add(del.addNew(new ui::widgets::StaticText(tx("To"), util::SkinColor::Static, "+", m_root.provider())));
                g.add(maxSel);
                win.add(g);

                ui::widgets::StandardDialogButtons& btns = del.addNew(new ui::widgets::StandardDialogButtons(m_root, tx));
                btns.addStop(m_loop);
                win.add(btns);

                ui::widgets::FocusIterator& fi = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Vertical));
                fi.add(minSel);
                fi.add(maxSel);
                win.add(fi);
                minSel.requestFocus();

                win.pack();;
                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run();
            }

        gsi::IntRange_t getResult() const
            { return gsi::IntRange_t(m_minValue.get(), m_maxValue.get()); }

        void onMinEdit()
            {
                if (m_minValue.get() > m_maxValue.get()) {
                    m_maxValue.set(m_minValue.get());
                }
            }

        void onMaxEdit()
            {
                if (m_minValue.get() > m_maxValue.get()) {
                    m_minValue.set(m_maxValue.get());
                }
            }

     private:
        ui::Root& m_root;
        ui::EventLoop m_loop;
        gsi::IntRange_t m_maxRange;

        afl::base::Observable<int32_t> m_minValue;
        afl::base::Observable<int32_t> m_maxValue;
    };



    bool editRange(ui::Root& root, const String_t& title, gsi::IntRange_t& range, const gsi::IntRange_t& maxRange, afl::string::Translator& tx)
    {
        if (range.empty()) {
            range = maxRange;
        }

        EditRangeDialog dlg(root, range, maxRange);
        if (dlg.run(title, tx)) {
            range = dlg.getResult();
            return true;
        } else {
            return false;
        }
    }



    bool editPlayer(ui::Root& root, const String_t& title, int32_t& player, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
    {
        // Fetch player list
        game::proxy::PlayerProxy proxy(gameSender);
        client::Downlink link(root, tx);
        game::PlayerArray<String_t> names = proxy.getPlayerNames(link, game::Player::ShortName);

        // Build list widget
        ui::widgets::StringListbox list(root.provider(), root.colorScheme());
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (!names.get(i).empty()) {
                list.addItem(i, afl::string::Format("%c - %s", game::PlayerList::getCharacterFromPlayer(i), names.get(i)));
            }
        }
        list.setCurrentKey(player);

        // Dialog
        ui::widgets::ScrollbarContainer cont(list, root);
        if (ui::widgets::doStandardDialog(title, String_t(), cont, true, root, tx)) {
            list.getCurrentKey(player);
            return true;
        } else {
            return false;
        }
    }


    bool editSearch(ui::Root& root, const String_t& title, String_t& value, afl::string::Translator& tx)
    {
        ui::widgets::InputLine inp(200, root);
        inp.setText(value);
        inp.setFont("+");
        if (inp.doStandardDialog(tx("Search"), title, tx)) {
            value = inp.getText();
            return true;
        } else {
            return false;
        }
    }


    void drawCorner(gfx::Context<uint8_t>& ctx, gfx::Point pos, uint8_t color, int size)
    {
        ctx.setColor(color);
        for (int i = 0; i < size; ++i) {
            drawHLine(ctx, pos.getX(), pos.getY() + i, pos.getX() + size-i-1);
        }
    }

    class AbilityIconObject : public ui::icons::Icon {
     public:
        static const int SIZE = 32 + 2;
        static const int GAP = 1;
        AbilityIconObject(ui::Root& root, int width)
            : m_root(root),
              m_width(width)
            { }

        void add(String_t imageName, game::spec::info::AbilityFlags_t flags)
            {
                m_imageNames.push_back(imageName);
                m_imageFlags.push_back(flags);
            }

        bool empty() const
            { return m_imageNames.empty(); }

        virtual gfx::Point getSize() const
            {
                int columns = std::max(1, (m_width + GAP) / (SIZE + GAP));
                int lines = (int(m_imageNames.size()) + columns-1) / columns;

                return gfx::Point(m_width, lines * (SIZE+GAP) - GAP);
            }

        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::ButtonFlags_t /*flags*/) const
            {
                gfx::Context<uint8_t> ctx2(ctx.canvas(), m_root.colorScheme());
                gfx::Rectangle line;
                for (size_t i = 0; i < m_imageNames.size(); ++i) {
                    if (line.getWidth() < SIZE) {
                        line = area.splitY(SIZE);
                        area.consumeY(GAP);
                    }

                    gfx::Rectangle pixArea = line.splitX(SIZE);
                    line.consumeX(GAP);
                    ui::drawFrameDown(ctx2, pixArea);
                    pixArea.grow(-1, -1);
                    gfx::drawSolidBar(ctx2, pixArea, ui::Color_Black);

                    afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(m_imageNames[i]);

                    // Image (has already been given appropriate color depending on flags)
                    if (image.get() != 0) {
                        ctx.canvas().blit(pixArea.getTopLeft(), *image, gfx::Rectangle(0, 0, pixArea.getWidth(), pixArea.getHeight()));
                    }

                    // Add corner marker
                    game::spec::info::AbilityFlags_t flags = m_imageFlags[i];
                    if (flags.contains(game::spec::info::ForeignAbility)) {
                        // red corner
                        drawCorner(ctx2, pixArea.getTopLeft(), ui::Color_Red, 5);
                    } else if (flags.contains(game::spec::info::ReachableAbility) || flags.contains(game::spec::info::OutgrownAbility)) {
                        // green corner
                        drawCorner(ctx2, pixArea.getTopLeft(), ui::Color_DarkGreen, 5);
                    } else {
                        // no corner
                    }
                }
            }
     private:
        ui::Root& m_root;
        int m_width;
        std::vector<String_t> m_imageNames;
        std::vector<game::spec::info::AbilityFlags_t> m_imageFlags;
    };


    class SpecBrowserDialog {
     public:
        SpecBrowserDialog(ui::Root& root, afl::string::Translator& tx, SpecBrowserProxy& proxy, util::RequestSender<game::Session> gameSender)
            : m_root(root), m_translator(tx), m_proxy(proxy), m_gameSender(gameSender),
              m_deleter(), m_loop(root),
              m_tabs(root),
              m_list(m_root.provider(), m_root.colorScheme()),
              m_filterDisplay(m_root, tx),
              m_docView(m_root.provider().getFont("")->getCellSize().scaledBy(30, 20), 0, m_root.provider()),
              m_window(tx("Almanac of the Universe"), root.provider(), root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5),
              m_playersButton(tx("Players..."), 'p', root),
              m_hullsButton(tx("Hulls..."), 't', root),
              m_racialAbilitiesButton(tx("Abilities..."), 'a', root),
              m_buttonGroup(ui::layout::HBox::instance5),
              m_buttonSpacer(),
              m_playerListGroup(ui::layout::HBox::instance5),
              m_playerLists(),
              m_playerNames(),
              m_pageContent(),
              m_availableFilters(),
              m_activeSort(),
              m_availableSorts(),
              m_connImageChange(m_root.provider().sig_imageChange.add(this, &SpecBrowserDialog::onImageChange)),
              m_handleImageChange(false),
              m_handleListSelectionChange(true),
              m_lastSelectedId(-1)
            {
                init();
            }

        void run()
            {
                m_proxy.setPage(gsi::PlayerPage);

                // Window [VBox]
                //   TabBar
                //   HBox (1)
                //     VBox (11)
                //       Filter
                //       List
                //     VBox (12)
                //       Document
                //       Race links
                //       Other links
                //   HBox (2)
                //     "OK"

                Group& g1 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
                Group& g11 = m_deleter.addNew(new Group(ui::layout::VBox::instance5));
                Group& g12 = m_deleter.addNew(new Group(ui::layout::VBox::instance5));
                Group& g2 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));

                client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:almanac");
                Button& btnOK = m_deleter.addNew(new Button(m_translator("Close"), util::Key_Escape, m_root));
                Button& btnHelp = m_deleter.addNew(new Button(m_translator("Help"), 'h', m_root));
                btnOK.sig_fire.addNewClosure(m_loop.makeStop(0));
                btnHelp.dispatchKeyTo(help);

                ui::widgets::FrameGroup& listGroup = m_deleter.addNew(new ui::widgets::FrameGroup(ui::layout::VBox::instance0, m_root.colorScheme(), ui::LoweredFrame));
                listGroup.add(m_deleter.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root)));
                listGroup.setColorScheme(m_deleter.addNew(new ui::SkinColorScheme(ui::GRAY_COLOR_SET, m_root.colorScheme())));

                g11.add(m_filterDisplay);
                g11.add(listGroup);
                g12.add(m_docView);
                g12.add(m_deleter.addNew(new ui::Spacer()));
                g12.add(m_playerListGroup);
                g12.add(m_buttonGroup);
                g2.add(btnOK);
                g2.add(m_deleter.addNew(new ui::Spacer()));
                g2.add(btnHelp);

                g1.add(g11);
                g1.add(g12);
                m_window.add(help);
                m_window.add(m_tabs);
                m_window.add(g1);
                m_window.add(g2);
                m_window.add(m_deleter.addNew(new ui::widgets::Quit(m_root, m_loop)));

                ui::widgets::KeyDispatcher& disp = m_deleter.addNew(new ui::widgets::KeyDispatcher());
                m_window.add(disp);
                disp.add('/', this, &SpecBrowserDialog::onSearch);
                disp.add(util::Key_F7, this, &SpecBrowserDialog::onSearch);
                disp.add(util::Key_Insert, this, &SpecBrowserDialog::onFilterAdd);

                ui::widgets::FocusIterator& it = m_deleter.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
                it.add(m_list);
                it.add(m_filterDisplay);
                m_window.add(it);

                m_list.requestFocus();

                m_window.pack();
                m_root.centerWidget(m_window);
                m_root.add(m_window);
                m_loop.run();
                m_root.remove(m_window);
            }

        void onListChange(const gsi::ListContent& content, size_t index)
            {
                util::StringList list;
                for (size_t i = 0, n = content.content.size(); i < n; ++i) {
                    list.add(content.content[i].id, content.content[i].name);
                }
                m_handleListSelectionChange = false;
                m_list.swapItems(list);
                m_list.setCurrentItem(index);
                m_list.getCurrentKey(m_lastSelectedId);
                m_handleListSelectionChange = true;
            }

        void onPageChange(const gsi::PageContent& content)
            {
                bool pageChange = (m_pageContent.pageLinks != content.pageLinks);
                m_pageContent = content;
                render();
                if (pageChange) {
                    updateButtons();
                }
                updatePlayerLists();
            }

        void onListSelectionChange()
            {
                if (m_handleListSelectionChange) {
                    int32_t key;
                    if (m_list.getCurrentKey(key)) {
                        if (key != m_lastSelectedId) {
                            m_proxy.setId(key);
                            m_lastSelectedId = key;
                        }
                    }
                }
            }

        void onTabClick(size_t id)
            {
                m_proxy.setPage(static_cast<gsi::Page>(id));
            }

        void onFilterDelete(size_t index)
            {
                m_proxy.eraseFilter(index);
            }

        void onFilterAdd()
            {
                // Quick exit
                if (m_availableFilters.empty()) {
                    return;
                }

                // Build list box
                ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
                for (size_t i = 0; i < m_availableFilters.size(); ++i) {
                    list.addItem(int32_t(i), m_availableFilters[i].name);
                }
                list.sortItemsAlphabetically();
                list.setPreferredHeight(static_cast<int>(list.getNumItems()));

                // Menu
                ui::EventLoop loop(m_root);
                if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(list, m_filterDisplay.getFilterAnchor())) {
                    // Need to validate index because proxy could have updated the list in the meantime.
                    // If it does (it normally doesn't) this means our index will be off, but at least do not crash.
                    int32_t key;
                    if (list.getCurrentKey(key)) {
                        size_t index = size_t(key);
                        if (index < m_availableFilters.size()) {
                            addFilter(m_availableFilters[index]);
                        }
                    }
                }
            }

        void onFilterEdit(size_t index)
            {
                if (index < m_existingFilters.size()) {
                    gsi::FilterInfo f = m_existingFilters[index];
                    if (editFilter(f)) {
                        if (f.mode == gsi::EditString) {
                            m_proxy.setNameFilter(f.value);
                        } else {
                            m_proxy.setFilter(index, f.elem);
                        }
                    }
                }
            }

        void onSort()
            {
                if ((m_availableSorts - gsi::String_Name - gsi::Range_Id).empty()) {
                    // Only name/Id, if any. Just toggle.
                    if (m_availableSorts.contains(gsi::String_Name) && m_activeSort != gsi::String_Name) {
                        m_proxy.setSortOrder(gsi::String_Name);
                    } else if (m_availableSorts.contains(gsi::Range_Id) && m_activeSort != gsi::Range_Id) {
                        m_proxy.setSortOrder(gsi::Range_Id);
                    } else {
                        // impossible/invalid
                    }
                } else {
                    // Nontrivial number of sorts, let user choose
                    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
                    gsi::FilterAttributes_t atts = m_availableSorts;
                    for (int i = 0; !atts.empty(); ++i) {
                        gsi::FilterAttribute att = gsi::FilterAttribute(i);
                        if (atts.contains(att)) {
                            list.addItem(i, gsi::toString(att, m_translator));
                            atts -= att;
                        }
                    }
                    list.sortItemsAlphabetically();
                    list.setCurrentKey(m_activeSort);

                    ui::EventLoop loop(m_root);
                    if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(list, m_filterDisplay.getSortAnchor())) {
                        int32_t key = 0;
                        if (list.getCurrentKey(key)) {
                            m_proxy.setSortOrder(gsi::FilterAttribute(key));
                        }
                    }
                }
            }

        void onFilterChange(const gsi::FilterInfos_t& existing, const gsi::FilterInfos_t& available)
            {
                m_filterDisplay.setContent(existing);
                m_filterDisplay.setFilterAvailable(!available.empty());
                if (ui::LayoutableGroup* w = dynamic_cast<ui::LayoutableGroup*>(m_filterDisplay.getParent())) {
                    // Re-layout the immediate container.
                    // Re-layouting the window will determine that nothing changes for that container,
                    // and not re-layout it at all.
                    w->doLayout();

                    // Redraw the window. This will redraw the gaps between widgets not claimed by anyone.
                    m_window.requestRedraw();
                }

                m_existingFilters = existing;
                m_availableFilters = available;
            }

        void onSortChange(gsi::FilterAttribute active, gsi::FilterAttributes_t available)
            {
                m_activeSort = active;
                m_availableSorts = available;
                m_filterDisplay.setSort(gsi::toString(active, m_translator), available.contains(active));
            }

        void onAddFilterOnPage(gsi::Page page)
            {
                m_proxy.addCurrentAsFilter();
                m_proxy.setPage(page);
                m_tabs.setFocusedTab(page);
            }

        void addFilter(gsi::FilterInfo f)
            {
                // Note: f passed by copy in case proxy updates the filter list in the meantime.
                if (editFilter(f)) {
                    if (f.mode == gsi::EditString) {
                        m_proxy.setNameFilter(f.value);
                    } else {
                        m_proxy.addFilter(f.elem);
                    }
                }
            }

        bool editFilter(gsi::FilterInfo& f)
            {
                switch (f.mode) {
                 case gsi::NotEditable:
                    break;
                 case gsi::EditRange:
                 case gsi::EditRangeLevel:
                    // EditRangeXXX: XXX describes type to edit.
                    // elem.range is current resp. default; edit to be subrange of maxRange; elem.value is fixed.
                    // Call set()/add().
                    return editRange(m_root, f.name, f.elem.range, f.maxRange, m_translator);
                 case gsi::SetValueRange:
                    // SetRange: elem is fixed with the NEW values. Call set() or add().
                    return true;
                 case gsi::EditValuePlayer:
                    // EditValueXXX
                    // elem.value is current resp. default; edit to be element of maxRange; elem.range is fixed.
                    // Call set()/add().
                    return editPlayer(m_root, f.name, f.elem.value, m_translator, m_gameSender);
                 case gsi::EditValueHull:
                    return client::dialogs::chooseHull(m_root, f.name, f.elem.value, m_translator, m_gameSender, false);
                 case gsi::EditString:
                    return editSearch(m_root, f.name, f.value, m_translator);
                }
                return false;
            }

        void onPlayerClick(int player)
            {
                if (m_pageContent.players.contains(player)) {
                    m_proxy.addFilter(gsi::FilterElement(gsi::Value_Player, player, gsi::IntRange_t()));
                }
            }

        void onSearch()
            {
                String_t text;
                for (size_t i = 0; i < m_existingFilters.size(); ++i) {
                    if (m_existingFilters[i].elem.att == gsi::String_Name) {
                        text = m_existingFilters[i].value;
                        break;
                    }
                }
                if (editSearch(m_root, m_translator("Name"), text, m_translator)) {
                    m_proxy.setNameFilter(text);
                }
            }

        void init()
            {
                // Player names
                client::Downlink link(m_root, m_translator);
                m_playerNames = game::proxy::PlayerProxy(m_gameSender).getPlayerNames(link, game::Player::AdjectiveName);

                // Make the document view flexible
                m_docView.setPreferredSize(m_root.provider().getFont("")->getCellSize().scaledBy(30, 30));

                m_proxy.sig_listChange.add(this, &SpecBrowserDialog::onListChange);
                m_proxy.sig_pageChange.add(this, &SpecBrowserDialog::onPageChange);
                m_proxy.sig_filterChange.add(this, &SpecBrowserDialog::onFilterChange);
                m_proxy.sig_sortChange.add(this, &SpecBrowserDialog::onSortChange);

                m_list.sig_change.add(this, &SpecBrowserDialog::onListSelectionChange);
                m_list.setPreferredHeight(5);
                m_list.setPreferredWidth(20, false);

                m_filterDisplay.sig_delete.add(this, &SpecBrowserDialog::onFilterDelete);
                m_filterDisplay.sig_add.add(this, &SpecBrowserDialog::onFilterAdd);
                m_filterDisplay.sig_edit.add(this, &SpecBrowserDialog::onFilterEdit);
                m_filterDisplay.sig_sort.add(this, &SpecBrowserDialog::onSort);

                m_tabs.sig_tabClick.add(this, &SpecBrowserDialog::onTabClick);
                m_tabs.addPage(gsi::PlayerPage,          m_translator("Races"),            '1');
                m_tabs.addPage(gsi::RacialAbilitiesPage, m_translator("Racial Abilities"), '2');
                m_tabs.addPage(gsi::HullPage,            m_translator("Ship Hulls"),       '3');
                m_tabs.addPage(gsi::ShipAbilitiesPage,   m_translator("Ship Abilities"),   '4');
                m_tabs.addPage(gsi::EnginePage,          m_translator("Engines"),          '5');
                m_tabs.addPage(gsi::BeamPage,            m_translator("Beams"),            '6');
                m_tabs.addPage(gsi::TorpedoPage,         m_translator("Torpedoes"),        '7');
                m_tabs.addPage(gsi::FighterPage,         m_translator("Fighters"),         '8');
                m_tabs.setFont(gfx::FontRequest());
                m_tabs.setKeys(ui::widgets::TabBar::CtrlTab | ui::widgets::TabBar::F6);

                class PageFilterClosure : public afl::base::Closure<void(int)> {
                 public:
                    PageFilterClosure(SpecBrowserDialog& parent, gsi::Page page)
                        : m_parent(parent), m_page(page)
                        { }
                    virtual void call(int)
                        { m_parent.onAddFilterOnPage(m_page); }
                 private:
                    SpecBrowserDialog& m_parent;
                    gsi::Page m_page;
                };
                m_playersButton.sig_fire.addNewClosure(new PageFilterClosure(*this, gsi::PlayerPage));
                m_hullsButton.sig_fire.addNewClosure(new PageFilterClosure(*this, gsi::HullPage));
                m_racialAbilitiesButton.sig_fire.addNewClosure(new PageFilterClosure(*this, gsi::RacialAbilitiesPage));
            }

        void render()
            {
                ui::rich::Document& doc = m_docView.getDocument();
                doc.clear();
                m_handleImageChange = false;

                doc.add(util::rich::Text(m_pageContent.title)
                        .withStyle(util::rich::StyleAttribute::Big)
                        .withColor(util::SkinColor::Heading));
                doc.addParagraph();

                // FIXME: if image not yet loaded, do not render anything at all so stuff does not jump
                if (!m_pageContent.pictureName.empty()) {
                    bool final = true;
                    afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(m_pageContent.pictureName, &final);
                    if (image.get() != 0) {
                        ui::icons::StylableIcon& obj = doc.deleter().addNew(new ui::icons::StylableIcon(doc.deleter().addNew(new ui::icons::Image(*image)), m_root.colorScheme()));
                        obj.setBackgroundColor(m_root.colorScheme().getColor(ui::Color_Black));
                        obj.setFrameWidth(1);
                        obj.setFrameType(ui::LoweredFrame);
                        obj.setMarginBefore(gfx::Point(5, 0));   // Leave some room for text before
                        doc.addFloatObject(obj, false /* = right */);
                    } else {
                        if (!final) {
                            m_handleImageChange = true;
                        }
                    }
                }

                client::dialogs::renderHullInformation(doc, m_root, m_pageContent, m_translator);

                m_docView.handleDocumentUpdate();
                m_docView.setTopY(0);
            }

        void updateButtons()
            {
                while (ui::Widget* w = m_buttonGroup.getFirstChild()) {
                    m_buttonGroup.removeChild(*w);
                }
                if (m_pageContent.pageLinks.contains(gsi::PlayerPage)) {
                    m_buttonGroup.add(m_playersButton);
                }
                if (m_pageContent.pageLinks.contains(gsi::HullPage)) {
                    m_buttonGroup.add(m_hullsButton);
                }
                if (m_pageContent.pageLinks.contains(gsi::RacialAbilitiesPage)) {
                    m_buttonGroup.add(m_racialAbilitiesButton);
                }
                m_buttonGroup.add(m_buttonSpacer);
                if (ui::LayoutableGroup* w = dynamic_cast<ui::LayoutableGroup*>(m_buttonGroup.getParent())) {
                    w->doLayout();
                    m_buttonGroup.doLayout();
                    m_window.requestRedraw();
                }
            }

        void updatePlayerLists()
            {
                bool relayout = false;
                if (m_pageContent.players.empty()) {
                    if (!m_playerLists.empty()) {
                        m_playerLists.clear();
                        relayout = true;
                    }
                } else {
                    if (m_playerLists.empty()) {
                        game::PlayerSet_t allPlayers = game::PlayerSet_t::allUpTo(12) - 0;
                        int numPlayers = 0;
                        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                            if (allPlayers.contains(i)) {
                                ++numPlayers;
                            }
                        }
                        int numLines = (numPlayers == 0 ? 1 : (numPlayers + 2) / 3);

                        for (int i = 0; i < 3; ++i) {
                            m_playerLists.pushBackNew(new client::widgets::PlayerList(m_root, client::widgets::PlayerList::VerticalLayout, client::widgets::PlayerList::ShowNames, client::widgets::PlayerList::SameColors, 100, allPlayers.take(numLines)));
                            m_playerLists.back()->setNames(m_playerNames);
                            m_playerLists.back()->sig_playerClick.add(this, &SpecBrowserDialog::onPlayerClick);
                            m_playerListGroup.add(*m_playerLists.back());
                        }

                        relayout = true;
                    }
                    for (size_t i = 0; i < m_playerLists.size(); ++i) {
                        m_playerLists[i]->setHighlightedPlayers(m_pageContent.players);
                    }
                }
                if (relayout) {
                    if (ui::LayoutableGroup* w = dynamic_cast<ui::LayoutableGroup*>(m_playerListGroup.getParent())) {
                        w->doLayout();
                        m_playerListGroup.doLayout();
                        m_window.requestRedraw();
                    }
                }
            }

        void onImageChange()
            {
                if (m_handleImageChange) {
                    render();
                }
            }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        SpecBrowserProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;

        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;

        ui::widgets::TabBar m_tabs;
        ui::widgets::StringListbox m_list;
        client::widgets::FilterDisplay m_filterDisplay;
        ui::rich::DocumentView m_docView;
        ui::Window m_window;
        ui::widgets::Button m_playersButton;
        ui::widgets::Button m_hullsButton;
        ui::widgets::Button m_racialAbilitiesButton;
        ui::Group m_buttonGroup;
        ui::Spacer m_buttonSpacer;

        ui::Group m_playerListGroup;
        afl::container::PtrVector<client::widgets::PlayerList> m_playerLists;
        game::PlayerArray<String_t> m_playerNames;

        gsi::PageContent m_pageContent;
        gsi::FilterInfos_t m_existingFilters;
        gsi::FilterInfos_t m_availableFilters;
        gsi::FilterAttribute m_activeSort;
        gsi::FilterAttributes_t m_availableSorts;

        afl::base::SignalConnection m_connImageChange;
        bool m_handleImageChange;

        /* We need to protect against excess events.
           (a) when processing list updates from the proxy, do not handle onListSelectionChange().
               The list update comes with a new position. Handling onListSelectionChange while
               processing the update would mean that we see intermediate states and report these
               as questions to the proxy.
           (b) The listbox widget occasionally emits null events (e.g. during layout). Those must
               not cause questions to the proxy to be generated. */
        bool m_handleListSelectionChange;
        int m_lastSelectedId;
    };

    util::rich::Text renderAbility(const game::spec::info::Ability& ab)
    {
        return util::rich::Text(ab.flags.contains(game::spec::info::DamagedAbility) ? util::SkinColor::Red : util::SkinColor::Static, ab.info);
    }
}



void
client::dialogs::doSpecificationBrowserDialog(ui::Root& root,
                                              util::RequestSender<game::Session> gameSender,
                                              afl::string::Translator& tx)
{
    SpecBrowserProxy proxy(gameSender, root.engine().dispatcher(), std::auto_ptr<game::spec::info::PictureNamer>(new PictureNamer()));
    SpecBrowserDialog dialog(root, tx, proxy, gameSender);
    dialog.run();
}

void
client::dialogs::renderHullInformation(ui::rich::Document& doc, ui::Root& root, const game::spec::info::PageContent& content, afl::string::Translator& tx)
{
    for (size_t i = 0, n = content.attributes.size(); i < n; ++i) {
        const gsi::Attribute& att = content.attributes[i];
        doc.add(att.name);
        if (!att.value.empty()) {
            doc.add(": ");
            doc.add(util::rich::Text(att.value).withColor(util::SkinColor::Green));
        }
        doc.addNewline();
    }

    // FIXME: make this configurable
    const bool useIcons = true;
    renderAbilityList(doc, root, content.abilities, useIcons, content.abilities.size(), tx);
}

void
client::dialogs::renderAbilityList(ui::rich::Document& doc, ui::Root& root, const game::spec::info::Abilities_t& abilities, bool useIcons, size_t maxLines, afl::string::Translator& tx)
{
    std::auto_ptr<AbilityIconObject> obj(new AbilityIconObject(root, doc.getPageWidth()));
    size_t usedLines = 0;       // Number of consumed lines
    size_t excessLines = 0;     // Number of lines that didn't fit
    size_t lastIndex = 0;       // Index of first line that didn't fit
    for (size_t i = 0, n = abilities.size(); i < n; ++i) {
        const gsi::Ability& a = abilities[i];
        if (useIcons && !a.pictureName.empty()) {
            obj->add(a.pictureName, a.flags);
        } else {
            if (usedLines >= maxLines-1) {
                if (excessLines == 0) {
                    lastIndex = i;
                }
                ++excessLines;
            } else {
                doc.add(renderAbility(a));
                doc.addNewline();
                ++usedLines;
            }
        }
    }
    if (excessLines != 0) {
        if (excessLines == 1) {
            doc.add(renderAbility(abilities[lastIndex]));
        } else {
            doc.add(afl::string::Format(tx("(+%d more)"), excessLines));
        }
        doc.addNewline();
    }
    if (!obj->empty()) {
        doc.addCenterObject(doc.deleter().addNew(obj.release()));
    }
}
