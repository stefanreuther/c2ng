/**
  *  \file game/parser/messagetemplate.hpp
  *  \brief Class game::parser::MessageTemplate
  */
#ifndef C2NG_GAME_PARSER_MESSAGETEMPLATE_HPP
#define C2NG_GAME_PARSER_MESSAGETEMPLATE_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace parser {

    class DataInterface;

    typedef std::vector<String_t> MessageLines_t;

    /** Message template.
        This object contains information and logic to parse a single message into a set of variables.
        MessageTemplate objects are independant from their environment.
        Message parsing will need a DataInterface to access the environment. */
    class MessageTemplate {
        /** Number of players.
            To improve robustness a little, it makes sense to limit message parsing to the maximum number of players
            that are actually used in parsed messages.
            Since we currently don't parse stuff for more-than-11-player games, let's use this. */
        static const int NUM_PLAYERS = 11;
     public:
        // Instruction opcodes
        enum {
            // Simple instructions
            iMatchKind  = 0x00,                 // index=chr(key)
            iMatchSubId = 0x01,                 // index=chr(subid)
            iMatchBigId = 0x02,                 // index=BigId (integer)
            iValue      = 0x03,                 // index=index into string table

            // Instructions that have a scope
            iCheck      = 0x10,                 // plus scope, index=index into string table
            iFail       = 0x20,                 // plus scope, index=index into string table
            iFind       = 0x30,                 // plus scope, index=index into string table
            iParse      = 0x40,                 // plus scope, index=index into string table, count=number of variables
            iArray      = 0x50,                 // plus scope, index=index into string table, count=number of variables

            iMask       = 0xF0
        };

        // Scopes
        enum {
            sAny        = 0,
            sRelative   = 1,
            sFixed      = 2
        };

        /** Constructor.
            Create a blank message template.
            \param type Message information produced by this template
            \param name Template name, for use in messages */
        MessageTemplate(MessageInformation::Type type, String_t name);

        /** Destructor. */
        ~MessageTemplate();

        /** Add "match" instruction.
            \param opcode One of iMatchKind, iMatchSubId, iMatchBigId
            \param value Value to match on */
        void addMatchInstruction(uint8_t opcode, uint16_t value);

        /** Add "value" instruction.
            \param opcode Must be iValue
            \param value Value to produce (reserved name or value) */
        void addValueInstruction(uint8_t opcode, String_t value);

        /** Add "check" instruction.
            \param opcode One of iCheck, iFail, iFind, iParse, iArray with optional scope modifier added
            \param offset Distance, if required by scope modifier
            \param value List of values, separated by commas. Each entry will generate a value. */
        void addCheckInstruction(uint8_t opcode, int8_t offset, String_t value);

        /** Add a single variable.
            \param name NAME or NAME:TYPE of variable (in either case) */
        void addVariable(String_t name);

        /** Add list of variables.
            \param names Comma-separated list of NAME or NAME:TYPE.
            \see addVariable */
        void addVariables(String_t names);

        /** Set continuation flag.
            If set, further templates will be tried after this one matched.
            If clear (default), a successful match of this template stops message parsing.
            \param flag Continuation flag */
        void setContinueFlag(bool flag);

        /** Get continuation flag
            \return continuation flag
            \see setContinueFlag */
        bool getContinueFlag() const;

        /** Get number of variables.
            This is the number of variables in "assign" statements that this template will generate.
            For a valid complete template, getNumVariables() = getNumWildcards(),
            i.e. there is a placeholder for every variable,
            but during parsing of a template from its definition there may be differences.
            \return Number of variables */
        size_t getNumVariables() const;

        /** Get number of wildcards.
            This is the number of values this template will produce when matching.
            For a valid complete template, getNumVariables() = getNumWildcards(),
            i.e. there is a placeholder for every variable,
            but during parsing of a template from its definition there may be differences.
            \return Number of placeholders */
        size_t getNumWildcards() const;

        /** Get number of restrictions.
            This is the number of non-empty matches.
            A template with no restrictions will match every message.
            \return Number of restrictions */
        size_t getNumRestrictions() const;

        /** Find variable slot by name.
            \param name [in] Name to search for, in upper-case
            \param out [out] Result slot
            \retval true Variable was found, \c out was updated such that getVariableName(out) == name
            \retval false Variable not found */
        bool getVariableSlotByName(String_t name, size_t& out) const;

        /** Get variable name by index.
            \param index Index [0,getNumVariables())
            \return name; empty string if index is out of range */
        String_t getVariableName(size_t index) const;

        /** Get name of template (as set in constructor).
            \return Name */
        String_t getTemplateName() const;

        /** Get message information type (as set in constructor).
            \return message information type */
        MessageInformation::Type getMessageType() const;

        /** Match message against this template.
            \param message [in] Message
            \param iface [in] Data interface to produce context information
            \param values [out] Placeholder values; empty on call.
            \retval true if message matches, values have been filled out
            \retval false if message does not match, values has unspecified content */
        bool match(const MessageLines_t& message, const DataInterface& iface, std::vector<String_t>& values) const;

     private:
        // Instructions
        struct Instruction {
            uint8_t opcode;
            int8_t offset;
            uint16_t index;
            uint16_t count;
            Instruction(uint8_t opcode, int8_t offset, uint16_t index, uint16_t count)
                : opcode(opcode), offset(offset), index(index), count(count)
                { }
        };

        /** Check for a string.
            \param message [in] Message text
            \param line    [in/out] Current line pointer
            \param insn    [in] Instruction to process; this checks the index and scope fields
            \param iface   [in] Data interface to produce context information */
        bool check(const MessageLines_t& message, size_t& line, const Instruction& insn, const DataInterface& iface) const;

        /** Match a single line.
            \param line   [in] Text line from message
            \param index  [in] Index into string table of first string
            \param nvar   [in] Number of variables to produce
            \param typeIndex [in] Index into type array
            \param values [out] Produce output here
            \param iface   [in] Data interface to produce context information
            \retval true match succeeded; values have been produced and postprocessed according to their types
            \retval false match failed; values have not been changed */
        bool matchLine(const String_t& line, size_t index, size_t nvar, size_t typeIndex, std::vector<String_t>& values, const DataInterface& iface) const;

        /** Match a partial line.
            \param line    [in] Text line from message
            \param startAt [in] Value of variable starts at this position
            \param index   [in] Index into string table of first string
            \param nvar    [in] Number of variables to produce
            \param values  [out] Produce output here
            \param iface   [in] Data interface to produce context information
            \retval true match succeeded; values have been produced (but not postprocessed)
            \retval false match failed; values have not been changed */
        bool matchPart(const String_t& line, size_t startAt, size_t index, size_t nvar, std::vector<String_t>& values, const DataInterface& iface) const;

        /** Consolidate an array.
            Matching has produced nvar*nelems items.
            Combine that down to nvar elements, each containing a list of values.
            \param values [in/out] Values
            \param nvar   [in] Number of variables (columns)
            \param nelems [in] Number of array items (rows) */
        void consolidateArray(std::vector<String_t>& values, size_t nvar, size_t nelems) const;

        const MessageInformation::Type m_messageType;
        const String_t m_name;
        bool m_continueFlag;
        std::vector<Instruction> m_instructions;
        std::vector<String_t> m_strings;
        std::vector<String_t> m_variables;
        std::vector<String_t> m_types;
    };

    enum MessageHeaderInformation {
        MsgHdrKind,                 ///< Message kind. Character code.
        MsgHdrSubId,                ///< Message SubId. Character code.
        MsgHdrId,                   ///< Message Id. Integer.
        MsgHdrBigId,                ///< Message BigId, SubId and Id concatenated. Integer.
        MsgHdrAge                   ///< Message age. Flag, 0 for current, 1 for old.
    };

    /** Split message into lines.
        \param out [out] Result
        \param in [in] Message as single, readable (i.e. rot-13, "\r" already resolved) string */
    void splitMessage(MessageLines_t& out, const String_t& in);

    /** Extract information from message header.
        \param msg  Message text
        \param what Requested information
        \return Information. 0 if not found. */
    int32_t getMessageHeaderInformation(const MessageLines_t& msg, MessageHeaderInformation what);

    /** Parse integer value.
        This strips possible suffixes (as in "10 kt" or "10 : 1").
        \param value Value received from user
        \return Integer value of initial part of \c value. -1 if it cannot be parsed. */
    int32_t parseIntegerValue(const String_t& value);

} }

// Get continuation flag
inline bool
game::parser::MessageTemplate::getContinueFlag() const
{
    return m_continueFlag;
}

// Get message information type (as set in constructor).
inline game::parser::MessageInformation::Type
game::parser::MessageTemplate::getMessageType() const
{
    // ex GMessageTemplate::getMessageObject
    return m_messageType;
}

#endif
