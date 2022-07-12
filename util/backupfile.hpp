/**
  *  \file util/backupfile.hpp
  *  \brief Class util::BackupFile
  */
#ifndef C2NG_UTIL_BACKUPFILE_HPP
#define C2NG_UTIL_BACKUPFILE_HPP

#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"

namespace util {

    /** Backup file creation.
        This contains the logic to expand backup file path specifications, and to create the backup files. */
    class BackupFile {
     public:
        /** Create blank template. */
        BackupFile();
        ~BackupFile();

        /*
         *  Configuration
         */

        /** Set directory name for '%d' variable.
            \param dir Name of directory */
        void setGameDirectoryName(String_t dir);

        /** Set player number for '%p' variable.
            \param nr Player number */
        void setPlayerNumber(int nr);

        /** Set turn number for '%t' variable.
            \param nr Turn number */
        void setTurnNumber(int nr);

        /*
         *  Templates & Backups
         */

        /** Expand a template.
            Returns the expanded path name.
            \param fs File System instance
            \param tpl Template to expand
            \return expanded name */
        String_t expandFileName(afl::io::FileSystem& fs, String_t tpl);

        /** Copy a file, using a template.
            \param fs File System instance
            \param tpl Template file name (can be empty; in this case, the function does nothing)
            \param src Source file */
        void copyFile(afl::io::FileSystem& fs, String_t tpl, afl::io::Stream& src);

        /** Erase a file, using a template.
            \param fs File System instance
            \param tpl Template file name (can be empty; in this case, the function does nothing) */
        void eraseFile(afl::io::FileSystem& fs, String_t tpl);

        /** Check existance of a file, using a template.
            \param fs File System instance
            \param tpl Template file name (can be empty; in this case, the function does nothing) */
        bool hasFile(afl::io::FileSystem& fs, String_t tpl);

        /** Open file for reading, using a template.
            \param fs File System instance
            \param tpl Template file name (can be empty; in this case, the function does nothing)
            \param tx Translator (for error messages)
            \return file, never null
            \throw FileProblemException if file does not exist */
        afl::base::Ref<afl::io::Stream> openFile(afl::io::FileSystem& fs, String_t tpl, afl::string::Translator& tx);

     private:
        String_t m_gameDirectory;
        int m_playerNumber;
        int m_turnNumber;
    };

}

#endif
