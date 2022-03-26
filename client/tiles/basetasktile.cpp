/**
  *  \file client/tiles/basetasktile.cpp
  *  \brief Class client::tiles::BaseTaskTile
  */

#include "client/tiles/basetasktile.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/unicodechars.hpp"

using util::rich::Text;
using util::SkinColor;

client::tiles::BaseTaskTile::BaseTaskTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx)
    : TaskMessageTile(root, keyHandler, tx),
      m_statusView(gfx::Point(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 7)), 0, root.provider()),
      m_editButton("E", 'e', root)
{
    // ex WBaseAutoTaskCommandTile::init (sort-of)
    statusPart().add(m_statusView);

    m_editButton.dispatchKeyTo(keyHandler);
    addCommandButton('1', tx("1 - Orders"));
    addCommandButton('2', tx("2 - Cargo"));
    addCommandButton('3', tx("3 - Misc."));
    commandPart().add(deleter().addNew(new ui::Spacer()));

    ui::Group& g = deleter().addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(deleter().addNew(new ui::widgets::StaticText(tx("Edit"), SkinColor::Static, gfx::FontRequest(), root.provider(), gfx::RightAlign))
          .setIsFlexible(true));
    g.add(m_editButton);
    commandPart().add(g);

    setBaseStatus(game::proxy::TaskEditorProxy::BaseStatus());
}

client::tiles::BaseTaskTile::~BaseTaskTile()
{ }

void
client::tiles::BaseTaskTile::setBaseStatus(const game::proxy::TaskEditorProxy::BaseStatus& st)
{
    // ex WBaseAutoTaskCommandTile::drawData
    ui::rich::Document& doc = m_statusView.getDocument();
    doc.clear();
    if (!st.buildOrder.empty()) {
        const String_t pfx = UTF_BULLET " ";
        afl::string::Translator& tx = translator();

        // Render build order
        doc.add(Text(SkinColor::White, tx("Ship Build Order:")));
        doc.addNewline();
        for (size_t i = 0; i < st.buildOrder.size(); ++i) {
            doc.add(pfx + st.buildOrder[i]);
            doc.addNewline();
        }

        // Render missing minerals
        if (!st.missingMinerals.empty()) {
            doc.add(Text(SkinColor::Yellow, tx("Additional resources needed:")));
            doc.addNewline();
            doc.add(Text(SkinColor::Yellow, pfx + st.missingMinerals));
        } else {
            doc.add(tx("Sufficient resources available"));
        }
        doc.addNewline();
    }
    doc.finish();
    m_statusView.handleDocumentUpdate();

    m_editButton.setState(DisabledState, st.buildOrder.empty());
}
