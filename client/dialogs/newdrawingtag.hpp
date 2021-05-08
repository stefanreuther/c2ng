/**
  *  \file client/dialogs/newdrawingtag.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_NEWDRAWINGTAG_HPP
#define C2NG_CLIENT_DIALOGS_NEWDRAWINGTAG_HPP

#include "ui/widgets/stringlistbox.hpp"
#include "ui/widgets/inputline.hpp"
#include "util/atomtable.hpp"
#include "ui/root.hpp"
#include "afl/string/translator.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    class NewDrawingTag {
     public:
        NewDrawingTag(util::StringList& tagList, ui::Root& root, util::RequestSender<game::Session> gameSender);
        ~NewDrawingTag();

        void setTag(util::Atom_t atom);
        void setTagName(String_t atomName);

        String_t getTagName() const;

        bool run(String_t title, afl::string::Translator& tx, bool* pAdjacent);

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        ui::widgets::InputLine m_input;
        ui::widgets::StringListbox m_list;
        size_t m_lastPosition;

        void onMove();
        void onEdit();
    };

} }

#endif
