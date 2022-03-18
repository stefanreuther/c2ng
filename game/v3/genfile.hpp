/**
  *  \file game/v3/genfile.hpp
  *  \brief Class game::v3::GenFile
  */
#ifndef C2NG_GAME_V3_GENFILE_HPP
#define C2NG_GAME_V3_GENFILE_HPP

#include "game/v3/structures.hpp"
#include "game/timestamp.hpp"
#include "game/score/turnscorelist.hpp"
#include "afl/io/stream.hpp"

namespace game { namespace v3 {

    /** GEN file parser.

        This class provides functions to access Gen and ResultGen records, as read from GENx.DAT resp. RST files.
        Those records contain
        - checksums
        - scores
        - passwords
        - administrative information */
    class GenFile {
     public:
        enum Score {
            NumPlanets,
            NumCapitalShips,
            NumFreighters,
            NumBases
        };

        typedef uint8_t Signature_t[10];

        typedef game::v3::structures::Section Section_t;

        /** Default constructor.
            Makes a blank file. */
        GenFile();

        /** Construct from data.
            \param data 'Gen' structure */
        GenFile(const game::v3::structures::Gen& data);

        /** Load from GENx.DAT file.
            \param in Input stream, positioned at beginning of file. */
        void loadFromFile(afl::io::Stream& in);

        /** Load from result file.
            \param in Input stream, positioned at ResultFile::GenSection. */
        void loadFromResult(afl::io::Stream& in);

        /** Get turn number as contained in the file.
            \return turn number */
        int getTurnNumber() const;

        /** Get player number as contained in the file.
            \return player Id */
        int getPlayerId() const;

        /** Get timestamp.
            \return timestamp */
        Timestamp getTimestamp() const;

        /** Get score.
            \param player Player to query
            \param what Score to query
            \return Score. -1 if parameters are out of range */
        int getScore(int player, Score what) const;

        /** Check password.
            Compares the provided password to the current or new password and returns a yes/no answer.
            Note that we deliberately do not export a "give me the password" function.
            \param pass Password co compare against
            \return true if password was correct */
        bool isPassword(const String_t& pass) const;

        /** Check presence of password.
            \return true if this file is password-protected. */
        bool hasPassword() const;

        /** Change password.
            Updates the "new password" field.
            \param pass Password. Up to 10 characters, ASCII only */
        void setPassword(const String_t& pass);

        /** Set password (from TRN file).
            \param pass password data as read from turn file (10 bytes, encrypted, not null-terminated) */
        void setNewPasswordData(afl::base::ConstBytes_t pass);

        /** Get password (for TRN file).
            \return password data. Empty if the password was not changed, 10 bytes if the password was changed */
        afl::base::ConstBytes_t getNewPasswordData() const;

        /** Get data.
            \param data [out] GENx.DAT file image */
        void getData(game::v3::structures::Gen& data);

        /** Get signature 1 (*.dis files).
            \return signature */
        const Signature_t& getSignature1() const;

        /** Get signature 2 (*.dat files).
            \return signature */
        const Signature_t& getSignature2() const;

        /** Get section checksum.
            The player-side GEN file stores checksums; this routine returns the checksum for a particular area.

            The checksum is appropriate to the file this GenFile object was constructed from:
            - when constructed from a GEN file, it's appropriate for a game directory
              (sum of all bytes in DAT+DIS including count and signature)
            - when constructed from a ResultGen, it's appropriate for a result file
              (sum of all data bytes, not including count).

            \param sec Section to query
            \return stored checksum
            \see setSectionChecksum */
        uint32_t getSectionChecksum(Section_t sec) const;

        /** Set section checksum.
            Modifies the checksum fields that will be stored in the player-side GEN file.
            \param sec Section
            \param value Checksum, sum of all bytes (including count and signature) in the respective file pair. */
        void setSectionChecksum(Section_t sec, uint32_t value);

        /** Copy scores to TurnScoreList object.
            Updates the given TurnScoreList with the scores contained in this file.
            \param scores [out] Scores */
        void copyScoresTo(game::score::TurnScoreList& scores) const;

     private:
        game::v3::structures::Gen m_data;
        Signature_t m_signature1;
        Signature_t m_signature2;

        void setSignatures();
    };

} }

#endif
