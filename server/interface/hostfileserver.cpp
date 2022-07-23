/**
  *  \file server/interface/hostfileserver.cpp
  */

#include <memory>
#include "server/interface/hostfileserver.hpp"
#include "server/interface/filebaseserver.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::HostFileServer::HostFileServer(HostFile& impl)
    : m_implementation(impl)
{ }

server::interface::HostFileServer::~HostFileServer()
{ }

bool
server::interface::HostFileServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "LS") {
        /* @q LS dir:Str (Host Command)
           List files.
           Returns a list of {@type HostFileStat} elements.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @rettype HostFileStat */
        args.checkArgumentCount(1);
        String_t dirName = toString(args.getNext());
        HostFile::InfoVector_t infos;
        m_implementation.getDirectoryContent(dirName, infos);
        result.reset(packInfos(infos));
        return true;
    } else if (upcasedCommand == "GET") {
        /* @q GET dir:Str (Host Command)
           @retval Blob file content */
        args.checkArgumentCount(1);
        String_t fileName = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.getFile(fileName)));
        return true;
    } else if (upcasedCommand == "STAT") {
        /* @q STAT dir:Str (Host Command)
           Get file status.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval HostFileStat Information for requested file */
        args.checkArgumentCount(1);
        String_t fileName = toString(args.getNext());
        result.reset(packInfo(m_implementation.getFileInformation(fileName)));
        return true;
    } else if (upcasedCommand == "PSTAT") {
        /* @q PSTAT dir:Str (Host Command)
           Get path information.
           Returns a list of {@type HostFileStat} elements, one per path component,
           with the final component at the end.
           @err 404 Not found (file does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @rettype HostFileStat */
        args.checkArgumentCount(1);
        String_t dirName = toString(args.getNext());
        HostFile::InfoVector_t infos;
        m_implementation.getPathDescription(dirName, infos);
        result.reset(packInfos(infos));
        return true;
    } else {
        return false;
    }
}

afl::data::HashValue*
server::interface::HostFileServer::packInfo(const HostFile::Info& info)
{
    /* @type HostFileStat
       File information for file from host server.
       This is an expanded version of the {@type FileStat} structure.
       @key type:FileType             (type of file)
       @key visibility:FileVisibility (directories: visibility indicator)
       @key size:Int                  (files: size in bytes)
       @key id:Str                    (content id, optional)
       @key name:Str                  (file/directory name)
       @key label:Str                 (instructions how to create the label of file)
       @key turn:Int                  (turn number, optional)
       @key slot:Int                  (slot number, optional)
       @key slotname:Str              (slot name, optional)
       @key game:Int                  (game number, optional)
       @key gamename:Str              (game name, optional)
       @key toolname:Str              (tool name, optional) */

    // Pack FileBase part
    std::auto_ptr<HashValue> pp(FileBaseServer::packInfo(info));
    Hash::Ref_t result = pp->getValue();

    // Pack HostFile part
    result->setNew("name", makeStringValue(info.name));
    result->setNew("label", makeStringValue(HostFile::formatLabel(info.label)));
    addOptionalIntegerKey(*result, "turn", info.turnNumber);
    addOptionalIntegerKey(*result, "slot", info.slotId);
    addOptionalStringKey(*result, "slotname", info.slotName);
    addOptionalIntegerKey(*result, "game", info.gameId);
    addOptionalStringKey(*result, "gamename", info.gameName);
    addOptionalStringKey(*result, "toolname", info.toolName);

    return pp.release();
}

afl::data::VectorValue*
server::interface::HostFileServer::packInfos(afl::base::Memory<const HostFile::Info> infos)
{
    Vector::Ref_t result = Vector::create();
    while (const HostFile::Info* p = infos.eat()) {
        // Packing the name separately is redundant, but makes FileBase able to talk to HostFile.
        result->pushBackNew(makeStringValue(p->name));
        result->pushBackNew(packInfo(*p));
    }
    return new VectorValue(result);
}
