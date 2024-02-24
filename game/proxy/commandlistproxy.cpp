/**
  *  \file game/proxy/commandlistproxy.cpp
  *  \brief Class game::proxy::CommandListProxy
  */

#include "game/proxy/commandlistproxy.hpp"
#include "game/game.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

namespace {
    using game::v3::Command;
    using game::v3::CommandExtra;
    using game::v3::CommandContainer;
    using game::proxy::CommandListProxy;

    typedef std::pair<game::Turn*,int> Context_t;

    // Get context for a session
    Context_t getContext(game::Session& session)
    {
        game::Game* g = session.getGame().get();
        if (g == 0) {
            return Context_t(0, 0);
        } else {
            return Context_t(&g->viewpointTurn(), g->getViewpointPlayer());
        }
    }

    // Get CommandContainer for a sesssion
    CommandContainer* getCommandContainer(game::Session& session)
    {
        Context_t ctx = getContext(session);
        if (ctx.first == 0) {
            return 0;
        } else {
            return CommandExtra::get(*ctx.first, ctx.second);
        }
    }

    // Get CommandExtra
    CommandExtra* getCommandExtra(game::Session& session)
    {
        Context_t ctx = getContext(session);
        if (ctx.first == 0) {
            return 0;
        } else {
            return CommandExtra::get(*ctx.first);
        }
    }

    // Check validity of reference
    bool isValidReference(game::Session& session, game::Reference ref)
    {
        Context_t ctx = getContext(session);
        if (ctx.first == 0) {
            return false;
        } else {
            return ctx.first->universe().getObject(ref) != 0;
        }
    }

    // Build list of commands from a session.
    bool buildList(game::Session& session, CommandListProxy::Infos_t& out, CommandListProxy::MetaInfo* pMeta, const Command* findThis, size_t& foundIndex)
    {
        // No CommandExtra means feature not supported
        CommandExtra* extra = getCommandExtra(session);
        if (extra == 0) {
            return false;
        }

        // We might have commands; update meta
        if (pMeta != 0) {
            Context_t ctx = getContext(session);
            if (ctx.first != 0) {
                pMeta->playerNr = ctx.second;
                pMeta->editable = ctx.first->getCommandPlayers().contains(ctx.second);
            }
        }

        // No CommandContainer means we have no commands yet
        CommandContainer* cc = getCommandContainer(session);
        if (cc == 0) {
            return true;
        }

        for (CommandContainer::ConstIterator_t it = cc->begin(); it != cc->end(); ++it) {
            Command* cmd = *it;
            if (cmd) {
                if (cmd == findThis) {
                    foundIndex = out.size();
                }
                game::Reference ref = cmd->getAffectedUnit();
                out.push_back(CommandListProxy::Info(cmd->getCommandText(),
                                                     Command::getCommandInfo(cmd->getCommand(), session.translator()),
                                                     isValidReference(session, ref) ? ref : game::Reference()));
            }
        }
        return true;
    }
}


game::proxy::CommandListProxy::CommandListProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

bool
game::proxy::CommandListProxy::init(WaitIndicator& link, Infos_t& out, MetaInfo& metaOut)
{
    class Request : public util::Request<Session> {
     public:
        Request(Infos_t& out, MetaInfo& metaOut, bool& result)
            : m_out(out), m_metaOut(metaOut), m_result(result)
            { }
        virtual void handle(Session& session)
            {
                size_t foundIndex = 0;
                m_result = buildList(session, m_out, &m_metaOut, 0, foundIndex);
            }
     private:
        Infos_t& m_out;
        MetaInfo& m_metaOut;
        bool& m_result;
    };

    bool result = false;
    Request req(out, metaOut, result);
    link.call(m_gameSender, req);
    return result;
}

bool
game::proxy::CommandListProxy::addCommand(WaitIndicator& link, const String_t& cmd, Infos_t& newList, size_t& newPos)
{
    class Request : public util::Request<Session> {
     public:
        Request(const String_t& cmd, Infos_t& newList, size_t& newPos, bool& result)
            : m_cmd(cmd), m_newList(newList), m_newPos(newPos), m_result(result)
            { }
        virtual void handle(Session& session)
            {
                CommandExtra* extra = getCommandExtra(session);
                Game* g = session.getGame().get();
                if (extra == 0 || g == 0) {
                    // Unsupported by game; should normally have been caught by init()
                    m_result = false;
                } else {
                    // Parse the command
                    std::auto_ptr<Command> cmd(Command::parseCommand(m_cmd, true, false));
                    if (!cmd.get()) {
                        // Invalid command
                        m_result = false;
                    } else {
                        // Create container if necessary
                        CommandContainer& cc = extra->create(g->getViewpointPlayer());

                        // Add to container
                        const Command* actualCommand = cc.addNewCommand(cmd.release());
                        m_result = buildList(session, m_newList, 0, actualCommand, m_newPos);

                        // Notify session listeners.
                        // The connection to game (affected object marked dirty) is done by CommandExtra.
                        session.notifyListeners();
                    }
                }
            }
     private:
        const String_t& m_cmd;
        Infos_t& m_newList;
        size_t& m_newPos;
        bool& m_result;
    };

    bool result = false;
    Request req(cmd, newList, newPos, result);
    link.call(m_gameSender, req);
    return result;
}

void
game::proxy::CommandListProxy::removeCommand(WaitIndicator& link, const String_t& cmd, Infos_t& newList)
{
    class Request : public util::Request<Session> {
     public:
        Request(const String_t& cmd, Infos_t& newList)
            : m_cmd(cmd), m_newList(newList)
            { }
        virtual void handle(Session& session)
            {
                CommandContainer* cc = getCommandContainer(session);
                if (cc != 0) {
                    // Delete by parsing into a proto-command, and removing that by command/id.
                    // An alternative would have been to compare against stringification of each command and then delete that by pointer.
                    std::auto_ptr<Command> cmd(Command::parseCommand(m_cmd, true, true));
                    if (cmd.get()) {
                        cc->removeCommand(cmd->getCommand(), cmd->getId());
                        session.notifyListeners();
                    }
                }

                // Update list
                size_t newPos;
                buildList(session, m_newList, 0, 0, newPos);
            }
     private:
        const String_t& m_cmd;
        Infos_t& m_newList;
    };

    Request req(cmd, newList);
    link.call(m_gameSender, req);
}
