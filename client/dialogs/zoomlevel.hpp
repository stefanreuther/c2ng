/**
  *  \file client/dialogs/zoomlevel.hpp
  *  \brief Zoom Level Input
  */
#ifndef C2NG_CLIENT_DIALOGS_ZOOMLEVEL_HPP
#define C2NG_CLIENT_DIALOGS_ZOOMLEVEL_HPP

#include "afl/string/translator.hpp"
#include "client/map/renderer.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Zoom level. */
    struct ZoomLevel {
        int mult;               ///< Multiplier.
        int divi;               ///< Divisor.
    };

    /** Zoom level input.
        @param [in]  renderer       Map renderer; used to provide initial value and validate input.
        @param [out] result         Result
        @param [in]  root           UI root
        @param [in]  tx             Translator
        @retval true Success, result has been updated
        @retval false Canceled, result has not been set */
    bool editZoomLevel(const client::map::Renderer& renderer, ZoomLevel& result, ui::Root& root, afl::string::Translator& tx);

} }

#endif
