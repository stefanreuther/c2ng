/**
  *  \file game/sim/loader.hpp
  *  \brief Class game::sim::Loader
  */
#ifndef C2NG_GAME_SIM_LOADER_HPP
#define C2NG_GAME_SIM_LOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"

namespace game { namespace sim {

    class Setup;

    /** Simulation setup loader. */
    class Loader {
     public:
        /** Constructor.
            \param cs Character set */
        explicit Loader(afl::charset::Charset& cs);

        /** Load a setup.
            \param in Stream
            \param setup [out] Simulation setup

            The file will be loaded and appended to the given simulation setup.
            The file content will not be verified against a ship list, and will not be verified for well-formedness (e.g. unique Ids).
            If loading the file fails mid-way, the setup will contain a partial result.

            \change PCC2 version would access ship list data here */
        void load(afl::io::Stream& in, Setup& setup);

     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
