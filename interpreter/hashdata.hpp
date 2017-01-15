/**
  *  \file interpreter/hashdata.hpp
  */
#ifndef C2NG_INTERPRETER_HASHDATA_HPP
#define C2NG_INTERPRETER_HASHDATA_HPP

#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/base/refcounted.hpp"

namespace interpreter {

    /** Storage for hash data.
        Stores keys and values.
        It relies on NameMap to do the actual hashing.

        The exposed interface is a cross of NameMap and DataSegment.
        Iteration can therefore simply iterate over DataSegment indexes.

        FIXME: can we somehow use afl::data::Hash instead? */
    class HashData : public afl::base::RefCounted {
     public:
        HashData();
        ~HashData();

        // Name interface
        void setNew(const String_t& name, afl::data::Value* value);
        afl::data::Value* get(const String_t& name) const;

        // Index interface
        void setNew(afl::data::NameMap::Index_t index, afl::data::Value* value);
        afl::data::Value* get(afl::data::NameMap::Index_t index) const;
        String_t getName(afl::data::NameMap::Index_t index);
        afl::data::NameMap::Index_t getNumNames() const;

        // Direct access (needed for saving/loading). FIXME: can we get rid of this?
        const afl::data::NameMap& getNames() const
            { return m_names; }
        afl::data::NameMap& getNames()
            { return m_names; }
        const afl::data::Segment& getContent() const
            { return m_content; }
        afl::data::Segment& getContent()
            { return m_content; }

     private:
        afl::data::NameMap m_names;
        afl::data::Segment m_content;
    };

}


#endif
