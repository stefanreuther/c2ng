/**
  *  \file client/dialogs/visibilityrange.hpp
  *  \brief Visibility Range Editor
  */
#ifndef C2NG_CLIENT_DIALOGS_VISIBILITYRANGE_HPP
#define C2NG_CLIENT_DIALOGS_VISIBILITYRANGE_HPP

#include <memory>
#include "afl/string/translator.hpp"
#include "game/map/rangeset.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Edit visibility range and obtain a RangeSet.
        Allows the user to configure a VisibilityRangeProxy.

        @param root       UI root
        @param gameSender Game sender
        @param tx         Translator

        @return newly-allocated, non-empty RangeSet on success,
                null if user canceled or selected an impossible set */
    std::auto_ptr<game::map::RangeSet> editVisibilityRange(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
