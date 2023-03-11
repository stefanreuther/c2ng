/**
  *  \file util/profiledirectory.hpp
  *  \brief Class util::ProfileDirectory
  */
#ifndef C2NG_UTIL_PROFILEDIRECTORY_HPP
#define C2NG_UTIL_PROFILEDIRECTORY_HPP

#include "afl/base/ptr.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/loglistener.hpp"

namespace util {

    /** Profile directory handling.
        This class provides functions to operate with the profile directory.
        The profile directory stores the user's configuration files; compare afl::sys::Environment::getSettingsDirectoryName(). */
    class ProfileDirectory {
     public:
        /** Constructor.
            \param env Environment
            \param fileSystem File system to use
            \param tx Translator (FIXME: needed?)
            \param log Logger (FIXME: needed?) */
        ProfileDirectory(afl::sys::Environment& env, afl::io::FileSystem& fileSystem, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Open file in profile directory for reading.
            If the file or the profile directory does not exist, returns 0.
            \return file, 0 on error.
            \param name File name */
        afl::base::Ptr<afl::io::Stream> openFileNT(String_t name);

        /** Create file.
            If the profile directory does not exist, creates it.
            \param name File name
            \return file
            \throw afl::except::FileProblemException on error */
        afl::base::Ref<afl::io::Stream> createFile(String_t name);

        /** Open profile directory for writing.
            If the profile directory does not exist, creates it.
            \return profile directory
            \throw afl::except::FileProblemException on error */
        afl::base::Ref<afl::io::Directory> open();

     private:
        /// Name of profile directory.
        String_t m_name;

        afl::io::FileSystem& m_fileSystem;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

}

#endif
