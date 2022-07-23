/**
  *  \file server/interface/filebaseserver.cpp
  */

#include <stdexcept>
#include "server/interface/filebaseserver.hpp"
#include "interpreter/arguments.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::FileBaseServer::FileBaseServer(FileBase& impl)
    : m_implementation(impl)
{ }

server::interface::FileBaseServer::~FileBaseServer()
{ }

bool
server::interface::FileBaseServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // ex Connection::handleRequest, sort-of
    if (upcasedCommand == "CP") {
        /* @q CP from:FileName, to:FileName (File Command)
           Copies a file.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 450 Unable to create file (operating system error)
           @err 500 Internal error (operating system error)
           @err 413 File too large (file cannot be passed through file service; see {File.SizeLimit (Config)}) */
        args.checkArgumentCount(2);
        String_t sourceFile = toString(args.getNext());
        String_t destFile = toString(args.getNext());
        m_implementation.copyFile(sourceFile, destFile);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "FORGET") {
        /* @q FORGET dir:FileName (File Command)
           Forget cached information about a directory.
           This is used to tell the file server about an external change to the underlying files.
           This command cannot fail. */
        args.checkArgumentCount(1);
        m_implementation.forgetDirectory(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "FTEST") {
        /* @q FTEST file:FileName... (File Command)
           Quick file test.
           This command quickly checks accessability of a bunch of files.
           It returns one integer for each file: 0=does not exist or not readable, 1=exists and readable
           @retval IntList results */
        afl::data::StringList_t fileNames;
        while (args.getNumArgs() > 0) {
            fileNames.push_back(toString(args.getNext()));
        }

        afl::data::IntegerList_t resultFlags;
        m_implementation.testFiles(fileNames, resultFlags);

        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(resultFlags);
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "GET") {
        /* @q GET file:FileName (File Command)
           Get file content.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 500 Internal error (operating system error)
           @err 413 File too large (file cannot be passed through file service; see {File.SizeLimit (Config)})
           @retval Blob file content */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getFile(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "LS") {
        /* @q LS dir:FileName (File Command)
           List directory content.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval Hash keys are file/folder names, content is {@type FileStat} elements.
           @rettype FileStat */
        args.checkArgumentCount(1);

        afl::container::PtrMap<String_t, FileBase::Info> infos;
        m_implementation.getDirectoryContent(toString(args.getNext()), infos);

        // Create a vector, not a hash!
        // FileBaseClient assumes that so far.
        Vector::Ref_t vec = Vector::create();
        for (FileBase::ContentInfoMap_t::const_iterator it = infos.begin(); it != infos.end(); ++it) {
            vec->pushBackString(it->first);
            vec->pushBackNew(packInfo(*it->second));
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "LSPERM") {
        /* @q LSPERM dir:FileName (File Command)
           List permissions.
           @err 404 Not found (directory does not exist game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retkey owner:UID (owner of the directory)
           @retkey perms:List (list of hashes, each with keys %user:{@type UID} and %perms:{@type FilePermissions})
           @rettype UID
           @rettype FilePermissions */
        args.checkArgumentCount(1);

        String_t ownerUserId;
        std::vector<FileBase::Permission> perms;
        m_implementation.getDirectoryPermission(toString(args.getNext()), ownerUserId, perms);

        Vector::Ref_t permVector = Vector::create();
        for (size_t i = 0, n = perms.size(); i < n; ++i) {
            Hash::Ref_t h = Hash::create();
            h->setNew("user", makeStringValue(perms[i].userId));
            h->setNew("perms", makeStringValue(perms[i].permission));
            permVector->pushBackNew(new HashValue(h));
        }

        Hash::Ref_t h = Hash::create();
        h->setNew("owner", makeStringValue(ownerUserId));
        h->setNew("perms", new VectorValue(permVector));
        result.reset(new HashValue(h));
        return true;
    } else if (upcasedCommand == "MKDIR") {
        /* @q MKDIR dir:FileName (File Command)
           Create directory.
           The containing directory must already exist.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 409 Exists (directory already exists)
           @err 450 Failed (operating system failure) */
        args.checkArgumentCount(1);
        m_implementation.createDirectory(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "MKDIRHIER") {
        /* @q MKDIRHIER dir:FileName (File Command)
           Create directory hierarchy.
           Like {MKDIR}, but can create multiple levels at once,
           and does not fail if one component already exists.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 409 Exists (name already exists but is not a directory)
           @err 450 Failed (operating system failure) */
        args.checkArgumentCount(1);
        m_implementation.createDirectoryTree(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "MKDIRAS") {
        /* @q MKDIRAS dir:FileName, user:UID (File Command)
           Create a directory as user.
           The user will become owner of the new directory.
           This is used when creating new user accounts.
           The %user must not be an empty string.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 409 Exists (directory already exists)
           @err 450 Failed (operating system failure) */
        args.checkArgumentCount(2);
        String_t dirName = toString(args.getNext());
        String_t userId  = toString(args.getNext());
        m_implementation.createDirectoryAsUser(dirName, userId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PROPGET") {
        /* @q PROPGET dir:FileName, prop:Str (File Command)
           Get directory property.
           @err 404 Not found (directory does not exist game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval Str property value
           @see PROPSET */
        args.checkArgumentCount(2);
        String_t dirName = toString(args.getNext());
        String_t propName = toString(args.getNext());
        result.reset(m_implementation.getDirectoryProperty(dirName, propName));
        return true;
    } else if (upcasedCommand == "PROPSET") {
        /* @q PROPSET dir:FileName, prop:Str, value:Str (File Command)
           Set directory property.
           A directory can have an arbitrary number of properties attached.
           These store "invisible" meta-information.

           Property names can contain any character except "\r", "\n" and "=".
           Property values can contain any character except "\r", "\n".

           @err 404 Not found (directory does not exist game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name, property name, or value) */
        args.checkArgumentCount(3);
        String_t dirName = toString(args.getNext());
        String_t propName = toString(args.getNext());
        String_t propValue = toString(args.getNext());
        m_implementation.setDirectoryProperty(dirName, propName, propValue);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PUT") {
        /* @q PUT file:Str, content:Blob (File Command)
           Create file.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @err 450 Unable to create file (operating system error) */
        args.checkArgumentCount(2);
        String_t fileName = toString(args.getNext());
        String_t content  = toString(args.getNext());
        m_implementation.putFile(fileName, content);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "RM") {
        /* @q RM dir:FileName (File Command)
           Remove file or directory.
           A directory must be empty to be removable using RM.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @see RMDIR */
        args.checkArgumentCount(1);
        m_implementation.removeFile(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "RMDIR") {
        /* @q RMDIR dir:FileName (File Command)
           Remove directory, recursively.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @see RM */
        args.checkArgumentCount(1);
        m_implementation.removeDirectory(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SETPERM") {
        /* @q SETPERM dir:FileName, user:UID, perms:FilePermissions (File Command)
           Set directory permissions.
           The owner of a directory can grant permissions for other users to access their files.
           The special %user value "*" defines permissions for everyone (world permissions).
           @err 404 Not found (directory does not exist game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name) */
        args.checkArgumentCount(3);
        String_t dirName = toString(args.getNext());
        String_t userId = toString(args.getNext());
        String_t permission = toString(args.getNext());
        m_implementation.setDirectoryPermissions(dirName, userId, permission);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "STAT") {
        /* @q STAT file:FileName (File Command)
           Get file status.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval FileStat file information */
        args.checkArgumentCount(1);
        result.reset(packInfo(m_implementation.getFileInformation(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "USAGE") {
        /* @q USAGE dir:FileName (File Command)
           Get directory statistics
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retkey files:Int Number of files/directories
           @retkey kbytes:Int Disk usage in kilobytes */
        args.checkArgumentCount(1);

        FileBase::Usage usage = m_implementation.getDiskUsage(toString(args.getNext()));

        Hash::Ref_t h = Hash::create();
        h->setNew("files", makeIntegerValue(usage.numItems));
        h->setNew("kbytes", makeIntegerValue(usage.totalKBytes));
        result.reset(new HashValue(h));
        return true;
    } else {
        return false;
    }
}

afl::data::HashValue*
server::interface::FileBaseServer::packInfo(const FileBase::Info& info)
{
    /* @type FileStat
       File information.
       @key type:FileType             (type of file)
       @key visibility:FileVisibility (directories: visibility indicator)
       @key size:Int                  (files: size in bytes)
       @key id:Str                    (content id, optional)

       The content Id is optional.
       If a content Id is present and matches the Id from a previous query, the file is unchanged.

       See also {@type FileInfo}. */
    Hash::Ref_t result = Hash::create();
    switch (info.type) {
     case FileBase::IsFile:      result->setNew("type", makeStringValue("file"));    break;
     case FileBase::IsDirectory: result->setNew("type", makeStringValue("dir"));     break;
     case FileBase::IsUnknown:   result->setNew("type", makeStringValue("unknown")); break;
    }
    addOptionalIntegerKey(*result, "visibility", info.visibility);
    addOptionalIntegerKey(*result, "size", info.size);
    addOptionalStringKey(*result, "id", info.contentId);
    return new HashValue(result);
}
