/**
  *  \file util/fileparser.hpp
  *  \brief Class util::FileParser
  */
#ifndef C2NG_UTIL_FILEPARSER_HPP
#define C2NG_UTIL_FILEPARSER_HPP

#include <memory>
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/directory.hpp"

namespace util {

    /** Abstract File Parser.
        This class provides the logic to read text files.
        It will split the file into lines, wield out empty and comment lines,
        and call handleLine() or handleIgnoredLine() for each line.
        Derived classes will override these methods to actually do some parsing.

        Comments are started by using a configurable comment character as first non-blank character. */
    class FileParser {
     public:
        /** Constructor.
            \param commentCharacters List of comment characters (can be null, then we don't have comment characters) */
        explicit FileParser(const char* commentCharacters);

        /** Destructor. */
        virtual ~FileParser();

        /** Parse a file.
            \param in stream to read from */
        void parseFile(afl::io::Stream& s);

        /** Parse an optional file.
            \param dir Directory
            \param fileName File name
            \retval true File was found and parsed
            \retval false File did not exist */
        bool parseOptionalFile(afl::io::Directory& dir, String_t fileName);

        /** Callback: parse a text line.
            This function is called for each line which is neither empty nor a comment.
            \param fileName file name
            \param lineNr line number
            \param line line text */
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line) = 0;

        /** Callback: ignore a line.
            This function is called for each line which is empty or a comment.
            \param fileName file name
            \param lineNr line number
            \param line line text */
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line) = 0;

        /** Trim comments.
            If the line contains a comment, remove it.
            This is a utility function for use by derived classes.
            \param line [in/out] Line to work on */
        void trimComments(String_t& line);

        /** Set character set.
            If this is called, the requested character set is forced for the next parse.
            If this function is not called, or given a null parameter,
            afl::io::TextFile's auto-detection is used.
            \param cs Newly-allocated character coder. FileParser takes ownership. */
        void setCharsetNew(afl::charset::Charset* cs);

        /** Configure a text file.
            Sets up the text file as requested.
            Currently, this configures the character set.
            \param textFile text file to configure */
        void configureTextFile(afl::io::TextFile& textFile);

     private:
        const char* const m_commentCharacters;
        std::auto_ptr<afl::charset::Charset> m_charset;
    };

}

#endif
