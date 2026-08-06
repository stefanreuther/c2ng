/**
  *  \file server/file/fileshareimpl.cpp
  *  \brief Class server::file::FileShareImpl
  */

#include "server/file/fileshareimpl.hpp"

#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "server/file/filegame.hpp"
#include "server/file/gamestatus.hpp"
#include "server/file/pathresolver.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/file/sharestore.hpp"

using afl::sys::LogListener;
using afl::string::Format;

namespace {
    const char* const LOG_NAME = "file";
}

server::file::FileShareImpl::FileShareImpl(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

int32_t
server::file::FileShareImpl::getSequenceNumber(String_t userId)
{
    checkUserId(userId);
    ShareStore st;
    st.load(userId, m_root);

    return st.getSequenceNumber();
}

void
server::file::FileShareImpl::listDirectories(String_t userId, afl::container::PtrVector<Info>& result)
{
    checkUserId(userId);
    ShareStore st;
    st.load(userId, m_root);

    for (size_t i = 0, n = st.getNumShares(); i < n; ++i) {
        Info* out = result.pushBackNew(new Info());
        const ShareStore::Info* in = st.getShareByIndex(i);
        out->pathName = st.getSharePathName(in);
        out->owner = st.getShareOwner(in);
        out->seqNr = st.getShareSequenceNumber(in);
    }
}

void
server::file::FileShareImpl::listGameInfo(String_t userId, afl::container::PtrVector<server::interface::FileGame::GameInfo>& result)
{
    checkUserId(userId);
    ShareStore st;
    st.load(userId, m_root);

    for (size_t i = 0, n = st.getNumShares(); i < n; ++i) {
        String_t pathName = st.getSharePathName(st.getShareByIndex(i));
        try {
            // Resolve path (throws on error)
            PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
            DirectoryItem& dir = res.resolveToDirectory(pathName, DirectoryItem::AllowRead);
            dir.readContent(m_root);

            // Read status
            GameStatus& status = dir.readGameStatus(m_root);
            if (const GameStatus::GameInfo* info = status.getGameInfo()) {
                FileGame::GameInfo* out = result.pushBackNew(new FileGame::GameInfo());
                FileGame::copyGameInfo(*out, *info, pathName, dir);
            }
        }
        catch (std::exception& e) {
            m_root.log().write(LogListener::Warn, LOG_NAME, Format("Error reading game info from %s", pathName), e);
        }
    }
}

void
server::file::FileShareImpl::checkUserId(const String_t& userId) const
{
    if (!m_session.isAdmin() && userId != m_session.getUser()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}
