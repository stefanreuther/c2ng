/**
  *  \file client/dialogs/newdrawing.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_NEWDRAWING_HPP
#define C2NG_CLIENT_DIALOGS_NEWDRAWING_HPP

#include "afl/string/translator.hpp"
#include "game/map/drawing.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    struct NewDrawingInfo {
        game::map::Drawing::Type type;
        uint8_t color;
        String_t tagName;

        NewDrawingInfo()
            : type(),
              color(9),
              tagName()
            { }
    };

    bool chooseNewDrawingParameters(NewDrawingInfo& result, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
