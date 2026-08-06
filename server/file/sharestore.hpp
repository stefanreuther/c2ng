/**
  *  \file server/file/sharestore.hpp
  *  \brief Class server::file::ShareStore
  */
#ifndef C2NG_SERVER_FILE_SHARESTORE_HPP
#define C2NG_SERVER_FILE_SHARESTORE_HPP

#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"

namespace server { namespace file {

    class Root;

    /** Storage for incoming file shares.
        Permissions for a directory ("outgoing file shares") are stored in properties of the directory itself.
        To inform users that they can access another user's directory, we store a list of accessible directories
        ("incoming file shares") in a separate file under "_share/&lt;userId&gtM".
        This class manages these files.

        To use,
        - create
        - call load() to load an existing file
        - query or modify
        - call save() or saveIfModified() if desired */
    class ShareStore {
     public:
        struct Info;

        /** Constructor.
            Makes an empty ShareStore. */
        ShareStore();

        /** Destructor. */
        ~ShareStore();

        /** Load from storage.
            This function never fails; errors are logged.
            @param userId  User Id (=name of file in ".share")
            @param root    Service root; provides rootDirectory() and logging */
        void load(String_t userId, Root& root);

        /** Save to storage.
            This function never fails; errors are logged.
            @param userId  User Id (=name of file in ".share")
            @param root    Service root; provides rootDirectory() and logging */
        void save(String_t userId, Root& root) const;

        /** Save if modified.
            Like save(), but is a no-op if addShare() or removeShare() has not been called since load().
            This function never fails; errors are logged.
            @param userId  User Id (=name of file in ".share")
            @param root    Service root; provides rootDirectory() and logging */
        void saveIfModified(String_t userId, Root& root);

        /** Get current sequence number.
            This is the most recently-used sequence number of any file share,
            and can used to implement a "you have new shares" notification.
            @return sequence number
            @see server::interface::FileShare::getSequenceNumber() */
        int32_t getSequenceNumber() const;

        /** Add file share.
            If this file share is already known, this function is a no-op.
            @param pathName  Path name
            @param owner     Owner (=user sharing the file) */
        void addShare(String_t pathName, String_t owner);

        /** Remove a file share.
            If this file share does not exist, this function is a no-op.
            @param pathName  Path name */
        void removeShare(String_t pathName);

        /** Get number of file shares.
            @return count */
        size_t getNumShares() const;

        /** Get file share, given an index.
            @param idx Index [0,getNumShares())
            @return Handle; null if idx is out of range */
        const Info* getShareByIndex(size_t idx) const;

        /** Get path name of a share.
            @param p Handle
            @return path name; empty string if p is null */
        String_t getSharePathName(const Info* p) const;

        /** Get owner of a share.
            @param p Handle
            @return owner; empty string if p is null */
        String_t getShareOwner(const Info* p) const;

        /** Get sequence number of a share.
            @param p Handle
            @return sequence number; 0 if p is null */
        int32_t getShareSequenceNumber(const Info* p) const;

     private:
        afl::container::PtrVector<Info> m_shares;
        int32_t m_sequenceNumber;
        bool m_changed;

        size_t findShareByPathName(const String_t& pathName) const;
        void handleLine(const String_t& userId, Root& root, const String_t& line);
        void saveContent(afl::io::Stream& out) const;
    };

} }


#endif
