/**
  *  \file util/helpindex.hpp
  *  \brief Class util::HelpIndex
  */
#ifndef C2NG_UTIL_HELPINDEX_HPP
#define C2NG_UTIL_HELPINDEX_HPP

#include <vector>
#include <map>
#include "afl/container/ptrvector.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace util {

    /** Help index.
        Manages a set of help files.

        Help files are XML files consisting of pages
           <page id="name:of:the:page">...</page>
        Pages can be grouped and given a priority
           <group priority=123><page...></page>...</group>
           <help priority=123><page...></page>...</group>

        This class (lazily) pre-parses the help files to find all pages and associated priorities.
        It does not interpret the content of the pages, it just needs to be balanced XML.

        Users can then obtain a list of file positions to combine into a help page:
        - if multiple pages are provided with different priorities, those are concatenated in increasing priority order.
          Pages that have no priority are treated as priority 100.
        - if multiple pages are provided with the same priority, the one from the file added last is used
          (this is the case that the help file is replaced by a different file, e.g. in a new language). */
    class HelpIndex {
     public:
        /** Type alias: file position. */
        typedef afl::io::Stream::FileSize_t FileSize_t;

        /** Definition of a help file. */
        struct File {
            String_t name;          /**< File name. For use with afl::io::FileSystem::openFile. */
            String_t origin;        /**< Origin marker. Can be used to identify the set of files this file is part of. */
            bool scanned;           /**< true if this file has already been scanned. */
            int serial;             /**< Serial number to resolve ties in priority resolution. */
            File(const String_t& name, const String_t& origin, int serial)
                : name(name), origin(origin), scanned(false), serial(serial)
                { }
        };

        /** Definition of a (part of) a help page. */
        struct Node {
            int priority;           /**< Page priority. */
            const File& file;       /**< File this page is part of. */
            FileSize_t pos;         /**< File position of the <page> tag. */
            Node(int pri, const File& file, FileSize_t pos)
                : priority(pri), file(file), pos(pos)
                { }
        };

        /** Type of a query result. */
        typedef std::vector<const Node*> NodeVector_t;

        /** Default constructor.
            Makes an empty HelpIndex. */
        HelpIndex();

        /** Destructor. */
        ~HelpIndex();

        /** Add a file to the help index.
            The file will be added but not yet scanned.
            \param name File name (for afl::io::FileSystem)
            \param origin Origin marker. Can be used with removeFilesByOrigin() */
        void addFile(String_t name, String_t origin);

        /** Remove files from the help index.
            Removes all files and pages that were added with the given origin marker.
            \param origin Origin marker. */
        void removeFilesByOrigin(String_t origin);

        /** Find help page.
            Locates pages (fragments) for the given pages, resolves priorities, and returns a list of nodes.
            If the page does not exist, the result will be an empty vector.
            If multiple files provide page fragments, result can have more than one node.
            Caller can use this information to render the help page.

            This call will scan files that have not yet been scanned.

            \param page [in] Name of help page to find
            \param out  [out] Result goes here
            \param fs   Handle to file system
            \param log  Logger
            \param tx   Translator */
        void find(String_t page, NodeVector_t& out, afl::io::FileSystem& fs, afl::sys::LogListener& log, afl::string::Translator& tx);

     private:
        struct Page {
            std::vector<Node> nodes;
        };
        afl::container::PtrVector<File> m_files;
        std::multimap<String_t, Node> m_nodes;
        int m_counter;

        void scanNewFiles(afl::io::FileSystem& fs, afl::sys::LogListener& log, afl::string::Translator& tx);
        void scanFile(const File& file, afl::io::FileSystem& fs);
    };

}

#endif
