/**
  *  \file interpreter/arraydata.hpp
  *  \brief Class interpreter::ArrayData
  */
#ifndef C2NG_INTERPRETER_ARRAYDATA_HPP
#define C2NG_INTERPRETER_ARRAYDATA_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/data/segment.hpp"

namespace interpreter {

    class Arguments;

    /** Storage for array data.
        Consists of a set of dimensions, plus a Segment containing the array data. */
    class ArrayData : public afl::base::RefCounted {
     public:
        /** Constructor.
            Makes an empty array with no dimensions. */
        ArrayData();

        /** Destructor. */
        ~ArrayData();

        /** Add a dimension.
            \param n New dimension
            \return true if dimension accepted, false on error */
        bool addDimension(int32_t n);

        /** Compute index for this array.
            \param args   [in/out] Argument block containing user-supplied indices (will be consumed)
            \param result [out] Linear index
            \retval true Result has been produced
            \retval false One of the indices was null
            \throw Error One of the indices was out-of-range or wrong type */
        bool computeIndex(Arguments& args, size_t& result) const;

        /** Get number of dimensions.
            \return number of dimensions */
        size_t getNumDimensions() const;

        /** Get dimension.
            \param i dimension to query [0,getNumDimensions())
            \return dimension */
        size_t getDimension(size_t i) const;

        /** Resize array.
            Updates the array in-place, keeping the values if possible.
            \param tpl Template for new array. The new array will have the same dimensions as this one.
            \throw Error The template does not have the same number of dimensions as this array */
        void resize(const ArrayData& tpl);

        /** Access content.
            \return content segment */
        afl::data::Segment& content()
            { return m_content; }

        /** Access content.
            \return content segment */
        const afl::data::Segment& content() const
            { return m_content; }

        /** Get all dimensions.
            \return array */
        afl::base::Memory<const size_t> getDimensions() const
            { return m_dimensions; }

     private:
        /** Content. */
        afl::data::Segment m_content;

        /** Total size. Used to keep track of the maximum total number of elements. */
        size_t m_totalSize;

        /** Dimensions of the array. */
        std::vector<size_t> m_dimensions;
    };

}

#endif
