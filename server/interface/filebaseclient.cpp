/**
  *  \file server/interface/filebaseclient.cpp
  */

#include "server/interface/filebaseclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/types.hpp"

using afl::data::Segment;
using afl::data::Access;

server::interface::FileBaseClient::FileBaseClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{
    // ex FileClient::FileClient
}

server::interface::FileBaseClient::~FileBaseClient()
{ }

void
server::interface::FileBaseClient::copyFile(String_t sourceFile, String_t destFile)
{
    m_commandHandler.callVoid(Segment().pushBackString("CP").pushBackString(sourceFile).pushBackString(destFile));
}

void
server::interface::FileBaseClient::forgetDirectory(String_t dirName)
{
    // ex FileClient::forgetDirectory
    m_commandHandler.callVoid(Segment().pushBackString("FORGET").pushBackString(dirName));
}
void
server::interface::FileBaseClient::testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags)
{
    Segment cmd;
    cmd.pushBackString("FTEST");
    while (const String_t* p = fileNames.eat()) {
        cmd.pushBackString(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    Access(p).toIntegerList(resultFlags);
}

String_t
server::interface::FileBaseClient::getFile(String_t fileName)
{
    // ex FileClient::getFile
    return m_commandHandler.callString(Segment().pushBackString("GET").pushBackString(fileName));
}

void
server::interface::FileBaseClient::getDirectoryContent(String_t dirName, ContentInfoMap_t& result)
{
    // ex FileClient::getDirectoryContent (sort-of)
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("LS").pushBackString(dirName)));
    Access a(p);

    // The object is transferred as an array, so process it as such.
    // FIXME: we should dearly make a key/value iterator for Access.
    // This code fails when given an actual hash.
    // Accessing this using the hash interface would have O(n) complexity though.
    for (size_t i = 0, n = a.getArraySize(); i+1 < n; i += 2) {
        String_t name = a[i].toString();
        std::auto_ptr<Info> info(new Info(unpackInfo(a[i+1].getValue())));
        result.insertNew(name, info.release());
    }
}

void
server::interface::FileBaseClient::getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("LSPERM").pushBackString(dirName)));
    Access a(p);

    // Owner
    ownerUserId = a("owner").toString();

    // Permissions
    a = a("perms");
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        Access e(a[i]);
        result.push_back(Permission(e("user").toString(), e("perms").toString()));
    }
}

void
server::interface::FileBaseClient::createDirectory(String_t dirName)
{
    // ex FileClient::createDirectory
    m_commandHandler.callVoid(Segment().pushBackString("MKDIR").pushBackString(dirName));
}

void
server::interface::FileBaseClient::createDirectoryTree(String_t dirName)
{
    // ex FileClient::createDirectoryTree
    m_commandHandler.callVoid(Segment().pushBackString("MKDIRHIER").pushBackString(dirName));
}

void
server::interface::FileBaseClient::createDirectoryAsUser(String_t dirName, String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("MKDIRAS").pushBackString(dirName).pushBackString(userId));
}

afl::data::Value*
server::interface::FileBaseClient::getDirectoryProperty(String_t dirName, String_t propName)
{
    // ex FileClient::getStringProperty, FileClient::getIntProperty
    return m_commandHandler.call(Segment().pushBackString("PROPGET").pushBackString(dirName).pushBackString(propName));
}

void
server::interface::FileBaseClient::setDirectoryProperty(String_t dirName, String_t propName, String_t propValue)
{
    // ex FileClient::setProperty
    m_commandHandler.callVoid(Segment().pushBackString("PROPSET").pushBackString(dirName).pushBackString(propName).pushBackString(propValue));
}

void
server::interface::FileBaseClient::putFile(String_t fileName, String_t content)
{
    // ex FileClient::putFile
    m_commandHandler.callVoid(Segment().pushBackString("PUT").pushBackString(fileName).pushBackString(content));
}

void
server::interface::FileBaseClient::removeFile(String_t fileName)
{
    // ex FileClient::removeFile
    m_commandHandler.callVoid(Segment().pushBackString("RM").pushBackString(fileName));
}

void
server::interface::FileBaseClient::removeDirectory(String_t dirName)
{
    // ex FileClient::removeDirectory
    m_commandHandler.callVoid(Segment().pushBackString("RMDIR").pushBackString(dirName));
}

void
server::interface::FileBaseClient::setDirectoryPermissions(String_t dirName, String_t userId, String_t permission)
{
    // ex FileClient::setFilePermissions
    m_commandHandler.callVoid(Segment().pushBackString("SETPERM").pushBackString(dirName).pushBackString(userId).pushBackString(permission));
}

server::interface::FileBase::Info
server::interface::FileBaseClient::getFileInformation(String_t fileName)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("STAT").pushBackString(fileName)));
    return unpackInfo(p.get());
}

server::interface::FileBase::Usage
server::interface::FileBaseClient::getDiskUsage(String_t dirName)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("USAGE").pushBackString(dirName)));
    Access a(p);

    Usage result;
    result.numItems = a("files").toInteger();
    result.totalKBytes = a("kbytes").toInteger();
    return result;
}

server::interface::FileBase::Info
server::interface::FileBaseClient::unpackInfo(const afl::data::Value* p)
{
    Info result;
    Access a(p);

    // Type
    String_t type = a("type").toString();
    result.type = (type == "file" ? IsFile : type == "dir" ? IsDirectory : IsUnknown);

    // Size
    result.size = toOptionalInteger(a("size").getValue());

    // Visibilty
    result.visibility = toOptionalInteger(a("visibility").getValue());

    // Content Id
    result.contentId = toOptionalString(a("id").getValue());

    return result;
}
