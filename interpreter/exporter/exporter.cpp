/**
  *  \file interpreter/exporter/exporter.cpp
  *  \brief Base class interpreter::exporter::Exporter
  */

#include "interpreter/exporter/exporter.hpp"
#include "game/map/object.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/propertyacceptor.hpp"

interpreter::exporter::Exporter::Exporter()
{ }

interpreter::exporter::Exporter::~Exporter()
{ }

void
interpreter::exporter::Exporter::doExport(Context& ctx, util::AnswerProvider& filter, const FieldList& fields)
{
    // ex IntExporter::doExport
    // ex export.pas:DoExport (sans GUI)

    // Populate type hints
    // ex export.pas:CompleteFieldList
    struct TypeHintCollector : public PropertyAcceptor {
        const FieldList& fields;
        std::vector<TypeHint> typeHints;
        std::vector<bool> seenTypeHints;

        TypeHintCollector(const FieldList& fields)
            : fields(fields),
              typeHints(fields.size(), thNone),
              seenTypeHints(fields.size())
            { }
        void addProperty(const String_t& name, TypeHint th)
            {
                for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
                    if (name == fields.getFieldName(i)) {
                        typeHints[i] = th;
                        seenTypeHints[i] = true;
                    }
                }
            }
    };
    TypeHintCollector thc(fields);
    ctx.enumProperties(thc);

    // Anything missing?
    for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
        if (!thc.seenTypeHints[i]) {
            throw Error("Unknown field, " + fields.getFieldName(i));
        }
    }

    // Do it!
    startTable(fields, thc.typeHints);
    do {
        // Filter objects as requested
        game::map::Object* obj = ctx.getObject();
        // FIXME: PCC2/c2ng change: PCC2 would use the object's name in filter.ask.
        bool use;
        if (obj == 0) {
            use = true;
        } else {
            util::AnswerProvider::Result result = filter.ask(obj->getId(), String_t());
            if (result == util::AnswerProvider::Yes) {
                use = true;
            } else if (result == util::AnswerProvider::No) {
                use = false;
            } else {
                break;
            }
        }
        if (use) {
            // Object should be exported
            startRecord();
            for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
                // Obtain value.
                // If anything throws or the lookup fails, the value is left as null.
                std::auto_ptr<afl::data::Value> value;
                try {
                    Context::PropertyIndex_t adr;
                    if (Context::PropertyAccessor* foundContext = ctx.lookup(fields.getFieldName(i), adr)) {
                        value.reset(foundContext->get(adr));
                    }
                }
                catch (Error&)
                { }
                addField(value.get(), fields.getFieldName(i), thc.typeHints[i]);
            }
            endRecord();
        }
    } while (ctx.next());
    endTable();
}
