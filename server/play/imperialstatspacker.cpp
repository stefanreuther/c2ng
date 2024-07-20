/**
  *  \file server/play/imperialstatspacker.cpp
  *  \brief Class server::play::ImperialStatsPacker
  */

#include "server/play/imperialstatspacker.hpp"
#include "afl/string/format.hpp"
#include "game/map/info/types.hpp"
#include "game/map/info/linkbuilder.hpp"
#include "game/map/info/browser.hpp"
#include "game/root.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/xml/visitor.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/data/integervalue.hpp"

namespace gmi = game::map::info;
namespace aix = afl::io::xml;

using afl::string::Format;
using afl::data::IntegerValue;
using afl::data::NameMap;
using afl::data::StringValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using game::SearchQuery;

namespace {
    /*
     *  LinkBuilder
     */

    String_t toString(SearchQuery::MatchType ty)
    {
        switch (ty) {
         case SearchQuery::MatchName:     return "n";
         case SearchQuery::MatchTrue:     return "t";
         case SearchQuery::MatchFalse:    return "f";
         case SearchQuery::MatchLocation: return "l";
        }
        return String_t();
    }

    class LinkBuilder : public gmi::LinkBuilder {
        virtual String_t makePlanetLink(const game::map::Planet& pl) const
            { return Format("planet:%d", pl.getId()); }
        virtual String_t makeSearchLink(const SearchQuery& q) const
            { return Format("search:%s:%s:%s", q.getSearchObjectsAsString(), toString(q.getMatchType()), q.getQuery()); }
    };

    /*
     *  Serializer
     *
     *  A node list is translated into a vector.
     *  Within that vector,
     *  - text is represented as-is
     *  - tag is translated into ["tag", {"att":"val"...}, content].
     */

    class Serializer : public aix::Visitor {
     public:
        Serializer(Vector& out)
            : m_out(out)
            { }

        virtual void visitPI(const aix::PINode& /*node*/)
            { }

        virtual void visitTag(const aix::TagNode& node)
            {
                // Name
                Vector::Ref_t child = Vector::create();
                child->pushBackString(node.getName());

                // Attributes
                Hash::Ref_t atts = Hash::create();
                const NameMap& m = node.getAttributeNames();
                for (size_t i = 0; i < m.getNumNames(); ++i) {
                    atts->setNew(m.getNameByIndex(i), new StringValue(node.getAttributeByIndex(i)));
                }
                child->pushBackNew(new HashValue(atts));

                // Children
                Serializer(*child).visit(node.getChildren());
                m_out.pushBackNew(new VectorValue(child));
            }

        virtual void visitText(const aix::TextNode& node)
            { m_out.pushBackString(node.get()); }

     private:
        Vector& m_out;
    };
}

/*
 *  ImperialStatsPacker
 */

server::play::ImperialStatsPacker::ImperialStatsPacker(game::Session& session, int page, int opts)
    : m_session(session),
      m_page(page),
      m_options(opts)
{ }

server::Value_t*
server::play::ImperialStatsPacker::buildValue() const
{
    // Validate parameters
    if (m_page < 0 || m_page >= int(game::map::info::NUM_PAGES)) {
        return 0;
    }
    const gmi::Page page = static_cast<gmi::Page>(m_page);
    const gmi::PageOptions_t selectedOptions = static_cast<gmi::PageOptions_t>(m_options);

    // Validate inputs
    const game::Root* root = m_session.getRoot().get();
    if (root == 0) {
        return 0;
    }

    // Do it
    LinkBuilder lb;
    gmi::Browser bro(m_session, lb, root->userConfiguration().getNumberFormatter());
    bro.setPageOptions(page, selectedOptions);

    gmi::Nodes_t nodes;
    bro.renderPage(page, nodes);

    util::StringList availableOptions;
    bro.renderPageOptions(page, availableOptions);

    // Pack it
    Hash::Ref_t hv = Hash::create();

    // - Content
    Vector::Ref_t content = Vector::create();
    Serializer(*content).visit(nodes);
    hv->setNew("content", new VectorValue(content));

    // - Options
    Vector::Ref_t packedOptions = Vector::create();
    for (size_t i = 0; i < availableOptions.size(); ++i) {
        String_t text;
        int32_t key;
        if (availableOptions.get(i, key, text)) {
            Hash::Ref_t me = Hash::create();
            me->setNew("text", new StringValue(text));
            me->setNew("value", new IntegerValue(key));
            packedOptions->pushBackNew(new HashValue(me));
        }
    }
    hv->setNew("options", new VectorValue(packedOptions));

    return new HashValue(hv);
}

String_t
server::play::ImperialStatsPacker::getName() const
{
    return Format("istat%d.%d", m_page, m_options);
}
