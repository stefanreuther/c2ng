/**
  *  \file client/dialogs/simulationflakratings.cpp
  *  \brief Simulator: FLAK Rating Editor
  */

#include <algorithm>
#include "client/dialogs/simulationflakratings.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/observable.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    /* Make number input compound:
          FocusableGroup
            StaticText
            Group
              Button "-"
              NumberSelector
              Button "+" */
    ui::Widget& makeInput(afl::base::Deleter& del, const String_t& label, int labelWidth, const gfx::FontRequest& fontReq, ui::Root& root, ui::widgets::NumberSelector& sel)
    {
        ui::widgets::FocusableGroup& g = del.addNew(new ui::widgets::FocusableGroup(ui::layout::HBox::instance5));
        g.add(del.addNew(new ui::widgets::StaticText(label, util::SkinColor::Static, fontReq, root.provider()))
              .setForcedWidth(labelWidth));
        g.add(sel.addButtons(del, root));
        return g;
    }

    class Dialog {
     public:
        Dialog(ui::Root& root, const client::dialogs::SimulationFlakRatings& values, afl::string::Translator& tx)
            : m_root(root), m_translator(tx),
              m_useDefaults(values.useDefaults),
              m_flakRating(values.flakRating),
              m_flakCompensation(values.flakCompensation),
              m_useDefaultsCheckbox(root, 'a', tx("Automatic"), m_useDefaults),
              m_flakRatingSelector(root, tx, m_flakRating, 1, 1000000, 100),
              m_flakCompensationSelector(root, tx, m_flakCompensation, 0, 32000, 100),
              m_defaultFlakRating(values.defaultFlakRating),
              m_defaultFlakCompensation(values.defaultFlakCompensation),
              m_ignoreValueChange()
            {
                m_useDefaultsCheckbox.addDefaultImages();
                m_useDefaults.sig_change.add(this, &Dialog::onUseDefaultsChange);
                m_flakRating.sig_change.add(this, &Dialog::onValueChange);
                m_flakCompensation.sig_change.add(this, &Dialog::onValueChange);
            }

        bool run()
            {
                // VBox
                //   Checkbox
                //   FocusableGroup [StaticText, DecimalSelector, Buttons]
                //   FocusableGroup [StaticText, DecimalSelector, Buttons]
                //   StandardDialogButtons
                afl::base::Deleter del;
                ui::EventLoop loop(m_root);
                ui::Window& win = del.addNew(new ui::Window(m_translator("FLAK Ratings"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(m_useDefaultsCheckbox);

                // Labels
                const String_t ratingLabel       = m_translator("Targeting");
                const String_t compensationLabel = m_translator("Compensation");
                const gfx::FontRequest fontReq("+");
                const afl::base::Ref<gfx::Font> font(m_root.provider().getFont(fontReq));
                const int labelWidth = std::max(font->getTextWidth(ratingLabel),
                                                font->getTextWidth(compensationLabel)) + 10;

                // Inputs
                win.add(makeInput(del, ratingLabel,       labelWidth, fontReq, m_root, m_flakRatingSelector));
                win.add(makeInput(del, compensationLabel, labelWidth, fontReq, m_root, m_flakCompensationSelector));

                // Buttons
                ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
                btn.addStop(loop);
                win.add(btn);

                // Helpers
                ui::widgets::FocusIterator& iter = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab));
                iter.add(m_useDefaultsCheckbox);
                iter.add(m_flakRatingSelector);
                iter.add(m_flakCompensationSelector);
                win.add(iter);
                win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

                // Initial focus
                m_flakRatingSelector.requestFocus();

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return loop.run() != 0;
            }

        void writeBack(client::dialogs::SimulationFlakRatings& values)
            {
                values.useDefaults      = m_useDefaults.get() != 0;
                values.flakRating       = m_flakRating.get();
                values.flakCompensation = m_flakCompensation.get();
            }

     private:
        /* Checkbox change: when activating "use defaults", set defaults */
        void onUseDefaultsChange()
            {
                if (m_useDefaults.get() != 0) {
                    // set() will trigger onValueChange(); must suppress that so it does not undo the change that triggered us
                    m_ignoreValueChange = true;
                    m_flakRating.set(m_defaultFlakRating);
                    m_flakCompensation.set(m_defaultFlakCompensation);
                    m_ignoreValueChange = false;
                }
            }

        /* Value change: when changing values to differ from defaults, deactivate "use defaults" */
        void onValueChange()
            {
                if (!m_ignoreValueChange && (m_flakRating.get() != m_defaultFlakRating || m_flakCompensation.get() != m_defaultFlakCompensation)) {
                    m_useDefaults.set(0);
                }
            }

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Observable<int32_t> m_useDefaults;
        afl::base::Observable<int32_t> m_flakRating;
        afl::base::Observable<int32_t> m_flakCompensation;

        ui::widgets::Checkbox m_useDefaultsCheckbox;
        ui::widgets::DecimalSelector m_flakRatingSelector;
        ui::widgets::DecimalSelector m_flakCompensationSelector;

        int32_t m_defaultFlakRating;
        int m_defaultFlakCompensation;

        bool m_ignoreValueChange;
    };
}

bool
client::dialogs::editSimulationFlakRatings(ui::Root& root, SimulationFlakRatings& values, afl::string::Translator& tx)
{
    // ex ccsim.pas:RatingEditor, editSimulatorFlakRatings
    Dialog dlg(root, values, tx);
    bool ok = dlg.run();
    if (ok) {
        dlg.writeBack(values);
    }
    return ok;
}
