/**
  *  \file server/interface/talkpm.hpp
  *  \brief Class server::interface::TalkPM
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPM_HPP
#define C2NG_SERVER_INTERFACE_TALKPM_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "server/interface/talkrender.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Interface for accessing Personal Mail. */
    class TalkPM : public afl::base::Deletable {
     public:
        static const int32_t PMStateRead      = 1;
        static const int32_t PMStateReplied   = 2;
        static const int32_t PMStateForwarded = 4;

        typedef TalkRender::Options Options;

        /** Information about a message. */
        struct Info {
            String_t author;                                   ///< Author user Id.
            String_t receivers;                                ///< Receiver list (comma-separated).
            Time_t time;                                       ///< Time of message.
            String_t subject;                                  ///< Subject.
            int32_t flags;                                     ///< Flags (PMState...).
            afl::base::Optional<int32_t> parent;               ///< Parent message Id.
            afl::base::Optional<String_t> parentSubject;       ///< Parent message subject.
            afl::base::Optional<int32_t> parentFolder;         ///< Folder containing parent message, if known.
            afl::base::Optional<String_t> parentFolderName;    ///< Name of folder containing parent message.
            afl::base::Optional<int32_t> suggestedFolder;      ///< Suggested folder to move message to, if known.
            afl::base::Optional<String_t> suggestedFolderName; ///< Name of suggested folder.

            Info()
                : author(), receivers(), time(), subject(), flags(),
                  parent(), parentSubject(), parentFolder(), parentFolderName(), suggestedFolder(), suggestedFolderName()
                { }
        };

        /** Create a message (PMNEW).
            @param receivers   Receiver list (comma-separated)
            @param subject     Message subject
            @param text        Message text
            @param parent      Parent message (=message we're replying to)
            @return message Id */
        virtual int32_t create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent) = 0;

        /** Describe message (PMSTAT).
            @param folder Folder Id
            @param pmid   Message Id
            @return Information */
        virtual Info getInfo(int32_t folder, int32_t pmid) = 0;

        /** Describe messages (PMMSTAT).
            @param [in]  folder   Folder Id
            @param [in]  pmids    Message Ids
            @param [out] results  Information; null for nonexistant messages */
        virtual void getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results) = 0;

        /** Copy messages (PMCP).
            A message is copied if it does exist in the source folder, but not in the target folder.
            It is not an error if these preconditions are not fulfilled.
            @param sourceFolder  Copy from this folder
            @param destFolder    Copy to this folder
            @param pmids         Message Ids to copy
            @return Number of messages copied */
        virtual int32_t copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids) = 0;

        /** Move messages (PMMV).
            A message is moved if it does exist in the source folder, but not in the target folder.
            It is not an error if these preconditions are not fulfilled.
            @param sourceFolder  Move from this folder
            @param destFolder    Move to this folder
            @param pmids         Message Ids to copy
            @return Number of messages moved */
        virtual int32_t move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids) = 0;

        /** Remove messages (PMRM).
            A message is removed if it exists in the given folder.
            @param folder  Remove from this folder
            @param pmids   Message Ids to remove
            @return Number of messages removed */
        virtual int32_t remove(int32_t folder, afl::base::Memory<const int32_t> pmids) = 0;

        /** Render message (PMRENDER).
            @param folder  Folder Id
            @param pmid    Message Id
            @param options Render options
            @return rendered message */
        virtual String_t render(int32_t folder, int32_t pmid, const Options& options) = 0;

        /** Render messages (PMMRENDER).
            @param [in]  folder Folder Id
            @param [in]  pmids  Message Ids
            @param [out] result Rendered messages; null for nonexistant messages */
        virtual void render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result) = 0;

        /** Change flags (PMFLAG).
            @param folder       Folder Id
            @param flagsToClear Clear these flags
            @param flagsToSet   Set these flags
            @param pmids        Process these messages
            @return Number of messages affected */
        virtual int32_t changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids) = 0;
    };

} }

#endif
