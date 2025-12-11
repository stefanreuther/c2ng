/**
  *  \file server/interface/filesnapshot.hpp
  *  \brief Interface server::interface::FileSnapshot
  */
#ifndef C2NG_SERVER_INTERFACE_FILESNAPSHOT_HPP
#define C2NG_SERVER_INTERFACE_FILESNAPSHOT_HPP

#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace server { namespace interface {

    /** File Server Snapshot Interface.
        This interface allows access to snapshots. */
    class FileSnapshot : public afl::base::Deletable {
     public:
        /** Create a snapshot.
            @param name Name of snapshot */
        virtual void createSnapshot(String_t name) = 0;

        /** Copy snapshot.
            @param oldName Old (existing) name
            @param newName New (copy) name */
        virtual void copySnapshot(String_t oldName, String_t newName) = 0;

        /** Remove snapshot.
            @param name Name of snapshot */
        virtual void removeSnapshot(String_t name) = 0;

        /** Get names of snapshot.
            @param [out] out Result */
        virtual void listSnapshots(afl::data::StringList_t& out) = 0;
    };

} }

#endif
