/**
  *  \file server/file/fileitem.hpp
  */
#ifndef C2NG_SERVER_FILE_FILEITEM_HPP
#define C2NG_SERVER_FILE_FILEITEM_HPP

#include "server/file/item.hpp"
#include "afl/base/optional.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file {

    class FileItem : public Item {
     public:
        explicit FileItem(const DirectoryHandler::Info& itemInfo);

        const DirectoryHandler::Info& getInfo() const;
        void setInfo(const DirectoryHandler::Info& info);

     private:
        DirectoryHandler::Info m_info;
    };

} }

#endif
