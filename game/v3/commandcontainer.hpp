/**
  *  \file game/v3/commandcontainer.hpp
  */
#ifndef C2NG_GAME_V3_COMMANDCONTAINER_HPP
#define C2NG_GAME_V3_COMMANDCONTAINER_HPP

#include "afl/container/ptrmultilist.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "game/v3/command.hpp"
#include "game/timestamp.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace v3 {

    // /** Command Container

    //     This class manages a set of commands (GCommand). It implements
    //     storing / querying the command list, and the replace policy for
    //     the commands.

    //     This container owns all the GCommand objects it allocates.

    //     \todo currently, this is a simple list, like it was in PCC v1.
    //     Should performance be a problem, change it to std::map or
    //     somesuch. */
    class CommandContainer {
     public:
        // \change was std::list<Command*> in PCC2 - is PtrMultiList the right replacement?
        typedef afl::container::PtrMultiList<Command> CommandList_t;

        typedef CommandList_t::iterator Iterator_t;
        typedef CommandList_t::const_iterator ConstIterator_t;

        CommandContainer();
        ~CommandContainer();

        void clear();

        const Command* getCommand(Command::Type typ, Id_t id);
        const Command* addCommand(Command::Type typ, Id_t id, String_t arg);
        const Command* addNewCommand(Command* cmd);
        bool           removeCommand(Command::Type typ, Id_t id);
        void           removeCommand(Command* cmd);

        // Iterator_t begin()
        //     { return cmds.begin(); }
        ConstIterator_t begin() const
            { return cmds.begin(); }
        // Iterator_t end()
        //     { return cmds.end(); }
        ConstIterator_t end() const
            { return cmds.end(); }
        // void erase(Iterator_t it) // \change: previously returned iterator
        //     { return cmds.erase(it); }

        void loadCommandFile(afl::io::Stream& file, const Timestamp& time);

        /** Signal: single command changed.
            \param command Command that changed
            \param added true: command was added or modified. false: command is going to be deleted */
        afl::base::Signal<void(Command&, bool)> sig_commandChange;

     private:
        CommandList_t cmds;

        Iterator_t findCommand(Command::Type typ, Id_t id);

        static int getValue(Command::Type type);
        void insertNewCommand(Command* cmd);
    };
} }

#endif
