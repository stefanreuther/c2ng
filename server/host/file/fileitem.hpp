/**
  *  \file server/host/file/fileitem.hpp
  *  \brief Class server::host::file::FileItem
  */
#ifndef C2NG_SERVER_HOST_FILE_FILEITEM_HPP
#define C2NG_SERVER_HOST_FILE_FILEITEM_HPP

#include "afl/data/stringlist.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/host/file/item.hpp"

namespace server { namespace host { namespace file {

    /** File in c2host's virtual filespace.

        Represents a file node.
        All meta-information is provided by the caller.
        Content is retrieved from the actual filer instance on request. */
    class FileItem : public Item {
     public:
        /** Constructor.
            @param filer       CommandHandler for filer
            @param fullName    Full name of the file on the filer (for FileBase::getFile())
            @param userName    User name on the filer, for access checking (for Base::setUserContext())
            @param info        Meta-information to publish in getInfo() */
        FileItem(afl::net::CommandHandler& filer, String_t fullName, String_t userName, Info_t info);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

        /** List file server content, unfiltered.
            Produces an ItemVector_t containing a FileItem for each file in the filer.
            @param [in]  filer     CommandHandler for filer
            @param [in]  pathName  Path name on filer (for FileBase::getDirectoryContent())
            @param [in]  userName  User name on the filer (for Base::setUserContext())
            @param [out] out       Result */
        static void listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, ItemVector_t& out);

        /** List file server content, filtered.
            Produces an ItemVector_t containing a FileItem for each file in the filer that appears on the filter.
            @param [in]  filer     CommandHandler for filer
            @param [in]  pathName  Path name on filer (for FileBase::getDirectoryContent())
            @param [in]  userName  User name on the filer (for Base::setUserContext())
            @param [in]  filter    List of file names (without path names); only files on this list are returned
            @param [out] out       Result */
        static void listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, const afl::data::StringList_t& filter, ItemVector_t& out);

     private:
        afl::net::CommandHandler& m_filer;
        const String_t m_fullName;
        const String_t m_userName;
        const Info_t m_info;
    };

} } }

#endif
