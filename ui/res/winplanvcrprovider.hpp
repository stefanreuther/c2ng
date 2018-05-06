/**
  *  \file ui/res/winplanvcrprovider.hpp
  *  \brief Class ui::res::WinplanVcrProvider
  */
#ifndef C2NG_UI_RES_WINPLANVCRPROVIDER_HPP
#define C2NG_UI_RES_WINPLANVCRPROVIDER_HPP

#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "ui/res/provider.hpp"

namespace ui { namespace res {

    /** Winplan WPVCR.DLL Resource Provider.

        This class provides functionality for reading the WPVCR.DLL file from Winplan.
        That file contains pictures to use in the VCR (left/right moving small ships). */
    class WinplanVcrProvider : public Provider {
     public:
        /** Constructor.
            \param file WPVCR.DLL */
        WinplanVcrProvider(afl::base::Ref<afl::io::Stream> file);

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        /** Initialize by loading file header. */
        void init();

        /** Number of image slots. */
        static const int NUM = 160;

        /** File header.
            File contains two of these (left and right pointing ships). */
        struct Header {
            afl::bits::Value<afl::bits::UInt32LE> position[NUM];
            uint8_t pad1[9];
            afl::bits::Value<afl::bits::UInt16LE> size[NUM];
            uint8_t pad2[31];
        };
        Header m_header[2];

        /** File. */
        const afl::base::Ref<afl::io::Stream> m_file;
    };

} }

#endif
