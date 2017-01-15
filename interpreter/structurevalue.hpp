/**
  *  \file interpreter/structurevalue.hpp
  */
#ifndef C2NG_INTERPRETER_STRUCTUREVALUE_HPP
#define C2NG_INTERPRETER_STRUCTUREVALUE_HPP

#include "afl/base/ref.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/structuretype.hpp"
#include "afl/base/refcounted.hpp"

namespace interpreter {


    /** Structure value. */
    class StructureValueData : public afl::base::RefCounted {
     public:
        StructureValueData(afl::base::Ref<StructureTypeData> type);
        ~StructureValueData();

        afl::base::Ref<StructureTypeData> type;
        afl::data::Segment data;
    };

    /** Handle to a structure value. */
    class StructureValue : public SingleContext {
     public:
        StructureValue(afl::base::Ref<StructureValueData> value);
        ~StructureValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        // Value:
        virtual StructureValue* clone() const;

        // Context:
        virtual StructureValue* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual game::map::Object* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor);

        afl::base::Ref<StructureValueData> getValue() const
            { return m_value; }

     private:
        afl::base::Ref<StructureValueData> m_value;
    };

}

#endif
