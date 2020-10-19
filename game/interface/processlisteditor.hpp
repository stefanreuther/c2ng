/**
  *  \file game/interface/processlisteditor.hpp
  *  \brief Class game::interface::ProcessListEditor
  */
#ifndef C2NG_GAME_INTERFACE_PROCESSLISTEDITOR_HPP
#define C2NG_GAME_INTERFACE_PROCESSLISTEDITOR_HPP

#include <map>
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/reference.hpp"
#include "interpreter/processlist.hpp"

namespace game { namespace interface {

    class NotificationStore;

    /** Process List Editor.
        This class permits manipulation and inquiry of a interpreter::ProcessList.
        It also augments process information with game information (namely, notification status).

        - Suspended processes can be set to Terminated or Runnable (or back to Suspended);
          status changes become effective upon commit.
        - Process priorities can be changed.
        - Processes can be described in human-readable form.

        @change In PCC2, the Process Manager just works on the actual ProcessList object,
        and immediately changes process states; to commit, it just does runRunnableProcesses() etc.
        We collect these operations in a transaction so the scripting session always is in a
        consistent state, and Process Manager can actually be called from a script. */
    class ProcessListEditor {
     public:
        /** Target state of a process. */
        enum State {
            Suspended,           ///< Set process to "Suspended" (=no change).
            Terminated,          ///< Set process to "Termianted" (terminate it).
            Runnable             ///< Set process to "Runnable" (run it once).
        };

        /** Notification status for process. */
        enum NotificationStatus {
            NoMessage,           ///< Process has no notification.
            UnreadMessage,       ///< Process has an unread notification.
            ConfirmedMessage     ///< Notification has been confirmed and process run.
        };

        /** Human-readable process information. */
        struct Info {
            uint32_t processId;                       ///< Process Id (primary key).
            int priority;                             ///< Process priority.
            String_t name;                            ///< Process name.
            String_t status;                          ///< Process status (after application of possible change), stringified.
            Reference invokingObject;                 ///< Invoking object (for auto tasks, unit this is the auto task for).
            bool isChanged;                           ///< true if a status change is desired but not executed yet.
            NotificationStatus notificationStatus;    ///< Notification status.
        };

        /** Constructor.
            Makes a new ProcessListEditor with no changes queued.
            \param list Process list */
        explicit ProcessListEditor(interpreter::ProcessList& list);

        /** Get number of processes.
            \return number of processes */
        size_t getNumProcesses() const;

        /** Describe a process.
            \param [in] slotNr   Slot number [0,getNumProcesses())
            \param [out] info    Result
            \param [in] notif    Notification store (for notificationStatus field)
            \param [in] tx       Translator (for stringification)
            \return true on success, false if slotNr out of range */
        bool describe(size_t slotNr, Info& info, const NotificationStore& notif, afl::string::Translator& tx) const;

        /** Prepare a state change.
            The change will be executed when commit() is called.
            The call is ignored if \c pid doesn't refer to an applicable (=Suspended) process.
            \param pid   Process id
            \param state Target state */
        void setProcessState(uint32_t pid, State state);

        /** Prepare a state change for all processes.
            The changes will be executed when commit() is called.
            \param state Target state */
        void setAllProcessState(State state);

        /** Set process priority.
            This will immediately update the process list.
            There is no restriction on the affected process's state.
            \param pid   Process id
            \param pri   Priority */
        void setProcessPriority(uint32_t pid, int pri);

        /** Perform all prepared state changes.
            Processes that are made Runnable are placed in the given process group Id;
            call ProcessList::startProcessGroup() on it.
            \param pgid Process group Id */
        void commit(uint32_t pgid);

     private:
        interpreter::ProcessList& m_list;
        std::map<uint32_t, State> m_changes;
    };

} }

#endif
