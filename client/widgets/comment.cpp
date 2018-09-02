/**
  *  \file client/widgets/comment.cpp
  */

#include "client/widgets/comment.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "ui/layout/hbox.hpp"
#include "util/translation.hpp"

client::widgets::Comment::Comment(ui::Root& root, KeymapWidget& kmw)
    : Group(ui::layout::HBox::instance0),
      m_button("F9", util::Key_F9, root),
      m_text(String_t(), util::SkinColor::Static, gfx::FontRequest(), root.provider(), 0)
{
    // ex WCommentTile::WCommentTile
    // FIXME: leave a 5-pixel gap for the text?
    m_text.setIsFlexible(true);
    add(m_text);
    add(m_button);
    m_button.dispatchKeyTo(kmw);
}

client::widgets::Comment::~Comment()
{ }

void
client::widgets::Comment::setComment(String_t comment)
{
    // ex WCommentTile::drawData (sort-of)
    if (comment.empty()) {
        m_text.setText(_("(press [F9] to add a note)"));
        m_text.setColor(util::SkinColor::Static);
    } else {
        m_text.setText(comment);
        m_text.setColor(util::SkinColor::Yellow);
    }
}
