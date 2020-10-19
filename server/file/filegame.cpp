/**
  *  \file server/file/filegame.cpp
  */

#include <stdexcept>
#include "server/file/filegame.hpp"
#include "server/file/pathresolver.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/errors.hpp"
#include "server/file/gamestatus.hpp"
#include "afl/string/parse.hpp"

namespace {
    int32_t safeToInteger(const String_t& val)
    {
        int32_t result = 0;
        if (!afl::string::strToInteger(val, result)) {
            result = 0;
        }
        return result;
    }

    void copyKeyInfo(server::interface::FileGame::KeyInfo& out,
                     const server::file::GameStatus::KeyInfo& in,
                     const String_t& path)
    {
        out.pathName = path;
        out.fileName = path + "/" + in.fileName;
        out.isRegistered = in.isRegistered;
        out.label1 = in.label1;
        out.label2 = in.label2;
        out.keyId = in.keyId;
    }

    void copyGameInfo(server::interface::FileGame::GameInfo& out,
                      const server::file::GameStatus::GameInfo& in,
                      const String_t& path,
                      const server::file::DirectoryItem& dir)
    {
        out.pathName     = path;
        out.gameName     = dir.getProperty("prop:name");
        out.gameId       = safeToInteger(dir.getProperty("prop:game"));
        out.hostTime     = safeToInteger(dir.getProperty("prop:hosttime"));
        out.isFinished   = safeToInteger(dir.getProperty("prop:finished")) != 0;
        out.slots        = in.slots;
        out.missingFiles = in.missingFiles;
        out.conflictSlots.clear();
    }

    bool matchKey(const server::interface::FileGame::Filter& filter, const server::file::GameStatus::KeyInfo& in)
    {
        if (const String_t* p = filter.keyId.get()) {
            if (in.keyId != *p) {
                return false;
            }
        }
        return true;
    }
}

server::file::FileGame::FileGame(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::file::FileGame::getGameInfo(String_t path, GameInfo& result)
{
    // ex Connection::doStatGame
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(path, DirectoryItem::AllowRead);
    dir.readContent(m_root);

    // Read status
    GameStatus& status = dir.readGameStatus(m_root);
    if (const GameStatus::GameInfo* info = status.getGameInfo()) {
        copyGameInfo(result, *info, path, dir);
    } else {
        throw std::runtime_error(FILE_NOT_FOUND);
    }
}

void
server::file::FileGame::listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result)
{
    // ex Connection::doListGame
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(path, DirectoryItem::AllowRead);

    // Process recursively
    std::vector<std::pair<String_t, DirectoryItem*> > work;
    work.push_back(std::make_pair(path, &dir));
    while (!work.empty()) {
        // Fetch work item
        DirectoryItem& thisDir = *work.back().second;
        const String_t thisName = work.back().first;
        work.pop_back();

        // Read directory
        thisDir.readContent(m_root);

        // Check game information
        GameStatus& status = thisDir.readGameStatus(m_root);
        if (const GameStatus::GameInfo* info = status.getGameInfo()) {
            std::auto_ptr<GameInfo> p(new GameInfo());
            copyGameInfo(*p, *info, thisName, thisDir);
            result.pushBackNew(p.release());
        }

        // Gather subdirs
        for (size_t i = 0, n = thisDir.getNumDirectories(); i < n; ++i) {
            if (DirectoryItem* subDir = thisDir.getDirectoryByIndex(i)) {
                subDir->readContent(m_root);
                if (subDir->hasPermission(m_session.getUser(), DirectoryItem::AllowRead)) {
                    work.push_back(std::make_pair(thisName + "/" + subDir->getName(), subDir));
                }
            }
        }
    }
}

void
server::file::FileGame::getKeyInfo(const String_t path, KeyInfo& result)
{
    // ex Connection::doStatReg
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(path, DirectoryItem::AllowRead);
    dir.readContent(m_root);

    // Read status
    GameStatus& status = dir.readGameStatus(m_root);
    if (const GameStatus::KeyInfo* info = status.getKeyInfo()) {
        copyKeyInfo(result, *info, path);
    } else {
        throw std::runtime_error(FILE_NOT_FOUND);
    }
}

void
server::file::FileGame::listKeyInfo(const String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result)
{
    // ex Connection::doListReg
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(path, DirectoryItem::AllowRead);

    // Process recursively
    std::map<String_t,KeyInfo*> index;
    std::vector<std::pair<String_t, DirectoryItem*> > work;
    work.push_back(std::make_pair(path, &dir));
    while (!work.empty()) {
        // Fetch work item
        DirectoryItem& thisDir = *work.back().second;
        const String_t thisName = work.back().first;
        work.pop_back();

        // Read directory
        thisDir.readContent(m_root);

        // Check game information
        GameStatus& status = thisDir.readGameStatus(m_root);
        if (const GameStatus::KeyInfo* info = status.getKeyInfo()) {
            if (matchKey(filter, *info)) {
                if (filter.unique) {
                    // Uniquisation filter
                    std::map<String_t,KeyInfo*>::const_iterator it = index.find(info->keyId);
                    if (it != index.end()) {
                        // We already have that one. Just bump counter
                        if (int32_t* counter = it->second->useCount.get()) {
                            ++*counter;
                        }
                    } else {
                        // Not seen yet; add it
                        std::auto_ptr<KeyInfo> p(new KeyInfo());
                        copyKeyInfo(*p, *info, thisName);
                        index[info->keyId] = p.get();
                        p->useCount = 1;
                        result.pushBackNew(p.release());
                    }
                } else {
                    // Just collect
                    std::auto_ptr<KeyInfo> p(new KeyInfo());
                    copyKeyInfo(*p, *info, thisName);
                    result.pushBackNew(p.release());
                }
            }
        }

        // Gather subdirs
        for (size_t i = 0, n = thisDir.getNumDirectories(); i < n; ++i) {
            if (DirectoryItem* subDir = thisDir.getDirectoryByIndex(i)) {
                subDir->readContent(m_root);
                if (subDir->hasPermission(m_session.getUser(), DirectoryItem::AllowRead)) {
                    work.push_back(std::make_pair(thisName + "/" + subDir->getName(), subDir));
                }
            }
        }
    }
}
