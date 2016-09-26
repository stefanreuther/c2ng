/**
  *  \file util/profiledirectory.hpp
  */
#ifndef C2NG_UTIL_PROFILEDIRECTORY_HPP
#define C2NG_UTIL_PROFILEDIRECTORY_HPP

#include "afl/sys/environment.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/io/directory.hpp"

namespace util {

    class ProfileDirectory {
     public:
        ProfileDirectory(afl::sys::Environment& env,
                         afl::io::FileSystem& fileSystem,
                         afl::string::Translator& tx, afl::sys::LogListener& log);

        afl::base::Ptr<afl::io::Stream> openFile(String_t name);

        afl::base::Ptr<afl::io::Stream> createFile(String_t name);

        afl::base::Ptr<afl::io::Directory> open();

     private:
        String_t m_name;

        afl::io::FileSystem& m_fileSystem;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        
    };

}

#endif
