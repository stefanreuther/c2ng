/**
  *  \file util/processrunner.hpp
  *  \brief Class util::ProcessRunner
  */
#ifndef C2NG_UTIL_PROCESSRUNNER_HPP
#define C2NG_UTIL_PROCESSRUNNER_HPP

#include <memory>
#include "afl/data/stringlist.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/uncopyable.hpp"

namespace util {

    /** Invoking external processes.

        The main program will be using a number of file descriptors for
        various things. In particular the file descriptors obtained by
        'accept' cannot necessarily be made close-on-exec reliably. To
        avoid these fds leaking into child processes, we isolate running
        child processes into a helper process:

            Main Program <-> Helper Process <-> Actual Child

        The helper process must be started as early as possible when the
        environment is still clean and no background threads are running.
        Therefore, create the ProcessRunner as early as possible, before
        going multithreaded.

        The main process talks to the helper process through two pipes.
        It uses our standard protocol framing to pass commands and result;
        the actual "on-wire" format is implementation dependant.

        Note that creating multiple ProcessRunner's will create multiple
        helper processes, which means the second helper inherits the
        first's pipes. This is actually a variation of the very same
        problem we want to avoid, but with a much lesser impact (child
        processes are not connected to the network), and we know they are
        close-on-exec.

        Each ProcessRunner can run one child process at a time, and thus
        automatically serializes its run() calls.

        FIXME: A class like this ought to be in afl, with an implementation
        for all operating systems, and support for more attributes of the
        subprocesses (e.g. environment, interactive I/O). Until we have that,
        this is a ad-hoc POSIX-only implementation suffices for our needs
        in c2host. */
    class ProcessRunner : afl::base::Uncopyable {
     public:
        /** A command to execute. */
        struct Command {
            /** Command line.
                First entry is the command name used to invoke the command, and as argv[0] for the command.
                Subsequent entries are more argv[] parameters. */
            afl::data::StringList_t command;

            /** Work directory.
                If given, this is the work directory for the command.
                If unset, the command inherits the ProcessRunner's work directory. */
            afl::base::Optional<String_t> workDirectory;
        };

        /** Constructor.
            Creates the helper process. Call as early as possible. */
        ProcessRunner();

        /** Destructor.
            Stops the helper process. */
        ~ProcessRunner();

        /** Run child process.
            \param [in]  cmd    Command to run (argv)
            \param [out] output Output will be collected here
            \return Return value: 0..255 for regular exit, 1000+ for signal death, 1999 for unknown.
            \throw runtime_error if helper process encounters an API error (e.g. system resources exhausted) */
        int run(const Command& cmd, String_t& output);

     private:
        class Impl;
        std::auto_ptr<Impl> m_pImpl;
    };

}

#endif
