/**
  *  \file util/resourcefilewriter.hpp
  *  \brief Class util::ResourceFileWriter
  */
#ifndef C2NG_UTIL_RESOURCEFILEWRITER_HPP
#define C2NG_UTIL_RESOURCEFILEWRITER_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/types.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/base/uncopyable.hpp"

namespace util {

    /** PCC 1.x Resource File Writer.
        Allows creation of files that can be read with ResourceFileReader (and PCC 1.x).

        To use,
        - create object to start file
        - create members by using createMember(), and writing to the returned stream
        - create hardlinks using createHardlink()
        - call finishFile() as last operation to finish the file

        Unlike ResourceFileReader, this class does not support usage of multiple streams to access the file. */
    class ResourceFileWriter : private afl::base::Uncopyable {
     public:
        /** Constructor.
            @param file  File. Should be an empty file open for output. Output will be created starting from position 0.
            @param tx    Translator for error messages */
        ResourceFileWriter(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx);

        /** Destructor.
            Call finishFile() before destroying the object. */
        ~ResourceFileWriter();

        /** Create a new member.

            Note that this call finishes the member previously being made,
            and therefore invalidates the stream returned by the previous call;
            that stream shall no longer be used.

            Note that this method will not reject creation of duplicates.

            @param id    Id used to access member. Should not already be in use.
            @return Stream to write into that member

            @throw afl::except::FileProblemException This operation or a previous one causes a file format limit to be exceeded */
        afl::base::Ref<afl::io::Stream> createMember(uint16_t id);

        /** Finish current member.
            After this call, the stream returned by the previous createMember() shall no longer be used.

            This call is optional.
            It causes error messages related for the current member be generated (size limit exceeded)
            which would otherwise be generated on the next createMember() call.

            @throw afl::except::FileProblemException A previous operation caused a file format limit to be exceeded */
        void finishMember();

        /** Check whether member exists.
            @param id Id to check
            @return true if a member with this Id has already been created using createMember() or createHardlink(). */
        bool hasMember(uint16_t id) const;

        /** Create a hardlink.
            A hardlink is a second Id referring to the same content as another one.

            Note that this call finishes the member previously being made,
            the stream returned by the previous createMember() shall no longer be used.

            Note that this method will not reject creation of duplicates.

            @param oldId   Id of existing member
            @param newId   New Id to create

            @return true on success, false on failure (oldId was invalid)

            @throw afl::except::FileProblemException This operation or a previous one causes a file format limit to be exceeded */
        bool createHardlink(uint16_t oldId, uint16_t newId);

        /** Finish file.
            Writes out all headers.
            After this call, no more calls to create or modify members shall be made.

            @throw afl::except::FileProblemException This operation or a previous one causes a file format limit to be exceeded */
        void finishFile();

     private:
        /// Underlying file.
        afl::base::Ref<afl::io::Stream> m_file;

        /// Translator (for error messages).
        afl::string::Translator& m_translator;

        /// true if file is open. false if finishFile() was called.
        bool m_fileOpen;

        /// true if we are currently writing a member. Metadata for it is in m_index.back().
        bool m_memberOpen;

        /// Index entry (cooked).
        struct Entry {
            uint16_t id;
            uint32_t position;
            uint32_t length;

            Entry(uint16_t id, uint32_t position, uint32_t length)
                : id(id), position(position), length(length)
                { }
        };

        /// Index.
        std::vector<Entry> m_index;

        const Entry* findMember(uint16_t id) const;

        void validateFileMustBeOpen() const;
        void validateMustHaveRoom() const;
        uint32_t validateFileSize(afl::io::Stream::FileSize_t size) const;
    };

}

#endif
