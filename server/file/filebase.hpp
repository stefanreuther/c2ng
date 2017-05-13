/**
  *  \file server/file/filebase.hpp
  *  \brief Class server::file::FileBase
  */
#ifndef C2NG_SERVER_FILE_FILEBASE_HPP
#define C2NG_SERVER_FILE_FILEBASE_HPP

#include "server/interface/filebase.hpp"

namespace server { namespace file {

    class Session;
    class Root;
    class Item;

    /** Implementation of FileBase interface for c2file server. */
    class FileBase : public server::interface::FileBase {
     public:
        /** Constructor.
            \param session Session object (provides user context)
            \param root Root (provides file space, logging, config) */
        FileBase(Session& session, Root& root);

        // FileBase operations
        virtual void copyFile(String_t sourceFile, String_t destFile);
        virtual void forgetDirectory(String_t dirName);
        virtual void testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags);
        virtual String_t getFile(String_t fileName);
        virtual void getDirectoryContent(String_t dirName, ContentInfoMap_t& result);
        virtual void getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result);
        virtual void createDirectory(String_t dirName);
        virtual void createDirectoryTree(String_t dirName);
        virtual void createDirectoryAsUser(String_t dirName, String_t userId);
        virtual afl::data::Value* getDirectoryProperty(String_t dirName, String_t propName);
        virtual void setDirectoryProperty(String_t dirName, String_t propName, String_t propValue);
        virtual void putFile(String_t fileName, String_t content);
        virtual void removeFile(String_t fileName);
        virtual void removeDirectory(String_t dirName);
        virtual void setDirectoryPermissions(String_t dirName, String_t userId, String_t permission);
        virtual Info getFileInformation(String_t fileName);
        virtual Usage getDiskUsage(String_t dirName);

     private:
        void createDirectoryCommon(String_t dirName, String_t userId);
        Info describeItem(Item& it);

        Session& m_session;
        Root& m_root;
    };

} }

#endif
