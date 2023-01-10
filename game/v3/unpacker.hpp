/**
  *  \file game/v3/unpacker.hpp
  *  \brief Class game::v3::Unpacker
  */
#ifndef C2NG_GAME_V3_UNPACKER_HPP
#define C2NG_GAME_V3_UNPACKER_HPP

#include "afl/base/growablememory.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/log.hpp"
#include "game/msg/outbox.hpp"
#include "game/playerlist.hpp"
#include "game/v3/controlfile.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/trn/turnprocessor.hpp"

namespace game { namespace v3 {

    class ResultFile;

    /** Result file unpacker.

        To unpack a result file,
        - instantiate an Unpacker
        - configure it (in particular, attach a log listener)
        - call prepare() to load the result file
        - optionally, use turnProcessor() to apply a turn file
        - call finish() to produce the unpacked output

        Changes to PCC2 version:
        - This inverts control of turn file application.
          Whereas PCC2 provides a "pull" mechanism (unpacker asks turn file to apply changes),
          c2ng uses a "push" mechanism (turn file can push changes into the unpacker).
        - This version is not interactive, i.e. does not ask whether a race name file is to be replaced.
          It can be either enabled or disabled. */
    class Unpacker : private game::v3::trn::TurnProcessor {
     public:
        enum DirectoryFormat {
            WindowsFormat,
            DosFormat
        };

        /** Constructor.
            \param tx Translator
            \param playerList Player list (race names, for Windows-style messages; lifetime must exceed that of the Unpacker) */
        explicit Unpacker(afl::string::Translator& tx, const PlayerList& playerList);

        /** Destructor. */
        ~Unpacker();

        /*
         *  Configuration
         */

        /** Set file format to produce.
            This defines what format the checksums are generated in,
            it does not prevent the unpacker from extracting a possible Windows part of the RST (see setIgnore35Part).
            The default is the more flexible WindowsFormat.
            \param fmt Format */
        void setFormat(DirectoryFormat fmt);

        /** Set version 3.5 part handling.
            If this option is set, a version 3.5 part of the RST will be ignored as if it were not present.
            The default is to prrocess these files.
            \param flag Value */
        void setIgnore35Part(bool flag);

        /** Set target.ext creation flag.
            If this option is set, a file "target.ext" will be made containing all targets beyond the 50th
            from the regular target section, plus all AVCs from the Winplan part of the RST.
            When disabled (default), files will be unpacked as-is.
            \param flag Value */
        void setCreateTargetExt(bool flag);

        /** Set error correction flag.
            If enabled (default), a number of host-side errors will be corrected to make the produced game directory a little more compatible.
            If disabled, files will be unpacked as-is.
            \param flag Value */
        void setFixErrors(bool flag);

        /** Set checksum ignore flag.
            By default, checksum errors cause the unpack process to be aborted.
            Enabling this option will unpack the file anyway.
            \param flag Value */
        void setForceIgnoreErrors(bool flag);

        /** Set verbosity flag.
            If enabled, some more messages will be logged.
            \param flag Value */
        void setVerbose(bool flag);

        /** Get configured file format.
            \return file format
            \see setFormat */
        DirectoryFormat getFormat() const;


        /*
         *  Entry Points
         */

        /** Prepare unpacking a file.
            Resets internal state, pre-parses and validates the file.

            The \c player parameter determines the output player number.
            For example, with \c player=3, we will create a pdata3.dat.
            If error checking is enabled (default, see setForceIgnoreErrors),
            a FileFormatException is raised if the result belongs to a different player.

            \param file ResultFile instance
            \param player Expected player

            \throws afl::except::FileFormatException on file format problems
            \throws afl::except::FileProblemException on other file problems */
        void prepare(ResultFile& file, int player);

        /** Access TurnProcessor.
            Can be used to modify the *.dat files before they are written out.
            This is used to apply a turn file.
            \return TurnProcessor */
        game::v3::trn::TurnProcessor& turnProcessor();

        /** Finish unpacking a file.
            This creates the new game directory files.
            Before this call, prepare() must have been called.

            \param dir Output directory
            \param file ResultFile instance (same as for prepare())

            \throws afl::except::FileFormatException on file format problems
            \throws afl::except::FileProblemException on other file problems */
        void finish(afl::io::Directory& dir, ResultFile& file);

        /** Get turn number.
            Call after prepare().
            \return turn number */
        int getTurnNumber() const;

        /** Access logger.
            Attach a listener to receive messages from Unpacker.
            \return logger */
        afl::sys::Log& log();

        /** Access character set.
            Returns the "Latin-1" character set.

            In general, unpacking is a character-set-neutral operation, that is,
            game data is written in the same character encoding it came in.
            Therefore, there is no way to configure a character set for unpacking.

            However, when you interface with external data (e.g. using the turnProcessor() interface),
            you need to provide it in this encoding so that it comes out character-set-neutral.

            \return character set */
        afl::charset::Charset& charset();

     private:
        typedef afl::base::GrowableMemory<game::v3::structures::ShipTarget> TargetBuffer_t;

        afl::string::Translator& m_translator;
        afl::sys::Log m_log;

        DirectoryFormat m_format;        ///< Format of directory to create.
        bool m_ignore35;                 ///< true to ignore 3.5 part of RST.
        bool m_createTargetExt;          ///< true to generate TARGETx.EXT file.
        bool m_fixErrors;                ///< true to fix some errors.
        bool m_ignoreErrors;             ///< true to unpack despite checksum errors.
        bool m_verbose;                  ///< true for verbose output.

        afl::base::GrowableMemory<game::v3::structures::Ship> m_datShips;
        afl::base::GrowableMemory<game::v3::structures::Ship> m_disShips;

        afl::base::GrowableMemory<game::v3::structures::Planet> m_datPlanets;
        afl::base::GrowableMemory<game::v3::structures::Planet> m_disPlanets;

        afl::base::GrowableMemory<game::v3::structures::Base> m_datBases;
        afl::base::GrowableMemory<game::v3::structures::Base> m_disBases;

        const PlayerList& m_playerList;

        game::msg::Outbox m_outbox;
        String_t m_allianceCommands;

        GenFile m_gen;
        ControlFile m_control;
        int m_playerId;

        afl::charset::CodepageCharset m_charset;

        // Internal:
        void loadShips(ResultFile& result);
        void loadPlanets(ResultFile& result);
        void loadBases(ResultFile& result);

        void saveShips(afl::io::Directory& dir);
        void savePlanets(afl::io::Directory& dir);
        void saveBases(afl::io::Directory& dir);
        void saveGen(afl::io::Directory& dir);
        void saveTargetExt(afl::io::Directory& dir, const TargetBuffer_t& targetBuffer);

        void unpackTargets(afl::io::Directory& dir, ResultFile& result, TargetBuffer_t& targetBuffer);
        void unpackVcrs(afl::io::Directory& dir, ResultFile& result);
        void unpackShipXY(afl::io::Directory& dir, ResultFile& result);
        void unpackMessages(afl::io::Directory& dir, ResultFile& result);
        void unpackKore(afl::io::Directory& dir, ResultFile& result, TargetBuffer_t& targetBuffer);
        void unpackSkore(afl::io::Directory& dir, ResultFile& result);

        void updateIndex(afl::io::Directory& dir);
        void createBlankFiles(afl::io::Directory& dir);
        void removeGameFile(afl::io::Directory& dir, const String_t& name);
        void fail(const char* tpl, int arg);

        // TurnProcessor:
        virtual void handleInvalidCommand(int code);
        virtual void validateShip(int id);
        virtual void validatePlanet(int id);
        virtual void validateBase(int id);

        virtual void getShipData(int id, Ship_t& out, afl::charset::Charset& charset);
        virtual void getPlanetData(int id, Planet_t& out, afl::charset::Charset& charset);
        virtual void getBaseData(int id, Base_t& out, afl::charset::Charset& charset);

        virtual void storeShipData(int id, const Ship_t& in, afl::charset::Charset& charset);
        virtual void storePlanetData(int id, const Planet_t& in, afl::charset::Charset& charset);
        virtual void storeBaseData(int id, const Base_t& in, afl::charset::Charset& charset);

        virtual void addMessage(int to, String_t text);
        virtual void addNewPassword(const NewPassword_t& pass);
        virtual void addAllianceCommand(String_t text);
    };

} }

#endif
