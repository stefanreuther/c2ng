/**
  *  \file server/host/installer.cpp
  *  \brief Class server::host::Installer
  */

#include "server/host/installer.hpp"
#include "afl/string/format.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"

using server::interface::FileBase;
using server::interface::HostGame;

namespace {
    const char LOG_NAME[] = "install";

    const int32_t HARD_SIZE_LIMIT = 100L*1024*1024;

    /** Check file name match. In perl terms, checks $name =~ /$pre[0-9]+$post/. */
    bool match(const String_t& name, const char* pre, const char* post)
    {
        // Check length
        size_t preLen = std::strlen(pre);
        size_t postLen = std::strlen(post);
        if (name.size() <= preLen + postLen) {
            return false;
        }

        // Check content
        if (name.compare(0, preLen, pre, preLen) != 0 || name.compare(name.size()-postLen, postLen, post, postLen) != 0) {
            return false;
        }
        if (name.find_first_not_of("0123456789", preLen) < name.size() - postLen) {
            return false;
        }
        return true;
    }

    /** List files. Generates a list of files in a directory.
        \param file Filer to check
        \param dirName Directory on filer
        \param files Set of file names (excluding paths) will be produced here */
    void listFiles(FileBase& file, String_t dirName, std::set<String_t>& files)
    {
        // Fetch content.
        FileBase::ContentInfoMap_t content;
        file.getDirectoryContent(dirName, content);

        for (FileBase::ContentInfoMap_t::iterator it = content.begin(), end = content.end(); it != end; ++it) {
            if (const FileBase::Info* props = it->second) {
                if (props->type == FileBase::IsFile) {
                    files.insert(it->first);
                }
            }
        }
    }

    /** Install file.
        \param s File content
        \param file Copy to this filer...
        \param dirName ...into this directory...
        \param name ...under this name */
    void installFile(const String_t& content, FileBase& file, String_t dirName, String_t name)
    {
        if (content.size() > size_t(HARD_SIZE_LIMIT)) {
            // Make sure we don't accidentally crash. This normally does not
            // happen because files come from a trusted source (sort of) which
            // does not create files that big. 100meg still is a lot because
            // it's going to be copied around a little...
            throw std::runtime_error("File size error");
        }
        file.putFile(dirName + "/" + name, content);
    }

    /** Install files from a directory.
        \param srcDirName Copy all files from this directory...
        \param file ...to this filer...
        \param dirName ...into this directory.
        \param filesToDelete Copied files are removed from here.
        \param except This file is not copied. */
    void installFiles(FileBase& hostFile, String_t srcDirName,
                      FileBase& userFile, String_t dirName,
                      std::set<String_t>& filesToDelete,
                      String_t except,
                      afl::sys::LogListener& log)
    {
        // Log message
        log.write(log.Info, LOG_NAME, afl::string::Format("copying files from 'host:%s' to 'user:%s'", srcDirName, dirName));

        // Do it
        FileBase::ContentInfoMap_t entries;
        hostFile.getDirectoryContent(srcDirName, entries);
        for (FileBase::ContentInfoMap_t::iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
            // A file is accepted if
            // ...it does not start with a '.' (e.g. ".." or ".c2file") [does not need to be checked in c2ng]
            // ...it is not explicitly excempted
            // ...its name is entirely lowercase, otherwise it won't be accessible to the user [does not need to be not checked in c2ng]
            // ...it actually is a file
            // ...does not exceed HARD_SIZE_LIMIT [new in c2ng]
            const FileBase::Info* props = it->second;
            if (it->first != except
                && props != 0
                && props->type == FileBase::IsFile)
            {
                if (const int32_t* size = props->size.get()) {
                    if (*size >= 0 && *size <= HARD_SIZE_LIMIT) {
                        // Must copy this file
                        installFile(hostFile.getFile(afl::string::Format("%s/%s", srcDirName, it->first)), userFile, dirName, it->first);

                        // Must not erase this file
                        filesToDelete.erase(it->first);
                    }
                }
            }
        }
    }
}


// Constructor.
server::host::Installer::Installer(Root& root)
    : m_root(root)
{ }

// Check for precious file.
bool
server::host::Installer::isPreciousFile(const String_t& name) const
{
    // ex planetscentral/host/install.h:isPreciousFile
    // The idea is that specification files which are recreated by a new
    // install are erased, and all specification files which are not part
    // of the new install are erased. That is, there's no need to keep a
    // hconfig.hst file because if the game contains one, it will be
    // recreated, and if the game does not contain one, there shouldn't
    // be any. So all we need to take care of is database files. Check
    // for the important files known to be created by PCC, Winplan, VPA.
    //
    // This uses the paradigm to delete everything I don't know. It is
    // probably debatable whether that should be the other way around,
    // so we only delete foreign playerX.rsts and keep other files intact.
    return name == "fizz.bin"
        || name == "stat.cc"
        || name == "score.cc"
        || name == "config.cc"
        || name == "config2.cc"
        || match(name, "auto", ".dat")
        || match(name, "chart", ".cc")
        || match(name, "fleet", ".cc")
        || match(name, "notes", ".dat")
        || match(name, "script", ".cc")
        || match(name, "team", ".cc")
        || match(name, "vm", ".cc")
        || match(name, "vpa", ".db");
}

// Install game data.
void
server::host::Installer::installGameData(Game& game, game::PlayerSet_t players, String_t userId, String_t dirName)
{
    // ex planetscentral/host/install.h:installGameData

    // Kill all running PCC2 Web sessions in this directory.
    m_root.tryCloseRouterSessions(afl::string::Format("WDIR=%s", dirName));

    // Server instances
    server::interface::BaseClient(m_root.userFile()).setUserContext(userId);
    server::interface::FileBaseClient userFile(m_root.userFile());
    server::interface::FileBaseClient hostFile(m_root.hostFile());

    // Start by configuring the game properties
    HostGame::State gameState = game.getState();

    // (If there is a problem with the dirName, this line will throw and abort this function.)
    userFile.createDirectoryTree(dirName);
    userFile.setDirectoryProperty(dirName, "game", afl::string::Format("%d", game.getId()));
    userFile.setDirectoryProperty(dirName, "name", game.getName());
    userFile.setDirectoryProperty(dirName, "nofilewarning", "1");
    userFile.setDirectoryProperty(dirName, "finished", afl::string::Format("%d", int(gameState == HostGame::Finished || gameState == HostGame::Deleted)));
    userFile.setDirectoryProperty(dirName, "hosttime", "0");    // FIXME: would be nice to have this correct

    // List files
    std::set<String_t> filesToDelete;
    listFiles(userFile, dirName, filesToDelete);

    // Install files
    String_t gameDir = game.getDirectory();
    installFiles(hostFile, gameDir + "/out/all", userFile, dirName, filesToDelete, "playerfiles.zip", m_root.log());
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        if (players.contains(i)) {
            // Outgoing files
            installFiles(hostFile, afl::string::Format("%s/out/%d", gameDir, i),
                         userFile, dirName,
                         filesToDelete, afl::string::Format("player%d.zip", i), m_root.log());

            // Turn file
            String_t trnName = afl::string::Format("player%d.trn", i);
            afl::base::Optional<String_t> trnContent = hostFile.getFileNT(afl::string::Format("%s/in/%s", gameDir, trnName));
            if (const String_t* p = trnContent.get()) {
                installFile(*p, userFile, dirName, trnName);
            }
            filesToDelete.erase(trnName);
        }
    }

    // Remove surplus files
    for (std::set<String_t>::iterator i = filesToDelete.begin(); i != filesToDelete.end(); ++i) {
        if (!isPreciousFile(*i)) {
            userFile.removeFile(dirName + "/" + *i);
        }
    }
}

// Install single file to multiple players' game directory.
void
server::host::Installer::installFileMulti(Game& game, const afl::data::StringList_t& players, String_t fileName, String_t fileContent, int32_t slot)
{
    // ex planetscentral/host/install.h:installFileMulti
    int did = 0, fail = 0;
    for (size_t i = 0; i < players.size(); ++i) {
        // Try each player in turn with his own permissions.
        // Don't fail if the player has messed with the directory.
        String_t dirName = game.getPlayerConfig(players[i], "gameDir");
        if (!dirName.empty()) {
            try {
                server::interface::BaseClient(m_root.userFile()).setUserContext(players[i]);
                server::interface::FileBaseClient file(m_root.userFile());
                if (file.getDirectoryIntegerProperty(dirName, "game") == game.getId()) {
                    // FIXME: This logic needs some refinement. Currently, this is only used with nonzero slots to upload turn files.
                    // (a) if someone uploads a specification file [which would be slot=0], we should normally close all sessions.
                    // (b) if a session is uploading a temporary turn file, this shouldn't cause that very session to close
                    if (slot != 0) {
                        m_root.tryCloseRouterSessions(afl::string::Format("WDIRPL=%s/%d\n", dirName, slot));
                    }
                    file.putFile(dirName + "/" + fileName, fileContent);
                }
                ++did;
            }
            catch (...) {
                ++fail;
            }
        }
    }

    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("file '%s' copied to %d directories, failed %d'", fileName.c_str(), did, fail));
}

// Process a change due to an (un)subscription.
void
server::host::Installer::installChangedGameFiles(Game& game, String_t player, int32_t /*slot*/, bool /*added*/)
{
    // ex planetscentral/host/install.h:installChangedGameFiles

    // Is there a directory?
    String_t dirName = game.getPlayerConfig(player, "gameDir");
    if (dirName.empty()) {
        return;
    }

    // Figure out all subscriptions of that player
    game::PlayerSet_t set = game.getSlotsByPlayer(player);

    // Do it
    if (set.empty()) {
        // Last unsubscription: unconfigure game
        game.setPlayerConfig(player, "gameDir", String_t());
        uninstallGameData(player, dirName);
    } else {
        // Some subscriptions remain: regenerate the game directory.
        // We could optimize here, but let's start simple.
        // FIXME: actually we should optimize here. This kills a user's
        // PCC2 Web session - and his turn file! - when someone
        // appoints him as a replacement.
        installGameData(game, set, player, dirName);
    }
}

// Uninstall game data.
void
server::host::Installer::uninstallGameData(String_t userId, String_t dirName)
{
    // ex planetscentral/host/install.h:uninstallGameData
    try {
        // Setup filer
        server::interface::BaseClient(m_root.userFile()).setUserContext(userId);
        server::interface::FileBaseClient userFile(m_root.userFile());

        // We're leaving an old game directory. Change its properties.
        // However, if anything goes wrong in the process, don't fail.
        // Actually, the only thing we have to do is to retract the
        // 'game' property. But retract the others as well.
        userFile.setDirectoryProperty(dirName, "game", "0");
        userFile.setDirectoryProperty(dirName, "finished", "0");
        userFile.setDirectoryProperty(dirName, "hosttime", "0");
    }
    catch (...) { }
}
