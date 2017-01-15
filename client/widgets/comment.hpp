/**
  *  \file client/widgets/comment.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COMMENT_HPP
#define C2NG_CLIENT_WIDGETS_COMMENT_HPP

#include "ui/group.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/statictext.hpp"

namespace client { namespace widgets {

    class KeymapWidget;

    class Comment : public ui::Group {
     public:
        Comment(ui::Root& root, KeymapWidget& kmw);
        ~Comment();

        void setComment(String_t comment);

     private:
        ui::widgets::Button m_button;
        ui::widgets::StaticText m_text;
    };

} }

#endif
