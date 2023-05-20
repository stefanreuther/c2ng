/**
  *  \file game/v3/attachmentunpacker.hpp
  *  \brief Class game::v3::AttachmentUnpacker
  */
#ifndef C2NG_GAME_V3_ATTACHMENTUNPACKER_HPP
#define C2NG_GAME_V3_ATTACHMENTUNPACKER_HPP

#include "afl/bits/smallset.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/timestamp.hpp"

namespace game { namespace v3 {

    /** Attachment unpacker.
        In addition to the data for the current turn, result file packages can come with a variety of attachments.
        Attachments can be needed for more than the current turn (we don't know) and therefore need to be unpacked
        even if the user doesn't unpack the RST.
        Attachments are:
        - race names / leech file in RST
        - files embedded in UTIL.DAT
        This class implements unpacking those.

        To use,
        - create AttachmentUnpacker
        - configure using setAcceptableKind()
        - call one or more of the load() methods to search for attachments
          (for example, when unpacking multiple results, call loadResultFile() for each of them)
        - use getNumAttachments(), getAttachmentByIndex(), getAttachmentName() etc. to inquire status
        - optionally, call dropUnchangedFiles() to drop attachments the user already has
        - optionally, call selectAttachment() to select or deselect attachments to accept
        - call saveFiles() to save the selected attachments

        Files are classified into different kinds.
        Files that shall not be transmitted as attachments are classified as CriticalFile and by default not received.
        A few other files are classified to allow per-kind configuration and bulk selection.

        Since we're aiming at 32-bit or better systems, this class takes the easy way and just slurps
        all the attachments into memory.

        If files from multiple turns are loaded, only the newest files (according to the timestamp) are kept.

        Attachments are presented in a user-friendly order (i.e. sorted using strCollate). */
    class AttachmentUnpacker {
     public:
        class Attachment;
        class Reader;

        /** File kind. */
        enum Kind {
            NormalFile,                    ///< Normal (unclassified) file.
            ConfigurationFile,             ///< Configuration file.
            RaceNameFile,                  ///< Race name file.
            CriticalFile                   ///< Critical file that should better not be received.
        };
        typedef afl::bits::SmallSet<Kind> Kinds_t;


        /** Constructor.
            Makes an empty AttachmentUnpacker. */
        AttachmentUnpacker();

        /** Destructor. */
        ~AttachmentUnpacker();

        /** Set whether a file kind is acceptable.
            All future files of that kind will be selected or unselected depending on the \c enable parameter.
            \param k Kind
            \param enable Whether to enable or disable those files */
        void setAcceptableKind(Kind k, bool enable);

        /** Load attachments from directory.
            Loads result and utildata files if they exist.
            Errors are ignored.
            \param dir Directory
            \param playerNumber Player number
            \param log Logger (for traces)
            \param tx Translator (for error messages) */
        void loadDirectory(afl::io::Directory& dir, int playerNumber, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load result file.
            \param in Opened result file
            \param playerNumber Player number
            \param log Logger (for traces)
            \param tx Translator (for error messages) */
        void loadResultFile(afl::io::Stream& in, int playerNumber, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load utildata file.
            \param in Opened utildata (util.dat) file
            \param playerNumber Player number
            \param log Logger (for traces)
            \param tx Translator (for error messages) */
        void loadUtilData(afl::io::Stream& in, int playerNumber, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Drop unchanged files.
            Verifies whether any of the attachment files already exists in the given directory with identical content.
            In this case, drops the attachment; saveFiles() will not overwrite it.
            Use this method to offer users only files that actually changed.
            \param dir Directory to check
            \param log Logger (for traces)
            \param tx Translator (for error messages) */
        void dropUnchangedFiles(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Drop unselected attachment.
            Drops all attachments that are not currently selected.
            You need not call this method before saveFiles(); saveFiles() filters internally. */
        void dropUnselectedAttachments();

        /** Save attachments into files.
            For each selected attachment, creates the corresponding file in the given directory.
            \param dir Directory
            \param log Logger (for status/error reports)
            \param tx Translator (for status/error reports) */
        bool saveFiles(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Get number of attachments currently loaded.
            \return number */
        size_t getNumAttachments() const;

        /** Get handle to attachment, given an index.
            \param i Index, [0,getNumAttachments())
            \return handle; null if index out of range */
        Attachment* getAttachmentByIndex(size_t i) const;

        /** Get handle to attachment, given a file name.
            \param name File name (must be all-lowercase)
            \return handle; null if file not found */
        Attachment* getAttachmentByName(const String_t& name) const;

        /** Get kind of attachment.
            \param p Handle
            \return Kind */
        Kind getAttachmentKind(Attachment* p) const;

        /** Select attachment for saving.
            \param p Handle
            \param enable true to select (will be saved), false to deselect (will be skipped) */
        void selectAttachment(Attachment* p, bool enable);

        /** Select attachments for saving, by kind.
            \param k Kind
            \param enable true to select (will be saved), false to deselect (will be skipped) */
        void selectAttachmentsByKind(Kind k, bool enable);

        /** Select all attachments for saving.
            \param enable true to select (will be saved), false to deselect (will be skipped) */
        void selectAllAttachments(bool enable);

        /** Check whether attachment is selected for saving.
            \param p Handle
            \return true if attachment is selected */
        bool isAttachmentSelected(Attachment* p) const;

        /** Get file name of attachment.
            \param p Handle
            \return file name */
        String_t getAttachmentName(Attachment* p) const;

        /** Get size of attachment.
            \param p Handle
            \return size in bytes */
        size_t getAttachmentSize(Attachment* p) const;

        /** Get set of kinds of all attachments.
            \return Set of all kinds */
        Kinds_t getAllAttachmentKinds() const;

        /** Get timestamp of attachments.
            \return timestamp; value is unspecified if getNumAttachments()==0. */
        const Timestamp& getTimestamp() const;

        /** Format Kind value as string.
            \param k Value to format
            \param tx Translator */
        static String_t toString(Kind k, afl::string::Translator& tx);

     private:
        Kinds_t m_acceptableKinds;

        Timestamp m_timestamp;

        afl::container::PtrVector<Attachment> m_attachments;

        Attachment* createAttachment(String_t name, afl::sys::LogListener& log, afl::string::Translator& tx);

        bool checkTimestamp(const Timestamp& ts);
    };

} }

#endif
