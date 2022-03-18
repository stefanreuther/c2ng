/**
  *  \file interpreter/exporter/exporter.hpp
  *  \brief Base class interpreter::exporter::Exporter
  */
#ifndef C2NG_INTERPRETER_EXPORTER_EXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_EXPORTER_HPP

#include "afl/base/memory.hpp"
#include "interpreter/context.hpp"
#include "interpreter/typehint.hpp"
#include "util/answerprovider.hpp"

namespace interpreter { namespace exporter {

    class FieldList;

    /** Exporter base.
        This class contains the logic of obtaining export data. Actual output is provided by derived classes. */
    class Exporter {
     public:
        /** Constructor. */
        Exporter();

        /** Virtual destructor. */
        virtual ~Exporter();

        /** Main entry point.
            Invokes the virtual methods to produce the result.
            - startTable
            - for each record, startRecord; sequence of addField; endRecord
            - endTable

            \param ctx      Context looking at the first object to possibly export.
            \param filter   Object filter.
                            Will be called with the object Id as questionId to permit/reject export of an object.
            \param fields   Field list

            \throw interpreter::Error on error. */
        void doExport(Context& ctx, util::AnswerProvider& filter, const FieldList& fields);

     protected:
        /** Start output.
            Create possible headers.
            \param fields Field list
            \param types  Type hints for each field */
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types) = 0;

        /** Start a record.
            Called after startTable() for each object to produce a record (line, entry). */
        virtual void startRecord() = 0;

        /** Add a field.
            Called after startRecord() for each field to be exported.
            \param value Value
            \param name  Field name
            \param type  Type hint.
                         This is the same type hint that was given to startTable() for this field.
                         Note that this is not a hard guarantee to match the type of the value
                         if the Context's enumProperties reports inconsistent information. */
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type) = 0;

        /** End a record.
            Called after addField() sequence to end the record (line, entry). */
        virtual void endRecord() = 0;

        /** End output.
            Called after the final endRecord() to finish the export. */
        virtual void endTable() = 0;
    };

} }

#endif
