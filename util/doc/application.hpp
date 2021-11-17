/**
  *  \file util/doc/application.hpp
  *  \brief Class util::doc::Application
  */
#ifndef C2NG_UTIL_DOC_APPLICATION_HPP
#define C2NG_UTIL_DOC_APPLICATION_HPP

#include "afl/sys/commandlineparser.hpp"
#include "util/application.hpp"
#include "util/doc/index.hpp"

namespace util { namespace doc {

    /** Documentation Repository Manager Application.
        This command-line application allows creation, modification, and inquiry of documentation repositories. */
    class Application : public util::Application {
     public:
        /** Constructor.
            @param env   Environment
            @param fs    File System */
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs);

        /** Main entry point. */
        void appMain();

     private:
        // Data parameters:
        struct DataParameters;
        struct DataReference;
        bool handleDataOption(DataParameters& data, const String_t& text, afl::sys::CommandLineParser& parser);
        bool hasDataParameters(const DataParameters& data) const;
        void loadData(DataReference& ref, const DataParameters& data);
        void saveData(DataReference& ref, const DataParameters& data);

        // Node parameters:
        struct NodeParameters;
        bool handleNodeOption(NodeParameters& np, const String_t& text, afl::sys::CommandLineParser& parser);
        Index::Handle_t addDocument(DataReference& ref, const NodeParameters& np, bool asPage);

        // Commands:
        struct ListParameters;
        void addGroup(DataParameters& data, afl::sys::CommandLineParser& parser);
        void importHelp(DataParameters& data, afl::sys::CommandLineParser& parser);
        void importText(DataParameters& data, afl::sys::CommandLineParser& parser);
        void listContent(DataParameters& data, afl::sys::CommandLineParser& parser);
        void getContent(DataParameters& data, afl::sys::CommandLineParser& parser);
        void renderContent(DataParameters& data, afl::sys::CommandLineParser& parser);

        void help();

        void errorExitBadOption();
        void errorExitBadNonoption();
        void listContentRecursive(const DataReference& ref, const ListParameters& lp, Index::Handle_t hdl, String_t indent, String_t docName);
        void listNodeInfo(const DataReference& ref, const ListParameters& lp, Index::Handle_t hdl, String_t indent, String_t docName);
    };

} }

#endif
