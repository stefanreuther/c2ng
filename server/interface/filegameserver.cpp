/**
  *  \file server/interface/filegameserver.cpp
  *  \brief Class server::interface::FileGameServer
  */

#include <stdexcept>
#include "server/interface/filegameserver.hpp"
#include "server/errors.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "interpreter/arguments.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::FileGameServer::FileGameServer(FileGame& impl)
    : m_implementation(impl)
{ }

server::interface::FileGameServer::~FileGameServer()
{ }

bool
server::interface::FileGameServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // ex Connection::handleRequest, sort-of
    if (upcasedCommand == "STATGAME") {
        /* @q STATGAME dir:FileName (File Command)
           Game information for one directory.
           @err 404 Not found (directory does not exist or does not contain a game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval FileGameInfo Information about this directory */
        args.checkArgumentCount(1);
        FileGame::GameInfo gi;
        m_implementation.getGameInfo(toString(args.getNext()), gi);
        result.reset(packGameInfo(gi));
        return true;
    } else if (upcasedCommand == "LSGAME") {
        /* @q LSGAME dir:FileName (File Command)
           Game information, recursively
           @err 404 Not found (directory does not exist game)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval FileGameInfo[] All games in this directory and its subdirectories */
        args.checkArgumentCount(1);
        afl::container::PtrVector<FileGame::GameInfo> gis;
        m_implementation.listGameInfo(toString(args.getNext()), gis);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = gis.size(); i < n; ++i) {
            if (FileGame::GameInfo* gi = gis[i]) {
                vec->pushBackNew(packGameInfo(*gi));
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "STATREG") {
        /* @q STATREG dir:FileName (File Command)
           Get registration information
           @err 404 Not found (directory does not exist or no registration key)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval FileRegInfo Information about this directory */
        args.checkArgumentCount(1);
        FileGame::KeyInfo gi;
        m_implementation.getKeyInfo(toString(args.getNext()), gi);
        result.reset(packKeyInfo(gi));
        return true;
    } else if (upcasedCommand == "LSREG") {
        /* @q LSREG dir:FileName [UNIQ] [ID id:Str] (File Command)
           List registrations, recursively.

           With option UNIQ, list only unique entries (but include use count).
           With option ID, list only entries matching the given key Id.
           Both options supported since 2.40.9.

           @err 404 Not found (directory does not exist)
           @err 403 Forbidden (insufficient permissions)
           @err 400 Bad request (invalid file name)
           @retval FileRegInfo[] All registrations in this directory and its subdirectories. */
        args.checkArgumentCountAtLeast(1);

        FileGame::Filter filter;
        String_t dirName = toString(args.getNext());
        while (args.getNumArgs() != 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "UNIQ") {
                filter.unique = true;
            } else if (key == "ID") {
                args.checkArgumentCountAtLeast(1);
                filter.keyId = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        afl::container::PtrVector<FileGame::KeyInfo> gis;
        m_implementation.listKeyInfo(dirName, filter, gis);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = gis.size(); i < n; ++i) {
            if (FileGame::KeyInfo* gi = gis[i]) {
                vec->pushBackNew(packKeyInfo(*gi));
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else {
        return false;
    }
}

server::interface::FileGameServer::Value_t*
server::interface::FileGameServer::packGameInfo(const FileGame::GameInfo& info)
{
    // ex Connection::doStatGame (part)
    /* @type FileGameInfo
       Game information
       @key path:Str         (game directory name)
       @key name:Str         (game name**)
       @key game:GID         (game ID**)
       @key finished:Int     (0=running, 1=finished**)
       @key hosttime:Int     (next host time**)
       @key races:StrHash    (maps race numbers to race names for all played races)
       @key missing:StrList  (names of missing/not uploaded game files)
       @key conflict:IntList (list of races that have conflicting data)

       ** These are actually directory properties provided by the Host service (see {PROPSET}).
       Therefore, the "unset" value is an empty string, not 0. */
    Hash::Ref_t h = Hash::create();
    h->setNew("path", makeStringValue(info.pathName));
    h->setNew("name", makeStringValue(info.gameName));
    h->setNew("game", makeIntegerValue(info.gameId));
    h->setNew("hosttime", makeIntegerValue(info.hostTime));
    h->setNew("finished", makeIntegerValue(info.isFinished));

    {
        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = info.slots.size(); i < n; ++i) {
            vec->pushBackNew(makeIntegerValue(info.slots[i].first));
            vec->pushBackNew(makeStringValue(info.slots[i].second));
        }
        h->setNew("races", new VectorValue(vec));
    }
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(info.missingFiles);
        h->setNew("missing", new VectorValue(vec));
    }
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(info.conflictSlots);
        h->setNew("conflict", new VectorValue(vec));
    }
    return new HashValue(h);
}

server::interface::FileGameServer::Value_t*
server::interface::FileGameServer::packKeyInfo(const FileGame::KeyInfo& info)
{
    // ex Connection::doStatReg
    /* @type FileRegInfo
       Registration information.
       @key path:FileName  (directory name)
       @key file:FileName  (registration key file name)
       @key reg:Int        (0=unregistered, 1=registered)
       @key key1:Str       (registration key first line)
       @key key2:Str       (registration key second line)
       @key useCount:Int   (optional; number of uses, set with LSREG...UNIQ; since 2.40.9)
       @key id:Str         (optional; key Id; since 2.40.9) */
    Hash::Ref_t h = Hash::create();
    h->setNew("path", makeStringValue(info.pathName));
    h->setNew("file", makeStringValue(info.fileName));
    h->setNew("reg",  makeIntegerValue(info.isRegistered));
    h->setNew("key1", makeStringValue(info.label1));
    h->setNew("key2", makeStringValue(info.label2));
    if (const int32_t* p = info.useCount.get()) {
        h->setNew("useCount", makeIntegerValue(*p));
    }
    if (const String_t* p = info.keyId.get()) {
        h->setNew("id", makeStringValue(*p));
    }
    return new HashValue(h);
}
