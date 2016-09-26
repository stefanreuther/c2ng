/**
  *  \file interpreter/exporter/exporter.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_EXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_EXPORTER_HPP

#include "interpreter/context.hpp"
#include "util/answerprovider.hpp"
#include "interpreter/typehint.hpp"
#include "afl/base/memory.hpp"

namespace interpreter { namespace exporter {

    class FieldList;
 
    /** Exporter base.
        This class contains the logic of obtaining export data. Actual output is provided by derived classes. */
    class Exporter {
     public:
        Exporter();
        virtual ~Exporter();

        void doExport(Context* ctx, util::AnswerProvider& filter, const FieldList& fields);

     protected:
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types) = 0;
        virtual void startRecord() = 0;
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type) = 0;
        virtual void endRecord() = 0;
        virtual void endTable() = 0;
    };

} }

#endif
