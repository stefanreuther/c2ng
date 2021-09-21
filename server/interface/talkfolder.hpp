/**
  *  \file server/interface/talkfolder.hpp
  *  \brief Interface server::interface::TalkFolder
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFOLDER_HPP
#define C2NG_SERVER_INTERFACE_TALKFOLDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/integerlist.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace interface {

    /** Mail Folder Interface.
        This interface allows modification and inquiry of users' mail folders. */
    class TalkFolder : public afl::base::Deletable {
     public:
        /** Information about a folder. */
        struct Info {
            String_t name;           ///< Name of folder ("Inbox").
            String_t description;    ///< Description/subtitle of folder.
            int32_t numMessages;     ///< Total number of messages in folder.
            bool hasUnreadMessages;  ///< true if folder has unread messages (folder attribute independant of messages).
            bool isFixedFolder;      ///< true if folder is fixed (=cannot be deleted).
        };

        /** Filter parameters. */
        struct FilterParameters {
            int32_t flagMask;        ///< Bits to check in each message's flags.
            int32_t flagCheck;       ///< Expected value in message's flags.
            FilterParameters()
                : flagMask(0), flagCheck(0)
                { }
            bool hasFlags() const
                { return flagMask != 0 || flagCheck != 0; }
        };

        /** List reporting parameters. */
        typedef TalkForum::ListParameters ListParameters;

        /** Get list of folders (FOLDERLS).
            \param [out] result List of folder Ids */
        virtual void getFolders(afl::data::IntegerList_t& result) = 0;

        /** Get information about single folder (FOLDERSTAT).
            \param ufid Folder Id
            \return Folder information */
        virtual Info getInfo(int32_t ufid) = 0;

        /** Get information about multiple folders (FOLDERMSTAT).
            \param [in]  ufids   Folder Ids
            \param [out] results Folder information; null for nonexistant folders */
        virtual void getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results) = 0;

        /** Create new folder (FOLDERNEW).
            \param name  Folder name
            \param args  Parameters (keys+values), see configure().
            \return Folder Id */
        virtual int32_t create(String_t name, afl::base::Memory<const String_t> args) = 0;

        /** Remove folder (FOLDERRM).
            \param ufid Folder Id
            \retval true success
            \retval false folder did not exist or cannot be deleted */
        virtual bool remove(int32_t ufid) = 0;

        /** Configure folder (FOLDERSET).
            \param ufid Folder Id
            \param args Parameters (keys+values). In particular,
                        - name (folder name)
                        - description (description/subtitle)
                        - unread (has-unread-messages flag) */
        virtual void configure(int32_t ufid, afl::base::Memory<const String_t> args) = 0;

        /** Access list of PMs (FOLDERLSPM).

            Depending on list parameters, returns a list of folder Ids (WantAll, WantRange),
            a number (WantSize), or a flag (WantMemberCheck).

            \param ufid   Folder Id
            \param params List parameters
            \param filter Filter parameters

            \return newly-allocated value */
        virtual afl::data::Value* getPMs(int32_t ufid, const ListParameters& params, const FilterParameters& filter) = 0;
    };

} }

#endif
