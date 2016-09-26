/**
  *  \file game/v3/turnfile.hpp
  */
#ifndef C2NG_GAME_V3_TURNFILE_HPP
#define C2NG_GAME_V3_TURNFILE_HPP

#include "game/timestamp.hpp"
#include "afl/io/stream.hpp"
#include "game/v3/structures.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace v3 {

    class RegistrationKey;

// /** TRN Command codes. The names are the same as in UN-TRN, the file
//     format list, and some utilities inspired by the above.
//     Those marked "<-" have been renamed, which does not mean the list
//     would be consistent now.*/
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

// /** \class GTurnfile

//     This class encapsulates all the logic needed to read/write a turn
//     file.

//     The basic idea is to build the turn file in a buffer (/data/)
//     while maintaining some headers in ready-to-use form for easy
//     access. For performance reasons, the data buffer does not always
//     contain a valid turn, instead most manipulators blindly append.
//     update() can be used to convert this big mess into a turn file,
//     which can then be written out. In particular, loading a turn file
//     using the constructor and immediately writing it out again will
//     result in a 1:1 copy; calling update() inbetween will convert the
//     file to the "canonical" format.

//     GTurnfile does not impose any limits on the order in which TRN
//     commands are stored, but it generates them always tightly packed
//     in the same order as in the pointer table. GTurnfile automatically
//     deletes invalid commands when writing out the turn. Actually,
//     deleting a command is implemented as zeroing it out.

//     When making a new turn from scratch, use the following order:
//     - first set the format (setFeatures)
//     - then set the reginfo (setRegInfo)
//     - addCommand() can be called anywhere inbetween
//     - rebuild the turn (update)
//     - write it out.

//     \invariant !(features & trnf_Taccom) <=> taccom_header is zeroed
//     \invariant !(features & trnf_Winplan) <=> win_trailer is zeroed */
    class TurnFile {
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
            \param player owning player
            \param time timestamp */
        TurnFile(afl::charset::Charset& charset,
                 int player,
                 Timestamp time);

        /** Read turn file.
            Construct a TurnFile from parsing a file.
            \param stream Stream
            \param fullParse true to read full turn. false to read only the turn header (turn will behave as if it is empty).
            \pre File pointer for \c str points to beginning of TRN
            \post fullParse => !dirty
            \throws FileFormatException on error. */
        TurnFile(afl::charset::Charset& charset,
                 afl::io::Stream& stream,
                 bool fullParse = true);

        /** Destructor. */
        ~TurnFile();


        /*
         *  Header accessors
         */

        int getPlayer() const;
        Timestamp getTimestamp() const;
        size_t getNumCommands() const;
        FeatureSet_t getFeatures() const;
        int getVersion() const;

        void setTimestamp(const Timestamp& time);
        void setVersion(int n);
        void setFeatures(FeatureSet_t f);

        /*
         *  Trailer access
         */
        /** Try to get the turn number used to generate this turn.
            This is a guess only, and is only intended to be shown in un-trn listings (or, to generate a copy of this turn).
            \return turn number, or 0 if not known */
        int tryGetTurnNr() const;

//     void               setTemplock(const uint32_t* data);

        void setRegistrationKey(const RegistrationKey& key, int turnNr);


        /*
         *  Header structure accessors
         */
//     const TTurnWindowsTrailer& getWindowsTrailer() const;
//     const TTurnDosTrailer& getDosTrailer() const;
        const structures::TurnHeader& getTurnHeader() const;
//     const TTaccomTurnHeader& getTaccomHeader() const;

        /*
         *  Command accessors
         */
        bool               getCommandCode(size_t index, CommandCode_t& out) const;
        bool               getCommandLength(size_t index, int& out) const;
        bool               getCommandId(size_t index, int& out) const;
        bool               getCommandType(size_t index, CommandType& out) const;
        bool               getCommandPosition(size_t index, int32_t& out) const;
        const char*        getCommandName(size_t index) const;
        afl::base::ConstBytes_t getCommandData(size_t index) const;

        /*
         *  Command definition accessors
         */
        static CommandType getCommandCodeType(CommandCode_t code); // FIXME: these names suck
        static const char* getCommandCodeName(CommandCode_t code);
        static int         getCommandCodeRecordIndex(CommandCode_t code);

        /*
         *  Modificators
         */
        void               addCommand(CommandCode_t cmd, int id);
        void               addCommand(CommandCode_t cmd, int id, afl::base::ConstBytes_t data);
        void               addData(afl::base::ConstBytes_t data);
        void               deleteCommand(size_t index);

//     /* Maketurn */
//     void               makeShipCommands(int id, const char* old, const char* neu);
//     void               makePlanetCommands(int id, const char* old, const char* neu);
//     void               makeBaseCommands(int id, const char* old, const char* neu);

        /*
         *  Structure access
         */
        void               sortCommands();
        void               update();
        void               updateTrailer();
        uint32_t           computeTurnChecksum() const;

        /*
         *  Taccom access
         */
        int                addFile(afl::base::ConstBytes_t fileData, const String_t& name);
        void               deleteFile(size_t index);
        size_t             getNumFiles() const;
        int                getTaccomTurnPlace() const;

        /*
         *  Output
         */
        void               write(afl::io::Stream& stream);

     private:
        /* Integration */
        afl::charset::Charset& m_charset;
        
        /* Turn file structure */
        structures::TurnHeader m_turnHeader;             ///< TRN header.
        structures::TaccomTurnHeader m_taccomHeader;     ///< Taccom header (TRN directory). Verbatim from turn file (offsets 1-based).
        structures::TurnDosTrailer m_dosTrailer;         ///< DOS trailer.
        structures::TurnWindowsTrailer m_windowsTrailer; ///< Windows trailer.
        afl::base::GrowableMemory<uint8_t> m_data;       ///< Miscellaneous data. The TRN, usually ;)
        afl::base::GrowableMemory<uint32_t> m_offsets;   ///< Offsets of commands, pointing into data. zero-based. NOT the pointer array from the turn file!
        int m_version;                       ///< TRN file sub-version (Winplan only).
        FeatureSet_t m_features;             ///< TRN file features (bitfield trnf_Xxx).
        int m_turnPlacement;                 ///< Taccom: place TRN before Nth attachment.

        /* Internal stuff */
        bool m_isDirty;                        ///< True if data is dirty. If false, data is a valid turn file.

        // FIXME: reconsider using FileSize_t here. size_t or uint32_t should be enough.
        void init(afl::io::Stream& str, bool fullParse);
        void checkRange(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);
        void parseTurnFile(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);
        void parseTurnFileHeader(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length);

        void updateTurnFile(afl::base::GrowableMemory<uint8_t>& data, afl::base::GrowableMemory<uint32_t>& offsets);
//     void makeCommands(int id, int low, int up, const char* old, const char* neu);

        String_t encodeString(const String_t& in) const;
    };

} }

#endif
