/**
  *  \file interpreter/arraydata.cpp
  *  \brief Class interpreter::ArrayData
  */

#include "interpreter/arraydata.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"

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


// Constructor.
interpreter::ArrayData::ArrayData()
    : m_content(),
      m_totalSize(1),
      m_dimensions()
{
    // ex IntArrayData::IntArrayData()
}

// Destructor.
interpreter::ArrayData::~ArrayData()
{ }

// Add a dimension.
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

// Compute index for this array.
bool
interpreter::ArrayData::computeIndex(Arguments& args, size_t& result) const
{
    // ex IntArrayData::computeIndex
    args.checkArgumentCount(m_dimensions.size());
    result = 0;
    for (size_t i = 0; i < m_dimensions.size(); ++i) {
        // Read arg. Check for null.
        size_t me = 0;
        if (!checkIndexArg(me, args.getNext(), 0, m_dimensions[i])) {
            return false;
        }

        // Compute index. Horner schema.
        result = result * m_dimensions[i] + me;
    }
    return true;
}

// Get number of dimensions.
size_t
interpreter::ArrayData::getNumDimensions() const
{
    // ex IntArrayData::getNumDimensions
    return m_dimensions.size();
}

// Get dimension.
size_t
interpreter::ArrayData::getDimension(size_t i) const
{
    // ex IntArrayData::getDimension
    if (i < m_dimensions.size()) {
        return m_dimensions[i];
    } else {
        return 0;
    }
}

// Resize array.
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
        if (m_content.size() > tpl.m_totalSize) {
            m_content.popBackN(m_content.size() - tpl.m_totalSize);
        }
    } else {
        // Out-of-place resize. Slow.
        afl::data::Segment newData;
        moveArray(m_content, newData, m_dimensions, tpl.m_dimensions, 0, 0, 0);
        m_content.swap(newData);
    }

    // Copy new metadata
    m_totalSize = tpl.m_totalSize;
    m_dimensions = tpl.m_dimensions;
}
