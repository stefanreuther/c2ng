/**
  *  \file util/process/subprocess.hpp
  *  \brief Interface util::process::Subprocess
  */
#ifndef C2NG_UTIL_PROCESS_SUBPROCESS_HPP
#define C2NG_UTIL_PROCESS_SUBPROCESS_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace util { namespace process {

    /** Basic subprocess abstraction.
        This is a very simple abstraction mainly to implement c2router.

        It supports sending and receiving text lines to the subprocess. Standard output and error are not distinguished.
        It assumes that the subprocess behaves nicely, i.e. does not try to sabotage us and talks the agreed protocol.

        Missing features:
        - abstraction of process state (running, crashed, exited, etc.)
        - asynchronous communication
        - precise input/output redirection

        Also see ProcessRunner for a different take on this subject. */
    class Subprocess : public afl::base::Deletable {
     public:
        /** Check whether process is active (running).
            This needs not implement any fancy checks; it suffices to track the start/stop state.
            \return true if process is started and not stopped */
        virtual bool isActive() const = 0;

        /** Get process Id.
            This is mainly for human use to find the process in 'ps' output.
            \return process Id
            \pre isActive() */
        virtual uint32_t getProcessId() const = 0;

        /** Start the process.
            If the process is already started (isActive()), return successfully.
            \param path Path name of program
            \param args Command-line parameters
            \retval true process started successfully. Note that this does NOT mean that the program is actually running;
                         it means that just the fork() or the start of the shell succeeded.
            \retval false process start failed. Use getStatus() to obtain status. */
        virtual bool start(const String_t& path, afl::base::Memory<const String_t> args) = 0;

        /** Stop the process.
            If the process is already stopped (!isActive()), return successfully.
            This closes the process' standard input which hopefully causes it to exit.
            It waits for the process' standard output to finish.
            \retval true process stopped successfully
            \retval true process stopped unsuccessfully (e.g. crashed) */
        virtual bool stop() = 0;

        /** Write a line of text to the process.
            \param line Line INCLUDING TRAILING "\n". Can also be multiple lines, each ending in "\n".
            \retval true Line written successfully
            \retval false Write failure (process not taking input) */
        virtual bool writeLine(const String_t& line) = 0;

        /** Read a line of text from the process.
            \param result [out] Line INCLUDING TRAILING "\n".
            \retval true Line read correctly
            \retval false Process did not produce a whole line */
        virtual bool readLine(String_t& result) = 0;

        /** Get status.
            After stop(), this is the human-readable termination status.
            After a failed start(), this is the human-readable failure reason.
            \return status */
        virtual String_t getStatus() const = 0;
    };

} }

#endif
