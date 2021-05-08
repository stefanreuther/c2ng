/**
  *  \file game/v3/attachmentconfiguration.hpp
  *  \brief Attachment Configuration
  */
#ifndef C2NG_GAME_V3_ATTACHMENTCONFIGURATION_HPP
#define C2NG_GAME_V3_ATTACHMENTCONFIGURATION_HPP

#include "game/config/userconfiguration.hpp"

namespace game { namespace v3 {

    class AttachmentUnpacker;

    /** Check for new attachments.
        Call after loading all attachments into an AttachmentUnpacker.

        \param config User preferences
        \param unpacker Unpacker

        \retval true  AttachmentUnpacker has been configured; call unpacker.saveFiles() next
        \retval false Offer user selection of attachments, then possibly call unpacker.saveFiles()

        Note: after unpacker.saveFiles(), call markAttachmentsProcessed(). */
    bool checkNewAttachments(const game::config::UserConfiguration& config, AttachmentUnpacker& unpacker);

    /** Mark attachments processed.
        Call after unpacker.saveFiles().
        Do NOT call dropUnselectedAttachments() before calling this function.

        This updates the user configuration object to remember that these attachments were processed
        (regardless whether user chose to save or skip them).

        \param config User preferences
        \param unpacker Unpacker */
    void markAttachmentsProcessed(game::config::UserConfiguration& config, const AttachmentUnpacker& unpacker);

} }

#endif
