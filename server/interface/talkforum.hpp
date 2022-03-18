/**
  *  \file server/interface/talkforum.hpp
  *  \brief Interface server::interface::TalkForum
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFORUM_HPP
#define C2NG_SERVER_INTERFACE_TALKFORUM_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Talk Forum interface.
        Allows to create, modify, and inquire forums. */
    class TalkForum : public afl::base::Deletable {
     public:
        /** Forum header information. */
        struct Info {
            String_t name;                ///< Name of forum (plain text).
            String_t parentGroup;         ///< Containing group name (groupId).
            String_t description;         ///< Description (rendered according to options).
            String_t newsgroupName;       ///< Newsgroup name; can be empty.
        };

        /** Forum size information. */
        struct Size {
            int32_t numThreads;           ///< Number of threads.
            int32_t numStickyThreads;     ///< Number of sticky threads.
            int32_t numMessages;          ///< Number of messages.
        };

        /** List parameters. */
        struct ListParameters {
            /** Overall mode.
                Determines type of result. */
            enum Mode {
                WantAll,                  ///< Get the whole list. Result is array of Ids.
                WantRange,                ///< Get subrange of list. Result is array of Ids. Uses start+count.
                WantSize,                 ///< Get size of list. Result is single integer.
                WantMemberCheck           ///< Check presence of an item. Uses item. Result is single boolean.
            };
            Mode mode;                    ///< Overall mode.
            int32_t start;                ///< (WantRange) First item to return, 0-based.
            int32_t count;                ///< (WantRange) Number of items to return.
            int32_t item;                 ///< (WantMemberCheck) Item to check.
            afl::base::Optional<String_t> sortKey;  ///< Sort key.

            ListParameters()
                : mode(WantAll),
                  start(0),
                  count(0),
                  item(0),
                  sortKey()
                { }
        };

        /** Add forum (FORUMADD).
            \param config Forum configuration parameters (see configure())
            \return forum Id */
        virtual int32_t add(afl::base::Memory<const String_t> config) = 0;

        /** Configure forum (FORUMSET).
            \param [in] fid Forum Id
            \param [out] config List of keys/values for configuration */
        virtual void configure(int32_t fid, afl::base::Memory<const String_t> config) = 0;

        /** Get forum configuration value (FORUMGET).
            \param fid Forum Id
            \param keyName Configuration key
            \return raw value */
        virtual afl::data::Value* getValue(int32_t fid, String_t keyName) = 0;

        /** Get forum information (FORUMSTAT).
            \param fid Forum Id
            \return forum information */
        virtual Info getInfo(int32_t fid) = 0;

        /** Get multiple forums information (FORUMMSTAT).
            \param [in] fids List of forum Ids
            \param [out] result Forum information, null for invalid fids */
        virtual void getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result) = 0;

        /** Get forum permissions (FORUMPERMS).
            \param fid Forum Id
            \param permissionList Permissions to query
            \return Permission bits */
        virtual int32_t getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList) = 0;

        /** Get forum size information (FORUMSIZE).
            \param fid Forum Id
            \return size information */
        virtual Size getSize(int32_t fid) = 0;

        /** List threads in forum (FORUMLSTHREAD).
            \param fid Forum Id
            \param params List parameters (sort key, result type, etc.)
            \return result depending on ListParameters */
        virtual afl::data::Value* getThreads(int32_t fid, const ListParameters& params) = 0;

        /** List sticky threads in forum (FORUMLSSTICKY).
            \param fid Forum Id
            \param params List parameters (sort key, result type, etc.)
            \return result depending on ListParameters */
        virtual afl::data::Value* getStickyThreads(int32_t fid, const ListParameters& params) = 0;

        /** List postings in forum (FORUMLSPOST).
            \param fid Forum Id
            \param params List parameters (sort key, result type, etc.)
            \return result depending on ListParameters */
        virtual afl::data::Value* getPosts(int32_t fid, const ListParameters& params) = 0;

        /** Get forum by well-known name (FORUMBYNAME).
            \param key Key
            \return forum Id, 0 if not found */
        virtual int32_t findForum(String_t key) = 0;


        /** Get integer value.
            Convenience wrapper for getValue().
            \param fid Forum Id
            \param keyName Configuration key
            \return value */
        int32_t getIntegerValue(int32_t fid, String_t keyName);

        /** Get string value.
            Convenience wrapper for getValue().
            \param fid Forum Id
            \param keyName Configuration key
            \return value */
        String_t getStringValue(int32_t fid, String_t keyName);
    };

} }

#endif
