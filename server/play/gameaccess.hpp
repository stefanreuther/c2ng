/**
  *  \file server/play/gameaccess.hpp
  */
#ifndef C2NG_SERVER_PLAY_GAMEACCESS_HPP
#define C2NG_SERVER_PLAY_GAMEACCESS_HPP

#include <map>
#include "server/interface/gameaccess.hpp"
#include "game/session.hpp"
#include "util/messagecollector.hpp"
#include "util/stringparser.hpp"

namespace server { namespace play {

    class Packer;
    class CommandHandler;

    class GameAccess : public server::interface::GameAccess {
     public:
        GameAccess(game::Session& session, util::MessageCollector& console);

        virtual void save();
        virtual String_t getStatus();
        virtual Value_t* get(String_t objName);
        virtual Value_t* post(String_t objName, const Value_t* value);

     private:
        game::Session& m_session;
        util::MessageCollector& m_console;
        util::MessageCollector::MessageNumber_t m_lastMessage;

        Value_t* getObject(util::StringParser& p);
        Value_t* getQuery(util::StringParser& p);

        Packer* createPacker(util::StringParser& p);
        static Packer* createQueryPacker(util::StringParser& p, game::Session& session);
        static CommandHandler* createCommandHandler(util::StringParser& p, game::Session& session);
    };

} }

#endif
