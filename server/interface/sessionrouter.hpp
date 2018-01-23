/**
  *  \file server/interface/sessionrouter.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_SESSIONROUTER_HPP
#define C2NG_SERVER_INTERFACE_SESSIONROUTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/data/integerlist.hpp"

namespace server { namespace interface {

    class SessionRouter : public afl::base::Deletable {
     public:
        enum Action {
            Close,
            Restart,
            Save,
            SaveNN
        };
        
        // LIST. Produces a table, but we don't currently interpret that.
        virtual String_t getStatus() = 0;

        // INFO. Produces a list, but we don't currently interpret that.
        virtual String_t getInfo(int32_t sessionId) = 0;

        // S. Talks to a session and produces a result.
        virtual String_t talk(int32_t sessionId, String_t command) = 0;

        // CLOSE/RESTART/SAVE/SAVENN with session Id
        virtual void sessionAction(int32_t sessionId, Action action) = 0;

        // CLOSE/RESTART/SAVE/SAVENN with group key.
        virtual void groupAction(String_t key, Action action, afl::data::IntegerList_t& result) = 0;

        // NEW. Returns session Id.
        virtual int32_t create(afl::base::Memory<const String_t> args) = 0;

        // CONFIG. This is a utility command.
        virtual String_t getConfiguration() = 0;

        static String_t formatAction(Action action);
        static bool parseAction(const String_t& str, Action& result);
    };

} }

#endif
