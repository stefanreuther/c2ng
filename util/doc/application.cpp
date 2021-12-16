/**
  *  \file util/doc/application.cpp
  *  \brief Class util::doc::Application
  */

#include <memory>
#include "util/doc/application.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/ref.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/node.hpp"
#include "afl/io/xml/parser.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "util/charsetfactory.hpp"
#include "util/doc/blobstore.hpp"
#include "util/doc/fileblobstore.hpp"
#include "util/doc/helpimport.hpp"
#include "util/doc/htmlrenderer.hpp"
#include "util/doc/index.hpp"
#include "util/doc/loggingverifier.hpp"
#include "util/doc/renderoptions.hpp"
#include "util/doc/singleblobstore.hpp"
#include "util/doc/summarizingverifier.hpp"
#include "util/doc/textimport.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Optional;
using afl::base::Ptr;
using afl::base::Ref;
using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::io::ConstMemoryStream;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileMapping;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextWriter;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Nodes_t;
using afl::io::xml::Parser;
using afl::io::xml::Reader;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::StandardCommandLineParser;

/*
 *  DataParameters - Template to build a DataReference
 */

struct util::doc::Application::DataParameters {
    bool useSingle;
    Optional<String_t> dirName;
    DataParameters()
        : useSingle(false), dirName()
        { }
};

/*
 *  DataReference - Live Data
 */

struct util::doc::Application::DataReference {
    Index index;
    std::auto_ptr<BlobStore> blobStore;
};

/*
 *  NodeParameters - Parameters for building a Node
 */

struct util::doc::Application::NodeParameters {
    String_t below;
    std::vector<String_t> ids;
    std::vector<String_t> tags;
    String_t name;
    afl::base::Optional<bool> asPage;
};


/*
 *  Application - Main Entry Point
 */

util::doc::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : util::Application(env, fs)
{ }

void
util::doc::Application::appMain()
{
    StandardCommandLineParser parser(environment().getCommandLine());
    Translator& tx = translator();
    String_t text;
    bool option;
    Optional<String_t> command;
    DataParameters data;

    // Global options and command
    while (!command.isValid() && parser.getNext(option, text)) {
        if (option) {
            if (handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "log") {
                consoleLogger().setConfiguration(parser.getRequiredParameter("log"));
            } else if (text == "h" || text == "help") {
                help();
            } else {
                errorExitBadOption();
            }
        } else {
            command = text;
        }
    }

    // Do we have a command?
    const String_t* pc = command.get();
    if (pc == 0) {
        errorExit(Format(tx("no command specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Dispatch on command
    if (*pc == "help") {
        help();
    } else if (*pc == "add-group") {
        addGroup(data, parser);
    } else if (*pc == "import-help") {
        importHelp(data, parser);
    } else if (*pc == "import-text") {
        importText(data, parser);
    } else if (*pc == "ls") {
        listContent(data, parser);
    } else if (*pc == "get") {
        getContent(data, parser);
    } else if (*pc == "render") {
        renderContent(data, parser);
    } else if (*pc == "verify") {
        verifyContent(data, parser);
    } else {
        errorExit(Format(tx("unknown command specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }
}

/*
 *  Data Parameters
 */

bool
util::doc::Application::handleDataOption(DataParameters& data, const String_t& text, afl::sys::CommandLineParser& parser)
{
    if (text == "dir") {
        data.useSingle = false;
        data.dirName = parser.getRequiredParameter(text);
        return true;
    } else if (text == "single") {
        data.useSingle = true;
        data.dirName = parser.getRequiredParameter(text);
        return true;
    } else {
        return false;
    }
}

bool
util::doc::Application::hasDataParameters(const DataParameters& data) const
{
    return data.dirName.isValid();
}

void
util::doc::Application::loadData(DataReference& ref, const DataParameters& data)
{
    // Obtain directory name
    Translator& tx = translator();
    const String_t* dirName = data.dirName.get();
    if (dirName == 0) {
        errorExit(Format(tx("repository location not specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Open directory [must exist]
    Ref<Directory> dir = fileSystem().openDirectory(*dirName);
    Ptr<Stream> file = dir->openFileNT("index.xml", FileSystem::OpenRead);
    if (file.get() != 0) {
        ref.index.load(*file);
    }

    if (data.useSingle) {
        Ptr<Stream> file = dir->openFileNT("content.tar", FileSystem::OpenWrite);
        if (file.get() == 0) {
            file = dir->openFile("content.tar", FileSystem::Create).asPtr();
        }
        ref.blobStore.reset(new SingleBlobStore(*file));
    } else {
        Ref<DirectoryEntry> content = dir->getDirectoryEntryByName("content");
        if (content->getFileType() != DirectoryEntry::tDirectory) {
            content->createAsDirectory();
        }
        ref.blobStore.reset(new FileBlobStore(content->openDirectory()));
    }
}

void
util::doc::Application::saveData(DataReference& ref, const DataParameters& data)
{
    // Obtain directory name
    Translator& tx = translator();
    const String_t* dirName = data.dirName.get();
    if (dirName == 0) {
        errorExit(Format(tx("repository location not specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Save the XML file
    ref.index.save(*fileSystem().openDirectory(*dirName)->openFile("index.xml", FileSystem::Create));
}

/*
 *  Node Parameters
 */

bool
util::doc::Application::handleNodeOption(NodeParameters& np, const String_t& text, afl::sys::CommandLineParser& parser)
{
    if (text == "below") {
        np.below = parser.getRequiredParameter(text);
        return true;
    } else if (text == "id") {
        np.ids.push_back(parser.getRequiredParameter(text));
        return true;
    } else if (text == "tag") {
        np.tags.push_back(parser.getRequiredParameter(text));
        return true;
    } else if (text == "name") {
        np.name = parser.getRequiredParameter(text);
        return true;
    } else if (text == "page") {
        np.asPage = true;
        return true;
    } else if (text == "document") {
        np.asPage = false;
        return true;
    } else {
        return false;
    }
}

util::doc::Index::Handle_t
util::doc::Application::addDocument(DataReference& ref, const NodeParameters& np, bool asPage)
{
    // Look up 'below'
    Index::Handle_t parent;
    if (np.below.empty()) {
        parent = ref.index.root();
    } else {
        String_t tmp;
        if (!ref.index.findNodeByAddress(np.below, parent, tmp)) {
            errorExit(Format(translator()("unable to resolve node: \"%s\""), np.below));
        }
    }

    // Create the node
    Index::Handle_t newNode = np.asPage.orElse(asPage)
        ? ref.index.addPage    (parent, "", np.name, "")
        : ref.index.addDocument(parent, "", np.name, "");

    // Add parameters.
    // Instead of pre-verifying ("do we have a nonempty Id?"), let Index process it, and verify the result.
    for (size_t i = 0; i < np.ids.size(); ++i) {
        ref.index.addNodeIds(newNode, np.ids[i]);
    }
    for (size_t i = 0; i < np.tags.size(); ++i) {
        ref.index.addNodeTags(newNode, np.tags[i]);
    }

    if (ref.index.getNumNodeIds(newNode) == 0) {
        errorExit(translator()("missing node Id"));
    }

    return newNode;
}


/*
 *  Commands
 */

struct util::doc::Application::ListParameters {
    enum Mode { Content, Recursive, Self };
    Mode mode;
    enum Format { Address, Title, Long };
    Format format;
    bool tree;
    ListParameters()
        : mode(Content), format(Address), tree(false)
        { }
};

void
util::doc::Application::addGroup(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    NodeParameters np;
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleNodeOption(np, text, parser) || handleDataOption(data, text, parser)) {
                // ok
            } else {
                errorExitBadOption();
            }
        } else {
            errorExitBadNonoption();
        }
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    addDocument(ref, np, false);
    saveData(ref, data);
}

void
util::doc::Application::importHelp(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    NodeParameters np;
    std::vector<String_t> fileNames;
    String_t text;
    int flags = 0;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleNodeOption(np, text, parser) || handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "remove-source") {
                flags |= ImportHelp_RemoveSource;
            } else {
                errorExitBadOption();
            }
        } else {
            fileNames.push_back(text);
        }
    }

    if (fileNames.empty()) {
        errorExit(Format(translator()("no file name specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    Index::Handle_t hdl = addDocument(ref, np, false);
    for (size_t i = 0; i < fileNames.size(); ++i) {
        Ref<Stream> file = fileSystem().openFile(fileNames[i], FileSystem::OpenRead);
        util::doc::importHelp(ref.index, hdl, *ref.blobStore, *file, flags, log(), translator());
    }
    saveData(ref, data);
}

void
util::doc::Application::importText(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    NodeParameters np;
    std::auto_ptr<Charset> cs(new CodepageCharset(afl::charset::g_codepageLatin1));
    afl::base::Optional<String_t> fileName;
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleNodeOption(np, text, parser) || handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "charset") {
                Charset* p = CharsetFactory().createCharset(parser.getRequiredParameter(text));
                if (p == 0) {
                    errorExit(translator()("the specified character set is not known"));
                }
                cs.reset(p);
            } else {
                errorExitBadOption();
            }
        } else {
            if (fileName.isValid()) {
                errorExit(translator()("too many arguments"));
            }
            fileName = text;
        }
    }

    const String_t* pfn = fileName.get();
    if (pfn == 0) {
        errorExit(Format(translator()("no file name specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    Index::Handle_t hdl = addDocument(ref, np, true);
    Ref<Stream> file = fileSystem().openFile(*pfn, FileSystem::OpenRead);
    util::doc::importText(ref.index, hdl, *ref.blobStore, *file, *cs);
    saveData(ref, data);
}

void
util::doc::Application::listContent(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    std::vector<String_t> roots;
    ListParameters lp;
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "l" || text == "long") {
                lp.format = ListParameters::Long;
            } else if (text == "t" || text == "title") {
                lp.format = ListParameters::Title;
            } else if (text == "f" || text == "forest" || text == "tree") {
                lp.tree = true;
            } else if (text == "r" || text == "recursive" || text == "recurse") {
                lp.mode = ListParameters::Recursive;
            } else if (text == "d" || text == "self" || text == "directory") {
                lp.mode = ListParameters::Self;
            } else {
                errorExitBadOption();
            }
        } else {
            roots.push_back(text);
        }
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    if (roots.empty()) {
        listContentRecursive(ref, lp, ref.index.root(), String_t(), String_t());
    } else {
        for (size_t i = 0; i < roots.size(); ++i) {
            Index::Handle_t hdl;
            String_t tmp;
            if (ref.index.findNodeByAddress(roots[i], hdl, tmp)) {
                listContentRecursive(ref, lp, hdl, String_t(), tmp);
            } else {
                errorOutput().writeLine(Format(translator()("%s: not found"), roots[i]));
            }
        }
    }
}

void
util::doc::Application::getContent(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    std::vector<String_t> items;
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleDataOption(data, text, parser)) {
                // ok
            } else {
                errorExitBadOption();
            }
        } else {
            items.push_back(text);
        }
    }

    if (items.empty()) {
        errorExit(Format(translator()("no node name specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    for (size_t i = 0; i < items.size(); ++i) {
        Index::Handle_t hdl;
        String_t tmp;
        if (ref.index.findNodeByAddress(items[i], hdl, tmp)) {
            BlobStore::ObjectId_t objId = ref.index.getNodeContentId(hdl);
            if (!objId.empty()) {
                standardOutput().writeText(afl::string::fromBytes(ref.blobStore->getObject(objId)->get()));
            }
        } else {
            errorOutput().writeLine(Format(translator()("%s: not found"), items[i]));
        }
    }
}

void
util::doc::Application::renderContent(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    RenderOptions opts;
    std::vector<String_t> items;
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "site") {
                opts.setSiteRoot(parser.getRequiredParameter(text));
            } else if (text == "assets") {
                opts.setAssetRoot(parser.getRequiredParameter(text));
            } else if (text == "doc") {
                opts.setDocumentRoot(parser.getRequiredParameter(text));
            } else {
                errorExitBadOption();
            }
        } else {
            items.push_back(text);
        }
    }

    if (items.empty()) {
        errorExit(Format(translator()("no node name specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    for (size_t i = 0; i < items.size(); ++i) {
        Index::Handle_t hdl;
        String_t tmp;
        if (ref.index.findNodeByAddress(items[i], hdl, tmp)) {
            // Set document address for document-local links
            opts.setDocumentId(tmp);

            // Load item, if any
            BlobStore::ObjectId_t objId = ref.index.getNodeContentId(hdl);
            if (!objId.empty()) {
                // Parse XML
                Ref<FileMapping> content = ref.blobStore->getObject(objId);
                ConstMemoryStream ms(content->get());
                CharsetFactory csFactory;
                DefaultEntityHandler eh;
                Nodes_t nodes;
                Reader rdr(ms, eh, csFactory);
                Parser(rdr).parseNodes(nodes);

                // Render
                standardOutput().writeText(renderHTML(nodes, opts));
            }
        } else {
            errorOutput().writeLine(Format(translator()("%s: not found"), items[i]));
        }
    }
}

void
util::doc::Application::verifyContent(DataParameters& data, afl::sys::CommandLineParser& parser)
{
    // Parse
    String_t text;
    bool option;
    bool all = false;
    bool verbose = false;
    Verifier::Messages_t msg = Verifier::allMessages();
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleDataOption(data, text, parser)) {
                // ok
            } else if (text == "all") {
                all = true;
            } else if (text == "v") {
                verbose = true;
            } else if (text == "warn-only") {
                msg = Verifier::warningMessages();
            } else if (text == "info-only") {
                msg = Verifier::infoMessages();
            } else {
                errorExitBadOption();
            }
        } else {
            errorExit(translator()("too many arguments"));
        }
    }

    // Operate
    DataReference ref;
    loadData(ref, data);
    if (all) {
        LoggingVerifier log(translator(), standardOutput());
        log.setEnabledMessages(msg);
        log.verify(ref.index, *ref.blobStore);
    } else {
        SummarizingVerifier sum;
        sum.setEnabledMessages(msg);
        sum.verify(ref.index, *ref.blobStore);
        for (size_t i = 0; i < Verifier::MAX_MESSAGE; ++i) {
            const Verifier::Message msg = static_cast<Verifier::Message>(i);
            if (sum.hasMessage(msg)) {
                bool brief = Verifier::summaryMessages().contains(msg) && !verbose;
                sum.printMessage(msg, ref.index, brief, translator(), standardOutput());
            }
        }
    }
}

void
util::doc::Application::help()
{
    TextWriter& out = standardOutput();
    Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 Documentation Manager v%s - (c) 2021 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-OPTIONS] COMMAND [ARGS]\n\n"
                            "%s\n"
                            "Report bugs to <Streu@gmx.de>"),
                         environment().getInvocationName(),
                         util::formatOptions(tx("Global Options:\n"
                                                "--dir=DIR\tSet repository, directory mode\n"
                                                "--single=DIR\tSet repository, single-file mode\n"
                                                "--log=CONFIG\tSet logger configuration\n"
                                                "\n"
                                                "Commands:\n"
                                                "  add-group [OPTIONS...]\n\tAdd a group\n"
                                                "  get URL...\n\tGet page content\n"
                                                "  import-help [OPTIONS...] FILE...\n\tImport PCC2 Help files (*.xml)\n"
                                                "  import-text [OPTIONS...] FILE...\n\tImport plain-text file\n"
                                                "  ls [-l|-t|-f|-r|-d...] [URL...]\n\tList content, recursively\n"
                                                "  render [OPTIONS...] URL...\n\tRender page content as HTML\n"
                                                "  verify [OPTIONS...]\n\tVerify repository content\n"
                                                "\n"
                                                "Command options:\n"
                                                "--below=ID\t(import, add) Set parent group (default=root)\n"
                                                "--id=ID[,ID...]\t(import, add) Set Id for new element\n"
                                                "--tag=ID[,ID...]\t(import, add) Set tag for new element\n"
                                                "--name=NAME\t(import, add) Set name for new element\n"
                                                "--page\t(import, add) Create a page\n"
                                                "--document\t(import, add) Create a document\n"
                                                "--charset=CS\t(import-text) Set character set\n"
                                                "--remove-source\t(import-help) Remove source notes\n"
                                                "--all\t(verify) Report all individual messages (default=summarize)\n"
                                                "-v\t(verify) Do not abbreviate messages\n"
                                                "--warn-only\t(verify) Show only warnings\n"
                                                "--info-only\t(verify) Show only information messages\n"
                                                "-l, --long\t(ls) Long format\n"
                                                "-t, --title\t(ls) Show titles\n"
                                                "-f, --forest, --tree\t(ls) Indent to show tree structure\n"
                                                "-r, --recursive\t(ls) Recursive\n"
                                                "-d, --self, --directory\t(ls) Show element itself, not content\n"
                                                "--site=PFX\t(render) Set URL prefix for \"site:\" links\n"
                                                "--assets=PFX\t(render) Set URL prefix for \"asset:\" links\n"
                                                "--doc=PFX\t(render) Set URL prefix for document links\n"
))));
    exit(0);
}

void
util::doc::Application::errorExitBadOption()
{
    Translator& tx = translator();
    errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
}

void
util::doc::Application::errorExitBadNonoption()
{
    Translator& tx = translator();
    errorExit(Format(tx("non-option unexpected. Use \"%s -h\" for help"), environment().getInvocationName()));
}

void
util::doc::Application::listContentRecursive(const DataReference& ref, const ListParameters& lp, Index::Handle_t hdl, String_t indent, String_t docName)
{
    if (lp.mode == ListParameters::Self) {
        listNodeInfo(ref, lp, hdl, indent, docName);
    } else {
        for (size_t i = 0, n = ref.index.getNumNodeChildren(hdl); i < n; ++i) {
            Index::Handle_t child = ref.index.getNodeChildByIndex(hdl, i);
            listNodeInfo(ref, lp, child, indent, docName);
            if (lp.mode == ListParameters::Recursive) {
                String_t childIndent = indent;
                if (lp.tree) {
                    childIndent += "  ";
                }
                listContentRecursive(ref, lp, child, childIndent, docName);
            }
        }
    }
}

void
util::doc::Application::listNodeInfo(const DataReference& ref, const ListParameters& lp, Index::Handle_t hdl, String_t indent, String_t docName)
{
    switch (lp.format) {
     case ListParameters::Address:
        standardOutput().writeLine(indent + ref.index.getNodeAddress(hdl, docName));
        break;

     case ListParameters::Title:
        standardOutput().writeLine(indent + ref.index.getNodeTitle(hdl));
        break;

     case ListParameters::Long: {
        String_t contentId = ref.index.getNodeContentId(hdl);
        String_t line = Format("%s%-4s %-40s  %-20s '%s'")
            << indent
            << (ref.index.isNodePage(hdl) ? "PAGE" : "DOC")
            << (contentId.empty() ? "-" : contentId)
            << ref.index.getNodeAddress(hdl, docName)
            << ref.index.getNodeTitle(hdl);
        size_t numTags = ref.index.getNumNodeTags(hdl);
        if (numTags != 0) {
            line += " [";
            for (size_t i = 0; i < numTags; ++i) {
                if (i != 0) {
                    line += ",";
                }
                line += ref.index.getNodeTagByIndex(hdl, i);
            }
            line += "]";
        }
        standardOutput().writeLine(line);

        size_t numIds = ref.index.getNumNodeIds(hdl);
        for (size_t i = 1; i < numIds; ++i) {
            standardOutput().writeLine(Format("%sALSO %-40s  %s", "-", indent, ref.index.getNodeIdByIndex(hdl, i)));
        }
        break;
     }
    }
}
