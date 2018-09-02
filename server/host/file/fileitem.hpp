/**
  *  \file server/host/file/fileitem.hpp
  */
#ifndef C2NG_SERVER_HOST_FILE_FILEITEM_HPP
#define C2NG_SERVER_HOST_FILE_FILEITEM_HPP

#include "server/host/file/item.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace host { namespace file {

    class FileItem : public Item {
     public:
        FileItem(afl::net::CommandHandler& filer, String_t fullName, String_t userName, Info_t info);

        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

        static void listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, ItemVector_t& out);
        static void listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, const afl::data::StringList_t& filter, ItemVector_t& out);

     private:
        afl::net::CommandHandler& m_filer;
        String_t m_fullName;
        String_t m_userName;
        Info_t m_info;
    };

} } }

#endif
