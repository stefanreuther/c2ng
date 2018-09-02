/**
  *  \file client/dialogs/referencesortorder.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_REFERENCESORTORDER_HPP
#define C2NG_CLIENT_DIALOGS_REFERENCESORTORDER_HPP

#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "game/ref/configuration.hpp"

namespace client { namespace dialogs {

    bool doReferenceSortOrderMenu(game::ref::Configuration& order, gfx::Point anchor, ui::Root& root, afl::string::Translator& tx);

} }

#endif
