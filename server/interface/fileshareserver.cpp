/**
  *  \file server/interface/fileshareserver.cpp
  *  \brief Class server::interface::FileShareServer
  */

#include "server/interface/fileshareserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/interface/filegameserver.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::FileShareServer::FileShareServer(FileShare& impl)
    : m_implementation(impl)
{ }

bool
server::interface::FileShareServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SHARESEQ") {
        /* @q SHARESEQ user:UID (File Command)
           Get serial number of sharing information for a user.
           If this number has changed, user has received new file shares.
           @retval Int serial number
           @since PCC2 2.41.5 */
        args.checkArgumentCount(1);
        String_t userId = toString(args.getNext());
        result.reset(makeIntegerValue(m_implementation.getSequenceNumber(userId)));
        return true;
    } else if (upcasedCommand == "SHARELS") {
        /* @q SHARELS user:UID (File Command)
           List directories shared to a user.
           @retval FileShareInfo[] Shared directories
           @since PCC2 2.41.5 */
        args.checkArgumentCount(1);
        String_t userId = toString(args.getNext());

        afl::container::PtrVector<FileShare::Info> infos;
        m_implementation.listDirectories(userId, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (const FileShare::Info* p = infos[i]) {
                vec->pushBackNew(packInfo(*p));
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "SHARELSGAME") {
        /* @q SHARELSGAME user:UID (File Command)
           List game directories shared to a user.
           @retval FileGameInfo[] All accessible games in shared directories
           @since PCC2 2.41.5
           @see STATGAME */
        args.checkArgumentCount(1);
        String_t userId = toString(args.getNext());

        afl::container::PtrVector<FileGame::GameInfo> infos;
        m_implementation.listGameInfo(userId, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (const FileGame::GameInfo* p = infos[i]) {
                vec->pushBackNew(FileGameServer::packGameInfo(*p));
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else {
        return false;
    }
}

afl::data::Value*
server::interface::FileShareServer::packInfo(const FileShare::Info& info)
{
    /* @type FileShareInfo
       Information about a shared directory.
       @key path:FileName (directory name)
       @key owner:UID      (user owning this directory)
       @key seqNr:Int     (serial number, increases for every newly-added share) */
    Hash::Ref_t h = Hash::create();
    h->setNew("path", makeStringValue(info.pathName));
    h->setNew("owner", makeStringValue(info.owner));
    h->setNew("seqNr", makeIntegerValue(info.seqNr));
    return new HashValue(h);
}
