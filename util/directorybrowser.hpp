/**
  *  \file util/directorybrowser.hpp
  *  \brief Class util::DirectoryBrowser
  */
#ifndef C2NG_UTIL_DIRECTORYBROWSER_HPP
#define C2NG_UTIL_DIRECTORYBROWSER_HPP

#include <memory>
#include "afl/io/filesystem.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/base/inlineoptional.hpp"
#include "afl/io/directory.hpp"
#include "util/filenamepattern.hpp"

namespace util {

    /** Directory browser.
        This is the back-end to an interactive directory browser.

        - Construct a DirectoryBrowser object
        - configure it using addFileNamePattern(), setAcceptHiddenEntries().
          Note that without any file name pattern configured, this will not report any files.
        - either open a directory using openDirectory(), or call loadContent() to load the root (list of drives)
        - navigate by calling openChild(), openParent(), or openDirectory().
          Those will all implicitly call loadContent(), so you don't have to.

        Content will be provided as (see respective funictons for details)
        - path(): current path, as a list of Directory objects.
        - directories(): list of subdirectories.
        - files(): list of matching files in this directory
        - getSelectedChild(): one subdirectory can be selected.
          When going up in the hierarchy, the directory we're coming from will be selected.
        - getErrorText(): if loading the directory fails, this will be the error message. */
    class DirectoryBrowser {
     public:
        typedef afl::base::InlineOptional<size_t,size_t(-1)> OptionalIndex_t;

        typedef afl::base::Ptr<afl::io::Directory> DirectoryPtr_t;
        typedef afl::base::Ptr<afl::io::DirectoryEntry> DirectoryEntryPtr_t;

        struct DirectoryItem {
            DirectoryPtr_t dir;
            String_t title;
            DirectoryItem(const DirectoryPtr_t& dir, const String_t& title)
                : dir(dir), title(title)
                { }
        };

        /** Constructor.
            \param fs File System instance */
        explicit DirectoryBrowser(afl::io::FileSystem& fs);

        /** Destructor. */
        ~DirectoryBrowser();

        /** Add a file name pattern.
            Files matching that pattern will be reported.
            The change will take effect after the next loadContent() or open.
            \param pat Pattern */
        void addFileNamePattern(const FileNamePattern& pat);

        /** Clear file name patterns.
            Files will no longer be reported.
            The change will take effect after the next loadContent() or open. */
        void clearFileNamePatterns();

        /** Set whether hidden files/directories will be reported.
            The change will take effect after the next loadContent() or open.
            \param enable true to report hidden entries */
        void setAcceptHiddenEntries(bool enable);

        /** Open directory by name.
            \param name directory name */
        void openDirectory(String_t name);

        /** Open child directory.
            \param n Index into directories(). Out-of-range values are ignored. */
        void openChild(size_t n);

        /** Open parent directory.
            If there is no parent, the call is ignored. */
        void openParent();

        /** Open root.
            This is the list of entry points (drives). */
        void openRoot();

        /** Select a child directory.
            \param n Index into directories(). Out-of-range values are ignored. */
        void selectChild(size_t n);

        String_t createDirectory(String_t name);

        /** Get current directory.
            \return directory object */
        afl::base::Ref<afl::io::Directory> getCurrentDirectory() const;

        /** Load content.
            Reloads the content of the current directory.
            This must be called if you wish to read the directory immediately after constructing the object,
            or if you wish to reload changed content.
            The "open" functions implicitly call loadContent(). */
        void loadContent();

        /** Access current path.
            The path will be empty when we're looking at the root (list of drives/mounts).
            Otherwise, the list contains path components, with the final component at the end.
            This path is typically rendered as a crumb list or tree,
            using the items' getTitle() or, if that is empty, their getDirectoryName().
            \return path */
        const std::vector<DirectoryPtr_t>& path() const;

        /** Access subdirectories.
            Lists all subdirectories, correctly sorted.
            This list contains Directory objects and separate titles.
            Display typically uses the separate titles.
            \return content */
        const std::vector<DirectoryItem>& directories() const;

        /** Access files.
            Lists all files, correctly sorted.
            This list contains DirectoryEntry objects that allow opening the files or querying their properties.
            \return content */
        const std::vector<DirectoryEntryPtr_t>& files() const;

        /** Get selected child index.
            \return index into directories(), or Nothing */
        OptionalIndex_t getSelectedChild() const;

        /** Get error text.
            \return text if error occurred, empty string otherwise. */
        String_t getErrorText() const;

     private:
        afl::io::FileSystem& m_fileSystem;

        std::vector<DirectoryPtr_t> m_path;
        DirectoryPtr_t m_pathOrigin;
        std::vector<DirectoryItem> m_directories;
        std::vector<DirectoryEntryPtr_t> m_files;
        OptionalIndex_t m_selectedDirectory;
        String_t m_error;
        std::vector<FileNamePattern> m_patterns;
        bool m_acceptHiddenEntries;

        bool acceptFile(const String_t& name) const;
    };

}

#endif
