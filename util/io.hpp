/**
  *  \file util/io.hpp
  *  \brief I/O-related utilities
  */
#ifndef C2NG_UTIL_IO_HPP
#define C2NG_UTIL_IO_HPP

#include <memory>
#include "afl/charset/charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/value.hpp"
#include "afl/io/datasink.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Store Pascal string, all-or-nothing.
        Stores a length byte followed by the string data if the length can be correctly represented,
        nothing if the string is too long.
        \param out Data sink
        \param str String to store
        \param charset Character set
        \retval true String stored successfully
        \retval false String too long, nothing stored */
    bool storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);

    /** Store Pascal string, truncating version.
        Stores a length byte followed by the string data if the length can be correctly represented.
        If the string is too long, truncates it to 255 characters (in target character set!).
        \param out Data sink
        \param str String to store
        \param charset Character set
        \retval true String stored entirely
        \retval false String too long, stored truncated */
    bool storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);

    /** Load Pascal string.
        \param in Stream
        \param charset Character set
        \return loaded string */
    String_t loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset);

    /** Append file name extension.
        \param fs        File system
        \param pathName  Path name
        \param ext       Extension to append, not including leading dot
        \param force     true: replace an existing extension; false: append extension only if it is missing
        \return New path name */
    String_t appendFileNameExtension(afl::io::FileSystem& fs, String_t pathName, String_t ext, bool force);

    /** Get file name extension.
        \param fs        File system
        \param pathName  Path name
        \return Extension (including the dot), empty if path name has no extension */
    String_t getFileNameExtension(afl::io::FileSystem& fs, String_t pathName);

    /** Try to create a path.
        Creates a complete path that can contain multiple non-existant
        directory levels. This does not fail when the path cannot be created;
        in that case, subsequent operations using the path will fail.
        \param fs File System instance
        \param dirName Name of path to create */
    void createDirectoryTree(afl::io::FileSystem& fs, const String_t dirName);

    /** Create a search directory.
        Given a list of directory names,
        produces a Directory instance that allows opening files from all of them,
        starting at the first.
        This can be used to implement search paths.
        All the directories given should exist.
        The filesystem is not modified by this function.
        \param fs File System instance
        \param dirNames List of directory names
        \return Directory instance */
    afl::base::Ref<afl::io::Directory> makeSearchDirectory(afl::io::FileSystem& fs, afl::base::Memory<const String_t> dirNames);

    /** Parse JSON, given a byte array.
        On error, throws std::exception.
        \param data Data
        \return Newly-allocated parse result */
    std::auto_ptr<afl::data::Value> parseJSON(afl::base::ConstBytes_t data);

    /** Find array item in a list of objects.
        If array refers to an array (vector) of objects,
        returns handle to the first object that has the specific value in the given key.
        If none exists, or the parameter is not an actual array, returns null.

        \param array Array
        \param key   Key
        \param value Expected value
        \return Found value */
    afl::data::Access findArrayItemById(afl::data::Access array, String_t key, int value);

    /** Retrieve list of integers.
        If the given value is one of the supported formats,
        - an array of integers [1,2,3]
        - a string containing integers "1,2,3"
        - a single integer
        appends the values to the given list.
        Otherwise, does nothing.

        \param [in,out] list  List
        \param [in]     value Value */
    void toIntegerList(afl::data::IntegerList_t& list, afl::data::Access value);

    /** Convert byte array to string, normalizing linefeeds.
        \param in Byte array
        \return String */
    String_t normalizeLinefeeds(afl::base::ConstBytes_t in);

}

#endif
