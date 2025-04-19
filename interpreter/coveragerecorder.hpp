/**
  *  \file interpreter/coveragerecorder.hpp
  *  \brief Class interpreter::CoverageRecorder
  */
#ifndef C2NG_INTERPRETER_COVERAGERECORDER_HPP
#define C2NG_INTERPRETER_COVERAGERECORDER_HPP

#include <set>

#include "afl/container/ptrmap.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/string.hpp"
#include "interpreter/process.hpp"

namespace interpreter {

    class Process;
    class BytecodeObject;

    /** Coverage recorder.
        This class can produce lcov-compatible coverage output (*.info file).
        Note that this class only records boolean coverage (covered/not covered),
        does not record individual call counts for functions and lines.

        Coverage can only be computed for code that has a source location (BytecodeObject::getFileName()).

        Coverage recording slows down execution by a factor of roughly 1.75.

        To record coverage for a script,
        - addBCO() all subject code (optional; establishes baseline for measurement);
        - pass this as Process::Observer parameter to Process::run() or ProcessList::run() to record coverage,
          or call addProcessState() otherwise;
        - use save() to save to a *.info file;
        - use the lcov "genhtml" command to produce a HTML report. */
    class CoverageRecorder : public Process::Observer {
     public:
        /** Constructor. */
        CoverageRecorder();

        /** Destructor. */
        ~CoverageRecorder();

        /** Implementation of Process::Observer.
            Record coverage per addProcessState().
            @param p Process */
        virtual void checkProcess(Process& p);

        /** Add code.
            Records the presence of the given code with zero coverage.
            Can be called any number of times.
            This will add the given code, and code referenced by it as literals, recursively.
            Thus, adding the BytecodeObject representing a file will add all subroutines and hooks
            defined therein as well.
            @param bco Code */
        void addBCO(const BytecodeObject& bco);

        /** Handle process state.
            Records the line/function that process is currently in as taken.
            @param proc Process */
        void addProcessState(const Process& proc);

        /** Save to file.
            @param out         Output file, open for writing
            @param testName    Test name to use in output file ("TN:" tag) */
        void save(afl::io::Stream& out, String_t testName);

     private:
        struct Function;
        struct File;
        struct CompareFunctions;

        typedef std::set<const void*> SeenSet_t;
        typedef afl::container::PtrMap<String_t, File> FileMap_t;
        typedef afl::container::PtrMap<const void*, Function> FunctionMap_t;
        typedef std::map<uint32_t, bool> LineMap_t;

        /** Main data storage. */
        FileMap_t m_files;

        /** Save a "File" object.
            @param tf   TextFile for output
            @param file File object */
        void saveFile(afl::io::TextFile& tf, const File& file);

        /** Add a function.
            If function is already known, the call is ignored.
            Otherwise, recursively also adds all functions referenced from it.
            @param bco  Code
            @param seen Set of functions that were already seen
            @return Function object; null if function cannot be added because it has no source location */
        Function* addFunction(const BytecodeObject& bco, SeenSet_t& seen);

        /** Add a file.
            If the BCO's file is already known, returns it, otherwise, adds it.
            @param bco  Code
            @return File object; null if BCO has no source location */
        File* addFile(const BytecodeObject& bco);
    };

}

#endif
