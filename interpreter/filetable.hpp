/**
  *  \file interpreter/filetable.hpp
  */
#ifndef C2NG_INTERPRETER_FILETABLE_HPP
#define C2NG_INTERPRETER_FILETABLE_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/io/textfile.hpp"
#include "afl/data/value.hpp"

namespace interpreter {

    class FileTable {
     public:
        FileTable();
        ~FileTable();

        void setMaxFiles(size_t n);

        void openFile(size_t fd, afl::base::Ref<afl::io::Stream> ps);

        void closeFile(size_t fd);

        void prepareForAppend(size_t fd);

        bool checkFileArg(size_t& fd, afl::data::Value* arg, bool mustBeOpen);
        
        bool checkFileArg(afl::io::TextFile*& tf, afl::data::Value* arg);

        size_t getFreeFile() const;

     private:
        class State;

        // /** Open script files. Note that slot 0 exists here, but we never
        //     allocate it to the user unless he explicitly requests it. It is
        //     reserved to the implementation, just in case. In PCC 1.x, file #0
        //     was reserved to the core, and not accessible through the script
        //     side. It was used to implement the user interface's selection I/O
        //     functions in terms of the script SaveSelection/LoadSelection
        //     implementation. */
        afl::container::PtrVector<State> m_files;

    };

}

#endif
