/**
  *  \file client/help.cpp
  */

#include "client/help.hpp"
#include "game/extra.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "util/charsetfactory.hpp"
#include "afl/io/xml/parser.hpp"
#include "afl/string/format.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/constmemorystream.hpp"

using afl::io::xml::Node;
using afl::io::xml::Parser;
using afl::io::xml::Reader;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;

typedef std::auto_ptr<Node> NodePtr_t;

namespace {
    class HelpExtra : public game::Extra {
     public:
        util::HelpIndex index;
    };

    game::ExtraIdentifier<game::Session, HelpExtra> HELP_ID;

    Node* makeTextInTag(String_t tagName, String_t text)
    {
        std::auto_ptr<TagNode> infoNode(new TagNode(tagName));
        infoNode->addNewChild(new TextNode(text));
        return infoNode.release();
    }

    Node* makeTextInTag(String_t tagName, String_t text, NodePtr_t innerText)
    {
        // Outer tag
        std::auto_ptr<TagNode> infoNode(new TagNode(tagName));

        // Build inner text
        String_t::size_type pos = text.find("%s");
        if (pos != String_t::npos) {
            if (pos != 0) {
                infoNode->addNewChild(new TextNode(text.substr(0, pos)));
            }
            infoNode->addNewChild(innerText.release());
            if (pos+2 != text.size()) {
                infoNode->addNewChild(new TextNode(text.substr(pos+2)));
            }
        } else {
            infoNode->addNewChild(new TextNode(text));
        }

        return infoNode.release();
    }
}


util::HelpIndex&
client::getHelpIndex(game::Session& session)
{
    return session.extra().create(HELP_ID).index;
}

void
client::loadHelpPage(game::Session& session,
                     afl::io::xml::Nodes_t& result,
                     String_t pageName)
{
    // ex WHelpDialog::loadPage (sort-of)
    // Environment
    afl::io::FileSystem& fs = session.world().fileSystem();
    afl::string::Translator& tx = session.translator();
    afl::io::xml::DefaultEntityHandler eh;
    util::CharsetFactory csFactory;

    // Operate
    util::HelpIndex::NodeVector_t nodes;
    getHelpIndex(session).find(pageName, nodes, fs, session.log());
    if (nodes.empty()) {
        // Error page
        result.pushBackNew(makeTextInTag("h1", tx("Error")));
        result.pushBackNew(makeTextInTag("p", tx("The requested page \"%s\" could not be found."), NodePtr_t(makeTextInTag("b", pageName))));

        // Link footer. This is easier to do (and to translate) by parsing XML:
        String_t linkFooter = tx("<p><b>See also:</b>&#160;<a href=\"toc\">Help Content</a></p>");
        afl::io::ConstMemoryStream ms(afl::string::toBytes(linkFooter));
        Reader rdr(ms, eh, csFactory);
        Parser(rdr).parseNodes(result);
    } else {
        // Render individual pages
        for (size_t i = 0, n = nodes.size(); i < n; ++i) {
            const String_t fileName = nodes[i]->file.name;
            afl::base::Ptr<afl::io::Stream> file = fs.openFileNT(fileName, afl::io::FileSystem::OpenRead);
            if (file.get() == 0) {
                result.pushBackNew(makeTextInTag("h1", tx("Error")));
                result.pushBackNew(makeTextInTag("p",  tx("Help file \"%s\" could not be opened."), NodePtr_t(makeTextInTag("b", fileName))));
            } else {
                // Found. Read and render it.
                file->setPos(nodes[i]->pos);

                Reader rdr(*file, eh, csFactory);
                rdr.readNext();            // This will skip "<page"

                Parser(rdr).parseNodes(result);

                // Add origin reference
                if (n != 1 || nodes[i]->file.origin != "(PCC2)") {
                    // FIXME: custom tag, renders as right-justified + small + faded
                    result.pushBackNew(makeTextInTag("p-info", afl::string::Format(tx("from %s"), fs.getFileName(fileName))));
                }
            }
        }
    }
}
