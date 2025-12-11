/**
  *  \file server/interface/filesnapshotserver.cpp
  *  \brief Class server::interface::FileSnapshotServer
  */

#include "server/interface/filesnapshotserver.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Vector;
using afl::data::VectorValue;

server::interface::FileSnapshotServer::FileSnapshotServer(FileSnapshot& impl)
    : m_impl(impl)
{ }

server::interface::FileSnapshotServer::~FileSnapshotServer()
{ }

bool
server::interface::FileSnapshotServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SNAPSHOTADD") {
        /* @q SNAPSHOTADD name:Str (File Command)
           Create a snapshot of the current file system.
           @err 400 Bad request (invalid snapshot name)
           @err 500 Internal error (feature not implemented/not available)
           @since PCC2 2.41.5 */
        args.checkArgumentCount(1);
        String_t name = toString(args.getNext());
        m_impl.createSnapshot(name);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SNAPSHOTCP") {
        /* @q SNAPSHOTCP from:Str to:Str (File Command)
           Copy a snapshot.
           @err 400 Bad request (invalid snapshot name)
           @err 404 Not found (from name does not exist)
           @err 500 Internal error (feature not implemented/not available)
           @since PCC2 2.41.5  */
        args.checkArgumentCount(2);
        String_t oldName = toString(args.getNext());
        String_t newName = toString(args.getNext());
        m_impl.copySnapshot(oldName, newName);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SNAPSHOTRM") {
        /* @q SNAPSHOTRM name:Str (File Command)
           Remove a snapshot.
           If the snapshot does not exist, this is a no-op.
           @err 400 Bad request (invalid snapshot name)
           @err 500 Internal error (feature not implemented/not available)
           @since PCC2 2.41.5  */
        args.checkArgumentCount(1);
        String_t name = toString(args.getNext());
        m_impl.removeSnapshot(name);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SNAPSHOTLS") {
        /* @q SNAPSHOTLS (File Command)
           List snapshots.
           @retval StrList List of names
           @since PCC2 2.41.5  */
        args.checkArgumentCount(0);

        afl::data::StringList_t list;
        m_impl.listSnapshots(list);

        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(list);

        result.reset(new VectorValue(vec));
        return true;
    } else {
        return false;
    }
}
