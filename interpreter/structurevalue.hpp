/**
  *  \file interpreter/structurevalue.hpp
  */
#ifndef C2NG_INTERPRETER_STRUCTUREVALUE_HPP
#define C2NG_INTERPRETER_STRUCTUREVALUE_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/structuretype.hpp"

namespace interpreter {


    /** Structure value. */
    class StructureValueData {
     public:
        StructureValueData(afl::base::Ptr<StructureTypeData> type);
        ~StructureValueData();

        afl::base::Ptr<StructureTypeData> type;
        afl::data::Segment data;
    };

    /** Handle to a structure value. */
    class StructureValue : public SingleContext {
     public:
        StructureValue(afl::base::Ptr<StructureValueData> value);
        ~StructureValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext* ctx) const;

        // Value:
        virtual StructureValue* clone() const;

        // Context:
        virtual bool lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual game::map::Object* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor);

        afl::base::Ptr<StructureValueData> getValue() const
            { return m_value; }

     private:
        afl::base::Ptr<StructureValueData> m_value;
    };

}

#endif
