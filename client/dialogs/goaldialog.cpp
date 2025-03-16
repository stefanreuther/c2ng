/**
  *  \file client/dialogs/goaldialog.cpp
  */

#include "client/dialogs/goaldialog.hpp"
#include "afl/base/observable.hpp"
#include "afl/string/format.hpp"
#include "game/actions/buildstructures.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

/*
 *  Input Component
 *
 *  Widget assembly for one input value:
 *  a FocusableGroup containing "+"/"-" buttons and a DecimalSelector,
 *  as well as a DecimalSelector::Peer for the special GoalDialog handling.
 */

class client::dialogs::GoalDialog::InputComponent : public ui::widgets::DecimalSelector::Peer {
 public:
    InputComponent(GoalDialog& parent, bool isGoal)
        : Peer(),
          m_parent(parent),
          m_value(),
          m_widget(ui::layout::HBox::instance5)
        { init(isGoal); }

    ~InputComponent()
        { }

    // DecimalSelector::Peer:
    virtual String_t toString(const ui::widgets::BaseDecimalSelector& sel, int32_t value);
    virtual bool handleKey(const ui::widgets::BaseDecimalSelector& sel, util::Key_t key, int prefix);

    void setValue(int32_t n)
        { m_value.set(n); }

    int32_t getValue() const
        { return m_value.get(); }

    ui::Widget& widget()
        { return m_widget; }

 private:
    GoalDialog& m_parent;
    afl::base::Observable<int32_t> m_value;
    ui::widgets::FocusableGroup m_widget;

    void init(bool isGoal);
};

String_t
client::dialogs::GoalDialog::InputComponent::toString(const ui::widgets::BaseDecimalSelector& /*sel*/, int32_t value)
{
    // ex WGoalInput::getStringValue
    // ex pdata.pas:CBuildGoalInput.StrFunc
    if (value < 0) {
        return m_parent.m_translator("[keep]");
    } else if (value >= 1000) {
        return m_parent.m_translator("[max]");
    } else {
        return afl::string::Format("%d", value);
    }
}

bool
client::dialogs::GoalDialog::InputComponent::handleKey(const ui::widgets::BaseDecimalSelector& /*sel*/, util::Key_t key, int /*prefix*/)
{
    // ex WGoalInput::handleEvent
    if (m_parent.m_allowUnchanged && (key & ~util::KeyMod_Alt) == 'd') {
        m_value.set(-1);
        return true;
    } else {
        return false;
    }
}

void
client::dialogs::GoalDialog::InputComponent::init(bool isGoal)
{
    // ex WBuildGoalsDialog::createInputWidget
    // ex pdata.pas:CBuildGoalWindow.AddSelector
    // Bounds
    int lowerBound = m_parent.m_allowUnchanged ? -1 : 0;
    int upperBound = isGoal ? game::MAX_AUTOBUILD_GOAL : game::MAX_AUTOBUILD_SPEED;
    m_value.set(lowerBound);

    afl::base::Deleter& del = m_parent.m_deleter;
    ui::Root& root = m_parent.m_root;

    ui::widgets::Button& btnPlus  = del.addNew(new ui::widgets::Button("+", '+', root));
    ui::widgets::Button& btnMinus = del.addNew(new ui::widgets::Button("-", '-', root));
    ui::widgets::DecimalSelector& sel = del.addNew(new ui::widgets::DecimalSelector(root, m_parent.m_translator, m_value, lowerBound, upperBound, 10));
    m_widget.add(btnMinus);
    m_widget.add(sel);
    m_widget.add(btnPlus);
    sel.requestFocus();
    sel.setPeer(*this);

    btnPlus.dispatchKeyTo(sel);
    btnMinus.dispatchKeyTo(sel);
}


/*
 *  GoalDialog
 */

client::dialogs::GoalDialog::GoalDialog(ui::Root& root, afl::string::Translator& tx, bool allowUnchanged, ui::Widget* pHelp)
    : m_root(root),
      m_translator(tx),
      m_deleter(),
      m_loop(root),
      m_allowUnchanged(allowUnchanged),
      m_pWindow(0)
{
    init(pHelp);
}

client::dialogs::GoalDialog::~GoalDialog()
{
}

void
client::dialogs::GoalDialog::setGoal(game::PlanetaryBuilding building, int goal)
{
    m_goalInputs[building]->setValue(goal);
}

void
client::dialogs::GoalDialog::setSpeed(game::PlanetaryBuilding building, int speed)
{
    m_speedInputs[building]->setValue(speed);
}

int
client::dialogs::GoalDialog::getGoal(game::PlanetaryBuilding building) const
{
    // ex WBuildGoalsDialog::getGoal
    return m_goalInputs[building]->getValue();
}

int
client::dialogs::GoalDialog::getSpeed(game::PlanetaryBuilding building) const
{
    // ex WBuildGoalsDialog::getSpeed
    return m_speedInputs[building]->getValue();
}

game::map::Planet::AutobuildSettings
client::dialogs::GoalDialog::getResult() const
{
    game::map::Planet::AutobuildSettings result;
    for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
        int speed = m_speedInputs[i]->getValue();
        if (speed >= 0) {
            result.speed[i] = speed;
        }
        int goal = m_goalInputs[i]->getValue();
        if (goal >= 0) {
            result.goal[i] = goal;
        }
    }
    return result;
}

void
client::dialogs::GoalDialog::setFocusToStructure(game::PlanetaryBuilding building)
{
    // ex WBuildGoalsDialog::setFocusToStructure
    m_goalInputs[building]->widget().requestFocus();
}

bool
client::dialogs::GoalDialog::run()
{
    m_root.centerWidget(*m_pWindow);
    m_root.add(*m_pWindow);
    return m_loop.run() != 0;
}


void
client::dialogs::GoalDialog::init(ui::Widget* pHelp)
{
    // Window [VBox]
    //   Group [Grid]
    //     'Structure' 'Goal' 'Speed'
    //     'Mines'     Edit   Edit
    //     'Factories' Edit   Edit
    //     'Defense'   Edit   Edit
    //     'SBD'       Edit   Edit
    //   Group [UIHBoxLayout]
    //     Button 'OK'
    //     Button 'Cancel'
    //    [Button D]
    //     Spacer
    //     Button 'Help'
    using game::actions::BuildStructures;

    ui::Window& win = m_deleter.addNew(new ui::Window(m_translator("Auto-Build Goals"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Focus iterators
    ui::widgets::FocusIterator& tabIt   = m_deleter.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    ui::widgets::FocusIterator& goalIt  = m_deleter.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical));
    ui::widgets::FocusIterator& speedIt = m_deleter.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical));

    // Input grid
    ui::Group& gridGroup = m_deleter.addNew(new ui::Group(m_deleter.addNew(new ui::layout::Grid(3))));
    gridGroup.add(m_deleter.addNew(new ui::widgets::StaticText(m_translator("Structure"), ui::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    gridGroup.add(m_deleter.addNew(new ui::widgets::StaticText(m_translator("Goal"),      ui::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    gridGroup.add(m_deleter.addNew(new ui::widgets::StaticText(m_translator("Speed"),     ui::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));

    for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
        const BuildStructures::Description& desc = BuildStructures::describe(game::PlanetaryBuilding(i));
        gridGroup.add(m_deleter.addNew(new ui::widgets::StaticText(m_translator(desc.untranslatedBuildingName), ui::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));

        InputComponent& goal = m_deleter.addNew(new InputComponent(*this, true));
        m_goalInputs[i] = &goal;
        gridGroup.add(goal.widget());
        goalIt.add(goal.widget());
        tabIt.add(goal.widget());

        InputComponent& speed = m_deleter.addNew(new InputComponent(*this, false));
        m_speedInputs[i] = &speed;
        gridGroup.add(speed.widget());
        speedIt.add(speed.widget());
        tabIt.add(speed.widget());
    }
    win.add(gridGroup);
    win.add(tabIt);
    win.add(goalIt);
    win.add(speedIt);

    // Button row
    ui::Group& buttonGroup = m_deleter.addNew(new ui::Group(ui::layout::HBox::instance5));

    // - OK
    ui::widgets::Button& btnOK = m_deleter.addNew(new ui::widgets::Button(m_translator("OK"), util::Key_Return, m_root));
    buttonGroup.add(btnOK);
    btnOK.sig_fire.addNewClosure(m_loop.makeStop(1));

    // - Cancel
    ui::widgets::Button& btnCancel = m_deleter.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
    buttonGroup.add(btnCancel);
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));

    // - D
    if (m_allowUnchanged) {
        ui::widgets::Button& btnUnchange = m_deleter.addNew(new ui::widgets::Button(m_translator("D - Don't change"), 'd', m_root));
        buttonGroup.add(btnUnchange);

        // Dispatch to gridGroup: focused widget will handle it
        // (do not dispatch to window because that will retrigger again!)
        btnUnchange.dispatchKeyTo(gridGroup);
    }

    buttonGroup.add(m_deleter.addNew(new ui::Spacer()));

    if (pHelp != 0) {
        ui::widgets::Button& btnHelp = m_deleter.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
        buttonGroup.add(btnHelp);
        buttonGroup.add(*pHelp);
        btnHelp.dispatchKeyTo(*pHelp);
    }
    win.add(buttonGroup);
    win.pack();

    m_pWindow = &win;
}
