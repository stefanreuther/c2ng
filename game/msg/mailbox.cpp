/**
  *  \file game/msg/mailbox.cpp
  *  \brief Class game::msg::Mailbox
  */

#include "game/msg/mailbox.hpp"
#include "game/msg/format.hpp"
#include "game/parser/binarytransfer.hpp"
#include "game/parser/messagetemplate.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

namespace {
    bool acceptMessage(game::parser::MessageType type, const game::TeamSettings::MessageTypes_t configuredTypes)
    {
        switch (type) {
         case game::parser::NoMessage:
            return false;
         case game::parser::MinefieldMessage:
            return configuredTypes.contains(game::TeamSettings::MinefieldInformation);
         case game::parser::PlanetMessage:
            return configuredTypes.contains(game::TeamSettings::PlanetInformation);
         case game::parser::DrawingMessage:
            return configuredTypes.contains(game::TeamSettings::DrawingInformation);
         case game::parser::StatisticMessage:
            return false;
        }
        return false;
    }
}

String_t
game::msg::Mailbox::getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return getMessageHeaderText(index, tx, players)
        + getMessageBodyText(index, tx, players);
}

game::msg::Mailbox::DataStatus
game::msg::Mailbox::defaultReceiveMessageData(const String_t& text,
                                              int turnNumber,
                                              game::parser::InformationConsumer& consumer,
                                              const TeamSettings& teamSettings,
                                              bool onRequest,
                                              afl::charset::Charset& cs)
{
    game::parser::MessageLines_t lines;
    game::parser::splitMessage(lines, text);
    afl::container::PtrVector<game::parser::MessageInformation> info;
    game::parser::UnpackResultPair_t r = game::parser::unpackBinaryMessage(lines, turnNumber, info, cs);

    // Determine message sender
    int messageSender = 0;
    if (getMessageHeaderInformation(lines, game::parser::MsgHdrKind) == 'r') {
        if (!util::parsePlayerCharacter(char(getMessageHeaderInformation(lines, game::parser::MsgHdrSubId)), messageSender)) {
            messageSender = 0;
        }
    }

    // Evaluate. For now, pretend that onRequest=false means receive nothing.
    if ((onRequest || acceptMessage(r.second, teamSettings.getReceiveConfiguration(messageSender))) && r.first == game::parser::UnpackSuccess) {
        consumer.addMessageInformation(info);
        return DataReceived;
    } else {
        switch (r.first) {
         case game::parser::UnpackSuccess:       return DataReceivable;
         case game::parser::UnpackUnspecial:     return NoData;
         case game::parser::UnpackFailed:        return DataFailed;
         case game::parser::UnpackChecksumError: return DataWrongChecksum;
        }
        return DataFailed;
    }
}

String_t
game::msg::Mailbox::defaultGetMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return "--- Forwarded Message ---\n"
        + getMessageText(index, tx, players)
        + "\n--- End Forwarded Message ---";
}

String_t
game::msg::Mailbox::defaultGetMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return quoteMessageForReply(getMessageText(index, tx, players));
}

util::rich::Text
game::msg::Mailbox::defaultGetMessageDisplayText(const String_t& text, DataStatus status, afl::string::Translator& tx, const PlayerList& players)
{
    util::rich::Text result = formatMessage(text, players, tx).text.withStyle(util::rich::StyleAttribute::Fixed);

    switch (status) {
     case NoData:
        break;
     case DataReceivable:
        addStatus(result, UTF_RIGHT_POINTER, util::SkinColor::Green, tx("Data can be received"));
        break;
     case DataReceived:
        addStatus(result, UTF_CHECK_MARK, util::SkinColor::Green, tx("Data has been received"));
        break;
     case DataExpired:
        addStatus(result, UTF_BALLOT_CROSS, util::SkinColor::Yellow, tx("Data is expired"));
        break;
     case DataWrongPasscode:
        addStatus(result, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Wrong passcode"));
        break;
     case DataWrongChecksum:
        addStatus(result, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Checksum error"));
        break;
     case DataFailed:
        addStatus(result, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Data error"));
        break;
    }

    return result;
}

void
game::msg::Mailbox::addStatus(util::rich::Text& result, const char*const icon, util::SkinColor::Color color, String_t text)
{
    result += "\n\n";
    result += util::rich::Text(icon).withColor(color);
    result += " ";
    result += text;
}
