/**
  *  \file game/proxy/commandlistproxy.hpp
  *  \brief Class game::proxy::CommandListProxy
  */
#ifndef C2NG_GAME_PROXY_COMMANDLISTPROXY_HPP
#define C2NG_GAME_PROXY_COMMANDLISTPROXY_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for auxiliary command list access.
        This proxies a game::v3::CommandContainer object.

        Bidirectional synchronous: all operations are independant from each other.
        - initial data acquisition
        - add commands
        - delete commands */
    class CommandListProxy {
     public:
        /** Information about a command. */
        struct Info {
            /** Command text ("give ship 3 to 4"). */
            String_t text;

            /** Information about command (human-readable, translated). */
            String_t info;

            /** Reference to addressed unit.
                Null (!isSet()) if command is not addressed to a unit, or addressed unit is not known/visible. */
            Reference ref;

            Info(const String_t& text, const String_t& info, Reference ref)
                : text(text), info(info), ref(ref)
                { }
        };

        /** Information about all commands. */
        typedef std::vector<Info> Infos_t;

        /** Meta-information. */
        struct MetaInfo {
            /** Player number. */
            int playerNr;

            /** Editable flag. */
            bool editable;

            MetaInfo()
                : playerNr(), editable()
                { }
        };


        /** Constructor.
            \param gameSender Game sender */
        explicit CommandListProxy(util::RequestSender<Session> gameSender);

        /** Load initial state.
            \param link      WaitIndicator object for UI synchronisation
            \param out [out] List of commands
            \param metaOut [out] Metadata */
        bool init(WaitIndicator& link, Infos_t& out, MetaInfo& metaOut);

        /** Add a command.
            \param link          WaitIndicator object for UI synchronisation
            \param cmd     [in]  Command to add
            \param newList [out] Updated list of commands
            \param newPos  [out] Position of new command in list
            \retval true Success
            \retval false Command not accepted, out parameters not updated
            \see game::v3::CommandContainer::addNewCommand, game::v3::Command::parseCommand */
        bool addCommand(WaitIndicator& link, const String_t& cmd, Infos_t& newList, size_t& newPos);

        /** Remove a command.
            \param link          WaitIndicator object for UI synchronisation
            \param cmd     [in]  Command to remove; should be taken from a previous command list
            \param newList [out] Updated list of commands
            \see game::v3::CommandContainer::removeCommand */
        void removeCommand(WaitIndicator& link, const String_t& cmd, Infos_t& newList);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
