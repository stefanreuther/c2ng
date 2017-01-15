/**
  *  \file interpreter/arrayvalue.hpp
  */
#ifndef C2NG_INTERPRETER_ARRAYVALUE_HPP
#define C2NG_INTERPRETER_ARRAYVALUE_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/indexablevalue.hpp"
#include "afl/base/ref.hpp"

namespace interpreter {

    class Arguments;

    /** Storage for array data.
        Consists of a set of dimensions, plus an IntDataSegment containing the array data. */
    class ArrayData : public afl::base::RefCounted {
     public:
        ArrayData();
        ~ArrayData();

        bool addDimension(int32_t n);
        bool computeIndex(Arguments& args, size_t& result) const;

        size_t getNumDimensions() const;
        size_t getDimension(size_t i) const;

        void resize(const ArrayData& tpl);

        afl::data::Segment content;

        // FIXME: can we avoid this?
        const std::vector<size_t>& getDimensions() const
            { return m_dimensions; }

     private:
        /** Total size. Used to keep track of the maximum total number of elements. */
        size_t m_totalSize;

        /** Dimensions of the array. */
        std::vector<size_t> m_dimensions;
    };

    /** Array reference.
        Arrays are always by-reference, because ArrayValue objects are cloned when put on the stack.
        The actual data is stored in an ArrayData object. */
    class ArrayValue : public IndexableValue {
     public:
        ArrayValue(afl::base::Ref<ArrayData> data);

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual ArrayValue* clone() const;
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        // Inquiry
        afl::base::Ref<ArrayData> getData();

     private:
        afl::base::Ref<ArrayData> m_data;
    };
}

#endif
