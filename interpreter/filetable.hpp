/**
  *  \file interpreter/filetable.hpp
  *  \brief Class interpreter::FileTable
  */
#ifndef C2NG_INTERPRETER_FILETABLE_HPP
#define C2NG_INTERPRETER_FILETABLE_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/data/value.hpp"
#include "afl/io/textfile.hpp"

namespace interpreter {

    /** Table of open files for scripts.
        Scripts address files using a file descriptor, conventionally written as "#n" and represented as a FileValue.
        File descriptors are non-negative values that index a table; the FileTable is configured to a maximum table size.

        File number #0 is valid, but not used by convention.
        (In PCC 1.x, file #0 was used by the interpreter core to implement the user interface's selection I/O functions
        in terms of the script SaveSelection/LoadSelection commands. */
    class FileTable {
     public:
        /** Constructor.
            Makes an empty table. */
        FileTable();

        /** Destructor. */
        ~FileTable();

        /** Set maximum number of files.
            Valid files will be from range [0,n).
            Thus, to allow users to use 1..20, pass 21 here.

            If files outside the range are currently open, they will be closed.

            \param n Limit */
        void setMaxFiles(size_t n);

        /** Open new file.
            If a file is already open on the same file descriptor, it is closed.
            The given stream reference replaces the existing file descriptor.
            The stream's current state will not be changed.
            \param fd File number
            \param ps Stream reference
            \throw Error file number is out of range */
        void openFile(size_t fd, afl::base::Ref<afl::io::Stream> ps);

        /** Close a file.
            It is not an error to attempt to close a file that is not open, or an out-of-range slot.
            \param fd File number */
        void closeFile(size_t fd);

        /** Prepare a file for appending.
            Call immediately after openFile() with a newly-opened file.
            This will detect the file's character set and move the file pointer to the end.
            \param fd File number */
        void prepareForAppend(size_t fd);

        /** Check file argument, produce file number.
            \param fd [out] File number
            \param arg [in] User-supplied argument (integer or FileValue)
            \param mustBeOpen [in] true to accept only open files; false to accept valid but closed slots
            \retval true User-supplied argument was valid, \c fd has been updated
            \retval false User-supplied argument was null
            \throw Error User-supplied argument was out-of-range, wrong type, or not open but mustBeOpen was required */
        bool checkFileArg(size_t& fd, const afl::data::Value* arg, bool mustBeOpen);
        
        /** Check file argument, produce text file pointer.
            \param tf [out] Text file pointer
            \param arg [in] User-supplied argument (integer or FileValue)
            \retval true User-supplied argument was valid, \c tf has been updated to a non-null pointer to a TextFile to use for accessing the file
            \retval false User-supplied argument was null, \c tf has been set to null
            \throw Error User-supplied argument was out-of-range, wrong type, or not open */
        bool checkFileArg(afl::io::TextFile*& tf, const afl::data::Value* arg);

        /** Get a currently-unused slot.
            \return Unused file number; 0 if none */
        size_t getFreeFile() const;

     private:
        class State;

        /** Open script files.
            File numbers are indexes into this array.
            The array size determines the maximum number of files. */
        afl::container::PtrVector<State> m_files;
    };

}

#endif
