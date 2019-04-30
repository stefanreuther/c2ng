/**
  *  \file ui/widgets/numberselector.hpp
  */
#ifndef C2NG_UI_WIDGETS_NUMBERSELECTOR_HPP
#define C2NG_UI_WIDGETS_NUMBERSELECTOR_HPP

#include "afl/base/observable.hpp"
#include "ui/simplewidget.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/base/deleter.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    class NumberSelector : public SimpleWidget {
     public:
        NumberSelector(afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);
        ~NumberSelector();

        void setValue(int32_t value);
        int32_t getValue() const;

        int32_t getMin() const;
        int32_t getMax() const;
        int32_t getStep() const;

        void setRange(int32_t min, int32_t max);

        void increment(int32_t n);
        void decrement(int32_t n);

        bool defaultHandleKey(util::Key_t key, int prefix);

        afl::base::Observable<int32_t>& value();

        ui::Widget& addButtons(afl::base::Deleter& del, Root& root);

     private:
        afl::base::Observable<int32_t>& m_value;
        int32_t m_min;
        int32_t m_max;
        int32_t m_step;

        afl::base::SignalConnection conn_change;

        void onChange();
    };

} }

#endif
