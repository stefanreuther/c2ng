/**
  *  \file util/filenamepattern.hpp
  */
#ifndef C2NG_UTIL_FILENAMEPATTERN_HPP
#define C2NG_UTIL_FILENAMEPATTERN_HPP

#include <memory>
#include "afl/string/string.hpp"

namespace util {

    // /*! \class FileNamePattern
    //     \brief File Name Matching

    //     This class provides a way to match file names against patterns. 
    //     You have to place a pattern in a FileNamePattern object and can
    //     then use it to match against possibly many file names.

    //     FileNamePattern is the portable interface, it uses an instance of
    //     FileNameMatcher to implement actual matching rules. */
    class FileNamePattern {
     public:
        FileNamePattern();
        FileNamePattern(const String_t name);
        FileNamePattern(const FileNamePattern& other);
        FileNamePattern& operator=(const FileNamePattern& other);
        ~FileNamePattern();

        void setPattern(const String_t pattern);
        bool hasWildcard() const;
        bool getFileName(String_t& out) const;
        bool match(const String_t filename) const;
        bool empty() const;

        static String_t getAllFilesPattern();
        static String_t getAllFilesWithExtensionPattern(String_t ext);
        static String_t getSingleFilePattern(String_t name);
     private:
        class Impl;
        std::auto_ptr<Impl> m_pImpl;
    };

}

#endif
