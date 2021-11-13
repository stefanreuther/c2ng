/**
  *  \file server/host/hostkey.cpp
  *  \brief Class server::host::HostKey
  */

#include <map>
#include "server/host/hostkey.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/format.hpp"
#include "game/v3/registrationkey.hpp"
#include "server/errors.hpp"
#include "server/host/keystore.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/user.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/filegameclient.hpp"

const char*const LOG_NAME = "host.key";

using afl::string::Format;
using game::v3::RegistrationKey;
using server::host::Root;
using server::host::Session;
using server::interface::BaseClient;
using server::interface::FileBaseClient;
using server::interface::FileGame;
using server::interface::FileGameClient;

namespace {
    std::auto_ptr<RegistrationKey> makeServerKey(Session& session, Root& root)
    {
        std::auto_ptr<RegistrationKey> result;
        if (!session.getUser().empty() && !root.config().keyTitle.empty()) {
            // Build line 2 for the key
            afl::checksums::SHA1 hasher;
            hasher.add(afl::string::toBytes(session.getUser()));
            hasher.add(afl::string::toBytes(root.config().keySecret));
            const String_t line2 = Format("%s-%.12s", session.getUser(), hasher.getHashAsHexString());

            // Build the key
            result.reset(new RegistrationKey(std::auto_ptr<afl::charset::Charset>(root.defaultCharacterSet().clone())));
            result->initFromValues(root.config().keyTitle, line2);
        }
        return result;
    }
}


server::host::HostKey::HostKey(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::host::HostKey::listKeys(Infos_t& out)
{
    // Must be user
    m_session.checkUser();

    // Fetch user's keys
    User user(m_root, m_session.getUser());
    KeyStore::Infos_t localKeys;
    KeyStore(user.keyStore(), m_root.config()).listKeys(localKeys, m_root.defaultCharacterSet());

    // Convert to output format; remember what we've seen.
    std::map<String_t, size_t> seenKeyIndexes;
    for (size_t i = 0, n = localKeys.size(); i < n; ++i) {
        const KeyStore::Info& localKey = localKeys[i];
        Info outKey;
        outKey.keyId        = localKey.keyId;
        outKey.isRegistered = localKey.isRegistered;
        outKey.label1       = localKey.label1;
        outKey.label2       = localKey.label2;
        outKey.lastGame     = localKey.lastGame;
        if (localKey.lastGame) {
            outKey.lastGameName = Game(m_root, localKey.lastGame, Game::NoExistanceCheck).getName();
        }
        outKey.gameUseCount = localKey.useCount;
        outKey.gameLastUsed = localKey.lastUsed;

        seenKeyIndexes[outKey.keyId] = out.size();
        out.push_back(outKey);
    }

    // Merge server-generated user key
    std::auto_ptr<RegistrationKey> serverKey(makeServerKey(m_session, m_root));
    if (serverKey.get() != 0) {
        String_t keyId = serverKey->getKeyId();
        std::map<String_t, size_t>::iterator it = seenKeyIndexes.find(keyId);
        Info* p;
        if (it == seenKeyIndexes.end()) {
            // New (never used) key
            Info outKey;
            outKey.keyId        = keyId;
            outKey.isRegistered = (serverKey->getStatus() == RegistrationKey::Registered);
            outKey.label1       = serverKey->getLine(RegistrationKey::Line1);
            outKey.label2       = serverKey->getLine(RegistrationKey::Line2);

            seenKeyIndexes[outKey.keyId] = out.size();
            out.push_back(outKey);
            p = &out.back();
        } else {
            // Found, update the slot.
            p = &out[it->second];
        }
        p->isServerKey = true;
    }

    // Merge file information
    try {
        BaseClient(m_root.userFile()).setUserContext(m_session.getUser());

        // Retrieve from user filer
        afl::container::PtrVector<FileGame::KeyInfo> fileKeys;
        FileGame::Filter f;
        f.unique = true;
        FileGameClient(m_root.userFile()).listKeyInfo("u/" + user.getLoginName(), f, fileKeys);

        // Merge into previous data.
        // We have built an index, so this is at most O(n*logm), not O(n*m) complexity
        // (where n is user-controlled, m is configured (maxStoredKeys)).
        for (size_t i = 0, n = fileKeys.size(); i < n; ++i) {
            if (const FileGame::KeyInfo* fileKey = fileKeys[i]) {
                if (const String_t* keyId = fileKey->keyId.get()) {
                    Info* p;
                    std::map<String_t, size_t>::iterator it = seenKeyIndexes.find(*keyId);
                    if (it == seenKeyIndexes.end()) {
                        // Not found, make new slot.
                        Info outKey;
                        outKey.keyId = *keyId;
                        outKey.isRegistered = fileKey->isRegistered;
                        outKey.label1 = fileKey->label1;
                        outKey.label2 = fileKey->label2;
                        out.push_back(outKey);
                        p = &out.back();
                    } else {
                        // Found, re-use the slot.
                        // Remove it from index to not consider it again; listKeyInfo() output should already be unique.
                        p = &out[it->second];
                        seenKeyIndexes.erase(it);
                    }

                    p->filePathName = fileKey->pathName;
                    p->fileUseCount = fileKey->useCount;
                }
            }
        }
    }
    catch (std::exception& ex) {
        m_root.log().write(afl::sys::Log::Warn, LOG_NAME, "Failed to retrieve file keys", ex);
    }
}

String_t
server::host::HostKey::getKey(String_t keyId)
{
    // Must be user
    m_session.checkUser();

    // Try to fetch user key
    User user(m_root, m_session.getUser());
    game::v3::RegistrationKey key(std::auto_ptr<afl::charset::Charset>(m_root.defaultCharacterSet().clone()));
    if (KeyStore(user.keyStore(), m_root.config()).getKey(keyId, key)) {
        afl::io::InternalStream out;
        key.saveToStream(out);
        return afl::string::fromBytes(out.getContent());
    }

    // Try to fetch server-generated key
    std::auto_ptr<RegistrationKey> serverKey(makeServerKey(m_session, m_root));
    if (serverKey.get() != 0) {
        if (keyId == serverKey->getKeyId()) {
            afl::io::InternalStream out;
            serverKey->saveToStream(out);
            return afl::string::fromBytes(out.getContent());
        }
    }

    // Try to fetch from filer
    BaseClient(m_root.userFile()).setUserContext(m_session.getUser());

    afl::container::PtrVector<FileGame::KeyInfo> fileKeys;
    FileGame::Filter f;
    f.unique = true;
    f.keyId = keyId;
    FileGameClient(m_root.userFile()).listKeyInfo("u/" + user.getLoginName(), f, fileKeys);

    if (!fileKeys.empty() && fileKeys[0] != 0 && !fileKeys[0]->fileName.empty()) {
        return FileBaseClient(m_root.userFile()).getFile(fileKeys[0]->fileName);
    }

    // Not found
    throw std::runtime_error(FILE_NOT_FOUND);
}
