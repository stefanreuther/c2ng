/**
  *  \file util/charsetfactory.hpp
  *  \brief Class util::CharsetFactory
  */
#ifndef C2NG_UTIL_CHARSETFACTORY_HPP
#define C2NG_UTIL_CHARSETFACTORY_HPP

#include "afl/base/optional.hpp"
#include "afl/charset/charsetfactory.hpp"
#include "afl/string/translator.hpp"

namespace util {

    /** Character set handling.
        This is the project-specific implementation of afl::charset::CharsetFactory,
        with the project-specific repertoire of character sets.

        In addition, it provides a way to enumerate and describe character sets.
        This is required for GUI dialogs and configuration persistence.

        This is a value object, although it does not currently contain data. */
    class CharsetFactory : public afl::charset::CharsetFactory {
     public:
        /** Character set identifier.
            Values of this type can be used internally to describe a character set. */
        typedef size_t Index_t;

        /** Index for Unicode character set. */
        static const Index_t UNICODE_INDEX = 0;

        /** Index for Latin-1 character set. */
        static const Index_t LATIN1_INDEX = 9;


        /** Default constructor. */
        CharsetFactory();

        /** Get number of known character sets.
            Valid Ids range from 0 (inclusive) to this number (exclusive).
            \return Number of known character */
        Index_t getNumCharsets() const;

        /** Create character set.
            \param index Index [0,getNumCharsets())
            \return newly-allocated Charset object; 0 on error */
        afl::charset::Charset* createCharsetByIndex(Index_t index) const;

        /** Get key for a character set.
            The key identifies this character set and can be used with findIndexByKey() and createCharset().
            Use this to store in configuration files.
            \param index Index [0,getNumCharsets())
            \return key such that findIndexByKey() will return \c index; empty string on error */
        String_t getCharsetKey(Index_t index) const;

        /** Get name for a character set.
            The name identifies this character set in a human-readable way.
            \param index Index [0,getNumCharsets())
            \param tx Translator
            \return name; empty string on error */
        String_t getCharsetName(Index_t index, afl::string::Translator& tx) const;

        /** Get description for a character set.
            The descriptions briefly describes this character set in a human-readable way.
            \param index Index [0,getNumCharsets())
            \param tx Translator
            \return name; empty string on error */
        String_t getCharsetDescription(Index_t index, afl::string::Translator& tx) const;

        /** Look up a key, producing an index.
            Each character set can be recognized under multiple keys.
            \param name Key (name of charset)
            \return Resulting index if found */
        afl::base::Optional<Index_t> findIndexByKey(String_t name) const;

        // CharsetFactory:
        virtual afl::charset::Charset* createCharset(String_t name);
    };

}

inline
util::CharsetFactory::CharsetFactory()
{ }

#endif
