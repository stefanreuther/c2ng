/**
  *  \file gfx/threed/model.hpp
  *  \brief Class gfx::threed::Model
  */
#ifndef C2NG_GFX_THREED_MODEL_HPP
#define C2NG_GFX_THREED_MODEL_HPP

#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "gfx/threed/positionlist.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace threed {

    class ColorTransformation;
    class LineRenderer;
    class TriangleRenderer;

    /** 3-D model.
        Represents a set of related rendering instructions for a model, i.e. triangle meshes and wireframe grids.
        Models can be loaded from files, and rendered on appropriate renderers.

        TODO: As of 20230717, there is no code to create models or model files in c2ng;
        this is done in the separate cc2res project.
        We only allow creation of models on the heap to support possible future createMesh() and createGrid()
        methods that return TriangleRenderer and LineRenderer, respectively, which refer to us. */
    class Model : public afl::base::RefCounted {
     public:
        /** Constructor.
            Create an empty model. */
        static afl::base::Ref<Model> create();

        /** Destructor. */
        ~Model();

        /** Load from file.
            @param in   File
            @param tx   Translator (for error messages)
            @throw afl::except::FileProblemException on error */
        void load(afl::io::Stream& in, afl::string::Translator& tx);

        /** Get number of available meshes.
            @return number */
        size_t getNumMeshes() const;

        /** Get number of available grids.
            @return number */
        size_t getNumGrids() const;

        /** Access position list.
            @return PositionList object */
        const PositionList& positions() const;

        /** Render mesh on a TriangleRenderer.
            @param index Index, [0,getNumMeshes()). Call is ignored if out-of-range.
            @param r     Renderer */
        void renderMesh(size_t index, TriangleRenderer& r) const;

        /** Render mesh on a TriangleRenderer, with color transformation.
            @param index Index, [0,getNumMeshes()). Call is ignored if out-of-range.
            @param r     Renderer
            @param tr    Color transformation */
        void renderMesh(size_t index, TriangleRenderer& r, const ColorTransformation& tr) const;

        /** Render grid on a LineRenderer.
            @param index Index, [0,getNumGrids()). Call is ignored if out-of-range.
            @param r     Renderer
            @param color Color */
        void renderGrid(size_t index, LineRenderer& r, ColorQuad_t color) const;

     private:
        Model();

        struct Mesh;
        struct Grid;

        afl::container::PtrVector<Mesh> m_meshes;
        afl::container::PtrVector<Grid> m_grids;
        PositionList m_positions;
    };

} }

#endif
