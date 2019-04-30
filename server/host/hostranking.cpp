/**
  *  \file server/host/hostranking.cpp
  */

#include <memory>
#include <stdexcept>
#include "server/host/hostranking.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/access.hpp"
#include "server/errors.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "server/host/root.hpp"

using afl::data::Access;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

namespace {
    struct Field {
        const char* name;       ///< Name in interface (sortField, fieldsToGet).
        const char* dbName;     ///< Name in database.
        bool isAlphabetic;      ///< true if alphabetic.
    };

    const Field FIELDS[] = {
        { "name",            "user:*:name",                     true },
        { "screenname",      "user:*:profile->screenname",      true },
        { "rank",            "user:*:profile->rank",            false },
        { "rankpoints",      "user:*:profile->rankpoints",      false },
        { "turnreliability", "user:*:profile->turnreliability", false },
        { "turnsplayed",     "user:*:profile->turnsplayed",     false },
        { "turnsmissed",     "user:*:profile->turnsmissed",     false },
    };

    const Field& findField(const String_t& name, const char* errorMessage)
    {
        afl::base::Memory<const Field> fs = FIELDS;
        while (const Field* p = fs.eat()) {
            if (p->name == name) {
                return *p;
            }
        }
        throw std::runtime_error(errorMessage);
    }
}

server::host::HostRanking::HostRanking(Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

server::Value_t*
server::host::HostRanking::getUserList(const ListRequest& req)
{
    // Build a sort request
    afl::net::redis::SortOperation op = m_root.activeUsers().sort().get();

    // Sort
    if (const String_t* sortKey = req.sortField.get()) {
        const Field& sortField = findField(*sortKey, INVALID_SORT_KEY);
        op.by(sortField.dbName);
        if (sortField.isAlphabetic) {
            op.sortLexicographical();
        }
    }
    if (req.sortReverse) {
        op.sortReversed();
    }

    // Additional fields
    for (size_t i = 0, n = req.fieldsToGet.size(); i < n; ++i) {
        op.get(findField(req.fieldsToGet[i], INVALID_KEY).dbName);
    }

    // Do it
    std::auto_ptr<Value_t> dbResult(op.getResult());
    Access dbAccess(dbResult);
    size_t limit = dbAccess.getArraySize();
    size_t index = 0;

    // Produce output
    Vector::Ref_t resultVector = Vector::create();
    while (index < limit) {
        String_t userId = dbAccess[index++].toString();
#if 0
        /* Variant: return the field names with each user.
           On the test installation (12 active users, 2 fields), this takes 118 us per iteration. */
        Hash::Ref_t userHash = Hash::create();
        for (size_t i = 0, n = req.fieldsToGet.size(); i < n; ++i) {
            userHash->set(req.fieldsToGet[i], dbAccess[index++].getValue());
        }
        resultVector->pushBackNew(makeStringValue(userId));
        resultVector->pushBackNew(new HashValue(userHash));
#else
        /* Public version: do not return field names (caller knows them).
           Takes 98 us (=17% less) than the variant. */
        Vector::Ref_t userVector = Vector::create();
        for (size_t i = 0, n = req.fieldsToGet.size(); i < n; ++i) {
            userVector->pushBack(dbAccess[index++].getValue());
        }
        resultVector->pushBackNew(makeStringValue(userId));
        resultVector->pushBackNew(new VectorValue(userVector));
#endif
    }
    return new VectorValue(resultVector);
}
