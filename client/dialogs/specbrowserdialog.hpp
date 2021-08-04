/**
  *  \file client/dialogs/specbrowserdialog.hpp
  *  \brief Specification Browser Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/spec/info/types.hpp"
#include "ui/rich/document.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Display specification browser dialog (Universe Almanac).
        \param root        UI root
        \param gameSender  Game sender
        \param tx          Translator */
    void doSpecificationBrowserDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

    /** Render hull information.
        \param [out] doc        Document to render into
        \param [in]  root       UI root (for color scheme, font)
        \param [in]  content    PageContent to render
        \param [in]  tx         Translator */
    void renderHullInformation(ui::rich::Document& doc, ui::Root& root, const game::spec::info::PageContent& content, afl::string::Translator& tx);

    /** Render list of abilities.
        \param [out] doc        Document to render into
        \param [in]  root       UI root (for color scheme, font)
        \param [in]  abilities  Ability list to render
        \param [in]  useIcons   true to prefer icons, false to enforce textual rendering
        \param [in]  maxLines   Maximum number of lines to render
        \param [in]  tx         Translator */
    void renderAbilityList(ui::rich::Document& doc, ui::Root& root, const game::spec::info::Abilities_t& abilities, bool useIcons, size_t maxLines, afl::string::Translator& tx);

} }

#endif
