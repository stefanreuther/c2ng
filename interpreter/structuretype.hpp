/**
  *  \file interpreter/structuretype.hpp
  *  \brief Class interpreter::StructureType
  */
#ifndef C2NG_INTERPRETER_STRUCTURETYPE_HPP
#define C2NG_INTERPRETER_STRUCTURETYPE_HPP

#include "interpreter/basevalue.hpp"
#include "interpreter/structuretypedata.hpp"

namespace interpreter {

    /** Value of a structure type.
        The actual data is in a StructureTypeData object; this object stores a reference there-to.
        This type appears in data segments and is frequently copied.
        Multiple StructureType objects can and will often reference the same StructureTypeData. */
    class StructureType : public BaseValue {
     public:
        /** Constructor.
            \param type Structure type */
        explicit StructureType(StructureTypeData::Ref_t type);

        /** Destructor. */
        ~StructureType();

        /** Get contained type.
            \return contained type */
        StructureTypeData::Ref_t getType() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

        // Value:
        virtual StructureType* clone() const;

     private:
        const StructureTypeData::Ref_t m_type;
    };
}

inline interpreter::StructureTypeData::Ref_t
interpreter::StructureType::getType() const
{
    return m_type;
}

#endif
