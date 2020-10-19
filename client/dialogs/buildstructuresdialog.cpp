/**
  *  \file client/dialogs/buildstructuresdialog.cpp
  */

#include "client/dialogs/buildstructuresdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/functional/stringtable.hpp"
#include "afl/io/xml/nodereader.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/goaldialog.hpp"
#include "client/dialogs/grounddefensedialog.hpp"
#include "client/dialogs/sellsuppliesdialog.hpp"
#include "client/dialogs/taxationdialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costdisplay.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/planetmineralinfo.hpp"
#include "game/actions/buildstructures.hpp"
#include "game/proxy/buildstructuresproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/planetinfoproxy.hpp"
#include "gfx/context.hpp"
#include "ui/cardgroup.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/rich/documentparser.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/panel.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/keystring.hpp"
#include "util/numberformatter.hpp"
#include "util/translation.hpp"

using client::widgets::PlanetMineralInfo;
using game::actions::BuildStructures;
using game::proxy::BuildStructuresProxy;
using game::proxy::PlanetInfoProxy;
using game::spec::Cost;
using ui::Group;
using ui::Spacer;
using ui::Widget;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using util::KeyString;
using util::NumberFormatter;

namespace {
    /*
     *  Screen header
     *  (can we make this a generic widget?)
     */
    class StructureHeader : public ui::Widget {
     public:
        struct Item {
            String_t mainTitle;
            String_t subTitle;
            ui::Widget* widget;
        };

        StructureHeader(ui::Root& root, ui::CardGroup& g)
            : Widget(),
              m_root(root),
              m_group(g),
              m_btnNext(">", util::Key_Right, root),
              m_btnPrev("<", util::Key_Left, root),
              conn_focusChange(g.sig_handleFocusChange.add(this, (void (StructureHeader::*)()) &StructureHeader::requestRedraw)),
              m_items(),
              m_displayWidth(0)
            {
                // Caller will pack() the dialog, thus calling onResize() on this
                // object, which will fill in the actual button positions.
                addChild(m_btnNext, 0);
                addChild(m_btnPrev, 0);
                m_btnNext.sig_fire.add(this, &StructureHeader::onNext);
                m_btnPrev.sig_fire.add(this, &StructureHeader::onPrevious);
            }

        void addPage(String_t mainTitle, String_t subTitle, Widget& w)
            {
                Item it = { mainTitle, subTitle, &w };
                m_items.push_back(it);
                requestRedraw();
            }

        void setFocusedPage(size_t n)
            {
                if (n < m_items.size()) {
                    m_items[n].widget->requestFocus();
                }
            }

        std::pair<const Item*, size_t> getPage() const
            {
                for (size_t i = 0, n = m_items.size(); i < n; ++i) {
                    if (m_items[i].widget->hasState(FocusedState)) {
                        return std::make_pair(&m_items[i], i);
                    }
                }
                return std::make_pair((const Item*) 0, (size_t) 0);
            }

        void onNext()
            {
                size_t next = getPage().second + 1;
                if (next >= m_items.size()) {
                    next = 0;
                }
                setFocusedPage(next);
            }

        void onPrevious()
            {
                size_t next = getPage().second;
                if (next == 0) {
                    next = m_items.size();
                }
                --next;
                setFocusedPage(next);
            }

        // Widget:
        virtual void draw(gfx::Canvas& can)
            {
                // ex WPlanetStructureHeader::drawContent

                // Prepare
                const Item* it = getPage().first;

                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                ctx.setSolidBackground();

                gfx::Rectangle area = getExtent();
                area.setWidth(m_displayWidth);

                // First line
                ctx.setColor(util::SkinColor::Heading);
                ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(1)));
                outTextF(ctx, area.splitY(ctx.getFont()->getCellSize().getY()), it != 0 ? it->mainTitle : String_t());

                // Second line
                ctx.setColor(util::SkinColor::Yellow);
                ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
                outTextF(ctx, area.splitY(ctx.getFont()->getCellSize().getY()), it != 0 ? it->subTitle : String_t());

                defaultDrawChildren(can);
            }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
            { requestRedraw(area); }
        virtual void handleChildAdded(Widget& /*child*/)
            { }
        virtual void handleChildRemove(Widget& /*child*/)
            { }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            {
                // ex WPlanetStructureHeader::onResize
                gfx::Point prevSize = m_btnPrev.getLayoutInfo().getMinSize();
                gfx::Point nextSize = m_btnPrev.getLayoutInfo().getMinSize();

                gfx::Rectangle r = getExtent();

                m_btnNext.setExtent(gfx::Rectangle(r.getRightX() - nextSize.getX(),
                                                   r.getTopY(),
                                                   nextSize.getX(),
                                                   nextSize.getY()));
                m_btnPrev.setExtent(gfx::Rectangle(r.getRightX() - nextSize.getX() - 5 - prevSize.getX(),
                                                   r.getTopY(),
                                                   prevSize.getX(),
                                                   prevSize.getY()));

                m_displayWidth = r.getWidth() - nextSize.getX() - 5 - prevSize.getX();
            }
        virtual void handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            {
                // ex WPlanetStructureHeader::getLayoutInfo
                gfx::Point prevSize = m_btnPrev.getLayoutInfo().getMinSize();
                gfx::Point nextSize = m_btnPrev.getLayoutInfo().getMinSize();

                gfx::Point mainSize = m_root.provider().getFont(gfx::FontRequest().addSize(1))->getCellSize().scaledBy(30, 1);
                gfx::Point subSize  = m_root.provider().getFont(gfx::FontRequest())           ->getCellSize().scaledBy(30, 1);

                mainSize.extendBelow(subSize);
                mainSize.extendRight(prevSize);
                mainSize.addX(5);
                mainSize.extendRight(nextSize);

                return ui::layout::Info(mainSize, mainSize, ui::layout::Info::GrowHorizontal);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                if (key == util::Key_Tab) {
                    onNext();
                    return true;
                } else if (key == util::Key_Tab + util::KeyMod_Shift) {
                    onPrevious();
                    return true;
                } else {
                    return defaultHandleKey(key, prefix);
                }
            }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return defaultHandleMouse(pt, pressedButtons); }

     private:
        ui::Root& m_root;
        ui::CardGroup& m_group;
        ui::widgets::Button m_btnNext;
        ui::widgets::Button m_btnPrev;
        afl::base::SignalConnection conn_focusChange;
        std::vector<Item> m_items;
        int m_displayWidth;
    };


    /*
     *  Structure builder widget
     */
    class StructureWidget : public ui::Group {
     public:
        StructureWidget(ui::Root& root, BuildStructuresProxy& proxy, game::PlanetaryBuilding type, NumberFormatter fmt, afl::string::Translator& tx)
            : Group(ui::layout::HBox::instance5),
              m_root(root),
              m_proxy(proxy),
              m_type(type),
              m_translator(tx),
              m_numberFormatter(fmt),
              m_del(),
              m_table(root, 3, 4),
              m_btnPlus("+", '+', root),
              m_btnMinus("-", '-', root),
              conn_statusChange(proxy.sig_statusChange.add(this, &StructureWidget::onStatusChange))
            {
                // ex WPlanetStructureWidget, WPlanetStructureGroup
                // HBox
                //   Image
                //   VBox
                //     Static (header)
                //     Table 4x3
                //   VBox
                //     Button +
                //     Button -
                //     Spacer
                const BuildStructures::Description& desc = BuildStructures::describe(m_type);
                add(m_del.addNew(new ui::widgets::ImageButton(desc.imageName, 0, root, gfx::Point(105, 93))));

                Group& g1 = m_del.addNew(new Group(ui::layout::VBox::instance5));
                ui::widgets::StaticText& txt = m_del.addNew(new ui::widgets::StaticText(tx(desc.untranslatedBuildingName), util::SkinColor::Heading, gfx::FontRequest().addSize(1), root.provider()));
                txt.setForcedWidth(root.provider().getFont(gfx::FontRequest().addSize(1))->getEmWidth() * 15);
                g1.add(txt);
                g1.add(m_table);
                add(g1);

                Group& g2 = m_del.addNew(new Group(ui::layout::VBox::instance5));
                g2.add(m_btnPlus);
                g2.add(m_btnMinus);
                g2.add(m_del.addNew(new ui::Spacer()));
                add(g2);

                m_table.column(0).setColor(ui::Color_Gray);
                m_table.column(0).subrange(0, 3).setExtraColumns(1);
                m_table.cell(0, 0).setText(tx("Amount:"));
                m_table.cell(0, 1).setText(tx("Auto-B. Goal:"));
                m_table.cell(0, 2).setText(tx("Maximum:"));

                // FIXME: fine-tune table layout so that all StructureWidget's use same column widths
                m_table.column(2).subrange(0, 3).setColor(ui::Color_Green).setTextAlign(2, 0);

                m_table.cell(0, 3).setText(tx("Cost:"));
                m_table.cell(1, 3).setExtraColumns(1).setColor(ui::Color_Green).setText(tx(desc.untranslatedBuildingCost)).setTextAlign(2, 0);

                // Connect keys
                ui::widgets::KeyDispatcher& disp = m_del.addNew(new ui::widgets::KeyDispatcher());
                m_btnPlus.dispatchKeyTo(disp);
                m_btnMinus.dispatchKeyTo(disp);
                disp.add('+', this, &StructureWidget::onPlus);
                disp.add('-', this, &StructureWidget::onMinus);
            }

        void onPlus(int n)
            {
                // ex WPlanetStructureWidget::add
                m_proxy.addLimitCash(m_type, n != 0 ? n : 1);
            }

        void onMinus(int n)
            {
                // ex WPlanetStructureWidget::remove
                m_proxy.addLimitCash(m_type, -(n != 0 ? n : 1));
            }

        void setData(int have, int goal, int max)
            {
                m_table.cell(2, 0).setText(m_numberFormatter.formatNumber(have));
                m_table.cell(2, 1).setText(goal >= game::MAX_AUTOBUILD_SPEED ? m_translator("[max]") : m_numberFormatter.formatNumber(goal));
                m_table.cell(2, 2).setText(m_numberFormatter.formatNumber(max));
            }

        void onStatusChange(const BuildStructuresProxy::Status& st)
            {
                const BuildStructuresProxy::BuildingInfo& b = st.buildings[m_type];
                setData(b.have, b.want, b.max);
            }

     private:
        ui::Root& m_root;
        BuildStructuresProxy& m_proxy;
        game::PlanetaryBuilding m_type;
        afl::string::Translator& m_translator;
        NumberFormatter m_numberFormatter;

        afl::base::Deleter m_del;
        ui::widgets::SimpleTable m_table;
        ui::widgets::Button m_btnPlus;
        ui::widgets::Button m_btnMinus;

        afl::base::SignalConnection conn_statusChange;
    };

    /*
     *  Structure Cost
     */
#if 0
    /* Simple version: just re-use the CostDisplay widget.
       Looks ok but not great layout-wise; does not currently adapt to the skin. */
    class StructureCostWidget : public client::widgets::CostDisplay {
     public:
        StructureCostWidget(ui::Root& root,
                            bool withDuranium,
                            util::NumberFormatter fmt,
                            afl::string::Translator& /*tx*/,
                            BuildStructuresProxy& proxy)
            : CostDisplay(root,
                          withDuranium
                          ? CostDisplay::Types_t() + Cost::Money + Cost::Supplies + Cost::Duranium
                          : CostDisplay::Types_t() + Cost::Money + Cost::Supplies,
                          fmt),
              conn_statusChange(proxy.sig_statusChange.add(this, &StructureCostWidget::onStatusChange))
            {
            }

        void onStatusChange(const BuildStructuresProxy::Status& st)
            {
                setCost(st.needed);
                setAvailableAmount(st.available);
                setRemainingAmount(st.remaining);
            }

     private:
        afl::base::SignalConnection conn_statusChange;
    };
#else
    /* More extensive version closer to PCC1/PCC2 behaviour */
    class StructureCostWidget : public ui::widgets::SimpleTable {
     public:
        StructureCostWidget(ui::Root& root,
                            bool withDuranium,
                            util::NumberFormatter fmt,
                            afl::string::Translator& tx,
                            BuildStructuresProxy& proxy)
            : SimpleTable(root, 3, withDuranium ? 8 : 6),
              conn_statusChange(proxy.sig_statusChange.add(this, &StructureCostWidget::onStatusChange)),
              m_formatter(fmt),
              m_withDuranium(withDuranium)
            {
                static const char*const LABELS[] = {
                    N_("You have:"),
                    N_("Cost:"),
                    N_("Remaining:"),
                    N_("Duranium:"),
                    N_("  remaining:"),
                };

                // Overall layout
                afl::base::Ref<gfx::Font> font = root.provider().getFont(gfx::FontRequest());
                int numberWidth = font->getEmWidth() * 6;
                int spaceWidth = font->getTextWidth(" ");
                int lineHeight = font->getLineHeight();
                int blockSpacing = lineHeight / 2;

                // - fixed width for label column so adding/removing Duranium part does not move it
                setColumnWidth(0, font->getMaxTextWidth(afl::functional::createStringTable(LABELS).map(tx)));

                // - space after number column, minimum width
                setColumnWidth(1, numberWidth);
                setColumnPadding(1, spaceWidth);

                // - space after each block
                setRowPadding(1, blockSpacing);
                setRowPadding(3, blockSpacing);
                setRowPadding(5, blockSpacing);

                // Colors
                column(0).setColor(ui::Color_Gray);
                column(1).setColor(ui::Color_Green).setTextAlign(2, 0);
                column(2).setColor(ui::Color_Green);

                // Fixed text
                cell(0, 0).setText(tx(LABELS[0]));
                cell(0, 2).setText(tx(LABELS[1]));
                cell(0, 4).setText(tx(LABELS[2]));
                for (int i = 0; i < 6; i += 2) {
                    cell(2, i)  .setText(tx("mc"));
                    cell(2, i+1).setText(tx("kt supplies"));
                }
                if (withDuranium) {
                    cell(0, 6).setText(tx(LABELS[3]));
                    cell(0, 7).setText(tx(LABELS[4]));
                    cell(2, 6).setText(tx("kt"));
                    cell(2, 7).setText(tx("kt"));
                }
            }

        void onStatusChange(const BuildStructuresProxy::Status& st)
            {
                // ex WPlanetStructureCostWidget::drawContent
                /*
                    You have:      nn mc
                                   nn kt supplies
                    Cost:          nn mc
                                   nn kt supplies
                    You will have  nn mc
                    remaining:     nn kt supplies
                    Duranium:      nn kt
                      remaining:   nn kt
                */
                cell(1, 0).setText(m_formatter.formatNumber(st.available.get(Cost::Money)));
                cell(1, 1).setText(m_formatter.formatNumber(st.available.get(Cost::Supplies)));
                cell(1, 2).setText(m_formatter.formatNumber(st.needed.get(Cost::Money)));
                cell(1, 3).setText(m_formatter.formatNumber(st.needed.get(Cost::Supplies)));
                cell(1, 4).setText(m_formatter.formatNumber(st.remaining.get(Cost::Money)));
                cell(1, 5).setText(m_formatter.formatNumber(st.remaining.get(Cost::Supplies)));
                if (m_withDuranium) {
                    cell(1, 6).setText(m_formatter.formatNumber(st.available.get(Cost::Duranium)));
                    cell(1, 7).setText(m_formatter.formatNumber(st.remaining.get(Cost::Duranium)));
                }
            }

     private:
        afl::base::SignalConnection conn_statusChange;
        util::NumberFormatter m_formatter;
        bool m_withDuranium;
    };
#endif


    class BuildStructuresDialog {
     public:
        BuildStructuresDialog(ui::Root& root,
                              afl::string::Translator& tx,
                              BuildStructuresProxy& proxy,
                              PlanetInfoProxy& infoProxy,
                              game::Id_t planetId,
                              const BuildStructuresProxy::HeaderInfo& info,
                              NumberFormatter fmt,
                              util::RequestSender<game::Session> gameSender)
            : m_root(root),
              m_translator(tx),
              m_proxy(proxy),
              m_infoProxy(infoProxy),
              m_info(info),
              m_status(),
              m_formatter(fmt),
              m_del(),
              m_panelColors(ui::DARK_COLOR_SET, root.colorScheme()),
              m_loop(root),
              m_planetId(planetId),
              m_gameSender(gameSender)
            { }

        bool run(int page)
            {
                ui::widgets::Panel& panel = m_del.addNew(new ui::widgets::Panel(ui::layout::VBox::instance5, 2));
                panel.setColorScheme(m_panelColors);

                ui::CardGroup cards;
                StructureHeader header(m_root, cards);

                Widget& helpWidget = m_del.addNew(new client::widgets::HelpWidget(m_root, m_gameSender, "pcc2:buildings"));

                Widget& page1 = buildBuildScreen1(helpWidget);
                header.addPage(m_info.planetName + m_translator(" - Planetary Economy"), m_info.planetInfo, page1);
                cards.add(page1);

                Widget& page2 = buildBuildScreen2(helpWidget);
                header.addPage(m_info.planetName + m_translator(" - Mining Industry"), m_info.planetInfo, page2);
                cards.add(page2);

                Widget& page3 = buildBuildScreen3(helpWidget);
                header.addPage(m_info.planetName + m_translator(" - Defense"), m_info.planetInfo, page3);
                cards.add(page3);

                panel.add(header);
                panel.add(cards);
                panel.add(m_del.addNew(new ui::PrefixArgument(m_root)));
                panel.add(m_del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                panel.add(helpWidget);

                panel.setExtent(m_root.getExtent());
                panel.setState(ui::Widget::ModalState, true);
                header.setFocusedPage(page);

                m_dispatcher.add('a', this, &BuildStructuresDialog::onAutobuild);
                m_dispatcher.add('g', this, &BuildStructuresDialog::onGoalDialog);
                m_dispatcher.add('s', this, &BuildStructuresDialog::onSellSupplies);
                m_dispatcher.add('t', this, &BuildStructuresDialog::onTaxes);

                m_proxy.sig_statusChange.add(this, &BuildStructuresDialog::onStatusChange);

                m_root.add(panel);
                m_proxy.update();
                return m_loop.run() != 0;
            }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        BuildStructuresProxy& m_proxy;
        PlanetInfoProxy& m_infoProxy;
        BuildStructuresProxy::HeaderInfo m_info;
        BuildStructuresProxy::Status m_status;
        NumberFormatter m_formatter;
        afl::base::Deleter m_del;
        ui::SkinColorScheme m_panelColors;
        ui::EventLoop m_loop;
        ui::widgets::KeyDispatcher m_dispatcher;
        game::Id_t m_planetId;
        util::RequestSender<game::Session> m_gameSender;

        StructureWidget& makeStructure(game::PlanetaryBuilding which)
            { return m_del.addNew(new StructureWidget(m_root, m_proxy, which, m_formatter, m_translator)); }

        Button& makeOkButton()
            {
                Button& btn = m_del.addNew(new Button(m_translator("OK"), util::Key_Return, m_root));
                btn.sig_fire.addNewClosure(m_loop.makeStop(1));
                return btn;
            }

        Button& makeCancelButton()
            {
                Button& btn = m_del.addNew(new Button(m_translator("Cancel"), util::Key_Escape, m_root));
                btn.sig_fire.addNewClosure(m_loop.makeStop(0));
                return btn;
            }

        Button& makeKeyButton(KeyString text)
            {
                Button& btn = m_del.addNew(new Button(text, m_root));
                btn.dispatchKeyTo(m_dispatcher);
                return btn;
            }

        Button& makeHelpButton(String_t label, Widget& helpWidget)
            {
                Button& btn = m_del.addNew(new Button(label, 'h', m_root));
                btn.dispatchKeyTo(helpWidget);
                return btn;
            }

        Widget& makeMineralInfo(PlanetInfoProxy::Mineral ele, String_t name, PlanetMineralInfo::Mode mode)
            {
                class Callback : public afl::base::Closure<void()> {
                 public:
                    Callback(PlanetMineralInfo& widget,
                             PlanetInfoProxy& proxy,
                             PlanetInfoProxy::Mineral ele,
                             String_t name,
                             PlanetMineralInfo::Mode mode)
                        : m_widget(widget),
                          m_proxy(proxy),
                          m_element(ele),
                          m_name(name),
                          m_mode(mode)
                        { }
                    virtual Callback* clone() const
                        { return new Callback(*this); }
                    virtual void call()
                        { m_widget.setContent(m_name, m_proxy.getMineralInfo(m_element), m_mode); }
                 private:
                    PlanetMineralInfo& m_widget;
                    PlanetInfoProxy& m_proxy;
                    PlanetInfoProxy::Mineral m_element;
                    String_t m_name;
                    PlanetMineralInfo::Mode m_mode;
                };

                PlanetMineralInfo& result = m_del.addNew(new PlanetMineralInfo(m_root, m_translator));
                m_infoProxy.sig_change.addNewClosure(new Callback(result, m_infoProxy, ele, name, mode));
                return result;
            }

        Widget& makeStructureEffect()
            {
                class Callback : public afl::base::Closure<void()> {
                 public:
                    Callback(ui::rich::DocumentView& widget,
                             PlanetInfoProxy& proxy)
                        : m_widget(widget),
                          m_proxy(proxy)
                        { }
                    virtual Callback* clone() const
                        { return new Callback(*this); }
                    virtual void call()
                        {
                            // Set up parser
                            afl::io::xml::NodeReader rdr;
                            const afl::io::xml::Nodes_t& nodes = m_proxy.getBuildingEffectsInfo();
                            for (size_t i = 0; i < nodes.size(); ++i) {
                                rdr.addNode(nodes[i]);
                            }

                            // Update widget
                            ui::rich::Document& doc = m_widget.getDocument();
                            doc.clear();
                            ui::rich::DocumentParser(doc, rdr).parseDocument();
                            doc.finish();
                            m_widget.handleDocumentUpdate();
                        }
                 private:
                    ui::rich::DocumentView& m_widget;
                    PlanetInfoProxy& m_proxy;
                };
                ui::rich::DocumentView& result = m_del.addNew(new ui::rich::DocumentView(m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 5), 0, m_root.provider()));
                m_infoProxy.sig_change.addNewClosure(new Callback(result, m_infoProxy));
                return result;
            }

        Widget& makeDefenseEffect()
            {
                static const size_t NUM_LINES = 1 + game::map::MAX_DEFENSE_EFFECT_LINES;
                class Callback : public afl::base::Closure<void()> {
                 public:
                    Callback(ui::widgets::SimpleTable& widget, PlanetInfoProxy& proxy, String_t maxStr)
                        : m_widget(widget),
                          m_proxy(proxy),
                          m_maxStr(maxStr)
                        { }
                    virtual Callback* clone() const
                        { return new Callback(*this); }
                    virtual void call()
                        {
                            // ex WPlanetDefenseEffectWidget::showLine (sort-of)
                            // General alignment
                            m_widget.column(0).setTextAlign(0, 0);
                            m_widget.column(1).setTextAlign(2, 0);

                            // Content
                            const game::map::DefenseEffectInfos_t& infos = m_proxy.getDefenseEffectsInfo();
                            size_t line = 1;
                            for (size_t i = 0; i < infos.size(); ++i) {
                                String_t prefix = infos[i].isDetail ? "  " : "";
                                m_widget.cell(0, line)
                                    .setText(prefix + infos[i].name)
                                    .setColor(ui::Color_Green);
                                m_widget.cell(1, line)
                                    .setText(infos[i].nextAt == 0 ? m_maxStr : String_t(afl::string::Format("+%d", infos[i].nextAt)))
                                    .setColor(infos[i].isAchievable ? ui::Color_Green : ui::Color_Yellow);
                                ++line;
                            }
                            while (line < NUM_LINES) {
                                m_widget.row(line).setText(String_t());
                                ++line;
                            }
                        }
                 private:
                    ui::widgets::SimpleTable& m_widget;
                    PlanetInfoProxy& m_proxy;
                    String_t m_maxStr;
                };
                ui::widgets::SimpleTable& result = m_del.addNew(new ui::widgets::SimpleTable(m_root, 2, NUM_LINES));
                m_infoProxy.sig_change.addNewClosure(new Callback(result, m_infoProxy, m_translator("(max)")));

                // Preconfigure layout
                // FIXME: need to use skin colors!
                afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
                result.all().setText(" ");
                result.setColumnWidth(0, font->getEmWidth() * 22);   // FIXME: should be minimum width
                result.row(0).setColor(ui::Color_Gray).setUnderline(true);
                result.cell(0, 0).setText(m_translator("You have:"));
                result.cell(1, 0).setText(m_translator("Next at:"));
                return result;
            }

        Widget& wrapFocus(Widget& w)
            { return ui::widgets::FocusableGroup::wrapWidget(m_del, 5, w); }

        Widget& buildBuildScreen1(ui::Widget& helpWidget)
            {
                // ex client/dlg-structure.cc:buildBuildScreen1
                // Build screen 1:
                //   VBox
                //     HBox
                //       VBox
                //         WPlanetStructureCostWidget
                //         Spacer
                //         WPlanetStructureEffectWidget
                //       VBox
                //         WPlanetStructureWidget x 3
                //     HBox: "S-Sell Supplies", "T-Taxes"
                //     HBox: "OK", "Cancel", "H", "Auto", "Goals"

                Group& group1   = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group11  = m_del.addNew(new Group(ui::layout::HBox::instance0));
                Group& group111 = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group112 = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group12  = m_del.addNew(new Group(ui::layout::HBox::instance5));
                Group& group13  = m_del.addNew(new Group(ui::layout::HBox::instance5));

                group111.add(m_del.addNew(new StructureCostWidget(m_root, false, m_formatter, m_translator, m_proxy)));
                group111.add(m_del.addNew(new Spacer()));
                group111.add(makeStructureEffect());

                Widget& w1 = wrapFocus(makeStructure(game::MineBuilding));
                Widget& w2 = wrapFocus(makeStructure(game::FactoryBuilding));
                Widget& w3 = wrapFocus(makeStructure(game::DefenseBuilding));

                group112.add(w1);
                group112.add(w2);
                group112.add(w3);
                group112.add(m_del.addNew(new Spacer())); // needed to assimilate excess space

                group11.add(group111);
                group11.add(m_del.addNew(new Spacer()));
                group11.add(group112);

                group12.add(makeKeyButton(KeyString(m_translator("S - Sell Supplies"))));
                group12.add(makeKeyButton(KeyString(m_translator("T - Taxes"))));
                group12.add(m_del.addNew(new Spacer()));

                group13.add(makeOkButton());
                group13.add(makeCancelButton());
                group13.add(makeHelpButton("H", helpWidget));
                group13.add(makeKeyButton(KeyString(m_translator("A - Auto"))));
                group13.add(makeKeyButton(KeyString(m_translator("G - Goals"))));
                group13.add(m_del.addNew(new Spacer()));

                group1.add(group11);
                group1.add(group12);
                group1.add(group13);

                FocusIterator& fit = m_del.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Wrap));
                fit.add(w1);
                fit.add(w2);
                fit.add(w3);
                group1.add(fit);

                w1.requestFocus();

                return group1;
            }

        Widget& buildBuildScreen2(ui::Widget& helpWidget)
            {
                // ex client/dlg-structure.cc:buildBuildScreen2
                // Build screen 2:
                //   HBox
                //     VBox
                //       WPlanetStructureCostWidget
                //       WPlanetStructureGroup
                //       Spacer
                //       WPlanetStructureEffectWidget
                //       HBox "Sell", "Tax"
                //       HBox "OK", "Cancel", "H", "A", "G"
                //     VBox
                //       Mining forecast x 4
                Group& group1   = m_del.addNew(new Group(ui::layout::HBox::instance0));
                Group& group11  = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group111 = m_del.addNew(new Group(ui::layout::HBox::instance5));
                Group& group112 = m_del.addNew(new Group(ui::layout::HBox::instance5));
                Group& group12  = m_del.addNew(new Group(ui::layout::VBox::instance5));

                Widget& w1 = wrapFocus(makeStructure(game::MineBuilding));

                group11.add(m_del.addNew(new StructureCostWidget(m_root, false, m_formatter, m_translator, m_proxy)));
                group11.add(w1);
                group11.add(m_del.addNew(new Spacer()));
                group11.add(makeStructureEffect());
                group11.add(group111);
                group11.add(group112);

                group111.add(makeKeyButton(KeyString(m_translator("S - Sell Supplies"))));
                group111.add(makeKeyButton(KeyString(m_translator("T - Taxes"))));
                group111.add(m_del.addNew(new Spacer()));

                group112.add(makeOkButton());
                group112.add(makeCancelButton());
                group112.add(makeHelpButton("H", helpWidget));
                group112.add(makeKeyButton(KeyString("A", 'a')));
                group112.add(makeKeyButton(KeyString("G", 'g')));
                group112.add(m_del.addNew(new Spacer()));

                group12.add(makeMineralInfo(PlanetInfoProxy::Neutronium, m_translator("Neutronium"), PlanetMineralInfo::First));
                group12.add(makeMineralInfo(PlanetInfoProxy::Tritanium,  m_translator("Tritanium"),  PlanetMineralInfo::Second));
                group12.add(makeMineralInfo(PlanetInfoProxy::Duranium,   m_translator("Duranium"),   PlanetMineralInfo::Second));
                group12.add(makeMineralInfo(PlanetInfoProxy::Molybdenum, m_translator("Molybdenum"), PlanetMineralInfo::Second));
                group12.add(m_del.addNew(new Spacer()));

                group1.add(group11);
                group1.add(m_del.addNew(new Spacer()));
                group1.add(group12);

                w1.requestFocus();

                return group1;
            }

        Widget& buildBuildScreen3(ui::Widget& helpWidget)
            {
                // ex client/dlg-structure.cc:buildBuildScreen3
                // Build screen 3:
                //   VBox
                //     HBox
                //       VBox
                //         WPlanetStructureCostWidget
                //         Spacer
                //         WPlanetStructureEffectWidget
                //         HBox "Sell" "Tax"
                //       VBox
                //         WPlanetStructureGroup x 2
                //         WPlanetDefenseEffectWidget
                //     HBox "OK" "ESC" "H" "Auto" "Goals" "Ground Combat"

                // FIXME: this forces the WPlanetStructureGroup to the same size
                // as the WPlanetDefenseEffectWidget.

                Group& group1    = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group11   = m_del.addNew(new Group(ui::layout::HBox::instance0));
                Group& group111  = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group1111 = m_del.addNew(new Group(ui::layout::HBox::instance5));
                Group& group112  = m_del.addNew(new Group(ui::layout::VBox::instance5));
                Group& group12   = m_del.addNew(new Group(ui::layout::HBox::instance5));

                group1111.add(makeKeyButton(KeyString(m_translator("S - Sell Supplies"))));
                group1111.add(makeKeyButton(KeyString(m_translator("T - Taxes"))));
                group1111.add(m_del.addNew(new Spacer()));

                group111.add(m_del.addNew(new StructureCostWidget(m_root, true, m_formatter, m_translator, m_proxy)));
                group111.add(m_del.addNew(new Spacer()));
                group111.add(makeStructureEffect());
                group111.add(group1111);

                Widget& w1 = wrapFocus(makeStructure(game::DefenseBuilding));
                Widget& w2 = wrapFocus(makeStructure(game::BaseDefenseBuilding));

                group112.add(w1);
                group112.add(w2);
                group112.add(m_del.addNew(new Spacer()));
                group112.add(makeDefenseEffect());

                group11.add(group111);
                group11.add(m_del.addNew(new Spacer()));
                group11.add(group112);

                Button& btnGroundCombat = m_del.addNew(new Button(KeyString(m_translator("C - Gnd Combat")), m_root));
                btnGroundCombat.sig_fire.add(this, &BuildStructuresDialog::onGroundCombat);

                group12.add(makeOkButton());
                group12.add(makeCancelButton());
                group12.add(makeHelpButton("H", helpWidget));
                group12.add(makeKeyButton(KeyString(m_translator("A - Auto"))));
                group12.add(makeKeyButton(KeyString(m_translator("G - Goals"))));
                group12.add(btnGroundCombat);
                group12.add(m_del.addNew(new Spacer()));

                group1.add(group11);
                group1.add(group12);

                // Is there a base?
                if (!m_info.hasBase) {
                    // Base defense is disabled if no base
                    w2.setState(Widget::DisabledState, true);
                } else {
                    // If there is a base, we need a focus iterator
                    FocusIterator& fit = m_del.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Wrap));
                    fit.add(w1);
                    fit.add(w2);
                    group112.add(fit);
                }
                w1.requestFocus();

                return group1;
            }

        void onStatusChange(const BuildStructuresProxy::Status& st)
            {
                m_status = st;
                for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
                    m_infoProxy.setBuildingOverride(game::PlanetaryBuilding(i), st.buildings[i].have);
                }
            }

        void onAutobuild()
            {
                m_proxy.doStandardAutoBuild();
            }

        void onGoalDialog()
            {
                client::dialogs::GoalDialog dlg(m_root, m_translator, false);
                for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
                    game::PlanetaryBuilding bb = game::PlanetaryBuilding(i);
                    dlg.setGoal(bb, m_status.buildings[i].want);
                    dlg.setSpeed(bb, m_status.buildings[i].speed);
                }
                if (dlg.run()) {
                    m_proxy.applyAutobuildSettings(dlg.getResult());
                }
            }

        void onTaxes()
            {
                client::dialogs::doTaxationDialog(m_planetId,
                                                  m_status.buildings[game::MineBuilding].have + m_status.buildings[game::FactoryBuilding].have,
                                                  m_root, m_translator, m_gameSender);
            }

        void onSellSupplies()
            {
                // Compute "reserved supplies" in a way that ConvertSupplies will end up with maxiumum .remaining.
                // Just using .needed would be wrong here, because that does not include supplies that are used to compensate missing cash.
                int32_t reservedSupplies = m_status.available.get(Cost::Supplies) - m_status.remaining.get(Cost::Supplies);

                // Reserved money is just what we need; if the transaction ate all our money, we don't want anything buyable.
                int32_t reservedMoney = m_status.needed.get(Cost::Money);

                // Do it
                client::dialogs::doSellSuppliesDialog(m_root, m_gameSender, m_planetId, reservedSupplies, reservedMoney, m_translator);
            }

        void onGroundCombat()
            {
                client::dialogs::doGroundDefenseDialog(m_root, m_infoProxy.getGroundDefenseInfo(), m_translator);
            }
    };
}


void
client::dialogs::doBuildStructuresDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::Id_t pid, int page)
{
    // ex doPlanetBuildScreen
    Downlink link(root);
    BuildStructuresProxy proxy(gameSender, root.engine().dispatcher());
    BuildStructuresProxy::HeaderInfo info;
    proxy.init(link, pid, info);
    if (!info.ok) {
        return;
    }

    NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    PlanetInfoProxy infoProxy(gameSender, root.engine().dispatcher());
    infoProxy.setPlanet(pid);

    BuildStructuresDialog dialog(root, tx, proxy, infoProxy, pid, info, fmt, gameSender);
    if (dialog.run(page)) {
        proxy.commit();
    }
}
