/**
  *  \file util/prefixargument.cpp
  *  \brief Class util::PrefixArgument
  */

#include "util/prefixargument.hpp"
#include "afl/string/format.hpp"

// Constructor.
util::PrefixArgument::PrefixArgument(int initialValue)
    : m_value(initialValue),
      m_secondValue(0),
      m_operator(NoOp)
{
    // ex UIPrefixArgInput::UIPrefixArgInput
}

// Get current text for widget.
String_t
util::PrefixArgument::getText(afl::string::Translator& tx) const
{
    // ex UIPrefixArgInput::getText
    const String_t title = tx.translateString("Prefix: ");
    if (m_operator != NoOp) {
        const char opchar = (m_operator == MultiplyOp ? '*' : '/');
        if (m_secondValue != 0) {
            return afl::string::Format("%s%d%c%d", title, m_value, opchar, m_secondValue);
        } else {
            return afl::string::Format("%s%d%c", title, m_value, opchar);
        }
    }
    return afl::string::Format("%s%d", title, m_value);
}

// Get current effective value of prefix argument.
int
util::PrefixArgument::getValue() const
{
    // ex UIPrefixArgInput::getValue
    if (m_secondValue != 0) {
        if (m_operator == MultiplyOp) {
            return m_value * m_secondValue;
        } else if (m_operator == DivideOp) {
            return m_value / m_secondValue;
        } else {
            return m_value;
        }
    } else {
        return m_value;
    }
}

util::PrefixArgument::Action
util::PrefixArgument::handleKey(const Key_t key)
{
    // ex UIPrefixArgInput::handleKey
    const Key_t rawKey = key & ~KeyMod_Alt;
    if (rawKey >= '0' && rawKey <= '9') {
        int& theValue = (m_operator == NoOp ? m_value : m_secondValue);
        int oldValue = theValue;
        /* we only allow entry of a number if
           a. the resulting component is less than 10000
           b. the resulting value is less than 10000
           c. the resulting value is not zero */
        if (oldValue < 1000) {
            theValue = 10 * oldValue + (rawKey - '0');
            if (getValue() == 0 || getValue() >= 10000) {
                theValue = oldValue;
            }
        }
        return Accepted;
    } else if (rawKey == '*' || rawKey == '/') {
        /* when entering '*',
           '77/11' goes to '7*'
           '77/'   goes to '77*' */
        if (getValue() != 0) {
            m_value = getValue();
            m_secondValue = 0;
            m_operator = (rawKey == '*' ? MultiplyOp : DivideOp);
        }
        return Accepted;
    } else if (rawKey == Key_Backspace) {
        /* 77/11 goes to 77/1
           77/1  goes to 77/
           77/   goes to 77
           77    goes to 7 */
        if (m_operator != NoOp) {
            if (m_secondValue > 0) {
                m_secondValue /= 10;
            } else {
                m_operator = NoOp;
            }
        } else {
            m_value /= 10;
            if (m_value == 0) {
                return Canceled;
            }
        }
        return Accepted;
    } else if (rawKey == Key_Escape) {
        /* Cancellation */
        m_operator = NoOp;
        m_value = 0;
        m_secondValue = 0;
        return Canceled;
    } else {
        return NotHandled;
    }
}
