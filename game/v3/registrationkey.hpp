/**
  *  \file game/v3/registrationkey.hpp
  *  \brief Class game::v3::RegistrationKey
  */
#ifndef C2NG_GAME_V3_REGISTRATIONKEY_HPP
#define C2NG_GAME_V3_REGISTRATIONKEY_HPP

#include <memory>
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/registrationkey.hpp"

namespace game { namespace v3 {

    /** Registration key implementation for VGAP3.

        Complete registration information. The actual reg key (player name and town in DOS, serial and reg date in Winplan)
        is read from FIZZ.BIN, the additional information (name/address in Winplan) is read from REG.KEY.

        This does not include more knowledge than absolutely needed to fetch the above values from files.
        In particular, we don't check checksums; invalid keys are accepted and sent to host (which will then reject them).

        This class' invariant is that it always contains a syntactically valid registration key,
        even if it reports its status as unknown.
        This means it can always be used to create syntactically valid turn files. */
    class RegistrationKey : public game::RegistrationKey {
     public:
        /** Size of a key, in words. */
        static const size_t KEY_SIZE_WORDS = 51;

        /** Size of a key, in bytes. */
        static const size_t KEY_SIZE_BYTES = 4*KEY_SIZE_WORDS;


        /** Constructor.
            \param charset Game character set */
        explicit RegistrationKey(std::auto_ptr<afl::charset::Charset> charset);

        // RegistrationKey methods:
        virtual Status getStatus() const;
        virtual String_t getLine(Line which) const;
        virtual bool setLine(Line which, String_t value);
        virtual int getMaxTechLevel(TechLevel area) const;

        /** Create unregistered key. */
        void initUnregistered();

        /** Make this key "unowned" (name/address not set). */
        void initUnowned();

        /** Initialize by loading files from a directory.
            \param dir Directory to read.
            \param log Logger.
            \param tx Translator

            Change to PCC2: the "verbose" parameter has been replaced by a "log" parameter.
            To get non-verbose output, pass a Log instance with no listeners. */
        void initFromDirectory(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Save to given stream.
            Used to create a key file from given content.
            Do NOT use this during regular game save.
            \param file File */
        void saveToStream(afl::io::Stream& file);

        /** Initialize from a data array.
            \param bytes Bytes to read (should be KEY_SIZE_BYTES bytes) */
        void unpackFromBytes(afl::base::ConstBytes_t bytes);

        /** Store into data array.
            \param bytes Bytes to write (should be KEY_SIZE_BYTES bytes) */
        void packIntoBytes(afl::base::Bytes_t bytes) const;

        /** Get key Id.
            The key Id is a printable hex string derived from the content and uniquely identifies this key
            without listing its plaintext.
            \return key Id */
        String_t getKeyId() const;

     private:
        void initFromFizz(const uint32_t (&data)[KEY_SIZE_WORDS]);
        void parseFizz(afl::io::Stream& s);
        void parseKey(afl::io::Stream& s, afl::string::Translator& tx);

        String_t decode(size_t start) const;

        std::auto_ptr<afl::charset::Charset> m_charset;

        uint32_t m_fizz[KEY_SIZE_WORDS];

        String_t m_winplanString1;
        String_t m_winplanString2;

        /** Validity flag.
            Invalid means not loaded from a file; content is still initialized. */
        bool m_isValid;
    };

} }

#endif
