/**
  *  \file server/interface/filebaseclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FILEBASECLIENT_HPP
#define C2NG_SERVER_INTERFACE_FILEBASECLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/filebase.hpp"

namespace server { namespace interface {

    class FileBaseClient : public FileBase {
     public:
        FileBaseClient(afl::net::CommandHandler& commandHandler);
        ~FileBaseClient();

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

        static Info unpackInfo(const afl::data::Value* p);

     public:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
