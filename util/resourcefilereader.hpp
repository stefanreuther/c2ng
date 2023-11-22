/**
  *  \file util/resourcefilereader.hpp
  *  \brief Class util::ResourceFileReader
  */
#ifndef C2NG_UTIL_RESOURCEFILEREADER_HPP
#define C2NG_UTIL_RESOURCEFILEREADER_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/types.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"

namespace util {

    /** PCC 1.x Resource File Reader.
        Provides access to the individual files contained in a PCC 1.x ".res" file.

        A PCC 1.x resource file contains multiple sub-streams identified by a number each.
        You can use openMember() to obtain a stream object that allows you to read a member.
        Any number of these streams can be active at any given time.

        This class used to reside in ui::res. */
    class ResourceFileReader {
     public:
        /** Constructor.
            @param file File
            @param tx Translator (for error messages) */
        ResourceFileReader(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx);

        /** Destructor. */
        ~ResourceFileReader();

        /** Open a resource file member as stream.
            This is the primary method of working with these files.
            @param id Member Id
            @return stream object if a member with this Id exists; null if no such member exists */
        afl::base::Ptr<afl::io::Stream> openMember(uint16_t id);

        /** Get number of members.
            @return Number of members */
        size_t getNumMembers() const;

        /** Open a resource file member as stream, by index.
            @param index Index [0, getNumMembers())
            @return stream object on success; otherwise, null */
        afl::base::Ptr<afl::io::Stream> openMemberByIndex(size_t index);

        /** Get Member Id, given an index.
            This can be used to iterate a file's content;
            call openMember() with the return value to open the member.

            @param index Index [0, getNumMembers())
            @return Member Id, 0 if index is out of range */
        uint16_t getMemberIdByIndex(size_t index) const;

        /** Find primary member Id, given an index.
            Members can be hardlinked (=share the same content).
            This function retrieves the primary Id.
            The primary Id is the first (not lowest!) Id for that content.

            For example, if index #7 is the first entry referring to particular content,
            findPrimaryIdByIndex(7) == getMemberIdByIndex(7).

            If index #12 refers to the same content,
            findPrimaryIdByIndex(12) == getMemberIdByIndex(7).

            This is a O(n) operation.
            It is only required for detailed inspection of files,
            it is not needed in normal operation.

            @param index Index [0, getNumMembers())
            @return Primary member Id */
        uint16_t findPrimaryIdByIndex(size_t index) const;

     private:
        /// Underlying file.
        afl::base::Ref<afl::io::Stream> m_file;

        /// Index entry (cooked).
        struct Entry {
            uint16_t id;
            uint32_t position;
            uint32_t length;
        };

        /// Index.
        std::vector<Entry> m_index;

        void init(afl::string::Translator& tx);
    };

}

#endif
