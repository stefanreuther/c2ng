/**
  *  \file interpreter/arrayvalue.cpp
  */

#include "interpreter/arrayvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"

namespace {
    /** Maximum total size of an array.
        We must define an upper limit to avoid getting into regions where operator new fails too easily.
        By defining the maximum size as 10001-squared, we allow an array indexed by two minefield Ids (the highest-possible Id sequence),
        while having a maximum memory consumption of the data segment of about 400 megs, 1/10 of the 32-bit address space. */
    static const size_t MAX_ARRAY_ELEMS = 10001UL * 10001UL;

    /** Move part of an array.
        This is a recursive operation.
        For a two-dimensional array, level=0 copies the whole array,
        level=1 copies a row, and level=2 copies a cell.

        \param from        [in/out] Source array. Parts of it will be destroyed.
        \param to          [in/out] Target array. Must be blank.
        \param fromDim     [in] Dimensions of source array.
        \param toDim       [in] Dimensions of target array.
        \param level       [in] Dimension counter for recursion.
        \param fromAddress [in] First address to read.
        \param toAddress   [in] First address to write. */
    void moveArray(afl::data::Segment& from,
                   afl::data::Segment& to,
                   const std::vector<size_t>& fromDim,
                   const std::vector<size_t>& toDim,
                   const size_t level,
                   size_t fromAddress,
                   size_t toAddress)
    {
        if (fromAddress >= from.size()) {
            // Source address is beyond from's size; nothing to do.
        } else if (level < fromDim.size()) {
            // A row
            size_t fromSize = 1, toSize = 1;
            for (size_t i = level+1; i < fromDim.size(); ++i) {
                fromSize *= fromDim[i];
                toSize *= toDim[i];
            }
            for (size_t i = 0; i < fromDim[level] && i < toDim[level]; ++i) {
                moveArray(from, to, fromDim, toDim, level+1, fromAddress, toAddress);
                fromAddress += fromSize;
                toAddress += toSize;
            }
        } else {
            // A cell
            from.swapElements(fromAddress, to, toAddress);
        }
    }
}


/****************************** IntArrayData *****************************/

// /** Constructor. Makes a blank array with no dimensions. */
interpreter::ArrayData::ArrayData()
    : content(),
      m_totalSize(1),
      m_dimensions()
{
    // ex IntArrayData::IntArrayData()
}

// /** Destructor. */
interpreter::ArrayData::~ArrayData()
{ }

// /** Add a dimension.
//     \param n New dimension
//     \return true if dimension accepted, false on error */
bool
interpreter::ArrayData::addDimension(int32_t n)
{
    // ex IntArrayData::addDimension
    /* We must allow dimension 0 to allow empty arrays, so we cannot have just one single "n < 0" test. */
    if (n < 0) {
        return false;
    }

    size_t un = n;
    if (un > 0 && m_totalSize > 0 && un > MAX_ARRAY_ELEMS / m_totalSize) {
        return false;
    }

    m_dimensions.push_back(un);
    m_totalSize *= un;
    return true;
}

// /** Compute index for this array.
//     \param args   [in/out] Argument block containing user-supplied indices
//     \param result [out] Linear index
//     \retval true Result has been produced
//     \retval false One of the indices was null
//     \throw IntError One of the indices was out-of-range */
bool
interpreter::ArrayData::computeIndex(Arguments& args, size_t& result) const
{
    // ex IntArrayData::computeIndex
    args.checkArgumentCount(m_dimensions.size());
    result = 0;
    for (size_t i = 0; i < m_dimensions.size(); ++i) {
        // Read arg. Check for null.
        int32_t me;
        if (!checkIntegerArg(me, args.getNext())) {
            return false;
        }

        // Range check.
        if (me < 0 || int32_t(size_t(me)) != me || size_t(me) >= m_dimensions[i]) {
            throw Error::rangeError();
        }

        // Compute index. Horner schema.
        result = result * m_dimensions[i] + me;
    }
    return true;
}

size_t
interpreter::ArrayData::getNumDimensions() const
{
    // ex IntArrayData::getNumDimensions
    return m_dimensions.size();
}

size_t
interpreter::ArrayData::getDimension(size_t i) const
{
    // ex IntArrayData::getDimension
    return m_dimensions[i];
}

// /** Resize array. Updates the array in-place, keeping the values if possible.
//     \param tpl Template for new array. The new array will have the same dimensions as this one. */
void
interpreter::ArrayData::resize(const ArrayData& tpl)
{
    // ex IntArrayData::resize
    // Verify number of dimensions
    checkArgumentCount(tpl.getNumDimensions(), m_dimensions.size(), m_dimensions.size());

    // Can we resize in-place?
    bool allowInPlace = true;
    for (size_t i = 1; i < m_dimensions.size(); ++i) {
        if (tpl.m_dimensions[i] != m_dimensions[i]) {
            allowInPlace = false;
            break;
        }
    }

    if (allowInPlace) {
        // In-place resize. This means just discard excess elements, if any.
        if (content.size() > tpl.m_totalSize) {
            content.popBackN(content.size() - tpl.m_totalSize);
        }
    } else {
        // Out-of-place resize. Slow.
        afl::data::Segment newData;
        moveArray(content, newData, m_dimensions, tpl.m_dimensions, 0, 0, 0);
        content.swap(newData);
    }

    // Copy new metadata
    m_totalSize = tpl.m_totalSize;
    m_dimensions = tpl.m_dimensions;
}

/******************************* ArrayValue ******************************/

// /** Constructor.
//     \param data Our array */
interpreter::ArrayValue::ArrayValue(afl::base::Ref<ArrayData> data)
    : m_data(data)
{ }

// IndexableValue:
afl::data::Value*
interpreter::ArrayValue::get(Arguments& args)
{
    // ex IntArray::get
    size_t index;
    if (m_data->computeIndex(args, index)) {
        return afl::data::Value::cloneOf(m_data->content[index]);
    } else {
        return 0;
    }
}

void
interpreter::ArrayValue::set(Arguments& args, afl::data::Value* value)
{
    // ex IntArray::set
    size_t index;
    if (m_data->computeIndex(args, index)) {
        m_data->content.set(index, value);
    } else {
        throw Error::typeError(Error::ExpectInteger);
    }
}

// CallableValue:
int32_t
interpreter::ArrayValue::getDimension(int32_t which) const
{
    // ex IntArray::getDimension
    // FIXME: range checks?
    if (which == 0) {
        return m_data->getNumDimensions();
    } else {
        return m_data->getDimension(which-1);
    }
}

interpreter::Context*
interpreter::ArrayValue::makeFirstContext()
{
    // ex IntArray::makeFirstContext()
    throw Error::typeError(Error::ExpectIterable);
}

// BaseValue:
interpreter::ArrayValue*
interpreter::ArrayValue::clone() const
{
    // ex IntArray::clone
    return new ArrayValue(m_data);
}

String_t
interpreter::ArrayValue::toString(bool /*readable*/) const
{
    // ex IntArray::toString
    return "#<array>";
}

void
interpreter::ArrayValue::store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& ctx) const
{
    // ex IntArray::store
    out.tag   = TagNode::Tag_Array;
    out.value = ctx.addArray(*m_data);
}

// Inquiry
afl::base::Ref<interpreter::ArrayData>
interpreter::ArrayValue::getData()
{
    return m_data;
}
