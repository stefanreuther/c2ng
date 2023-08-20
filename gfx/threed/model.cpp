/**
  *  \file gfx/threed/model.cpp
  *  \brief Class gfx::threed::Model
  */

#include "gfx/threed/model.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "gfx/threed/colortransformation.hpp"
#include "gfx/threed/linerenderer.hpp"
#include "gfx/threed/trianglerenderer.hpp"
#include "gfx/threed/vecmath.hpp"

namespace {
    /*
     *  File format.
     *
     *  For now, this is an ad-hoc defined file format for our purposes.
     *  One objective was to keep the option for easy, low-tech compressibility,
     *  which is why I'm using arrays-of-components instead of arrays-of-vectors here.
     *  The format is upward-compatible in the sense that we may add different block types later.
     *
     *  The file consists of a number of blocks.
     *  Each block has a type tag.
     *  Blocks are loaded into the Model's slots accordingly.
     *
     *  Format constraint: If we introduce a "new version of block type X" block type,
     *  a file must not contain both types.
     *  This avoids that an application that can only read the old type, X,
     *  will mess up the indexes.
     *  Alternatively, increase the header version number
     *  to make sure the old app does not read the file at all.
     */
    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;

    /*
     *  File Header
     */

    struct Header {
        uint8_t signature[8];
        UInt16_t version;
        UInt16_t numElements;
    };
    static_assert(sizeof(Header) == 12, "sizeof(Header)");

    const uint8_t SIGNATURE[] = {'C','C','m','o','d','e','l',26};
    const uint16_t VERSION_1 = 1;
    const size_t MAX_ELEMENTS = 1000;      /* DoS protection, not a file format limit; increase if needed. */

    /*
     *  Block index
     */

    struct Block {
        UInt32_t type;
        UInt32_t size;
    };
    const size_t MAX_SIZE = 10*1000*1000;  /* DoS protection, not a file format limit; increase if needed. */

    /* Mesh block. Array of 16-bit integers.
       First is number of vertices (N), second is number of triangles (Nt), followed by
       - N X coordinates in 2.14 format
       - N Y coordinates in 2.14 format
       - N Z coordinates in 2.14 format
       - N X normal in 2.14 format
       - N Y normal in 2.14 format
       - N Z normal in 2.14 format
       - N R component in 6.10 format
       - N G component in 6.10 format
       - N B component in 6.10 format
       - 3*Nt point indexes */
    const uint32_t TYPE_MESH = 1;

    /* Grid block. Array of 16-bit integers.
       First is number of lines (N), followed by
       - 2*N X coordinates in 2.14 format
       - 2*N Y coordinates in 2.14 format
       - 2*N Z coordinates in 2.14 format */
    const uint32_t TYPE_GRID = 2;

    /* Positions block. Array of 16-bit integers.
       First is number of points (N), followed by
       - N IDs
       - N X coordinates in 2.14 format
       - N Y coordinates in 2.14 format
       - N Z coordinates in 2.14 format */
    const uint32_t TYPE_POSLIST = 3;


    void loadContent(afl::io::Stream& in, std::vector<Int16_t>& content, uint32_t size, afl::string::Translator& tx)
    {
        if ((size > MAX_SIZE) || (size & 1) != 0) {
            throw afl::except::FileFormatException(in, tx("Bad object size"));
        }

        content.resize(size / 2);
        in.fullRead(afl::base::Memory<Int16_t>(content).toBytes());
    }

    gfx::ColorQuad_t makeColor(int16_t r, int16_t g, int16_t b)
    {
        /* External colors are in 6.10 format, internal colors are in 0.8 format, so shift right.
           Colors brighter than maximum (1.0, 255) are clamped to maximum. */
        uint8_t rr = static_cast<uint8_t>(std::min(1023, std::max(0, int(r))) >> 2);
        uint8_t gg = static_cast<uint8_t>(std::min(1023, std::max(0, int(g))) >> 2);
        uint8_t bb = static_cast<uint8_t>(std::min(1023, std::max(0, int(b))) >> 2);
        return COLORQUAD_FROM_RGB(rr, gg, bb);
    }

    float makeCoordinate(int16_t c)
    {
        /* Floats are in 2.14 format. */
        return float(c) * float(1.0 / 16384);
    }

    void rejectFileIf(bool flag, afl::io::Stream& in, afl::string::Translator& tx)
    {
        if (flag) {
            throw afl::except::FileFormatException(in, tx("File contains invalid data"));
        }
    }

    void loadPosList(gfx::threed::PositionList& out,
                     afl::io::Stream& in,
                     afl::base::Memory<Int16_t> content,
                     afl::string::Translator& tx)
    {
        // Header
        rejectFileIf(content.size() < 1, in, tx);
        size_t numPoints = static_cast<uint16_t>(*content.at(0));
        content.split(1);

        // Sub-arrays
        afl::base::Memory<Int16_t> id = content.split(numPoints);
        afl::base::Memory<Int16_t> x  = content.split(numPoints);
        afl::base::Memory<Int16_t> y  = content.split(numPoints);
        afl::base::Memory<Int16_t> z  = content.split(numPoints);
        rejectFileIf(z.size() != numPoints, in, tx);

        // Parse
        for (size_t i = 0; i < numPoints; ++i) {
            out.add(static_cast<uint16_t>(*id.at(i)), gfx::threed::Vec3f(makeCoordinate(*x.at(i)), makeCoordinate(*y.at(i)), makeCoordinate(*z.at(i))));
        }
    }
}


/*
 *  Mesh
 */

struct gfx::threed::Model::Mesh {
    std::vector<Vec3f> m_points;
    std::vector<Vec3f> m_normals;
    std::vector<ColorQuad_t> m_colors;
    std::vector<size_t> m_indexes;
    void load(afl::io::Stream& in, afl::base::Memory<Int16_t> content, afl::string::Translator& tx);
};

inline void
gfx::threed::Model::Mesh::load(afl::io::Stream& in, afl::base::Memory<Int16_t> content, afl::string::Translator& tx)
{
    // Header
    rejectFileIf(content.size() < 2, in, tx);
    size_t numVertices = static_cast<uint16_t>(*content.at(0));
    size_t numTriangles = static_cast<uint16_t>(*content.at(1));
    content.split(2);

    size_t numPoints = 3*numTriangles;

    // Sub-arrays
    afl::base::Memory<Int16_t> x   = content.split(numVertices);
    afl::base::Memory<Int16_t> y   = content.split(numVertices);
    afl::base::Memory<Int16_t> z   = content.split(numVertices);
    afl::base::Memory<Int16_t> xn  = content.split(numVertices);
    afl::base::Memory<Int16_t> yn  = content.split(numVertices);
    afl::base::Memory<Int16_t> zn  = content.split(numVertices);
    afl::base::Memory<Int16_t> r   = content.split(numVertices);
    afl::base::Memory<Int16_t> g   = content.split(numVertices);
    afl::base::Memory<Int16_t> b   = content.split(numVertices);
    afl::base::Memory<Int16_t> pts = content.split(numPoints);

    // If we could not get the last section in its entirety, file is bad.
    rejectFileIf(pts.size() != numPoints, in, tx);

    // Create vertices
    m_points.reserve(numVertices);
    m_normals.reserve(numVertices);
    m_colors.reserve(numVertices);
    for (size_t i = 0; i < numVertices; ++i) {
        m_points .push_back(Vec3f(makeCoordinate(*x.at(i)),  makeCoordinate(*y.at(i)),  makeCoordinate(*z.at(i))));
        m_normals.push_back(Vec3f(makeCoordinate(*xn.at(i)), makeCoordinate(*yn.at(i)), makeCoordinate(*zn.at(i))));
        m_colors .push_back(makeColor(*r.at(i), *g.at(i), *b.at(i)));
    }

    // Connect vertices; verify indexes in the process
    m_indexes.reserve(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        size_t index = static_cast<uint16_t>(*pts.at(i));
        rejectFileIf(index >= numVertices, in, tx);
        m_indexes.push_back(index);
    }
}


/*
 *  Grid
 */

struct gfx::threed::Model::Grid {
    std::vector<Vec3f> m_points;
    void load(afl::io::Stream& in, afl::base::Memory<Int16_t> content, afl::string::Translator& tx);
};

inline void
gfx::threed::Model::Grid::load(afl::io::Stream& in, afl::base::Memory<Int16_t> content, afl::string::Translator& tx)
{
    // Header
    rejectFileIf(content.size() < 1, in, tx);
    size_t numLines = static_cast<uint16_t>(*content.at(0));
    content.split(1);

    size_t numPairs = 2*numLines;

    // Sub-arrays
    afl::base::Memory<Int16_t> x = content.split(numPairs);
    afl::base::Memory<Int16_t> y = content.split(numPairs);
    afl::base::Memory<Int16_t> z = content.split(numPairs);
    rejectFileIf(z.size() != numPairs, in, tx);

    // Parse the arrays
    for (size_t i = 0; i < numPairs; ++i) {
        m_points.push_back(Vec3f(makeCoordinate(*x.at(i)), makeCoordinate(*y.at(i)), makeCoordinate(*z.at(i))));
    }
}


/*
 *  Model
 */

afl::base::Ref<gfx::threed::Model>
gfx::threed::Model::create()
{
    return *new Model();
}

inline
gfx::threed::Model::Model()
    : RefCounted(),
      m_meshes(),
      m_grids(),
      m_positions()
{ }

gfx::threed::Model::~Model()
{ }

void
gfx::threed::Model::load(afl::io::Stream& in, afl::string::Translator& tx)
{
    // Header
    Header h;
    in.fullRead(afl::base::fromObject(h));
    if (!afl::base::ConstBytes_t(h.signature).equalContent(SIGNATURE)) {
        throw afl::except::FileFormatException(in, tx("File is missing required signature"));
    }
    if (h.version != VERSION_1) {
        throw afl::except::FileFormatException(in, tx("Unsupported file format version"));
    }

    // Number of elements
    size_t numElements = h.numElements;
    if (numElements > MAX_ELEMENTS) {
        throw afl::except::FileFormatException(in, tx("Too many objects in file"));
    }

    // Block index
    std::vector<Block> blocks(numElements);
    in.fullRead(afl::base::Memory<Block>(blocks).toBytes());

    // Read the blocks
    for (size_t i = 0; i < numElements; ++i) {
        switch (blocks[i].type) {
         case TYPE_MESH: {
            std::vector<Int16_t> content;
            loadContent(in, content, blocks[i].size, tx);
            std::auto_ptr<Mesh> p(new Mesh());
            p->load(in, content, tx);
            m_meshes.pushBackNew(p.release());     // Only add completed object so failure to parse does not leave us with a partial object
            break;
         }
         case TYPE_GRID: {
            std::vector<Int16_t> content;
            loadContent(in, content, blocks[i].size, tx);
            std::auto_ptr<Grid> p(new Grid());
            p->load(in, content, tx);
            m_grids.pushBackNew(p.release());     // Only add completed object so failure to parse does not leave us with a partial object
            break;
         }
         case TYPE_POSLIST: {
            std::vector<Int16_t> content;
            loadContent(in, content, blocks[i].size, tx);
            loadPosList(m_positions, in, content, tx);
            break;
         }
         default:
            in.setPos(in.getPos() + blocks[i].size);
            break;
        }
    }
}


size_t
gfx::threed::Model::getNumMeshes() const
{
    return m_meshes.size();
}

size_t
gfx::threed::Model::getNumGrids() const
{
    return m_grids.size();
}

const gfx::threed::PositionList&
gfx::threed::Model::positions() const
{
    return m_positions;
}

void
gfx::threed::Model::renderMesh(size_t index, TriangleRenderer& r) const
{
    if (index < m_meshes.size()) {
        const Mesh* p = m_meshes[index];
        r.addTriangles(r.addVertices(p->m_points, p->m_normals, p->m_colors), p->m_indexes);
    }
}

void
gfx::threed::Model::renderMesh(size_t index, TriangleRenderer& r, const ColorTransformation& tr) const
{
    // TODO: consider moving the color transformation inside the TriangleRenderer to execute on the GPU?
    if (index < m_meshes.size()) {
        const Mesh* p = m_meshes[index];

        std::vector<ColorQuad_t> colors;
        const size_t n = p->m_colors.size();
        colors.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            colors.push_back(tr.transform(p->m_colors[i]));
        }
        r.addTriangles(r.addVertices(p->m_points, p->m_normals, colors), p->m_indexes);
    }
}

void
gfx::threed::Model::renderGrid(size_t index, LineRenderer& r, ColorQuad_t color) const
{
    if (index < m_grids.size()) {
        const Grid* p = m_grids[index];
        for (size_t i = 0; i < p->m_points.size(); i += 2) {
            r.add(p->m_points[i], p->m_points[i+1], color);
        }
    }
}
