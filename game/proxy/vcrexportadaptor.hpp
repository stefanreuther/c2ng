/**
  *  \file game/proxy/vcrexportadaptor.hpp
  *  \brief VCR export adaptors
  */
#ifndef C2NG_GAME_PROXY_VCREXPORTADAPTOR_HPP
#define C2NG_GAME_PROXY_VCREXPORTADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/proxy/exportadaptor.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"

namespace game { namespace proxy {

    typedef afl::base::Closure<ExportAdaptor*(VcrDatabaseAdaptor&)> VcrExportAdaptor_t;

    /** Make (creator for) VCR database export adaptor.
        Use with RequestSender<VcrDatabaseAdaptor>::makeTemporary to create a RequestSender<ExportAdaptor>
        that exports the content of the VCR database.

        @return newly-allocated closure
        @see game::interface::VcrContext */
    VcrExportAdaptor_t* makeVcrExportAdaptor();

    /** Make (creator for) VCR unit adaptor.
        Use with RequestSender<VcrDatabaseAdaptor>::makeTemporary to create a RequestSender<ExportAdaptor>
        that exports all participants of the given VCR.

        @param battleNr 0-based battle number
        @return newly-allocated closure
        @see game::interface::VcrSideContext */
    VcrExportAdaptor_t* makeVcrSideExportAdaptor(size_t battleNr);

} }

#endif
