/**
  *  \file server/interface/talkgroupserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkgroupserver.hpp"
#include "server/interface/talkgroup.hpp"
#include "interpreter/arguments.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/errors.hpp"

server::interface::TalkGroupServer::TalkGroupServer(TalkGroup& impl)
    : m_implementation(impl)
{ }

server::interface::TalkGroupServer::~TalkGroupServer()
{ }

bool
server::interface::TalkGroupServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "GROUPADD") {
        /* @q GROUPADD id:GRID key:Str value:Str ... (Talk Command)
           Create a forum group.
           The command is followed by %key/%value pairs that configure the group, see {GROUPSET}.
           At least one pair ("name", typically) must be given.

           The %id must be a valid identifier (alphanumerics).

           Permissions: admin.

           @err 409 Already exists (the %id is already taken)
           @uses group:$GRID:header */
        // @change PCC2 would want at least 3 args; we require at least 1, which happens when someone gives an empty Description.
        args.checkArgumentCountAtLeast(1);
        String_t groupId = toString(args.getNext());
        m_implementation.add(groupId, parseDescription(args));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GROUPSET") {
        /* @q GROUPSET id:GRID [key:Str value:Str ...] (Talk Command)
           Configure forum group.
           Valid keys are the same as hash keys in {group:$GRID:header}:
           - %name ({@type Str|string}, name of group)
           - %description ({@type TalkText}, description/subtitle)
           - %parent ({@type GRID} of parent group)
           - %key ({@type Str|string}, sort key)
           - %unlisted ({@type Int}, nonzero to make this group unlistable)

           Permissions: admin.

           @err 404 Not found (%id does not correspond to a valid group)
           @uses group:$GRID:header, group:$GRID:groups */
        args.checkArgumentCountAtLeast(1);
        String_t groupId = toString(args.getNext());
        m_implementation.set(groupId, parseDescription(args));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GROUPGET") {
        /* @q GROUPGET id:GRID key:Str (Talk Command)
           Get property of group.
           The %key can be a hash key as given to {GROUPSET}.

           Permissions: none.

           @uses group:$GRID:header
           @rettype GRID
           @rettype Str
           @rettype TalkText */
        args.checkArgumentCount(2);
        String_t groupId = toString(args.getNext());
        String_t field = toString(args.getNext());

        // FIXME: c2talk classic passes on null values as is; this stringifies.
        result.reset(makeStringValue(m_implementation.getField(groupId, field)));
        return true;
    } else if (upcasedCommand == "GROUPLS") {
        /* @q GROUPLS id:GRID (Talk Command)
           List group contents.

           Permissions: none.

           Everyone may list every group, but unlistable groups will appear empty in user contexts.
           Nonexistant groups will appear empty.

           The results will be sorted by their sort key.

           @retkey groups:StrList (list of contained groups)
           @retkey forums:IntList (list of {@talk FID|forum Ids)
           @rettype FID
           @uses group:$GRID:groups, group:$GRID:forums */
        args.checkArgumentCount(1);
        String_t groupId = toString(args.getNext());

        afl::data::StringList_t subgroupList;
        afl::data::IntegerList_t forumList;
        m_implementation.list(groupId, subgroupList, forumList);

        afl::base::Ref<afl::data::Vector> resultVector = afl::data::Vector::create();

        afl::base::Ref<afl::data::Vector> subgroupVector = afl::data::Vector::create();
        subgroupVector->pushBackElements(subgroupList);
        resultVector->pushBackString("groups");
        resultVector->pushBackNew(new afl::data::VectorValue(subgroupVector));

        afl::base::Ref<afl::data::Vector> forumVector = afl::data::Vector::create();
        forumVector->pushBackElements(forumList);
        resultVector->pushBackString("forums");
        resultVector->pushBackNew(new afl::data::VectorValue(forumVector));

        result.reset(new afl::data::VectorValue(resultVector));
        return true;
    } else if (upcasedCommand == "GROUPSTAT") {
        /* @q GROUPSTAT id:GRID (Talk Command)
           Get information about one group.

           Permissions: none.

           Note that this command uses the rendering configuration set with {RENDEROPTION}
           to render the group description.

           @retval TalkGroupInfo information about group
           @see GROUPMSTAT
           @uses group:$GRID:header */
        args.checkArgumentCount(1);
        String_t groupId = toString(args.getNext());
        result.reset(formatDescription(m_implementation.getDescription(groupId)));
        return true;
    } else if (upcasedCommand == "GROUPMSTAT") {
        /* @q GROUPMSTAT id:GRID... (Talk Command)
           Get information about multiple groups.

           Permissions: none.

           Note that this command uses the rendering configuration set with {RENDEROPTION}
           to render the group descriptions.

           @retval TalkGroupInfo[] information about groups
           @see GROUPSTAT
           @uses group:$GRID:header */
        afl::data::StringList_t groups;
        while (args.getNumArgs() > 0) {
            groups.push_back(toString(args.getNext()));
        }

        afl::container::PtrVector<TalkGroup::Description> descriptions;
        m_implementation.getDescriptions(groups, descriptions);

        afl::base::Ref<afl::data::Vector> vec = afl::data::Vector::create();
        for (size_t i = 0, n = descriptions.size(); i < n; ++i) {
            if (TalkGroup::Description* p = descriptions[i]) {
                vec->pushBackNew(formatDescription(*p));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new afl::data::VectorValue(vec));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkGroup::Description
server::interface::TalkGroupServer::parseDescription(interpreter::Arguments& args)
{
    TalkGroup::Description result;
    while (args.getNumArgs() > 0) {
        args.checkArgumentCountAtLeast(2);
        String_t key = toString(args.getNext());
        if (key == "name") {
            result.name = toString(args.getNext());
        } else if (key == "description") {
            result.description = toString(args.getNext());
        } else if (key == "parent") {
            result.parentGroup = toString(args.getNext());
        } else if (key == "key") {
            result.key = toString(args.getNext());
        } else if (key == "unlisted") {
            result.unlisted = toInteger(args.getNext()) != 0;
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
    }
    return result;
}

server::Value_t*
server::interface::TalkGroupServer::formatDescription(const TalkGroup::Description& desc)
{
    afl::base::Ref<afl::data::Vector> result = afl::data::Vector::create();

    if (const String_t* p = desc.name.get()) {
        result->pushBackString("name");
        result->pushBackString(*p);
    }
    if (const String_t* p = desc.description.get()) {
        result->pushBackString("description");
        result->pushBackString(*p);
    }
    if (const String_t* p = desc.parentGroup.get()) {
        result->pushBackString("parent");
        result->pushBackString(*p);
    }
    if (const bool* p = desc.unlisted.get()) {
        result->pushBackString("unlisted");
        result->pushBackInteger(*p);
    }

    return new afl::data::VectorValue(result);
}
