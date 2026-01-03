/**
  *  \file ui/waitindicator.hpp
  *  \brief Class ui::WaitIndicator
  */
#ifndef C2NG_UI_WAITINDICATOR_HPP
#define C2NG_UI_WAITINDICATOR_HPP

#include "util/waitindicator.hpp"
#include "ui/root.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "ui/widgets/busyindicator.hpp"

namespace ui {

    /** Helper for calling into a background task with UI synchronisation.
        This implements a WaitIndicator using the UI framework.
        During the wait time, the sending thread (the UI thread) will be kept alive using an EventLoop,
        and the user sees a BusyIndicator.

        Use Downlink for information requests in reaction to user input.
        Do NOT use Downlink from a drawWidget() callback. */
    class WaitIndicator : public util::WaitIndicator {
     public:
        /** Constructor.
            @param root UI Root
            @param tx Translator */
        explicit WaitIndicator(Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~WaitIndicator();

        /** Access contained BusyIndicator widget.
            @return widget */
        ui::widgets::BusyIndicator& indicator()
            { return m_indicator; }

        // WaitIndicator:
        void post(bool success);
        bool wait();

     private:
        ui::Root& m_root;
        ui::widgets::BusyIndicator m_indicator;
        bool m_busy;
        EventLoop m_loop;

        void setBusy(bool flag);
    };

}

#endif
