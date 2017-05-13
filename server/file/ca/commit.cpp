/**
  *  \file server/file/ca/commit.cpp
  *  \brief Class server::file::ca::Commit
  */

#include "server/file/ca/commit.hpp"

// Default constructor.
server::file::ca::Commit::Commit()
    : m_treeId()
{ }

// Construct from tree Id.
server::file::ca::Commit::Commit(const ObjectId& treeId)
    : m_treeId(treeId)
{ }

// Parse a commit object.
bool
server::file::ca::Commit::parse(afl::base::ConstBytes_t in)
{
    // tree always is first, so this is simple
    if (!in.split(5).equalContent(afl::string::toBytes("tree "))) {
        return false;
    }

    String_t name = afl::string::fromBytes(in.split(2*sizeof(ObjectId)));
    m_treeId = ObjectId::fromHex(name);
    if (name != m_treeId.toHex()) {
        return false;
    }
    return true;
}

// Store into commit object.
void
server::file::ca::Commit::store(afl::base::GrowableMemory<uint8_t>& out) const
{
    // This is the minimum commit causing 'git fsck' to not complain.
    out.append(afl::string::toBytes("tree "));
    out.append(afl::string::toBytes(m_treeId.toHex()));
    out.append(afl::string::toBytes("\n"
                                    "author c2file <> 1 +0000\n"
                                    "committer c2file <> 1 +0000\n"
                                    "\nc2file commit\n"));
}

// Get tree Id.
const server::file::ca::ObjectId&
server::file::ca::Commit::getTreeId() const
{
    return m_treeId;
}
