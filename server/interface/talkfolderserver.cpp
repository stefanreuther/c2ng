/**
  *  \file server/interface/talkfolderserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkfolderserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/string.hpp"
#include "interpreter/arguments.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/data/stringlist.hpp"
#include "server/interface/talkforumserver.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::TalkFolderServer::TalkFolderServer(TalkFolder& impl)
    : m_implementation(impl)
{ }

server::interface::TalkFolderServer::~TalkFolderServer()
{ }

bool
server::interface::TalkFolderServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "FOLDERLS") {
        /* @q FOLDERLS (Talk Command)
           List user's PM folders.

           Permissions: user context required, accesses user's folders

           @retval UFID[] list of folder Ids
           @uses default:folder:all, user:$UID:pm:folder:all */
        args.checkArgumentCount(0);

        afl::data::IntegerList_t ufids;
        m_implementation.getFolders(ufids);

        Vector::Ref_t v = Vector::create();
        v->pushBackElements(ufids);
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "FOLDERSTAT") {
        /* @q FOLDERSTAT folder:UFID (Talk Command)
           Information about one folder.

           Permissions: user context required, accesses user's folders

           @err 404 Folder does not exist
           @retval TalkFolderInfo
           @uses user:$UID:pm:folder:$UFID:header, default:folder:$UFID:header, user:$UID:pm:folder:$UFID:messages
           @see FOLDERMSTAT */
        args.checkArgumentCount(1);
        int32_t ufid = toInteger(args.getNext());
        result.reset(packInfo(m_implementation.getInfo(ufid)));
        return true;
    } else if (upcasedCommand == "FOLDERMSTAT") {
        /* @q FOLDERMSTAT folder:UFID... (Talk Command)
           Information about multiple folders.

           Permissions: user context required, accesses user's folders

           @retval TalkFolderInfo[] information for all folders, null if a folder does not exist
           @uses user:$UID:pm:folder:$UFID:header, default:folder:$UFID:header, user:$UID:pm:folder:$UFID:messages
           @see FOLDERSTAT */
        afl::data::IntegerList_t ufids;
        while (args.getNumArgs() > 0) {
            ufids.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<TalkFolder::Info> infos;
        m_implementation.getInfo(ufids, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (const TalkFolder::Info* p = infos[i]) {
                vec->pushBackNew(packInfo(*p));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "FOLDERNEW") {
        /* @q FOLDERNEW name:Str [key:Str value:Str ...] (Talk Command)
           Create new folder.
           The command can include %key/%value pairs that are used to configure the folder, see {FOLDERSET}.

           Permissions: user context required, accesses user's folders

           @retval UFID new folder Id
           @uses user:$UID:pm:folder:id, user:$UID:pm:folder:$UFID:header */
        args.checkArgumentCountAtLeast(1);
        String_t name = toString(args.getNext());

        afl::data::StringList_t config;
        while (args.getNumArgs() > 0) {
            config.push_back(toString(args.getNext()));
        }

        result.reset(makeIntegerValue(m_implementation.create(name, config)));
        return true;
    } else if (upcasedCommand == "FOLDERRM") {
        /* @q FOLDERRM folder:UFID (Talk Command)
           Remove folder.
           Deletes the folder and all messages it contains.

           Permissions: user context required, accesses user's folders

           @retval Int 1=success, 0=folder not deleted because it did not exist or was not deletable
           @uses user:$UID:pm:folder:all, user:$UID:pm:folder:$UFID:messages */
        args.checkArgumentCount(1);
        int32_t ufid = toInteger(args.getNext());
        result.reset(makeIntegerValue(m_implementation.remove(ufid)));
        return true;
    } else if (upcasedCommand == "FOLDERSET") {
        /* @q FOLDERSET folder:UFID [key:Str value:Str ...] (Talk Command)
           Configure folder.
           The given %key/%value pairs are set in {user:$UID:pm:folder:$UFID:header}.

           Valid keys:
           - name (folder name)
           - description (description/subtitle)

           Permissions: user context required, accesses user's folders

           @uses user:$UID:pm:folder:$UFID:header */
        args.checkArgumentCountAtLeast(1);
        int32_t ufid = toInteger(args.getNext());

        afl::data::StringList_t config;
        while (args.getNumArgs() > 0) {
            config.push_back(toString(args.getNext()));
        }

        m_implementation.configure(ufid, config);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "FOLDERLSPM") {
        /* @q FOLDERLSPM folder:UFID [listParameters...] (Talk Command)
           Query list of PMs in a user's folder.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for folders are:
           - author
           - subject
           - time

           Permissions: user context required, accesses user's folders

           @rettype Any
           @uses user:$UID:pm:folder:$UFID:messages */
        args.checkArgumentCountAtLeast(1);
        int32_t ufid = toInteger(args.getNext());

        TalkFolder::ListParameters p;
        TalkForumServer::parseListParameters(p, args);

        result.reset(m_implementation.getPMs(ufid, p));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkFolderServer::Value_t*
server::interface::TalkFolderServer::packInfo(const TalkFolder::Info& info)
{
    /* @type TalkFolderInfo
       Information about a user's PM folder.
       @key name:Str          Name
       @key description:Str   Description
       @key messages:Int      Total number of messages in folder
       @key unread:Int        Nonzero if there are unread messages (not a message count!)
       @key fixed:Int         0=user-created folder, 1=system-created folder, e.g. Inbox (cannot be deleted) */
    Hash::Ref_t h = Hash::create();
    h->setNew("name",        makeStringValue(info.name));
    h->setNew("description", makeStringValue(info.description));
    h->setNew("messages",    makeIntegerValue(info.numMessages));
    h->setNew("unread",      makeIntegerValue(info.hasUnreadMessages));
    h->setNew("fixed",       makeIntegerValue(info.isFixedFolder));
    return new HashValue(h);
}
