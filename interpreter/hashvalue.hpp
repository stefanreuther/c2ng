/**
  *  \file interpreter/hashvalue.hpp
  *  \brief Class interpreter::HashValue
  */
#ifndef C2NG_INTERPRETER_HASHVALUE_HPP
#define C2NG_INTERPRETER_HASHVALUE_HPP

#include "afl/base/ref.hpp"
#include "interpreter/indexablevalue.hpp"
#include "afl/data/hash.hpp"

namespace interpreter {

    /** Hash reference.
        Hashes are always by-reference, because HashValue objects are cloned when put on the stack.
        The actual data is stored in a afl::data::Hash object. */
    class HashValue : public IndexableValue {
     public:
        /** Constructor.
            \param data Hash */
        explicit HashValue(afl::data::Hash::Ref_t data);

        /** Destructor. */
        ~HashValue();

        /** Access underlying actual hash.
            \return hash */
        afl::data::Hash::Ref_t getData();

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

        // Value:
        virtual HashValue* clone() const;

     private:
        afl::data::Hash::Ref_t m_data;
    };

}

#endif
