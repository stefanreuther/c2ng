/**
  *  \file game/v3/attachmentconfiguration.cpp
  *  \brief Attachment Configuration
  */

#include "game/v3/attachmentconfiguration.hpp"
#include "game/v3/attachmentunpacker.hpp"

using game::config::UserConfiguration;

bool
game::v3::checkNewAttachments(const game::config::UserConfiguration& config, AttachmentUnpacker& unpacker)
{
    // Do we have any attachments? (Should not call getTimestamp() without attachments.)
    // If we don't proceed with saving (which will be a no-op).
    if (unpacker.getNumAttachments() == 0) {
        return true;
    }

    // Check timestamp
    // If we already saw these attachments, deselect all and proceed with saving (which will be a no-op).
    if (unpacker.getTimestamp().getTimestampAsString() == config[UserConfiguration::Unpack_AttachmentTimestamp]()) {
        unpacker.selectAllAttachments(false);
        return true;
    }

    // Check attachment types
    AttachmentUnpacker::Kinds_t kinds = unpacker.getAllAttachmentKinds();

    // Reject CriticalFiles
    kinds -= AttachmentUnpacker::CriticalFile;
    unpacker.selectAttachmentsByKind(AttachmentUnpacker::CriticalFile, false);

    // Deal with race names
    int raceNames = config[UserConfiguration::Unpack_AcceptRaceNames]();
    if (raceNames != UserConfiguration::Unpack_Ask) {
        kinds -= AttachmentUnpacker::RaceNameFile;
        unpacker.selectAttachmentsByKind(AttachmentUnpacker::RaceNameFile, (raceNames == UserConfiguration::Unpack_Accept));
    }

    // If any attachments remain, let the user decide.
    // If no undecided attachment remains, proceed with saving.
    return kinds.empty();
}

void
game::v3::markAttachmentsProcessed(game::config::UserConfiguration& config, const AttachmentUnpacker& unpacker)
{
    if (unpacker.getNumAttachments() != 0) {
        game::config::StringOption& opt = config[UserConfiguration::Unpack_AttachmentTimestamp];
        opt.set(unpacker.getTimestamp().getTimestampAsString());
        opt.setSource(game::config::ConfigurationOption::Game);
    }
}
