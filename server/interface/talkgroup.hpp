/**
  *  \file server/interface/talkgroup.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKGROUP_HPP
#define C2NG_SERVER_INTERFACE_TALKGROUP_HPP

#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/deletable.hpp"

namespace server { namespace interface {

    class TalkGroup : public afl::base::Deletable {
     public:
        struct Description {
            // FIXME: field "key"
            afl::base::Optional<String_t> name;
            afl::base::Optional<String_t> description;
            afl::base::Optional<String_t> parentGroup;
            afl::base::Optional<bool> unlisted;
        };

        virtual void add(String_t groupId, const Description& info) = 0;
        virtual void set(String_t groupId, const Description& info) = 0;
        virtual String_t getField(String_t groupId, String_t fieldName) = 0;
        virtual void list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums) = 0;
        virtual Description getDescription(String_t groupId) = 0;
        // FIXME: use Memory<String> here?
        virtual void getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results) = 0;
    };

} }

#endif
