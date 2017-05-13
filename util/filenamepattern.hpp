/**
  *  \file util/filenamepattern.hpp
  *  \brief Class util::FileNamePattern
  */
#ifndef C2NG_UTIL_FILENAMEPATTERN_HPP
#define C2NG_UTIL_FILENAMEPATTERN_HPP

#include <memory>
#include "afl/string/string.hpp"

namespace util {

    /** File name pattern matching.

        This class provides a way to match file names against patterns.
        You have to place a pattern in a FileNamePattern object and can then use it to match against possibly many file names.

        This is implemented using the pimpl idiom to allow exchanging the implementation, for example, to operating-system-specific rules.
        Currently implemented rules on all operating systems:
        - "*" matches any sequence of characters, including nothing at all
        - "?" matches any single character
        - "\" quotes the next character, i.e. "\*" matches a single star */
    class FileNamePattern {
     public:
        /** Construct blank pattern.
            Call setPattern() before use. */
        FileNamePattern();

        /** Construct pattern.
            \param name Pattern
            \throw std::runtime_error on invalid patterns */
        FileNamePattern(const String_t name);

        /** Copy constructor.
            \param other Other pattern */
        FileNamePattern(const FileNamePattern& other);

        /** Assignment operator.
            \param other Other pattern */
        FileNamePattern& operator=(const FileNamePattern& other);

        /** Destructor. */
        ~FileNamePattern();

        /** Set pattern.
            Discards the old one and sets a new one.
            \param pattern [in] The new pattern
            \throw std::runtime_error on invalid patterns */
        void setPattern(const String_t pattern);

        /** Check whether pattern has a wildcard.
            If it does not have a wildcard, it is a proper file name which can be used directly (see getFileName()).
            \return true if this has a wildcard, false if it is a proper file name. */
        bool hasWildcard() const;

        /** Get file name.
            \param out [out] File name
            \retval true Pattern contained a single file name that has been returned
            \retval false Pattern contained wildcards, no file name could be returned */
        bool getFileName(String_t& out) const;

        /** Match.
            \param filename File name to test
            \return true on match */
        bool match(const String_t filename) const;

        /** Check emptiness.
            \return true if pattern is empty */
        bool empty() const;

        /** Create pattern that matches all files.
            \return pattern */
        static String_t getAllFilesPattern();

        /** Create pattern that matches all files with a given extension.
            \param ext extension (without the dot)
            \return pattern */
        static String_t getAllFilesWithExtensionPattern(String_t ext);

        /** Create pattern that matches a single file.
            \param name file name
            \return pattern */
        static String_t getSingleFilePattern(String_t name);
     private:
        class Impl;
        std::auto_ptr<Impl> m_pImpl;
    };

}

#endif
