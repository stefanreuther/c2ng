/**
  *  \file client/tiles/taskmessagetile.cpp
  *  \brief Class client::tiles::TaskMessageTile
  */

#include "client/tiles/taskmessagetile.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"


client::tiles::TaskMessageTile::TaskMessageTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx)
    : Group(ui::layout::HBox::instance5),
      m_deleter(),
      m_translator(tx),
      m_keyHandler(keyHandler),
      m_root(root),
      m_commandPart(ui::layout::VBox::instance5),
      m_statusPart(ui::layout::VBox::instance5),
      m_messagePart(ui::layout::VBox::instance5),
      m_cards(),
      m_messageView(gfx::Point(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 6)), 0, root.provider()),
      m_confirmButton(tx("M - Confirm"), 'm', root)
{
    // HBox
    //   CardGroup
    //     VBox (m_messagePart)
    //       DocumentView
    //       HBox [Spacer, Button]
    //     VBox (m_statusPart, populated by user)
    //   VBox (m_commandPart, populated by user/descendant)
    ui::Group& g = m_deleter.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(m_deleter.addNew(new ui::Spacer()));
    g.add(m_confirmButton);
    m_messagePart.add(m_messageView);
    m_messagePart.add(g);
    m_cards.add(m_messagePart);
    m_cards.add(m_statusPart);
    add(m_cards);
    add(m_commandPart);

    m_confirmButton.dispatchKeyTo(m_keyHandler);
}

client::tiles::TaskMessageTile::~TaskMessageTile()
{ }

ui::Group&
client::tiles::TaskMessageTile::commandPart()
{
    return m_commandPart;
}

ui::Group&
client::tiles::TaskMessageTile::statusPart()
{
    return m_statusPart;
}

void
client::tiles::TaskMessageTile::setMessageStatus(const game::proxy::TaskEditorProxy::MessageStatus& st)
{
    // ex WTaskMessageTile::drawTask, WTaskMessageTile::updateTask (sort-of)
    if (st.hasUnconfirmedMessage) {
        ui::rich::Document& doc = m_messageView.getDocument();
        doc.clear();
        doc.add(util::rich::Text(ui::SkinColor::Heading, m_translator("Notification")).withStyle(util::rich::StyleAttribute::Big));
        doc.addNewline();
        doc.add(st.text);
        doc.addNewline();
        doc.finish();

        m_messageView.handleDocumentUpdate();

        // Use setFocusedChild to just change the focus of the CardGroup, but do not focus the CardGroup itself.
        m_cards.setFocusedChild(&m_messagePart);
    } else {
        m_cards.setFocusedChild(&m_statusPart);
    }
}

void
client::tiles::TaskMessageTile::addCommandButton(util::Key_t key, String_t label)
{
    ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(label, key, m_root));
    btn.setFont(gfx::FontRequest());
    btn.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    btn.dispatchKeyTo(m_keyHandler);
    m_commandPart.add(btn);
}

ui::Root&
client::tiles::TaskMessageTile::root()
{
    return m_root;
}

afl::base::Deleter&
client::tiles::TaskMessageTile::deleter()
{
    return m_deleter;
}

afl::string::Translator&
client::tiles::TaskMessageTile::translator()
{
    return m_translator;
}
