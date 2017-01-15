/**
  *  \file interpreter/hashvalue.hpp
  */
#ifndef C2NG_INTERPRETER_HASHVALUE_HPP
#define C2NG_INTERPRETER_HASHVALUE_HPP

#include "afl/base/ref.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/hashdata.hpp"

namespace interpreter {

    /** Hash reference.
        Hashes are always by-reference, because HashValue objects are cloned when put on the stack.
        The actual data is stored in a HashData object. */
    class HashValue : public IndexableValue {
     public:
        HashValue(afl::base::Ref<HashData> data);
        ~HashValue();

        afl::base::Ref<HashData> getData();

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        // Value:
        virtual HashValue* clone() const;

     private:
        afl::base::Ref<HashData> m_data;
    };

}

#endif
