/**
  *  \file client/dialogs/export.hpp
  *  \brief Export Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_EXPORT_HPP
#define C2NG_CLIENT_DIALOGS_EXPORT_HPP

#include "game/proxy/exportadaptor.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Export dialog.
        Allows the user to choose fields to export and file format, and perform the export.
        @param root           UI root
        @param adaptorSender  Access to ExportAdaptor; provides field names and data to export
        @param gameSender     Access to game Session; for file handling etc.
        @param tx             Translator */
    void doExport(ui::Root& root,
                  util::RequestSender<game::proxy::ExportAdaptor> adaptorSender,
                  util::RequestSender<game::Session> gameSender,
                  afl::string::Translator& tx);

} }

#endif
