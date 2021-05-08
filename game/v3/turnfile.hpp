/**
  *  \file game/v3/turnfile.hpp
  *  \brief Class game::v3::TurnFile
  */
#ifndef C2NG_GAME_V3_TURNFILE_HPP
#define C2NG_GAME_V3_TURNFILE_HPP

#include "afl/base/growablememory.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "game/timestamp.hpp"
#include "game/v3/messagewriter.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace v3 {

    class RegistrationKey;

    /** TRN Command codes.
        The names are the same as in UN-TRN, the file format list, and some utilities inspired by the above.
        Those marked "<-" have been renamed, which does not mean the list would be consistent now.*/
    // FIXME: can we define these command codes in a better way than an enum?
    enum {
        /* Ship commands */
        tcm_ShipFIRST              = 1,
        tcm_ShipChangeFc           = 1,        // sid, 3 bytes FC
        tcm_ShipChangeSpeed        = 2,        // sid, 1 word
        tcm_ShipChangeWaypoint     = 3,        // sid, 2 words
        tcm_ShipChangeMission      = 4,        // sid, 1 word
        tcm_ShipChangePrimaryEnemy = 5,        // sid, 1 word
        tcm_ShipTowShip            = 6,        // sid, 1 word
        tcm_ShipChangeName         = 7,        // sid, 20 bytes
        tcm_ShipBeamDownCargo      = 8,        // sid, 7 words NTDMCS+id
        tcm_ShipTransferCargo      = 9,        // sid, 7 words NTDMCS+id
        tcm_ShipIntercept          = 10,       // sid, 1 word
        tcm_ShipChangeNeutronium   = 11,       // sid, 1 word
        tcm_ShipChangeTritanium    = 12,       // sid, 1 word
        tcm_ShipChangeDuranium     = 13,       // sid, 1 word
        tcm_ShipChangeMolybdenum   = 14,       // sid, 1 word
        tcm_ShipChangeSupplies     = 15,       // sid, 1 word
        tcm_ShipChangeColonists    = 16,       // sid, 1 word
        tcm_ShipChangeTorpedoes    = 17,       // sid, 1 word
        tcm_ShipChangeMoney        = 18,       // sid, 1 word
        tcm_ShipLAST               = 18,

        /* Planet commands */
        tcm_PlanetFIRST            = 21,
        tcm_PlanetChangeFc         = 21,       // pid, 3 bytes
        tcm_PlanetChangeMines      = 22,       // pid, 1 word <-
        tcm_PlanetChangeFactories  = 23,       // pid, 1 word
        tcm_PlanetChangeDefense    = 24,       // pid, 1 word
        tcm_PlanetChangeNeutronium = 25,       // pid, 1 dword
        tcm_PlanetChangeTritanium  = 26,       // pid, 1 dword
        tcm_PlanetChangeDuranium   = 27,       // pid, 1 dword
        tcm_PlanetChangeMolybdenum = 28,       // pid, 1 dword
        tcm_PlanetChangeColonists  = 29,       // pid, 1 dword
        tcm_PlanetChangeSupplies   = 30,       // pid, 1 dword
        tcm_PlanetChangeMoney      = 31,       // pid, 1 dword
        tcm_PlanetColonistTax      = 32,       // pid, 1 word
        tcm_PlanetNativeTax        = 33,       // pid, 1 word
        tcm_PlanetLAST             = 33,       // BuildBase is special
        tcm_PlanetBuildBase        = 34,       // pid, NO DATA

        /* Starbase commands */
        tcm_BaseFIRST              = 40,
        tcm_BaseChangeDefense      = 40,       // bid, 1 word
        tcm_BaseUpgradeEngineTech  = 41,       // bid, 1 word
        tcm_BaseUpgradeHullTech    = 42,       // bid, 1 word <-
        tcm_BaseUpgradeWeaponTech  = 43,       // bid, 1 word <-
        tcm_BaseBuildEngines       = 44,       // bid, 9 words
        tcm_BaseBuildHulls         = 45,       // bid, 20 words
        tcm_BaseBuildWeapons       = 46,       // bid, 10 words
        tcm_BaseBuildLaunchers     = 47,       // bid, 10 words
        tcm_BaseBuildTorpedoes     = 48,       // bid, 10 words
        tcm_BaseBuildFighters      = 49,       // bid, 1 word
        tcm_BaseFixRecycleShipId   = 50,       // bid, 1 word <-
        tcm_BaseFixRecycleShip     = 51,       // bid, 1 word action
        tcm_BaseChangeMission      = 52,       // bid, 1 word
        tcm_BaseBuildShip          = 53,       // bid, 7 words
        tcm_BaseUpgradeTorpTech    = 54,       // bid, 1 word
        tcm_BaseLAST               = 54,

        /* Rest */
        tcm_SendMessage            = 60,       // len, from, to, text
        tcm_ChangePassword         = 61,       // zero, 10 bytes
        tcm_SendBack               = 62        // recv, type, size, data
    };

    /** Turn file.

        This class encapsulates all the logic needed to read/write a turn file.

        The basic idea is to build the turn file in a memory buffer while maintaining some headers in ready-to-use form for easy access.
        For performance reasons, the data buffer does not always contain a valid turn, instead most manipulators blindly append.
        update() can be used to convert this big mess into a turn file, which can then be written out.
        In particular, loading a turn file using the constructor and immediately writing it out again will result in a 1:1 copy;
        calling update() inbetween will convert the file to the "canonical" format.

        Class TurnFile does not impose any limits on the order in which TRN commands are stored,
        but it generates them always tightly packed in the same order as in the pointer table.
        TurnFile automatically deletes invalid commands when writing out the turn.
        (Actually, deleting a command is implemented as zeroing it out and having update() delete it.)

        A TurnFile instance has an associated character set which is used to encode/decode strings in turn data structures.
        Users can refer to this character set for encoding/decoding command content.

        Each turn command consists of three components:
        - a 16-bit command code (tcm_XXX); see getCommandCode()
        - a 16-bit Id field (object Id for most commands, but can have different meaning for some); see getCommandId()
        - a (possibly empty) data field; see getCommandLength(), getCommandData()

        When making a new turn from scratch, use the following order:
        - construct using player, charset, timestamp
        - first set the format (setFeatures, setVersion)
        - then set the registration key (setRegistrationKey)
        - addCommand() can be called anywhere inbetween
        - rebuild the turn (update)
        - write it out.

        \invariant !getFeatures().contains(TaccomFeature) <=> m_taccomHeader is zeroed
        \invariant !getFeatures().contains(WinplanFeature) <=> m_windowsTrailer is zeroed */
    class TurnFile : public MessageWriter {
     public:
        typedef uint32_t CommandCode_t;

        /** Feature Flags. */
        enum Feature {
            WinplanFeature,       ///< File contains Winplan trailer.
            TaccomFeature         ///< File contains Taccom-style attachments.
        };
        typedef afl::bits::SmallSet<Feature> FeatureSet_t;

        /** Command Types. */
        enum CommandType {
            UndefinedCommand,       ///< Command not known to us.
            ShipCommand,            ///< Ship command. First word is Id.
            PlanetCommand,          ///< Planet command. First word is Id.
            BaseCommand,            ///< Base command. First word is Id.
            OtherCommand            ///< Other command. Password, Message, Sendback.
        };

        /*
         *  Constructor and Destructor
         */


        /** Create new turn file.
            Makes a new, empty file in memory.
            \param charset character set. Lifetime must exceed that of TurnFile.
            \param player owning player
            \param time timestamp */
        TurnFile(afl::charset::Charset& charset, int player, Timestamp time);

        /** Read turn file.
            Construct a TurnFile from parsing a file.
            \param charset character set. Lifetime must exceed that of TurnFile.
            \param tx Translator (for error messages during parse)
            \param stream Stream
            \param fullParse true to read full turn. false to read only the turn header (this will remove all attachments and commands)
            \pre File pointer for \c str points to beginning of TRN
            \post fullParse => !dirty
            \throws FileFormatException on error. */
        TurnFile(afl::charset::Charset& charset, afl::string::Translator& tx, afl::io::Stream& stream, bool fullParse = true);

        /** Destructor. */
        ~TurnFile();


        /*
         *  Header accessors
         */

        /** Get player number.
            \return player number */
        int getPlayer() const;

        /** Get turn timestamp.
            \return timestamp */
        Timestamp getTimestamp() const;

        /** Get number of commands stored in this turn.
            This is not necessary the number of commands the turn will have when written to disk,
            since there might be some deleted or invalid ones.
            Call update() before to get an exact count.
            \return number of commands */
        size_t getNumCommands() const;

        /** Get feature flags.
            \return feature flags */
        FeatureSet_t getFeatures() const;

        /** Get sub-version of turn file.
            Only valid for Winplan turns (getFeatures().contains(WinplanFeature)).
            The sub-version is the "xy" in "file format 3.5xy" (0..99; currently either 0 or 1).
            \return sub-version */
        int getVersion() const;

        /** Set turn timestamp.
            \param time New timestamp */
        void setTimestamp(const Timestamp& time);

        /** Set sub-version of turn file.
            Only valid for Winplan turns (getFeatures().contains(WinplanFeature)).
            The sub-version is the "xy" in "file format 3.5xy" (0..99; currently either 0 or 1).
            \param n sub-version */
        void setVersion(int n);

        /** Set turn format.
            \param f new feature flags */
        void setFeatures(FeatureSet_t f);


        /*
         *  Trailer access
         */

        /** Try to get the turn number used to generate this turn.
            This is a guess only, and is only intended to be shown in un-trn listings (or, to generate a copy of this turn).
            \return turn number, or 0 if not known */
        int tryGetTurnNr() const;

        /** Set player secret (templock, playerlog).
            A set of turn files for the same game and turn created by the same computer must bear the same player secret.

            This call updates the DOS trailer; it does not automatically update the turn,
            and does not implicitly mark it dirty.

            \param data Data */
        void setPlayerSecret(const structures::TurnPlayerSecret& data);

        /** Set registration info.
            The turn number must be passed in as well, because Host uses it to validate the registration info for Winplan clients.
            When copying the turns, this may be the turn number from tryGetTurnNr().
            \param key Registration key
            \param turnNr Turn number */
        void setRegistrationKey(const RegistrationKey& key, int turnNr);


        /*
         *  Header structure accessors
         */

        /** Get Windows (v3.5) trailer.
            This function allows access to the raw trailer.
            It is only intended to be used by low-level programs such as un-trn.
            The Windows trailer is only present if the WinplanFeature is active.
            \return trailer */
        const structures::TurnWindowsTrailer& getWindowsTrailer() const;

        /** Get DOS (v3.0) trailer.
            This function allows access to the raw trailer.
            It is only intended to be used by low-level programs such as un-trn.
            The DOS trailer is always present.
            \return trailer */
        const structures::TurnDosTrailer& getDosTrailer() const;

        /** Get turn header.
            This function allows access to the raw header.
            It is only intended to be used by low-level programs such as un-trn.
            The header is always present.
            \return header */
        const structures::TurnHeader& getTurnHeader() const;

        /** Get Taccom header.
            This function allows access to the Taccom header.
            It is only intended to be used by low-level programs such as un-trn.
            The Taccom header is only present if the TaccomFeature is active.
            \return header */
        const structures::TaccomTurnHeader& getTaccomHeader() const;

        /*
         *  Command accessors
         */

        /** Get command code.
            \param index [in] Command index, [0,getNumCommands())
            \param out   [out] Command code; one of the tcm_XXX values for valid turns
            \retval true Valid request, \c out was updated
            \retval false Invalid request (index out of range, turn file invalid), \c out unchanged */
        bool getCommandCode(size_t index, CommandCode_t& out) const;

        /** Get length of command data field.
            \param index [in] Command index, [0,getNumCommands())
            \param out   [out] Command length
            \retval true Valid request, \c out was updated
            \retval false Invalid request (index out of range, turn file invalid), \c out unchanged */
        bool getCommandLength(size_t index, int& out) const;

        /** Get command Id field.
            The Id field contains
            - the object Id for most commands
            - the length of the message for tcm_SendMessage
            - the receiving player for tcm_SendBack
            - zero for tcm_ChangePassword
            \param index [in] Command index, [0,getNumCommands())
            \param out   [out] Id field.
            \retval true Valid request, \c out was updated
            \retval false Invalid request (index out of range, turn file invalid), \c out unchanged */
        bool getCommandId(size_t index, int& out) const;

        /** Get command type.
            This is a high-level classification of the command.
            \param index [in] Command index, [0,getNumCommands())
            \param out   [out] Command type
            \retval true Valid request, \c out was updated
            \retval false Invalid request (index out of range, turn file invalid), \c out unchanged */
        bool getCommandType(size_t index, CommandType& out) const;

        /** Get position of a command in the file.
            This position is informative and only valid for complete, clean files.
            \param index [in] Command index, [0,getNumCommands())
            \param out   [out] Command position as 0-based index into the file
            \retval true Valid request, \c out was updated
            \retval false Invalid request (index out of range, turn file invalid), \c out unchanged */
        bool getCommandPosition(size_t index, int32_t& out) const;

        /** Get name of a command.
            \param index [in] Command index, [0,getNumCommands())
            \return Command name; null if command is invalid or unknown, or index is out of range */
        const char* getCommandName(size_t index) const;

        /** Find run of a series of commands addressed to the same unit.
            This function's behaviour is defined for PlanetCommand, ShipCommand, BaseCommand.
            For example, if this points at the first command for planet #38 in a sorted turnfile,
            this will return the number of commands for planet #38.

            Other commands cannot be sensibly grouped so using this function makes no sense.

            \param index [in] Command index, [0,getNumCommands())
            \return Number of commands for that unit. Zero if \c index is invalid, otherwise guaranteed to be at least 1. */
        size_t findCommandRunLength(size_t index) const;

        /** Get command data.
            This returns a memory descriptor to the command itself <b>and everything that follows</b>.
            Normally, the data is limited to the value returned by getCommandLength().
            Leaving the descriptor unlimited allows accessing the data of commands not known to getCommandLength().
            \param index [in] Command index, [0,getNumCommands())
            \return Command data; empty on error */
        afl::base::ConstBytes_t getCommandData(size_t index) const;

        /** Send message data (create tcm_SendMessage command).
            \param from Sender (player number)
            \param to Receiver (player number, 0 for host)
            \param data Message content (ROT13-encoded) */
        virtual void sendMessageData(int from, int to, afl::base::ConstBytes_t data);

        /** Send THost alliance commands.

            For THost, the mere presence of a friendly code change command triggers an alliance action.
            When we change the FCode back to what it should be, we can set alliances without sacrificing a ship for each action.
            This function generates this command sequence.

            \param commandSequence Command sequence (friendly codes, e.g. "ff1eea")
            \param shipId Id of ship to transmit commands
            \param shipFC Friendly code of ship to transmit commands */
        void sendTHostAllies(const String_t& commandSequence, int shipId, const String_t& shipFC);


        /*
         *  Command definition accessors
         *
         *  FIXME: these names suck; can we do better?
         *  (Consider making CommandCode_t a class with member functions.)
         */

        /** Get command type, given a command code.
            This is a high-level classification of the command.
            \param code Command code (tcm_XXX)
            \return Command code */
        static CommandType getCommandCodeType(CommandCode_t code);

        /** Get command name, given a command code.
            \param code Command code (tcm_XXX)
            \return Command name; null if command is invalid or unknown */
        static const char* getCommandCodeName(CommandCode_t code);

        /** Get command record index, given a command code.
            For ShipCommand, PlanetCommand, BaseCommand, the command data is a section of the *.dat / *.dis file record.
            This functions returns the index into the file record.
            \return Offset; zero if not applicable */
        static size_t getCommandCodeRecordIndex(CommandCode_t code);

        /*
         *  Modificators
         */

        /** Add a command.
            Call addData() to add the command's payload data.
            Call update() before writing the turn file out.
            \param cmd Command
            \param id Id field (see getCommandId()) */
        void addCommand(CommandCode_t cmd, int id);

        /** Add a command with data.
            This is a shortcut for addCommand() followed by addData().
            Call update() before writing the turn file out.
            \param cmd Command
            \param id Id field (see getCommandId())
            \param data Data */
        void addCommand(CommandCode_t cmd, int id, afl::base::ConstBytes_t data);

        /** Add command data.
            Call after addCommand().
            \param data Data */
        void addData(afl::base::ConstBytes_t data);

        /** Delete command.
            This only marks the command deleted (and therefore does not change getNumCommands()).
            Call update() before writing the turn file out.
            \param index [in] Command index, [0,getNumCommands()). Out-of-range values are ignored. */
        void deleteCommand(size_t index);

        /** Make commands for a ship.
            \param id      Ship Id
            \param oldShip Serialized old ship data (*.dis)
            \param newShip Serialized new ship data (*.dat) */
        void makeShipCommands(int id, const structures::Ship& oldShip, const structures::Ship& newShip);

        /** Make commands for a planet.
            \param id        Planet Id
            \param oldPlanet Serialized old planet data (*.dis)
            \param newPlanet Serialized new planet data (*.dat) */
        void makePlanetCommands(int id, const structures::Planet& oldPlanet, const structures::Planet& newPlanet);

        /** Make commands for a starbase.
            \param id      Base Id
            \param oldBase Serialized old base data (*.dis)
            \param newBase Serialized new base data (*.dat) */
        void makeBaseCommands(int id, const structures::Base& oldBase, const structures::Base& newBase);

        /*
         *  Structure access
         */

        /** Sort commands.
            Establishes the canonical order of commands. */
        void sortCommands();

        /** Update image.
            This removes deleted and invalid commands,
            brings command payloads into their correct order, and recomputes all checksums.
            Call this after all manipulations, before write(). */
        void update();

        /** Update trailer.
            This can be called after update() when there still have been changes done to the DOS trailer (i.e. templock processing).
            No other changes must have been made.
            \pre update() has been called, turn is not dirty */
        void updateTrailer();

        /** Compute turn checksum.
            \return computed checksum
            \pre turn is not dirty */
        uint32_t computeTurnChecksum() const;

        /*
         *  Taccom access
         */

        /** Attach a file.
            \param fileData [in] File data
            \param name [in] File name, 12 chars or less, in UTF-8
            \param pos [out] Position of file in attachment table, [0,MAX_TRN_ATTACHMENTS)
            \retval true file attached successfully
            \retval false file could not be attached, maximum number of attachments reached */
        bool addFile(afl::base::ConstBytes_t fileData, const String_t& name, size_t& pos);

        /** Delete an attached file.
            \param index Attachment to delete [0,MAX_TRN_ATTACHMENTS). Out-of-range values are ignored. */
        void deleteFile(size_t index);

        /** Get number of attachments.
            \return Number of attachments */
        size_t getNumFiles() const;

        /** Get relative position of turn data in Taccom file.
            \return Number of attachment slots that precede the turn file data.
            Zero means the turn file data appears before the first attachment data,
            one means the turn file data appears between the first and second attachment slot, etc. */
        size_t getTaccomTurnPlace() const;

        /*
         *  Output
         */

        /** Write turn file.
            \param stream Stream to write to
            \pre update() has been called, object is not dirty */
        void write(afl::io::Stream& stream) const;

        /** Get associated character set.
            \return character set as passed to the constructor */
        afl::charset::Charset& charset() const;

     private:
        /* Integration */
        afl::charset::Charset& m_charset;

        /* Turn file structure */
        structures::TurnHeader m_turnHeader;             ///< TRN header.
        structures::TaccomTurnHeader m_taccomHeader;     ///< Taccom header (TRN directory). Verbatim from turn file (offsets 1-based).
        structures::TurnDosTrailer m_dosTrailer;         ///< DOS trailer.
        structures::TurnWindowsTrailer m_windowsTrailer; ///< Windows trailer.
        afl::base::GrowableMemory<uint8_t> m_data;       ///< Miscellaneous data. The TRN, usually ;)
        afl::base::GrowableMemory<uint32_t> m_offsets;   ///< Offsets of commands, pointing into data. zero-based. NOT the pointer array from the turn file! FIXME: should be size_t?
        int m_version;                                   ///< TRN file sub-version (Winplan only).
        FeatureSet_t m_features;                         ///< TRN file features (bitfield trnf_Xxx).
        size_t m_turnPlacement;                          ///< Taccom: place TRN before Nth attachment.

        /* Internal stuff */
        bool m_isDirty;                                  ///< True if data is dirty. If false, data is a valid turn file.

        // FIXME: reconsider using FileSize_t here. size_t or uint32_t should be enough.
        void init(afl::io::Stream& str, afl::string::Translator& tx, bool fullParse);
        void checkRange(afl::io::Stream& stream, afl::string::Translator& tx, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);
        void parseTurnFile(afl::io::Stream& stream, afl::string::Translator& tx, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);
        void parseTurnFileHeader(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);

        void updateTurnFile(afl::base::GrowableMemory<uint8_t>& data, afl::base::GrowableMemory<uint32_t>& offsets);
        void makeCommands(int id, int low, int up, afl::base::ConstBytes_t oldObject, afl::base::ConstBytes_t newObject);

        String_t encodeString(const String_t& in) const;
    };

} }

#endif
