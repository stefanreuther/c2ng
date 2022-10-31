/**
  *  \file game/msg/mailbox.hpp
  *  \brief Class game::msg::Mailbox
  */
#ifndef C2NG_GAME_MSG_MAILBOX_HPP
#define C2NG_GAME_MSG_MAILBOX_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/parser/informationconsumer.hpp"
#include "game/playerlist.hpp"
#include "game/reference.hpp"
#include "game/teamsettings.hpp"
#include "util/rich/text.hpp"

namespace game { namespace msg {

    /** Mailbox.

        This interface shall eventually be able to provide multiple mailboxes which model things like PMs received on a website.
        It offers a rich set of function calls, although not all are available all the time.

        Messages are addressed using a 0-based index; a message is not an object itself.

        Conceptually, messages consist of metadata and text, whereby the text is composed of
        - header
        - body
        - extra information

        Functions exist to modify individual parts. */
    class Mailbox : public afl::base::Deletable {
     public:
        enum Flag {
            Confirmed
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        enum Action {
            ToggleConfirmed
        };
        typedef afl::bits::SmallSet<Action> Actions_t;

        enum DataStatus {
            NoData,             ///< No data (UnpackUnspecial).
            DataReceivable,     ///< Data can be received (ms_Receivable).
            DataReceived,       ///< Data successfully received (ms_Received, UnpackSuccess).
            DataExpired,        ///< Data is expired (ms_Expired).
            DataWrongPasscode,  ///< Wrong passcode (ms_BadCode).
            DataWrongChecksum,  ///< Wrong checksum (ms_BadCRC, UnpackChecksumError).
            DataFailed          ///< Data not decodable (UnpackFailed).
        };

        /** Message metadata.
            See getMessageMetadata(). */
        struct Metadata {
            int turnNumber;           ///< Message turn number. 0 if message not associated with a specific turn (default).
            Reference primaryLink;    ///< Primary link. Typically, an associated object.
            Reference secondaryLink;  ///< Secondary link. Typically, a starchart coordinate.
            PlayerSet_t reply;        ///< Set of players for "reply". Empty if message cannot be replied to.
            PlayerSet_t replyAll;     ///< Set of players for "reply all". Empty if message cannot be replied to.
            DataStatus dataStatus;    ///< Data status.
            Flags_t flags;            ///< Flags.

            Metadata()
                : turnNumber(), primaryLink(), secondaryLink(), reply(), replyAll(), dataStatus(NoData), flags()
                { }
        };

        /** Get number of messages in this mailbox.
            \return Number of messages */
        virtual size_t getNumMessages() const = 0;

        /** Get complete text of a message (header+body).
            This is a convenience function that returns the concatenation of getMessageHeaderText(), getMessageBodyText().
            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message text */
        String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;

        /** Get message header text.
            This is the message part that is added automatically (e.g. FROM/TO for outgoing messages, header for notifications).
            The message header text can be empty (e.g. for incoming messages).
            If non-empty, it should end in one or two linefeeds to separate it from the body.

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message header text */
        virtual String_t getMessageHeaderText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get message body text.
            This is the part of the message perceived by the user as "the message",
            i.e. the editable part of an outgoing message, the notification message, or the entire text of an incoming message.

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message body text */
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get message cited for forwarding.
            Returns the message text in a format for composing a "forward" message.
            For VGAP-style messages, call defaultGetMessageForwardText().

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message text */
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get message cited for forwarding.
            Returns the message text in a format for composing a "reply" message.
            For VGAP-style messages, call defaultGetMessageReplyText().

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message text */
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get message text formatted for display.
            Returns the message in rich-text format for display.
            In particular, it can apply highlighting and links.

            In addition to header and body, this function can add extra information, e.g. data transfer status or notification status.

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return formatted message text */
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get heading of a message.
            The heading is used for subject-sorting and the kill filter.

            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message heading */
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get message metadata.
            \param index message number, [0, getCount()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return metadata */
        virtual Metadata getMessageMetadata(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get permitted message actions.
            \param index message number, [0, getNumMessages()).
            \return set of actions */
        virtual Actions_t getMessageActions(size_t index) const = 0;

        /** Perform action on a message.
            \param index message number, [0, getNumMessages()).
            \param a     action, taken from getMessageActions() */
        virtual void performMessageAction(size_t index, Action a) = 0;

        /** Receive data attachment (binary transfer).
            This operation is available if Metadata::dataStatus is DataReceivable or DataReceived.
            If supported, it should call defaultReceiveMessageData().
            This function supports two usecases:
            - initial parsing during result file load (onRequest=false, honors configuration)
            - explicit parsing of a message on user request (onRequest=true, always receives configuration)

            \param index        message number, [0, getNumMessages()).
            \param consumer     information consumer
            \param teamSettings team settings, contains user configuration for messages that can be automatically received
            \param onRequest    true when parsing on explicit request; false to apply user configuration
            \param cs           game character set*/
        virtual void receiveMessageData(size_t index, game::parser::InformationConsumer& consumer, const TeamSettings& teamSettings, bool onRequest, afl::charset::Charset& cs) = 0;

     protected:
        /** Default implementation for receiveMessageData().
            \param text         message text
            \param turnNumber   message turn number, adjusted for unpackBinaryMessage()
            \param consumer     information consumer
            \param teamSettings team settings, contains user configuration for messages that can be automatically received
            \param onRequest    true when parsing on explicit request; false to apply user configuration
            \param cs           game character set
            \return new value for Metadata::dataStatus for this message */
        static DataStatus defaultReceiveMessageData(const String_t& text,
                                                    int turnNumber,
                                                    game::parser::InformationConsumer& consumer,
                                                    const TeamSettings& teamSettings,
                                                    bool onRequest,
                                                    afl::charset::Charset& cs);

        /** Default implementation for getMessageForwardText().
            Returns the getMessageText(), wrapped in "forward" markers.
            \param index   message number, [0, getNumMessages()).
            \param tx      Translator for internationalizable parts
            \param players Player list for player names
            \return text (multi-line string) */
        String_t defaultGetMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;

        /** Default implementation for getMessageReplyText().
            Returns the getMessageText(), with VGAP-style headers removed, prefixed with ">" markers.
            \param index   message number, [0, getNumMessages()).
            \param tx      Translator for internationalizable parts
            \param players Player list for player names
            \return text (multi-line string) */
        String_t defaultGetMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;

        /** Default implementation for getMessageDisplayText().
            Returns the provided text, converted to rich text, with coordinates transformed to links,
            and an appropriate notice when the given DataStatus indicates presence of a data attachment.
            \param text    Text to format (multi-line text); should be getMessageText()
            \param status  Data status; pass NoData for no data notice
            \param tx      Translator for internationalizable parts
            \param players Player list for player names
            \return text (multi-line rich-text) */
        static util::rich::Text defaultGetMessageDisplayText(const String_t& text, DataStatus status, afl::string::Translator& tx, const PlayerList& players);

        /** Add a status notification.
            Utility function to implement status notices.
            \param [in,out]  result  Text
            \param [in]      icon    Icon (an Unicode characters such as UTF_CHECK_MARK)
            \param [in]      color   Color to use for icon
            \param [in]      text    Additional text */
        static void addStatus(util::rich::Text& result, const char*const icon, util::SkinColor::Color color, String_t text);
    };

} }

#endif
