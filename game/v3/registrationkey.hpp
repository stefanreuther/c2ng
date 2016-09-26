/**
  *  \file game/v3/registrationkey.hpp
  *  \brief Class game::v3::RegistrationKey
  */
#ifndef C2NG_GAME_V3_REGISTRATIONKEY_HPP
#define C2NG_GAME_V3_REGISTRATIONKEY_HPP

#include "game/registrationkey.hpp"
#include "afl/string/string.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/directory.hpp"
#include "afl/sys/loglistener.hpp"

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
        static const size_t KEY_SIZE = 51;

        /** Constructor.
            \param charset Game character set */
        explicit RegistrationKey(afl::charset::Charset& charset);

        // RegistrationKey methods:
        virtual Status getStatus() const;
        virtual String_t getLine(Line which) const;
        virtual bool setLine(Line which, String_t value);

        /** Create unregistered key. */
        void initUnregistered();

        /** Make this key "unowned" (name/address not set). */
        void initUnowned();

        /** Initialize by loading files from a directory.
            \param dir Directory to read.
            \param log Logger.

            Change to PCC2: the "verbose" parameter has been replaced by a "log" parameter.
            To get non-verbose output, pass a Log instance with no listeners. */
        void initFromDirectory(afl::io::Directory& dir, afl::sys::LogListener& log);

        /** Get registration key in encoded form.
            This is used for turn files. */
        afl::base::Memory<const uint32_t> getKey() const;

     private:
        void initFromFizz(const uint32_t (&data)[KEY_SIZE]);
        void parseFizz(afl::io::Stream& s);
        void parseKey(afl::io::Stream& s);

        String_t decode(size_t start) const;

        afl::charset::Charset& m_charset;

        uint32_t m_fizz[KEY_SIZE];

        String_t m_winplanString1;
        String_t m_winplanString2;

        bool m_isValid;
    };

} }

#endif
