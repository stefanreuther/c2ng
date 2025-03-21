/**
  *  \file ui/widgets/numberselector.hpp
  *  \brief Class ui::widgets::NumberSelector
  */
#ifndef C2NG_UI_WIDGETS_NUMBERSELECTOR_HPP
#define C2NG_UI_WIDGETS_NUMBERSELECTOR_HPP

#include "afl/base/deleter.hpp"
#include "afl/base/observable.hpp"
#include "afl/base/signalconnection.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace widgets {

    /** Base class for a number selector widget. */
    class NumberSelector : public SimpleWidget {
     public:
        /** Constructor.
            @param value Value; user can observe it for changes.
            @param min   Lower limit
            @param max   Upper limit
            @param step  Step size */
        NumberSelector(afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);

        /** Destructor. */
        ~NumberSelector();

        /** Set value.
            Checks ranges and forces the value into range.
            @param value Value */
        void setValue(int32_t value);

        /** Get value.
            @return current value */
        int32_t getValue() const;

        /** Get configured minimum.
            @return minimum */
        int32_t getMin() const;

        /** Get configured maximum.
            @return maximum */
        int32_t getMax() const;

        /** Get configured step width.
            @return step width */
        int32_t getStep() const;

        /** Increment value.
            The result is limited to the configured maximum.
            @param n Value to add. If value given is <= 0, 1 is added instead. */
        void increment(int32_t n);

        /** Decrement value.
            The result is limited to the configured minimum.
            @param n Value to subtract. If value given is <= 0, 1 is subtracted instead. */
        void decrement(int32_t n);

        /** Default key handler.
            Call this from your handleKey() implementation.
            Handles combinations of "+"/"-" resp. Right/Left keys to modify the value.
            @param key Key
            @param prefix Prefix argument
            @return true if key was handled */
        bool defaultHandleKey(util::Key_t key, int prefix);

        /** Access value.
            @return value */
        afl::base::Observable<int32_t>& value();

        /** Create compound widget with buttons.
            Creates a widget containing a "-" button to the left, a "+" button to the right of this NumberSelector.
            @param del  Deleter that will own the created objects
            @param root UI root
            @return compound widget owned by del */
        ui::Widget& addButtons(afl::base::Deleter& del, Root& root);

     private:
        afl::base::Observable<int32_t>& m_value;
        const int32_t m_min;
        const int32_t m_max;
        const int32_t m_step;

        afl::base::SignalConnection conn_change;

        void onChange();
    };

} }


inline int32_t
ui::widgets::NumberSelector::getValue() const
{
    return m_value.get();
}

inline int32_t
ui::widgets::NumberSelector::getMin() const
{
    return m_min;
}

inline int32_t
ui::widgets::NumberSelector::getMax() const
{
    return m_max;
}

inline int32_t
ui::widgets::NumberSelector::getStep() const
{
    return m_step;
}

inline afl::base::Observable<int32_t>&
ui::widgets::NumberSelector::value()
{
    return m_value;
}

#endif
