/**
  *  \file client/tiles/shiptasktile.cpp
  *  \brief Class client::tiles::ShipTaskTile
  */

#include "client/tiles/shiptasktile.hpp"
#include "util/translation.hpp"
#include "afl/functional/stringtable.hpp"
#include "afl/string/format.hpp"
#include "ui/spacer.hpp"

namespace {
    const char*const HEADINGS[] = {
        N_("Fuel aboard:"),
        N_("- Movement:"),
        N_("- Cloaking:"),
        N_("= Remaining:"),
    };
}

client::tiles::ShipTaskTile::ShipTaskTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx)
    : TaskMessageTile(root, keyHandler, tx),
      m_statusView(gfx::Point(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 7)), 0, root.provider())
{
    statusPart().add(m_statusView);

    addCommandButton('1', tx("1 - Movement"));
    addCommandButton('2', tx("2 - Cargo"));
    addCommandButton('3', tx("3 - Mission"));
    addCommandButton('4', tx("4 - Misc."));
    commandPart().add(deleter().addNew(new ui::Spacer(gfx::Point())));
    addCommandButton('o', tx("O - Options"));
    commandPart().add(deleter().addNew(new ui::Spacer()));
}

client::tiles::ShipTaskTile::~ShipTaskTile()
{ }

void
client::tiles::ShipTaskTile::setShipStatus(const game::proxy::TaskEditorProxy::ShipStatus& st)
{
    // ex WShipAutoTaskCommandTile::drawData
    ui::rich::Document& doc = m_statusView.getDocument();
    doc.clear();

    // Compute layout
    const afl::base::Ref<gfx::Font> font = root().provider().getFont(gfx::FontRequest());
    const afl::functional::StringTable_t& rawTab = afl::functional::createStringTable(HEADINGS);
    const afl::functional::StringTable_t& tab = rawTab.map(translator());
    const int headingWidth = font->getMaxTextWidth(tab);
    const int numberWidth = 6*font->getEmWidth();
    const int totalWidth = headingWidth + numberWidth;
    afl::string::Translator& tx = translator();

    const util::NumberFormatter& fmt = st.numberFormatter;
    if (st.valid) {
        // Regular prediction
        doc.add(tab.get(0));
        doc.addRight(totalWidth, util::rich::Text(util::SkinColor::Green, fmt.formatNumber(st.startingFuel) + tx(" kt")));
        doc.addNewline();

        doc.add(tab.get(1));
        doc.addRight(totalWidth, util::rich::Text(util::SkinColor::Green, fmt.formatNumber(st.movementFuel) + tx(" kt")));
        doc.addNewline();

        doc.add(tab.get(2));
        doc.addRight(totalWidth, util::rich::Text(util::SkinColor::Green, fmt.formatNumber(st.cloakFuel) + tx(" kt")));
        doc.addNewline();

        doc.add(tab.get(3));
        doc.addRight(totalWidth, util::rich::Text(st.numFuelTurns < st.numTurns ? util::SkinColor::Red : util::SkinColor::Green,
                                                  fmt.formatNumber(st.remainingFuel) + tx(" kt")));
        doc.addParagraph();

        int now = st.currentTurn;
        int age;
        if (st.numFuelTurns < st.numTurns) {
            /* numFuelTurns() is the number of turns that have fuel.
               Since out-of-fuel happens during the next host run, we
               want it to say "out of fuel next turn" if there is not
               enough fuel for one step. Therefore, we need to add 1.
               This is the same as in PCC 1.x. */
            age = st.numFuelTurns + 1;
            doc.add(util::rich::Text(util::SkinColor::Red, afl::string::Format(tx("Out of fuel in turn %d"), age + now)));
        } else {
            age = st.numFuelTurns;
            doc.add(util::rich::Text(util::SkinColor::Green, afl::string::Format(tx("Prediction ends turn %d"), age + now)));
        }
        doc.addNewline();
        if (age == 0) {
            doc.add(tx("(this turn)"));
        } else if (age == 1) {
            doc.add(tx("(next turn)"));
        } else {
            doc.add(afl::string::Format(tx("(%d turns from now)"), age));
        }
    } else {
        // leave blank
    }
    doc.finish();
    m_statusView.handleDocumentUpdate();
}
