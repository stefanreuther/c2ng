/**
  *  \file server/interface/hostkeyserver.cpp
  *  \brief Class server::interface::HostKeyServer
  */

#include "server/interface/hostkeyserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::HostKeyServer::HostKeyServer(HostKey& impl)
    : m_implementation(impl)
{ }

server::interface::HostKeyServer::~HostKeyServer()
{ }

bool
server::interface::HostKeyServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "KEYLS") {
        /* @q KEYLS (Host Command)
           List available keys.
           Returns an array of keys.

           Permissions: user context required

           @rettype HostKeyInfo */
        args.checkArgumentCount(0);

        HostKey::Infos_t infos;
        m_implementation.listKeys(infos);

        Vector::Ref_t v = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            v->pushBackNew(packInfo(infos[i]));
        }

        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "KEYGET") {
        /* @q KEYGET keyId:Str (Host Command)
           Retrieve key identified by the key Id.
           Returns the file image of the key.

           Permissions: user context required

           @err 404 Key not found

           @retval Blob file content */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getKey(toString(args.getNext()))));
        return true;
    } else {
        return false;
    }
}

server::Value_t*
server::interface::HostKeyServer::packInfo(const HostKey::Info& info)
{
    /* @type HostKeyInfo
       Information about a registration key.

       @key id:Str             (key Id)
       @key reg:Int            (0=unregistered, 1=registered)
       @key key1:Str           (registration key first line)
       @key key2:Str           (registration key second line)
       @key filePathName:Str   (optional; path on file server)
       @key fileUseCount:Int   (optional; number of uses on file server)
       @key game:Int           (optional; number of game this key was used last)
       @key gameName:Str       (optional; name of game this key was used last)
       @key gameUseCount:Int   (optional; number of uses on host)
       @key gameLastUsed:Time  (optional; time of last use on host) */
    Hash::Ref_t hv = Hash::create();

    hv->setNew("id", makeStringValue(info.keyId));
    hv->setNew("reg", makeIntegerValue(info.isRegistered));
    hv->setNew("key1", makeStringValue(info.label1));
    hv->setNew("key2", makeStringValue(info.label2));

    // Information from file server
    if (const String_t* p = info.filePathName.get()) {
        hv->setNew("filePathName", makeStringValue(*p));
    }
    if (const int32_t* p = info.fileUseCount.get()) {
        hv->setNew("fileUseCount", makeIntegerValue(*p));
    }

    // Information from host key store
    if (const int32_t* p = info.lastGame.get()) {
        hv->setNew("game", makeIntegerValue(*p));
    }
    if (const String_t* p = info.lastGameName.get()) {
        hv->setNew("gameName", makeStringValue(*p));
    }
    if (const int32_t* p = info.gameUseCount.get()) {
        hv->setNew("gameUseCount", makeIntegerValue(*p));
    }
    if (const Time_t* p = info.gameLastUsed.get()) {
        hv->setNew("gameLastUsed", makeIntegerValue(*p));
    }

    return new afl::data::HashValue(hv);
}
