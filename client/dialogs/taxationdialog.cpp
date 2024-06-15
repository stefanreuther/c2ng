/**
  *  \file client/dialogs/taxationdialog.cpp
  *  \brief Taxation Dialog
  */

#include <cmath>
#include "client/dialogs/taxationdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/planetpredictorproxy.hpp"
#include "game/proxy/taxationproxy.hpp"
#include "game/root.hpp"
#include "game/tables/happinesschangename.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/listlikedecimalselector.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/skincolor.hpp"
#include "util/unicodechars.hpp"

using game::proxy::TaxationProxy;
using game::proxy::PlanetPredictorProxy;
using game::proxy::ConfigurationProxy;
using game::actions::TaxationAction;
using game::map::PlanetEffectors;
using ui::widgets::FocusIterator;
using util::SkinColor;

namespace {
    const int NUM_TURNS = 5;

    TaxationAction::Areas_t allAreas()
    {
        return TaxationAction::Areas_t() + TaxationAction::Colonists + TaxationAction::Natives;
    }

    int32_t get(const PlanetPredictorProxy::Vector_t& vec, size_t index)
    {
        return index < vec.size() ? vec[index] : 0;
    }

    void renderPopulationPrediction(ui::widgets::SimpleTable& tab,
                                    int line,
                                    const PlanetPredictorProxy::Vector_t& data,
                                    bool relative,
                                    bool ratio,
                                    util::NumberFormatter fmt)
    {
        // ex WPlanetGrowthTile::drawData (part), WPlanetGrowthTile::showItem
        int32_t oldPop = get(data, 0);
        for (int i = 1; i <= NUM_TURNS; ++i) {
            int32_t newPop = get(data, size_t(i));
            if (newPop == 0 && (oldPop == 0 || !relative)) {
                tab.cell(i, line).setText(UTF_MIDDLE_DOT);
            } else if (relative) {
                String_t diffs = ratio && oldPop != 0
                    ? String_t(afl::string::Format("%.1f%%", std::fabs(100.0 * double(newPop - oldPop) / oldPop)))
                    : fmt.formatPopulation(std::abs(newPop - oldPop));
                if (newPop < oldPop) {
                    diffs.insert(0, "-");
                }
                if (newPop > oldPop) {
                    diffs.insert(0, "+");
                }
                tab.cell(i, line).setText(diffs);
            } else {
                tab.cell(i, line).setText(fmt.formatPopulation(newPop));
            }
            oldPop = newPop;
        }
    }

    void renderExperiencePrediction(ui::widgets::SimpleTable& tab,
                                    int line,
                                    const PlanetPredictorProxy::Vector_t& points,
                                    const PlanetPredictorProxy::Vector_t& levels,
                                    bool relative,
                                    util::NumberFormatter fmt)
    {
        // ex WPlanetGrowthTile::showExp
        int32_t oldExp = get(points, 0);
        int32_t oldLevel = get(levels, 0);
        for (int i = 1; i <= NUM_TURNS; ++i) {
            int32_t newExp = get(points, i);
            int32_t newLevel = get(levels, i);
            String_t text;
            if (newExp == 0) {
                text = UTF_MIDDLE_DOT;
            } else if (relative) {
                text = fmt.formatNumber(std::abs(newExp - oldExp));
                if (newExp > oldExp) {
                    text.insert(0, "+");
                }
            } else {
                text = fmt.formatNumber(newExp);
            }

            if (oldLevel != newLevel) {
                text.insert(0, UTF_UP_ARROW " ");
            }
            tab.cell(i, line).setText(text);
            oldExp = newExp;
            oldLevel = newLevel;
        }
    }

    void setTaxes(PlanetPredictorProxy& ppProxy, const TaxationProxy::Status& st)
    {
        if (st.colonists.available) {
            ppProxy.setTax(TaxationAction::Colonists, st.colonists.tax);
        }
        if (st.natives.available) {
            ppProxy.setTax(TaxationAction::Natives, st.natives.tax);
        }
    }

    /*
     *  EffectorDialog - edit a PlanetEffectors object (+ utilities)
     */

    typedef afl::base::Observable<int32_t> Value_t;
    typedef afl::base::Signal<void(const PlanetEffectors&)> EffectorSignal_t;

    const size_t NUM_EFFECTORS = 4;

    const PlanetEffectors::Kind EFFECTOR_MAP[NUM_EFFECTORS] = {
        PlanetEffectors::Hiss,
        PlanetEffectors::CoolsTo50,
        PlanetEffectors::HeatsTo50,
        PlanetEffectors::HeatsTo100
    };

    struct EffectorLabels {
        String_t label[NUM_EFFECTORS];
    };

    String_t addHullName(String_t text, int basicFunctionId, game::Session& session)
    {
        const game::spec::ShipList* sl = session.getShipList().get();
        const game::Root* r = session.getRoot().get();
        if (sl != 0 && r != 0) {
            if (const game::spec::Hull* h = sl->findSpecimenHullForFunction(basicFunctionId, r->hostConfiguration(), r->playerList().getAllPlayers(), game::PlayerSet_t(), true)) {
                text += " (";
                text += h->getShortName(sl->componentNamer());
                text += ")";
            }
        }
        return text;
    }

    void getEffectorLabels(game::proxy::WaitIndicator& ind, util::RequestSender<game::Session> gameSender, EffectorLabels& labels)
    {
        class Task : public util::Request<game::Session> {
         public:
            Task(EffectorLabels& labels)
                : m_labels(labels)
                { }
            virtual void handle(game::Session& session)
                {
                    afl::string::Translator& tx = session.translator();
                    m_labels.label[0] = tx("Ships hissing");
                    m_labels.label[1] = addHullName(tx("Ships cooling to 50\xC2\xB0\x46"), game::spec::BasicHullFunction::CoolsTo50, session);
                    m_labels.label[2] = addHullName(tx("Ships heating to 50\xC2\xB0\x46"), game::spec::BasicHullFunction::HeatsTo50, session);
                    m_labels.label[3] = addHullName(tx("Ships heating to 100\xC2\xB0\x46"), game::spec::BasicHullFunction::HeatsTo100, session);
                }
         private:
            EffectorLabels& m_labels;
        };
        Task t(labels);
        ind.call(gameSender, t);
    }

    class EffectorDialog {
     public:
        EffectorDialog(ui::Root& root, const PlanetEffectors& eff, EffectorSignal_t& sig, afl::string::Translator& tx);

        bool run(util::RequestSender<game::Session> gameSender);
        void onWidgetChange();

     private:
        afl::string::Translator& m_translator;
        ui::Root& m_root;
        PlanetEffectors m_effectors;
        EffectorSignal_t& m_signal;
        afl::container::PtrVector<afl::base::Observable<int32_t> > m_counts;
    };

    /*
     *  TaxationKeyWidget: dispatch keys
     */
    class TaxationKeyWidget : public ui::InvisibleWidget {
     public:
        TaxationKeyWidget(TaxationProxy& proxy, TaxationAction::Area area);
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        TaxationProxy& m_proxy;
        TaxationAction::Area m_area;
    };

    /*
     *  TaxationWidget: widget assembly for one taxation area
     *
     *  Displays the content of a TaxationProxy::AreaStatus:
     *  - heading
     *  - textual information
     *  - "+"/"-" buttons
     *
     *  Size is 30em x 5 lines for both.
     *
     *  PCC2: 380px x 3 lines for colonists, 380px x 5 lines for natives.
     */
    class TaxationWidget : public ui::widgets::FocusableGroup {
     public:
        TaxationWidget(afl::base::Deleter& del, afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, TaxationAction::Area area);
        void setContent(const TaxationProxy::AreaStatus& st);

     private:
        afl::string::Translator& m_translator;
        ui::widgets::StaticText* m_pTitle;
        ui::rich::DocumentView* m_pInfo;
    };

    /*
     *  TaxationDialog: entire dialog
     */
    class TaxationDialog : private gfx::KeyEventConsumer {
     public:
        TaxationDialog(afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, PlanetPredictorProxy& ppProxy, util::RequestSender<game::Session> gameSender, util::NumberFormatter fmt);
        bool run(const TaxationProxy::Status& initialStatus, const PlanetPredictorProxy::Status& initialPrediction);
        void update(const TaxationProxy::Status& st);
        void updatePrediction(const PlanetPredictorProxy::Status& pred);
        void renderPrediction();
        void setEffectors(const PlanetEffectors& eff);

        void toggleMode();
        void toggleRatio();

        void setRelative(bool flag);
        bool isRelative() const;
        void setRatio(bool flag);
        bool isRatio() const;
        void editEffectors();

        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        // Links
        afl::string::Translator& m_translator;
        ui::Root& m_root;
        TaxationProxy& m_proxy;
        PlanetPredictorProxy& m_predictorProxy;
        util::RequestSender<game::Session> m_gameSender;
        util::NumberFormatter m_formatter;
        TaxationWidget* m_pNativeTaxes;
        TaxationWidget* m_pColonistTaxes;
        ui::widgets::SimpleTable* m_pPredictionTable;
        ui::widgets::StaticText* m_pEffectorLabel;

        // Signal connections
        afl::base::SignalConnection conn_change;
        afl::base::SignalConnection conn_predictionChange;

        // State
        PlanetPredictorProxy::Status m_lastPrediction;
        PlanetEffectors m_effectors;
        bool m_relative;
        bool m_ratio;
    };
}

/*
 *  EffectorDialog implementation
 */

EffectorDialog::EffectorDialog(ui::Root& root, const PlanetEffectors& eff, EffectorSignal_t& sig, afl::string::Translator& tx)
    : m_translator(tx), m_root(root), m_effectors(eff), m_signal(sig)
{ }

bool
EffectorDialog::run(util::RequestSender<game::Session> gameSender)
{
    // ex WEffectorDialog::init

    // Retrieve labels
    client::Downlink link(m_root, m_translator);
    EffectorLabels labels;
    getEffectorLabels(link, gameSender, labels);

    // Build dialog
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Prediction Effects"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Page | FocusIterator::Home));
    ui::widgets::NumberSelector* toFocus = 0;
    ui::Group& g = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    for (size_t i = 0; i < NUM_EFFECTORS; ++i) {
        Value_t& thisValue = *m_counts.pushBackNew(new Value_t(m_effectors.get(EFFECTOR_MAP[i])));
        thisValue.sig_change.add(this, &EffectorDialog::onWidgetChange);

        ui::widgets::ListLikeDecimalSelector& thisInput = del.addNew(new ui::widgets::ListLikeDecimalSelector(m_root, labels.label[i], thisValue, 0, 100, 1));
        if (toFocus == 0 || (toFocus->value().get() == 0 && thisValue.get() != 0)) {
            toFocus = &thisInput;
        }
        g.add(thisInput);
        it.add(thisInput);
    }
    win.add(g);
    win.add(it);

    ui::EventLoop loop(m_root);
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    win.add(btn);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    btn.addStop(loop);

    if (toFocus != 0) {
        toFocus->requestFocus();
    }

    win.pack();
    m_root.moveWidgetToEdge(win, gfx::RightAlign, gfx::BottomAlign, 10);
    m_root.add(win);
    return loop.run();
}

void
EffectorDialog::onWidgetChange()
{
    // ex WEffectorDialog::updateTo()
    for (size_t i = 0; i < NUM_EFFECTORS; ++i) {
        m_effectors.set(EFFECTOR_MAP[i], m_counts[i]->get());
    }
    m_signal.raise(m_effectors);
}



/*
 *  TaxationKeyWidget implementation
 */

TaxationKeyWidget::TaxationKeyWidget(TaxationProxy& proxy, TaxationAction::Area area)
    : m_proxy(proxy),
      m_area(area)
{ }

bool
TaxationKeyWidget::handleKey(util::Key_t key, int prefix)
{
    // ex WColonistTaxSelector::handleEvent, WNativeTaxSelector::handleEvent, UINumberSelector::handleEvent (part)
    switch (key) {
     case '-':
     case util::Key_Left:
        // Decrement
        m_proxy.changeTax(m_area, prefix ? -prefix : -1);
        return true;

     case '+':
     case util::Key_Right:
        // Increment
        m_proxy.changeTax(m_area, prefix ? +prefix : +1);
        return true;

     case util::KeyMod_Ctrl + '-':
     case util::KeyMod_Ctrl + util::Key_Left:
     case util::KeyMod_Alt + '-':
     case util::KeyMod_Alt + util::Key_Left:
        // Min
        m_proxy.setTaxLimited(m_area, 0);
        return true;

     case util::KeyMod_Ctrl + '+':
     case util::KeyMod_Ctrl + util::Key_Right:
     case util::KeyMod_Alt + '+':
     case util::KeyMod_Alt + util::Key_Right:
        // Max
        m_proxy.setTaxLimited(m_area, 100);
        return true;

     case util::KeyMod_Shift + util::Key_Left:
        // Decrease until income changes
        m_proxy.changeRevenue(m_area, TaxationAction::Down);
        return true;

     case util::KeyMod_Shift + util::Key_Right:
        // Increase until income changes
        m_proxy.changeRevenue(m_area, TaxationAction::Up);
        return true;

     case ' ':
        // Auto-tax all
        m_proxy.setSafeTax(allAreas());
        return true;

     case ' ' + util::KeyMod_Shift:
        // Auto-tax area
        m_proxy.setSafeTax(TaxationAction::Areas_t(m_area));
        return true;

     case '=':
     case '%':
        if (prefix != 0) {
            m_proxy.setTaxLimited(m_area, prefix);
        }
        return true;

     case 'u':
     case util::Key_Backspace:
        // Undo
        m_proxy.revert(TaxationAction::Areas_t(m_area));
        return true;

     default:
        return false;
    }
}


/*
 *  TaxationWidget implementation
 */

TaxationWidget::TaxationWidget(afl::base::Deleter& del, afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, TaxationAction::Area area)
    : FocusableGroup(ui::layout::HBox::instance5, 5),
      m_translator(tx),
      m_pTitle(&del.addNew(new ui::widgets::StaticText(String_t("?"), SkinColor::Static, gfx::FontRequest().addSize(1), root.provider()))),
      m_pInfo(&del.addNew(new ui::rich::DocumentView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(32, 5), 0, root.provider())))
{
    // HBox
    //   VBox
    //     Title
    //     Info
    //   VBox
    //     "+"
    //     "-"
    //     Spacer
    TaxationKeyWidget& keys = del.addNew(new TaxationKeyWidget(proxy, area));
    add(keys);
    add(del.addNew(new ui::PrefixArgument(root)));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g1.add(*m_pTitle);
    g1.add(*m_pInfo);
    add(g1);

    add(del.addNew(new ui::Spacer())); // FIXME: this should not be needed

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::widgets::Button& btnInc = del.addNew(new ui::widgets::Button("+", '+', root));
    ui::widgets::Button& btnDec = del.addNew(new ui::widgets::Button("-", '-', root));
    g2.add(btnInc);
    g2.add(btnDec);
    g2.add(del.addNew(new ui::Spacer()));

    btnInc.dispatchKeyTo(keys);
    btnDec.dispatchKeyTo(keys);

    add(g2);
}

void
TaxationWidget::setContent(const TaxationProxy::AreaStatus& st)
{
    // ex WColonistTaxSelector::drawContent, WNativeTaxSelector::drawContent, showChange, pdata.pas:ShowChange
    // Title
    m_pTitle->setText(st.title);

    // Info
    ui::rich::Document& doc = m_pInfo->getDocument();
    doc.clear();
    doc.setPageWidth(m_pInfo->getExtent().getWidth());
    doc.add(afl::string::Format(m_translator("Tax Rate: %d%%"), st.tax));
    doc.add(" " UTF_EM_DASH " ");

    String_t changeLabel = st.changeLabel;
    if (st.change != 0) {
        changeLabel += afl::string::Format(" (%d)", st.change);
    }
    doc.add(util::rich::Text(st.change < 0 ? SkinColor::Red : SkinColor::Green, changeLabel));
    doc.addParagraph();
    doc.add(st.description);
    doc.finish();
    m_pInfo->handleDocumentUpdate();
}


/*
 *  TaxationDialog implementation
 */

TaxationDialog::TaxationDialog(afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, PlanetPredictorProxy& ppProxy, util::RequestSender<game::Session> gameSender, util::NumberFormatter fmt)
    : m_translator(tx),
      m_root(root),
      m_proxy(proxy),
      m_predictorProxy(ppProxy),
      m_gameSender(gameSender),
      m_formatter(fmt),
      m_pNativeTaxes(0),
      m_pColonistTaxes(0),
      m_pPredictionTable(0),
      m_pEffectorLabel(0),
      conn_change(proxy.sig_change.add(this, &TaxationDialog::update)),
      conn_predictionChange(ppProxy.sig_update.add(this, &TaxationDialog::updatePrediction)),
      m_lastPrediction(),
      m_effectors(),
      m_relative(false),
      m_ratio(false)
{ }

bool
TaxationDialog::run(const TaxationProxy::Status& initialStatus, const PlanetPredictorProxy::Status& initialPrediction)
{
    // ex WTaxationDialog::init, CTaxWindow.Init
    // VBox
    //   VBox
    //     Colonists
    //     Natives
    //   SimpleTable (prediction)
    //   HBox
    //     StaticText (effectors)
    //     "R", "W"
    //   HBox
    //     "OK", "Cancel", "Space", "Help"
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Taxes"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Taxation widgets.
    // We put these into a separate group to have a safe target for the "Space" button.
    // The button needs to route its keypress into the currently-focused widget.
    // We cannot route the key into /win/ because that would possibly trigger the button again.
    FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Tab + FocusIterator::Vertical));
    ui::Group& top = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    if (initialStatus.colonists.available) {
        TaxationWidget& colTax = del.addNew(new TaxationWidget(del, m_translator, m_root, m_proxy, TaxationAction::Colonists));
        top.add(colTax);
        it.add(colTax);
        m_pColonistTaxes = &colTax;
    }
    if (initialStatus.natives.available) {
        TaxationWidget& natTax = del.addNew(new TaxationWidget(del, m_translator, m_root, m_proxy, TaxationAction::Natives));
        top.add(natTax);
        it.add(natTax);
        m_pNativeTaxes = &natTax;
    }
    win.add(top);

    // Prediction
    int numLines = 0;
    if (!initialPrediction.colonistClans.empty()) {
        ++numLines;
    }
    if (!initialPrediction.nativeClans.empty()) {
        ++numLines;
    }
    if (!initialPrediction.experiencePoints.empty()) {
        ++numLines;
    }
    if (numLines != 0) {
        ui::widgets::SimpleTable& table = del.addNew(new ui::widgets::SimpleTable(m_root, NUM_TURNS + 1, numLines));
        for (int i = 1; i <= NUM_TURNS; ++i) {
            table.column(i).setTextAlign(gfx::RightAlign, gfx::TopAlign);
            table.setColumnWidth(i, m_root.provider().getFont(gfx::FontRequest())->getTextWidth(" +99,999,999"));
        }
        table.all().setColor(SkinColor::Static);
        m_pPredictionTable = &table;
        win.add(table);
        updatePrediction(initialPrediction);
    }

    // Effectors/settings
    // ex WPlanetGrowthTile::WPlanetGrowthTile
    ui::Group& effGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::StaticText& effLabel = del.addNew(new ui::widgets::StaticText(String_t("?"), SkinColor::Static, gfx::FontRequest(), m_root.provider()));
    ui::widgets::Button& btnR = del.addNew(new ui::widgets::Button("R", 'r', m_root));
    ui::widgets::Button& btnW = del.addNew(new ui::widgets::Button("W", 'w', m_root));
    effLabel.setIsFlexible(true);
    btnR.setFont(gfx::FontRequest());
    btnW.setFont(gfx::FontRequest());
    effGroup.add(effLabel);
    effGroup.add(btnR);
    effGroup.add(btnW);
    win.add(effGroup);
    m_pEffectorLabel = &effLabel;

    // Buttons
    ui::EventLoop loop(m_root);
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Widget& helpWidget = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:taxes"));
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(m_translator("OK"),     util::Key_Return, m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
    ui::widgets::Button& btnAuto   = del.addNew(new ui::widgets::Button(m_translator("Space - Auto Tax"), ' ', m_root));
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    ui::widgets::KeyForwarder& forwarder = del.addNew(new ui::widgets::KeyForwarder(*this));
    g.add(btnOK);
    g.add(btnCancel);
    g.add(btnAuto);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnHelp);
    win.add(g);
    win.add(it);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.add(helpWidget);
    win.add(forwarder);
    win.pack();


    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    btnAuto.dispatchKeyTo(top);
    btnHelp.dispatchKeyTo(helpWidget);
    btnR.dispatchKeyTo(forwarder);
    btnW.sig_fire.add(this, &TaxationDialog::editEffectors);

    m_root.centerWidget(win);

    // Set initial content (after layout so word-wrap works ok)
    update(initialStatus);

    m_root.add(win);
    return loop.run() != 0;
}

void
TaxationDialog::update(const TaxationProxy::Status& st)
{
    // ex WTaxationDialog::onChange
    if (m_pNativeTaxes != 0) {
        m_pNativeTaxes->setContent(st.natives);
    }
    if (m_pColonistTaxes != 0) {
        m_pColonistTaxes->setContent(st.colonists);
    }
    setTaxes(m_predictorProxy, st);
}

void
TaxationDialog::updatePrediction(const PlanetPredictorProxy::Status& pred)
{
    m_lastPrediction = pred;
    renderPrediction();
}

void
TaxationDialog::renderPrediction()
{
    if (m_pPredictionTable != 0) {
        int line = 0;

        // Colonists
        if (!m_lastPrediction.colonistClans.empty()) {
            m_pPredictionTable->cell(0, line).setText(m_translator("Colonists"));
            renderPopulationPrediction(*m_pPredictionTable, line, m_lastPrediction.colonistClans, m_relative, m_ratio, m_formatter);
            ++line;
        }

        // Natives
        if (!m_lastPrediction.nativeClans.empty()) {
            m_pPredictionTable->cell(0, line).setText(m_translator("Natives"));
            renderPopulationPrediction(*m_pPredictionTable, line, m_lastPrediction.nativeClans, m_relative, m_ratio, m_formatter);
            ++line;
        }

        // Experience
        if (!m_lastPrediction.experiencePoints.empty()) {
            m_pPredictionTable->cell(0, line).setText(m_translator("Experience"));
            renderExperiencePrediction(*m_pPredictionTable, line, m_lastPrediction.experiencePoints, m_lastPrediction.experienceLevel, m_relative, m_formatter);
            ++line;
        }
    }

    // Effectors
    if (m_pEffectorLabel != 0) {
        m_pEffectorLabel->setText(m_lastPrediction.effectorLabel);
    }
}

void
TaxationDialog::setEffectors(const PlanetEffectors& eff)
{
    m_effectors = eff;
    m_proxy.setEffectors(eff);
    m_predictorProxy.setEffectors(eff);
}

void
TaxationDialog::toggleMode()
{
    if (!m_relative) {
        m_relative = true;
        m_ratio = false;
    } else if (m_ratio) {
        m_relative = false;
        m_ratio = false;
    } else {
        m_ratio = true;
    }
    renderPrediction();
}

void
TaxationDialog::toggleRatio()
{
    m_ratio = !m_ratio || !m_relative;
    m_relative = true;
    renderPrediction();
}

void
TaxationDialog::setRelative(bool flag)
{
    m_relative = flag;
    renderPrediction();
}

inline bool
TaxationDialog::isRelative() const
{
    return m_relative;
}

void
TaxationDialog::setRatio(bool flag)
{
    m_ratio = flag;
    renderPrediction();
}

bool
TaxationDialog::isRatio() const
{
    return m_ratio;
}

void
TaxationDialog::editEffectors()
{
    // ex WTaxationDialog::editEffectors
    PlanetEffectors oldValues = m_effectors;

    EffectorSignal_t sig;
    sig.add(this, &TaxationDialog::setEffectors);

    EffectorDialog dlg(m_root, m_effectors, sig, m_translator);
    if (!dlg.run(m_gameSender)) {
        setEffectors(oldValues);
    }
}

bool
TaxationDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    switch (key) {
     case 'r':
        toggleMode();
        return true;

     case 'R':                           // regular input
     case 'r' + util::KeyMod_Shift:      // Shift-Click to button
        // Cannot use '%', that already is "set to X% tax"
        toggleRatio();
        return true;

     default:
        return false;
    }
}


/*
 *  Main Entry
 */
void
client::dialogs::doTaxationDialog(game::Id_t planetId,
                                  afl::base::Optional<int> numBuildings,
                                  ui::Root& root,
                                  afl::string::Translator& tx,
                                  util::RequestSender<game::Session> gameSender)
{
    // ex doTaxation, pdata.pas:TaxRates
    // Set up proxy
    TaxationProxy proxy(root.engine().dispatcher(), gameSender, planetId);
    if (const int* p = numBuildings.get()) {
        proxy.setNumBuildings(*p);
    }

    // Check status
    TaxationProxy::Status st;
    Downlink link(root, tx);
    proxy.getStatus(link, st);
    if (!st.valid) {
        return;
    }

    // More proxies
    PlanetPredictorProxy ppProxy(root.engine().dispatcher(), gameSender, planetId);
    ppProxy.setNumTurns(NUM_TURNS);
    setTaxes(ppProxy, st);
    if (const int* p = numBuildings.get()) {
        // FIXME: we have to set buildings correctly but only have a sum here; split arbitrarily.
        // This can indirectly affect predictions to a small extent (different decay behaviour).
        int mines = *p/2;
        ppProxy.setNumBuildings(game::MineBuilding, mines);
        ppProxy.setNumBuildings(game::FactoryBuilding, *p - mines);
    }
    PlanetPredictorProxy::Status ppStatus;
    ppProxy.getStatus(link, ppStatus);
    PlanetEffectors eff = ppProxy.getEffectors(link);

    ConfigurationProxy cfgProxy(gameSender);

    // Build dialog
    TaxationDialog dlg(tx, root, proxy, ppProxy, gameSender, cfgProxy.getNumberFormatter(link));
    dlg.setEffectors(eff);
    dlg.setRelative(cfgProxy.getOption(link, game::config::UserConfiguration::Tax_PredictRelative));
    dlg.setRatio(cfgProxy.getOption(link, game::config::UserConfiguration::Tax_PredictRatio));
    if (dlg.run(st, ppStatus)) {
        proxy.commit();
    }
    cfgProxy.setOption(game::config::UserConfiguration::Tax_PredictRelative, dlg.isRelative());
    cfgProxy.setOption(game::config::UserConfiguration::Tax_PredictRatio, dlg.isRatio());
}
