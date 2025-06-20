/**
  *  \file client/widgets/testapplet.hpp
  *  \brief Class client::widgets::TestApplet
  */
#ifndef C2NG_CLIENT_WIDGETS_TESTAPPLET_HPP
#define C2NG_CLIENT_WIDGETS_TESTAPPLET_HPP

#include "ui/widgets/testapplet.hpp"

namespace client { namespace widgets {

    /** Client widget test applet.

        This extends ui::widgest::TestApplet with some tests for client-specific widgets. */
    class TestApplet : public ui::widgets::TestApplet {
     public:
        static TestApplet* makePlayerList(bool flow);
        static TestApplet* makeAllianceStatusList();
        static TestApplet* makeReferenceList();
        static TestApplet* makeFileList();
    };

} }

#endif
