/**
  *  \file game/parser/messagetemplate.hpp
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

        MessageTemplate(MessageInformation::Type type, String_t name);
        ~MessageTemplate();

        void addMatchInstruction(uint8_t opcode, uint16_t value);
        void addValueInstruction(uint8_t opcode, String_t value);
        void addCheckInstruction(uint8_t opcode, int8_t offset, String_t value);
        void addVariable(String_t name);
        void addVariables(String_t names);
        void setContinueFlag(bool flag);
        bool getContinueFlag() const;

        size_t getNumVariables() const;
        size_t getNumWildcards() const;
        size_t getNumRestrictions() const;
        bool getVariableSlotByName(String_t name, size_t& out) const;
        String_t getVariableName(size_t index) const;
        String_t getTemplateName() const;
        MessageInformation::Type getMessageType() const;

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

        bool check(const MessageLines_t& message, uint16_t& line, const Instruction& insn, const DataInterface& iface) const;
        bool matchLine(const String_t& line, size_t index, size_t nvar, size_t typeIndex, std::vector<String_t>& values, const DataInterface& iface) const;
        bool matchPart(const String_t& line, size_t startAt, size_t index, size_t nvar, std::vector<String_t>& values, const DataInterface& iface) const;
        void consolidateArray(std::vector<String_t>& values, size_t nvar, size_t nelems) const;

        MessageInformation::Type m_messageType;
        String_t m_name;
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

    void splitMessage(MessageLines_t& out, const String_t& in);
    int32_t getMessageHeaderInformation(const MessageLines_t& msg, MessageHeaderInformation what);
    int32_t parseIntegerValue(const String_t& value);

} }

inline bool
game::parser::MessageTemplate::getContinueFlag() const
{
    return m_continueFlag;
}

inline game::parser::MessageInformation::Type
game::parser::MessageTemplate::getMessageType() const
{
    // ex GMessageTemplate::getMessageObject
    return m_messageType;
}

#endif
