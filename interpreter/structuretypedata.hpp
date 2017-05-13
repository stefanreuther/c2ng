/**
  *  \file interpreter/structuretypedata.hpp
  *  \brief Class interpreter::StructureTypeData
  */
#ifndef C2NG_INTERPRETER_STRUCTURETYPEDATA_HPP
#define C2NG_INTERPRETER_STRUCTURETYPEDATA_HPP

#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/data/namemap.hpp"

namespace interpreter {

    /** Structure type.
        Contains everything that makes up a structure type.

        A structure type consists of a single name->slot mapping that is applied to all instances of the type.

        This is mainly just a wrapper for an afl::data::NameMap. */
    class StructureTypeData : public afl::base::RefCounted {
     public:
        /** Typedef for brevity.
            Most objects of this type are managed by reference-count. */
        typedef afl::base::Ref<StructureTypeData> Ref_t;

        /** Constructor.
            Makes an empty mapping. */
        StructureTypeData();

        /** Destructor. */
        ~StructureTypeData();

        /** Access name mapping.
            \return name mapping */
        afl::data::NameMap& names();

        /** Access name mapping (const).
            \return name mapping */
        const afl::data::NameMap& names() const;

     private:
        afl::data::NameMap m_names;
    };

}

inline afl::data::NameMap&
interpreter::StructureTypeData::names()
{
    return m_names;
}

inline const afl::data::NameMap&
interpreter::StructureTypeData::names() const
{
    return m_names;
}

#endif
