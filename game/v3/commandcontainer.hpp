/**
  *  \file game/v3/commandcontainer.hpp
  *  \brief Class game::v3::CommandContainer
  */
#ifndef C2NG_GAME_V3_COMMANDCONTAINER_HPP
#define C2NG_GAME_V3_COMMANDCONTAINER_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrmultilist.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "game/timestamp.hpp"
#include "game/v3/command.hpp"

namespace game { namespace v3 {

    /** Command Container

        This class manages a set of commands (Command).
        It implements storing / querying the command list, and the replace policy for the commands.

        This container owns all the Command objects.

        \todo currently, this is a simple list, like it was in PCC v1.
        Should performance be a problem, change it to std::map or somesuch. */
    class CommandContainer {
     public:
        // \change was std::list<Command*> in PCC2 - is PtrMultiList the right replacement?
        typedef afl::container::PtrMultiList<Command> CommandList_t;

        typedef CommandList_t::iterator Iterator_t;
        typedef CommandList_t::const_iterator ConstIterator_t;

        /** Constructor.
            Makes an empty container. */
        CommandContainer();

        /** Destructor. */
        ~CommandContainer();

        /** Clear this container.
            Discards all commands and invalidates all Command pointers. */
        void clear();

        /** Get command by type and Id.
            \param typ Type to search
            \param id  Id to search
            \return matching command, if any */
        const Command* getCommand(Command::Type typ, Id_t id) const;

        /** Add a command.
            If a command with the same typ/id pair exists, it is overwritten;
            otherwise, a new command is created.
            \param typ Type
            \param id Id
            \param arg Parameter
            \return command (owned by CommandContainer) */
        const Command* addCommand(Command::Type typ, Id_t id, String_t arg);

        /** Add a command.
            If a command with the same typ/id pair exists, it is overwritten.
            Otherwise, the \c cmd object is added to this container.
            \param cmd Command to add. Null values ignored.
                       Command becomes owned by CommandContainer and will be managed or deleted.
            \return command (owned by CommandContainer; can be same as \c cmd or not) */
        const Command* addNewCommand(Command* cmd);

        /** Remove a command.
            \param typ Type to search
            \param id  Id to search
            \retval true Command found and removed
            \retval false Command not found */
        bool removeCommand(Command::Type typ, Id_t id);

        /** Remove a command.
            \param cmd Command to remove. Must be a command owned by this CommandContainer. */
        void removeCommand(const Command* cmd);

        /** Remove commands by affected unit.
            \param ref Affected unit. See Command::getAffectedUnit */
        void removeCommandsByReference(Reference ref);

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

        /** Load command file (cmdX.txt).
            \param file File
            \param time Current turn timestamp, for validating file timestamp */
        void loadCommandFile(afl::io::Stream& file, const Timestamp& time);

        /** Signal: single command changed.
            \param command Command that changed
            \param added true: command was added or modified. false: command is going to be deleted */
        afl::base::Signal<void(Command&, bool)> sig_commandChange;

     private:
        CommandList_t cmds;

        /** Find command by type/id.
            \param typ Type to search
            \param id  Id to search
            \return iterator pointing to command, or end() */
        Iterator_t findCommand(Command::Type typ, Id_t id);

        /** Insert newly-created command at proper position.
            This is the backend of the addCommand() functions.

            This function assumes responsibility for cmd; if anything in it throws, the command will be deleted.
            \param cmd New command object */
        void insertNewCommand(Command* cmd);
    };

} }

#endif
