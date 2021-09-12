/**
  *  \file interpreter/structurevalue.hpp
  *  \brief Class interpreter::StructureValue
  */
#ifndef C2NG_INTERPRETER_STRUCTUREVALUE_HPP
#define C2NG_INTERPRETER_STRUCTUREVALUE_HPP

#include "interpreter/singlecontext.hpp"
#include "interpreter/structurevaluedata.hpp"

namespace interpreter {

    /** Structure value.
        The actual value (and type reference) is in a StructureValueData.

        This type appears in data segments and is frequently copied.
        Multiple StructureValue objects can and will often reference the same StructureValueData.

        This type provides integration with the remainder of the interpreter.
        In this case, this is the implementation of Context methods, making "With sv" or "sv->member" operations work. */
    class StructureValue : public SingleContext, public Context::PropertyAccessor {
     public:
        /** Constructor.
            \param value Structure value */
        explicit StructureValue(StructureValueData::Ref_t value);

        /** Destructor. */
        ~StructureValue();

        /** Get referenced structure value.
            \return value */
        StructureValueData::Ref_t getValue() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        // Value:
        virtual StructureValue* clone() const;

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual game::map::Object* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor);

     private:
        StructureValueData::Ref_t m_value;
    };

}

inline interpreter::StructureValueData::Ref_t
interpreter::StructureValue::getValue() const
{
    return m_value;
}

#endif
