/**
  *  \file interpreter/assembler.cpp
  *  \brief Class interpreter::Assembler
  *
  *  The syntax parsed by this assembler is (almost) a superset of the
  *  PCC2 c2asm.pl program, and the output of AssemblerSaveContext.
  *
  *  Because this re-uses the existing VM I/O infrastructure, it
  *  allows a little less control over the output format than the PCC2
  *  version. In particular, whereas the PCC2 version produces output
  *  strictly in the order as things are declared/defined in the
  *  script, this version uses the SaveContext's sequencing mechanism.
  *
  *  Syntax of input:
  *
  *  A source file consists of a sequence of special commands (see below) and
  *  subroutine definitions. A subroutine definition looks like this:
  *    sub foo(args)
  *       assembly-insn
  *     label:
  *       assembly-insn
  *       assembly-insn
  *    endsub
  *  The header has the same form as in regular CCScript; in particular, the
  *  'Optional' keyword can be used, and 'Function' instead of 'Sub' says
  *  this routine returns a value (the header line actually just preinitializes
  *  the local table ('.local'), and the '.flags', '.min_args', '.max_args').
  *
  *  Structure types can be defined as
  *    struct foo
  *      .field a, b
  *    endsub
  *
  *  Comments can appear anywhere and start with ';' or '%'. Everything is
  *  case-insensitive.
  *
  *  Assembly insns have their regular name and take arguments in their
  *  regular form:
  *  - integers (e.g. 'pushint 42', 'dup 1')
  *  - symbol name (e.g. 'pushvar A'). The symbol is added to the symbol pool.
  *  - local name (e,g. 'pushloc A'). The argument is a local name which must
  *    exist ('.local' or routine heading)
  *  - label name (e.g. 'j LAB'). The label must be defined somewhere in this sub.
  *  - literals (argument for 'pushlit'):
  *    . integers, floats
  *    . 'true', 'false', 'null' (note that these normally are encoded using
  *      'pushbool', not 'pushlit')
  *    . strings in single or double quotes; backslash escapes in
  *      double-quoted strings
  *    . identifiers, interpreted as subroutine or structure names
  *    . '(tag,value)' generates a raw tag/value node for interesting effects,
  *      e.g. '(130,5)' generates 'Minefield(5)'.
  *    . By default, duplicate literals are recycled. Prefix with '!' or 'new'
  *      to create a new instance.
  *  - as a special exception, '#nn' is always accepted and generates the
  *    argument specified using the integer 'nn', even if the insn doesn't
  *    expect an argument, i.e. 'pushlit #5' pushes the 5th literal; 'uinc #5'
  *    generates a "inc" unary instruction with 5 in the (unused) arg field.
  *  To generate arbitrary opcodes, use 'genTYPMAJOR.MINOR', where TYP is the
  *  kind of the argument expected ('int', 'loc', 'sym', 'label', 'lit') and
  *  MAJOR/MINOR are the decimal opcode. If no TYP is given, the instruction
  *  takes an optional 'int' argument. Also, the assembler accepts some invalid
  *  instructions that have a possible encoding, e.g. 'procmem' or 'popint'.
  *
  *  Unlike the PCC2-classic version, we do not allow assembling fused
  *  instructions ("pushlit(u)").
  *
  *  Pseudo-ops:
  *  - in subroutines
  *    .defsubs         generate pushlit/sdefsub instructions for all uniquely-
  *                     named subs so far, excluding this one
  *    .file NAME       set file name for debug info
  *    .flags NN        set this sub's flags
  *    .line LINE[,ADR] add line number info
  *    .lit LITERAL     add literal to literal pool (subject to recycling rules)
  *    .local NAME      add local variable ('.local -' to make a nameless one)
  *    .name NAME       set this sub's name hint ('.name -' to make it empty)
  *    .max_args NN     set this sub's maximum arg count
  *    .min_args NN     set this sub's minimum arg count
  *    .num_labels NN   set this sub's label count
  *    .sym SYMBOL      add symbol to symbol pool (recycles existing)
  *    .varargs         mark this as varargs function
  *
  *  - global
  *    .jumps abs|sym   set jump mode to absolute (default) or symbolic.
  *                     Symbolic mode generates actual 'label' instructions
  *                     which gives slower execution but permits the optimizer
  *                     to run; absolute mode resolves all jumps.
  *    declare sub N,N  predeclare subroutines but don't give them a body yet.
  *                     You can therefore already reference these subs/functions.
  *                     The actual definition must follow later.
  *    declare struct X predeclare a structure.
  *
  *  Whenever an identifier is required, we accept
  *  - an actual identifier (will be converted to upper-case)
  *  - a dash to make an empty identifier
  *  - a quoted string (will not be converted to upper-case)
  *
  *  Sub naming: each sub has three names:
  *  - assembler name, which is given in 'sub foo'
  *  - name hint, which defaults to the assembler name and can be changed
  *    with '.name'
  *  - real name, which is given by the 'sdefsub' instruction that defines the
  *    sub.
  *  An instruction taking a subroutine name always uses the assembler name.
  *  In case of a duplicate, the name refers to the most-recently defined one.
  *  To define multiple subs with the same name, and still be able to refer
  *  to both from the assembler, use the ".name" instruction. This CCScript
  *    If a Then
  *      Sub foo
  *        ...
  *      EndSub
  *    Else
  *      Sub foo
  *        ...
  *      EndSub
  *    EndIf
  *  can be assembled to
  *    sub foo_one
  *      .name foo
  *      ...
  *    endsub
  *    sub foo_two
  *      .name foo
  *      ...
  *    endsub
  *    sub main
  *      pushvar a
  *      jfep else
  *      pushsub foo_one
  *      j endif
  *     else:
  *      pushsub foo_two
  *     endif:
  *      sdefsub foo
  *    endsub
  *  Note that the '.name' is actually optional as it only affects optimisation
  *  in the interpreter (code of identically named subs will be merged if it is
  *  identical to avoid duplication across save/reload cycles). The script will
  *  work correctly even without name hints.
  *
  *  The last subroutine defined in the assembler file will be invoked when
  *  loading the object file. No subroutine will be automatically defined in the
  *  symbol table. Therefore, the last routine must enter them into the symbol
  *  table using 'pushlit/sdefsub' instruction pairs. To simplify this, you can
  *  use the '.defsubs' pseudo-op, which will generate all appropriate
  *  definitions (using the '.name' names, if given).
  */

#include "interpreter/assembler.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/string.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/error.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/ternaryoperation.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/vmio/structures.hpp"
#include "util/stringparser.hpp"

using afl::data::BooleanValue;
using afl::data::FloatValue;
using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::string::Format;
using interpreter::Opcode;

namespace {
    const char*const LOG_NAME = "interpreter.asm";

    /*
     *  Instruction templates
     */

    const uint32_t IntegerArg  = 1;    // 16-bit integer
    const uint32_t SymbolArg   = 2;    // symbol (BytecodeObject::addName)
    const uint32_t LocalArg    = 3;    // local name (BytecodeObject::addLocalVariable)
    const uint32_t LiteralArg  = 4;    // literal (BytecodeObject::addLiteral)
    const uint32_t LabelArg    = 5;    // label (also affects jSymbolic flac)
    const uint32_t ArgTypeMask = 15;

    const uint32_t OptionalArg = 16;   // optional argument

    uint32_t makeTemplate(Opcode::Major major, uint8_t minor, uint32_t args)
    { return 65536*major + 256*minor + args; }

    uint8_t getMajor(uint32_t tpl)
    { return uint8_t(tpl >> 16); }

    uint8_t getMinor(uint32_t tpl)
    { return uint8_t(tpl >> 8); }

    uint8_t getArgType(uint32_t tpl)
    { return uint8_t(tpl & ArgTypeMask); }

    /*
     *  Instruction Table
     */

    void addEnumeratedInstructions(std::map<String_t, uint32_t>& insn, Opcode::Major major, String_t prefix, const char* (&func)(uint8_t minor))
    {
        for (uint8_t op = 0; op < 254; ++op) {
            const char* p = func(op);
            if (p == 0 || *p == '?') {
                break;
            }
            insn[prefix + afl::string::strLCase(p)] = makeTemplate(major, op, 0);
        }
    }

    void addScopeInstruction(std::map<String_t, uint32_t>& insn, Opcode::Major major, String_t prefix)
    {
        insn[prefix + "var"]  = makeTemplate(major, Opcode::sNamedVariable, SymbolArg);
        insn[prefix + "loc"]  = makeTemplate(major, Opcode::sLocal,         LocalArg);
        insn[prefix + "top"]  = makeTemplate(major, Opcode::sStatic,        IntegerArg);
        insn[prefix + "glob"] = makeTemplate(major, Opcode::sShared,        IntegerArg);
        insn[prefix + "gvar"] = makeTemplate(major, Opcode::sNamedShared,   SymbolArg);
        insn[prefix + "lit"]  = makeTemplate(major, Opcode::sLiteral,       LiteralArg);
        insn[prefix + "int"]  = makeTemplate(major, Opcode::sInteger,       IntegerArg);
        insn[prefix + "bool"] = makeTemplate(major, Opcode::sBoolean,       IntegerArg);
    }

    void addIndirectInstruction(std::map<String_t, uint32_t>& insn, Opcode::Major major, String_t suffix, uint32_t argType)
    {
        insn["call"   + suffix] = makeTemplate(major, Opcode::miIMCall,                                 argType);
        insn["load"   + suffix] = makeTemplate(major, Opcode::miIMLoad,                                 argType);
        insn["store"  + suffix] = makeTemplate(major, Opcode::miIMStore,                                argType);
        insn["pop"    + suffix] = makeTemplate(major, Opcode::miIMPop,                                  argType);
        insn["proc"   + suffix] = makeTemplate(major, Opcode::miIMCall  + Opcode::miIMRefuseFunctions,  argType);
        insn["pload"  + suffix] = makeTemplate(major, Opcode::miIMLoad  + Opcode::miIMRefuseFunctions,  argType);
        insn["pstore" + suffix] = makeTemplate(major, Opcode::miIMStore + Opcode::miIMRefuseFunctions,  argType);
        insn["ppop"   + suffix] = makeTemplate(major, Opcode::miIMPop   + Opcode::miIMRefuseFunctions,  argType);
        insn["fcall"  + suffix] = makeTemplate(major, Opcode::miIMCall  + Opcode::miIMRefuseProcedures, argType);
        insn["func"   + suffix] = makeTemplate(major, Opcode::miIMLoad  + Opcode::miIMRefuseProcedures, argType);
        insn["fstore" + suffix] = makeTemplate(major, Opcode::miIMStore + Opcode::miIMRefuseProcedures, argType);
        insn["fpop"   + suffix] = makeTemplate(major, Opcode::miIMPop   + Opcode::miIMRefuseProcedures, argType);
    }

    /*
     *  Character classifiers
     */

    bool isSpace(char ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    bool isWord(char ch)
    {
        return (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z')
            || (ch >= '0' && ch <= '9')
            || ch == '.' || ch == '$' || ch == '_';
    }

    bool isComment(char ch)
    {
        return ch == ';' || ch == '%';
    }


    template<typename T>
    uint16_t packIndex(T nativeIndex)
    {
        uint16_t packedIndex = uint16_t(nativeIndex);
        if (nativeIndex != packedIndex) {
            throw interpreter::Error::tooComplex();
        }
        return packedIndex;
    }
}

/*
 *  Tokenizer
 */

class interpreter::Assembler::Tokenizer {
 public:
    enum Token {
        End,                    // End of line
        Punctuation,            // Any punctuation (single character)
        Identifier,             // Identifier
        Integer,                // Integer (sequence of digits only)
        Float,                  // Float (sequence of digits and period)
        String                  // Quoted string
    };

    // Construct from string
    explicit Tokenizer(const String_t& text);

    // Read next token
    void readNext();

    // "get" - retrieve properties of current token
    Token getCurrentToken() const;
    const String_t& getCurrentText() const;
    int32_t getCurrentInteger() const;
    double getCurrentFloat() const;

    // "is" - check whether current token is the given kind
    bool isPunctuation(const char* punct) const;

    // "accept" - check whether current token is the given kind; if so, consume it
    bool acceptIdentifier(const char* text);
    bool acceptPunctuation(const char* p);
    int32_t acceptSign();

    // "require" - like accept, but fail if it is not the expected token
    void requireEnd();
    void requirePunctuation(const char* punct);
    String_t requireName(const char* what);
    int32_t requireInteger(const char* what);

 private:
    const String_t m_text;
    size_t m_pos;
    Token m_currentToken;
    String_t m_currentText;
    int32_t m_currentInteger;
    double m_currentFloat;

    void skipWhitespace();
};

interpreter::Assembler::Tokenizer::Tokenizer(const String_t& text)
    : m_text(text),
      m_pos(0),
      m_currentToken(),
      m_currentText(),
      m_currentInteger(0),
      m_currentFloat(0.0)
{
    skipWhitespace();
}

void
interpreter::Assembler::Tokenizer::readNext()
{
    // Check for end
    if (m_pos >= m_text.size() || isComment(m_text[m_pos])) {
        m_pos = m_text.size();
        m_currentToken = End;
        return;
    }

    // Check for word
    size_t start = m_pos;
    while (m_pos < m_text.size() && isWord(m_text[m_pos])) {
        ++m_pos;
    }
    if (start != m_pos) {
        // Found word; could also be a number
        m_currentText = afl::string::strUCase(m_text.substr(start, m_pos-start));
        if (afl::string::strToInteger(m_currentText, m_currentInteger)) {
            m_currentToken = Integer;
        } else if (afl::string::strToFloat(m_currentText, m_currentFloat)) {
            m_currentToken = Float;
        } else {
            m_currentToken = Identifier;
        }
    } else {
        // Others
        char ch = m_text[m_pos++];
        m_currentText.clear();
        if (ch == '\'') {
            // Simple string
            while (m_pos < m_text.size() && m_text[m_pos] != '\'') {
                m_currentText += m_text[m_pos];
                ++m_pos;
            }
            if (m_pos >= m_text.size()) {
                throw Error::expectSymbol("'");
            }
            ++m_pos;
            m_currentToken = String;
        } else if (ch == '"') {
            // String with quotes
            bool quoted = false;
            while (m_pos < m_text.size() && (m_text[m_pos] != '"' || quoted)) {
                if (m_text[m_pos] == '\\' && !quoted) {
                    quoted = true;
                } else if (m_text[m_pos] == 'n' && quoted) {
                    m_currentText += "\n";
                    quoted = false;
                } else if (m_text[m_pos] == 't' && quoted) {
                    m_currentText += "\t";
                    quoted = false;
                } else {
                    m_currentText += m_text[m_pos];
                    quoted = false;
                }
                ++m_pos;
            }
            if (m_pos >= m_text.size()) {
                throw Error::expectSymbol("\"");
            }
            ++m_pos;
            m_currentToken = String;
        } else {
            m_currentText += ch;
            m_currentToken = Punctuation;
        }
    }
    skipWhitespace();
}

inline interpreter::Assembler::Tokenizer::Token
interpreter::Assembler::Tokenizer::getCurrentToken() const
{
    return m_currentToken;
}

inline const String_t&
interpreter::Assembler::Tokenizer::getCurrentText() const
{
    return m_currentText;
}

inline int32_t
interpreter::Assembler::Tokenizer::getCurrentInteger() const
{
    return m_currentInteger;
}

inline double
interpreter::Assembler::Tokenizer::getCurrentFloat() const
{
    return m_currentFloat;
}

inline bool
interpreter::Assembler::Tokenizer::isPunctuation(const char* punct) const
{
    return m_currentToken == Punctuation && m_currentText == punct;
}

inline bool
interpreter::Assembler::Tokenizer::acceptIdentifier(const char* text)
{
    if (m_currentToken == Identifier && m_currentText == text) {
        readNext();
        return true;
    } else {
        return false;
    }
}

inline bool
interpreter::Assembler::Tokenizer::acceptPunctuation(const char* p)
{
    if (isPunctuation(p)) {
        readNext();
        return true;
    } else {
        return false;
    }
}

int32_t
interpreter::Assembler::Tokenizer::acceptSign()
{
    return acceptPunctuation("+") ? +1
        : acceptPunctuation("-") ? -1
        : +1;
}

void
interpreter::Assembler::Tokenizer::requireEnd()
{
    if (m_currentToken != End) {
        throw Error::garbageAtEnd(false);
    }
}

void
interpreter::Assembler::Tokenizer::requirePunctuation(const char* punct)
{
    if (!isPunctuation(punct)) {
        throw Error::expectSymbol(punct);
    }
    readNext();
}

String_t
interpreter::Assembler::Tokenizer::requireName(const char* what)
{
    String_t result;
    if (m_currentToken == Identifier || m_currentToken == String) {
        result = m_currentText;
        readNext();
    } else if (m_currentToken == Punctuation && m_currentText == "-") {
        readNext();
    } else {
        throw Error::expectIdentifier(what);
    }
    return result;
}

int32_t
interpreter::Assembler::Tokenizer::requireInteger(const char* what)
{
    int factor = acceptSign();
    if (m_currentToken != Integer) {
        throw Error(Format("expecting number, %s", what));
    }
    int32_t result = m_currentInteger * factor;
    readNext();
    return result;
}

void
interpreter::Assembler::Tokenizer::skipWhitespace()
{
    while (m_pos < m_text.size() && isSpace(m_text[m_pos])) {
        ++m_pos;
    }
}

/*
 *  Element hierarchy
 */

class interpreter::Assembler::Element {
 public:
    Element(const String_t& name)
        : m_name(name),
          m_defined(false)
        { }
    virtual ~Element()
        { }
    virtual BaseValue* toValue() const = 0;
    virtual void saveTo(SaveContext& out) const = 0;

    void setDefined(bool flag)
        { m_defined = flag; }
    bool isDefined() const
        { return m_defined; }
    const String_t& getName() const
        { return m_name; }

 private:
    String_t m_name;
    bool m_defined;
};

class interpreter::Assembler::BytecodeElement : public Element {
 public:
    BytecodeElement(const String_t& name)
        : Element(name), m_bco(BytecodeObject::create(false))
        { }
    virtual BaseValue* toValue() const
        { return new SubroutineValue(m_bco); }
    virtual void saveTo(SaveContext& out) const
        { out.addBCO(m_bco); }
    BytecodeObject& get()
        { return *m_bco; }
 private:
    BCORef_t m_bco;
};

class interpreter::Assembler::StructureElement : public Element {
 public:
    StructureElement(const String_t& name)
        : Element(name), m_value(*new StructureTypeData())
        { }
    virtual BaseValue* toValue() const
        { return new StructureType(m_value); }
    virtual void saveTo(SaveContext& out) const
        { out.addStructureType(m_value); }
    StructureTypeData& get()
        { return *m_value; }
 private:
    StructureTypeData::Ref_t m_value;
};


/*
 *  Assembler
 */

interpreter::Assembler::Assembler(afl::io::TextReader& in)
    : m_input(in),
      m_instructions(),
      m_elements(),
      m_elementsByName(),
      m_lastCode(),
      m_symbolicJumps(false)
{
    initInstructions();
}

interpreter::Assembler::~Assembler()
{ }

void
interpreter::Assembler::compile()
{
    String_t line;
    while (m_input.readLine(line)) {
        Tokenizer tok(line);
        tok.readNext();

        if (tok.acceptIdentifier("DECLARE")) {
            // Declaration
            if (tok.acceptIdentifier("SUB") || tok.acceptIdentifier("FUNCTION")) {
                handleDeclaration(tok);
            } else if (tok.acceptIdentifier("STRUCT")) {
                handleStructureDeclaration(tok);
            } else {
                throw Error("Expected element to declare");
            }
        } else if (tok.acceptIdentifier("SUB")) {
            // Subroutine definition
            handleDefinition(tok, true);
        } else if (tok.acceptIdentifier("FUNCTION")) {
            // Function definition
            handleDefinition(tok, false);
        } else if (tok.acceptIdentifier("STRUCT")) {
            // Structure definition
            handleStructureDefinition(tok.requireName("structure name"));
        } else if (tok.acceptIdentifier(".JUMPS")) {
            // Configure jump mode for following subs
            // @change PCC2 would accept all possible abbreviations for abs/sym.
            if (tok.acceptIdentifier("ABSOLUTE") || tok.acceptIdentifier("ABS")) {
                m_symbolicJumps = false;
            } else if (tok.acceptIdentifier("SYMBOLIC") || tok.acceptIdentifier("SYM")) {
                m_symbolicJumps = true;
            } else {
                throw Error::expectKeyword("ABSOLUTE", "SYMBOLIC");
            }
        } else if (tok.getCurrentToken() == Tokenizer::End) {
            // ignore empty line
        } else {
            throw Error("invalid directive");
        }
        tok.requireEnd();
    }
}

void
interpreter::Assembler::finish(afl::sys::LogListener& log, afl::string::Translator& tx)
{
    bool hasErrors = false;
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (!m_elements[i]->isDefined()) {
            log.write(afl::sys::LogListener::Error, LOG_NAME, Format(tx("Error: \"%s\" declared but not defined"), m_elements[i]->getName()));
            hasErrors = true;
        }
    }
    if (hasErrors) {
        throw Error("Input has undefined elements");
    }
    verifyLastCode();
}

interpreter::BCORef_t
interpreter::Assembler::saveTo(SaveContext& out)
{
    for (size_t i = 0; i < m_elements.size(); ++i) {
        m_elements[i]->saveTo(out);
    }
    return verifyLastCode();
}

inline interpreter::BytecodeObject&
interpreter::Assembler::verifyLastCode()
{
    if (m_lastCode.get() == 0) {
        throw Error("Input does not contain any code");
    }
    return *m_lastCode;
}

inline void
interpreter::Assembler::handleDeclaration(Tokenizer& tok)
{
    while (1) {
        String_t name = tok.requireName("subroutine name");
        if (m_elementsByName.find(name) == m_elementsByName.end()) {
            m_elementsByName.insert(std::make_pair(name, m_elements.pushBackNew(new BytecodeElement(name))));
        }
        if (tok.getCurrentToken() == Tokenizer::End) {
            break;
        }
        tok.requirePunctuation(",");
    }
}

void
interpreter::Assembler::handleDefinition(Tokenizer& tok, bool isSub)
{
    String_t name = tok.requireName("subroutine name");

    // Find or create it
    std::map<String_t, Element*>::iterator it = m_elementsByName.find(name);
    BytecodeElement* elem = 0;
    if (it != m_elementsByName.end()) {
        elem = dynamic_cast<BytecodeElement*>(it->second);
    }
    if (elem == 0 || elem->isDefined()) {
        elem = new BytecodeElement(name);
        m_elements.pushBackNew(elem);
    }
    m_lastCode = &elem->get();
    elem->get().setIsProcedure(isSub);
    elem->get().setSubroutineName(name);
    elem->setDefined(true);

    // Handle args
    if (tok.acceptPunctuation("(")) {
        bool isOptional = false;
        while (!tok.acceptPunctuation(")")) {
            if (tok.acceptIdentifier("OPTIONAL")) {
                isOptional = true;
            }
            String_t name = tok.requireName("parameter name");
            if (tok.acceptPunctuation("(")) {
                // Must have two closing parens now; one for varargs, one to close parameter list
                tok.requirePunctuation(")");
                tok.requirePunctuation(")");
                elem->get().addLocalVariable(name);
                elem->get().setIsVarargs(true);
                break;
            } else {
                // Normal
                elem->get().addArgument(name, isOptional);
            }
            if (!tok.isPunctuation(")")) {
                tok.requirePunctuation(",");
            }
        }
    }

    // Assemble it
    assemble(elem->get());

    // Remember it
    m_elementsByName[name] = elem;
}

inline void
interpreter::Assembler::handleStructureDeclaration(Tokenizer& tok)
{
    while (1) {
        String_t name = tok.requireName("structure name");
        if (m_elementsByName.find(name) == m_elementsByName.end()) {
            m_elementsByName.insert(std::make_pair(name, m_elements.pushBackNew(new StructureElement(name))));
        }
        if (tok.getCurrentToken() == Tokenizer::End) {
            break;
        }
        tok.requirePunctuation(",");
    }
}

inline void
interpreter::Assembler::handleStructureDefinition(String_t name)
{
    // Find or create it
    std::map<String_t, Element*>::iterator it = m_elementsByName.find(name);
    StructureElement* elem = 0;
    if (it != m_elementsByName.end()) {
        elem = dynamic_cast<StructureElement*>(it->second);
    }
    if (elem == 0 || elem->isDefined()) {
        elem = new StructureElement(name);
        m_elements.pushBackNew(elem);
    }
    elem->setDefined(true);

    // Parse it
    bool running = true;
    while (running) {
        String_t line;
        if (!m_input.readLine(line)) {
            throw Error::expectKeyword("EndStruct");
        }

        Tokenizer tok(line);
        tok.readNext();
        if (tok.acceptIdentifier("ENDSTRUCT")) {
            running = false;
        } else if (tok.acceptIdentifier(".FIELD")) {
            while (1) {
                elem->get().names().add(tok.requireName("field name"));
                if (tok.getCurrentToken() == Tokenizer::End) {
                    break;
                }
                tok.requirePunctuation(",");
            }
        } else {
            throw Error::expectKeyword("EndStruct");
        }
        tok.requireEnd();
    }

    // Remember it
    m_elementsByName[name] = elem;
}

inline void
interpreter::Assembler::assemble(BytecodeObject& bco)
{
    // sub assemble
    std::map<String_t, size_t> labels;
    std::vector<std::pair<String_t, size_t> > relocations;

    bool running = true;
    while (running) {
        String_t line;
        if (!m_input.readLine(line)) {
            throw Error::expectKeyword(bco.isProcedure() ? "EndSub" : "EndFunction");
        }
        Tokenizer tok(line);
        tok.readNext();

        while (tok.getCurrentToken() != Tokenizer::End) {
            if (tok.getCurrentToken() != Tokenizer::Identifier) {
                throw Error("expected instruction or label");
            }

            const String_t name = tok.getCurrentText();
            tok.readNext();

            if (tok.acceptPunctuation(":")) {
                // Label
                if (labels.find(name) != labels.end()) {
                    throw Error(Format("duplicate label \"%s\"", name));
                }

                // Generate the label
                size_t address;
                if (m_symbolicJumps) {
                    uint16_t nr = bco.makeLabel();
                    bco.addLabel(nr);
                    address = nr;
                } else {
                    address = bco.getNumInstructions();
                }
                labels.insert(std::make_pair(name, address));
            } else {
                // Instruction
                // Check for end/specials
                if (name == "ENDSUB" || name == "ENDFUNCTION") {
                    running = false;
                } else if (name == ".LOCAL") {
                    bco.addLocalVariable(tok.requireName("local variable name"));
                } else if (name == ".LIT") {
                    parseLiteral(bco, tok);
                } else if (name == ".SYM") {
                    bco.addName(tok.requireName("symbol name"));
                } else if (name == ".NAME") {
                    bco.setSubroutineName(tok.requireName("subroutine name"));
                } else if (name == ".FILE") {
                    // @change Not in PCC2, but in disassembler
                    bco.setFileName(tok.requireName("file name"));
                } else if (name == ".LINE") {
                    // @change Not in PCC2, but in disassembler
                    uint32_t lineNr = tok.requireInteger("line");
                    uint32_t pc;
                    if (tok.acceptPunctuation(",")) {
                        pc = tok.requireInteger("address");
                    } else {
                        pc = static_cast<uint32_t>(bco.getNumInstructions());
                    }
                    bco.addLineNumber(lineNr, pc);
                } else if (name == ".MIN_ARGS") {
                    bco.setMinArgs(tok.requireInteger(".min_args"));
                } else if (name == ".MAX_ARGS") {
                    bco.setMaxArgs(tok.requireInteger(".max_args"));
                } else if (name == ".FLAGS") {
                    int32_t flag = tok.requireInteger(".flags");
                    bco.setIsVarargs((flag & interpreter::vmio::structures::BCOHeader::VarargsFlag) != 0);
                    bco.setIsProcedure((flag & interpreter::vmio::structures::BCOHeader::ProcedureFlag) != 0);
                } else if (name == ".VARARGS") {
                    // @change Not in PCC2, but in disassembler
                    bco.setIsVarargs(true);
                } else if (name == ".NUM_LABELS") {
                    bco.setNumLabels(packIndex(tok.requireInteger(".num_labels")));
                } else if (name == ".DEFSUBS") {
                    for (std::map<String_t, Element*>::iterator it = m_elementsByName.begin(); it != m_elementsByName.end(); ++it) {
                        // Generate 'pushlit, sdefsub' for each of these subs
                        if (BytecodeElement* ele = dynamic_cast<BytecodeElement*>(it->second)) {
                            SubroutineValue sub(ele->get());
                            bco.addPushLiteral(&sub);
                            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialDefSub, bco.addName(ele->get().getSubroutineName()));
                        }
                    }
                } else {
                    // Check instructions
                    uint32_t tpl = findInstruction(name);

                    // Optional arg? If user didn't give an argument, that's ok.
                    uint8_t argType = getArgType(tpl);
                    if ((tpl & OptionalArg) != 0 && tok.getCurrentToken() == Tokenizer::End) {
                        argType = 0;
                    }

                    // Parse arg
                    uint16_t arg;
                    if (tok.acceptPunctuation("#")) {
                        // Override: user wants integer arg no matter what the instruction wants
                        arg = static_cast<uint16_t>(tok.requireInteger("parameter"));
                    } else if (argType == 0) {
                        // No arg
                        if (tok.getCurrentToken() != Tokenizer::End) {
                            throw Error::tooManyArguments(name);
                        }
                        arg = 0;
                    } else if (argType == IntegerArg) {
                        // Integer
                        arg = static_cast<uint16_t>(tok.requireInteger("parameter"));
                    } else if (argType == SymbolArg) {
                        // Symbol
                        arg = bco.addName(tok.requireName("symbol"));
                    } else if (argType == LocalArg) {
                        // Local name
                        String_t varName = tok.requireName("local variable");
                        if (!bco.hasLocalVariable(varName)) {
                            throw Error::unknownIdentifier(varName);
                        }
                        arg = packIndex(bco.localVariables().getIndexByName(varName));
                    } else if (argType == LiteralArg) {
                        // Literal
                        arg = parseLiteral(bco, tok);
                    } else if (argType == LabelArg) {
                        // Label
                        String_t target = tok.requireName("label");
                        std::map<String_t, size_t>::const_iterator it = labels.find(target);
                        if (it != labels.end()) {
                            // Backward reference: use directly
                            arg = packIndex(it->second);
                        } else {
                            // Forward reference
                            relocations.push_back(std::make_pair(target, bco.getNumInstructions()));
                            arg = 0;
                        }
                    } else {
                        throw Error::internalError("invalid arg parsing information");
                    }

                    // Encode instruction
                    uint8_t major = getMajor(tpl);
                    uint8_t minor = getMinor(tpl);
                    if (argType == LabelArg && m_symbolicJumps) {
                        minor |= Opcode::jSymbolic;
                    }

                    bco.addInstruction(Opcode::Major(major), minor, arg);

                    // Verify end of line
                }
                tok.requireEnd();
            }
        }
    }

    for (size_t i = 0; i < relocations.size(); ++i) {
        std::map<String_t, size_t>::const_iterator it = labels.find(relocations[i].first);
        if (it == labels.end()) {
            throw Error(Format("label \"%s\" used but not defined", relocations[i].first));
        }
        bco(relocations[i].second).arg = packIndex(it->second);
    }
}

uint16_t
interpreter::Assembler::parseLiteral(BytecodeObject& bco, Tokenizer& tok)
{
    const bool forceNew = (tok.acceptPunctuation("!") || tok.acceptIdentifier("NEW"));

    std::auto_ptr<afl::data::Value> value;
    if (tok.acceptIdentifier("TRUE")) {
        // Boolean True
        value.reset(new BooleanValue(true));
    } else if (tok.acceptIdentifier("FALSE")) {
        // Boolean False
        value.reset(new BooleanValue(false));
    } else if (tok.acceptIdentifier("NULL")) {
        // Null
    } else if (tok.getCurrentToken() == Tokenizer::String) {
        // String
        value.reset(new StringValue(tok.getCurrentText()));
        tok.readNext();
    } else if (tok.getCurrentToken() == Tokenizer::Identifier) {
        // Identifier refering to an element
        std::map<String_t, Element*>::const_iterator it = m_elementsByName.find(tok.getCurrentText());
        if (it == m_elementsByName.end()) {
            throw Error::unknownIdentifier(tok.getCurrentText());
        }
        tok.readNext();
        value.reset(it->second->toValue());
    } else if (tok.acceptPunctuation("(")) {
        // (tag,value) for specific serialisation
        int32_t t = tok.requireInteger("tag");
        tok.requirePunctuation(",");
        int32_t v = tok.requireInteger("value");
        tok.requirePunctuation(")");

        class DummyValue : public BaseValue {
         public:
            DummyValue(int32_t tag, int32_t value)
                : m_tag(tag), m_value(value)
                { }
            virtual String_t toString(bool /*readable*/) const
                { return Format("(%d,%d)", m_tag, m_value); }
            virtual void store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
                { out.tag = static_cast<uint16_t>(256*m_tag); out.value = m_value; }
            virtual BaseValue* clone() const
                { return new DummyValue(m_tag, m_value); }
            virtual void visit(afl::data::Visitor& /*visit*/)
                { }
         private:
            int32_t m_tag;
            int32_t m_value;
        };
        value.reset(new DummyValue(t, v));
    } else {
        // Number (int or float)
        int32_t factor = tok.acceptSign();
        if (tok.getCurrentToken() == Tokenizer::Integer) {
            value.reset(new IntegerValue(factor * tok.getCurrentInteger()));
        } else if (tok.getCurrentToken() == Tokenizer::Float) {
            value.reset(new FloatValue(factor * tok.getCurrentFloat()));
        } else {
            throw Error("invalid literal");
        }
        tok.readNext();
    }

    // Recycle literal if possible
    if (!forceNew) {
        return bco.addLiteral(value.get());
    } else {
        bco.literals().pushBackNew(value.release());
        return packIndex(bco.literals().size()-1);
    }
}

void
interpreter::Assembler::initInstructions()
{
    // sub initInstructionTable
    m_instructions["nop"]   = makeTemplate(Opcode::maJump, 0, 0);
    m_instructions["label"] = makeTemplate(Opcode::maJump, 0, LabelArg);

    // Jump
    m_instructions["jt"]      = makeTemplate(Opcode::maJump, Opcode::jIfTrue,                                                            LabelArg);
    m_instructions["jf"]      = makeTemplate(Opcode::maJump,                   Opcode::jIfFalse,                                         LabelArg);
    m_instructions["jtf"]     = makeTemplate(Opcode::maJump, Opcode::jIfTrue | Opcode::jIfFalse,                                         LabelArg);
    m_instructions["je"]      = makeTemplate(Opcode::maJump,                                      Opcode::jIfEmpty,                      LabelArg);
    m_instructions["jte"]     = makeTemplate(Opcode::maJump, Opcode::jIfTrue |                    Opcode::jIfEmpty,                      LabelArg);
    m_instructions["jfe"]     = makeTemplate(Opcode::maJump,                   Opcode::jIfFalse | Opcode::jIfEmpty,                      LabelArg);
    m_instructions["j"]       = makeTemplate(Opcode::maJump, Opcode::jAlways,                                                            LabelArg);
    m_instructions["jneverp"] = makeTemplate(Opcode::maJump,                                                         Opcode::jPopAlways, LabelArg);
    m_instructions["jtp"]     = makeTemplate(Opcode::maJump, Opcode::jIfTrue |                                       Opcode::jPopAlways, LabelArg);
    m_instructions["jfp"]     = makeTemplate(Opcode::maJump,                   Opcode::jIfFalse |                    Opcode::jPopAlways, LabelArg);
    m_instructions["jtfp"]    = makeTemplate(Opcode::maJump, Opcode::jIfTrue | Opcode::jIfFalse |                    Opcode::jPopAlways, LabelArg);
    m_instructions["jep"]     = makeTemplate(Opcode::maJump,                                      Opcode::jIfEmpty | Opcode::jPopAlways, LabelArg);
    m_instructions["jtep"]    = makeTemplate(Opcode::maJump, Opcode::jIfTrue |                    Opcode::jIfEmpty | Opcode::jPopAlways, LabelArg);
    m_instructions["jfep"]    = makeTemplate(Opcode::maJump,                   Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, LabelArg);
    m_instructions["jp"]      = makeTemplate(Opcode::maJump, Opcode::jAlways |                                       Opcode::jPopAlways, LabelArg);
    m_instructions["catch"]   = makeTemplate(Opcode::maJump, Opcode::jCatch,                                                             LabelArg);
    m_instructions["jdz"]     = makeTemplate(Opcode::maJump, Opcode::jDecZero,                                                           LabelArg);

    // Stack
    m_instructions["dup"]     = makeTemplate(Opcode::maStack, Opcode::miStackDup,  IntegerArg);
    m_instructions["drop"]    = makeTemplate(Opcode::maStack, Opcode::miStackDrop, IntegerArg);
    m_instructions["swap"]    = makeTemplate(Opcode::maStack, Opcode::miStackSwap, IntegerArg);

    // Special
    m_instructions["suncatch"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    m_instructions["sreturn"]      = makeTemplate(Opcode::maSpecial, Opcode::miSpecialReturn, IntegerArg);
    m_instructions["swith"]        = makeTemplate(Opcode::maSpecial, Opcode::miSpecialWith, 0);
    m_instructions["sendwith"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
    m_instructions["sfirstindex"]  = makeTemplate(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    m_instructions["snextindex"]   = makeTemplate(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
    m_instructions["sendindex"]    = makeTemplate(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
    m_instructions["sevals"]       = makeTemplate(Opcode::maSpecial, Opcode::miSpecialEvalStatement, IntegerArg);
    m_instructions["sevalx"]       = makeTemplate(Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    m_instructions["sdefsub"]      = makeTemplate(Opcode::maSpecial, Opcode::miSpecialDefSub, SymbolArg);
    m_instructions["sdefshipp"]    = makeTemplate(Opcode::maSpecial, Opcode::miSpecialDefShipProperty, SymbolArg);
    m_instructions["sdefplanetp"]  = makeTemplate(Opcode::maSpecial, Opcode::miSpecialDefPlanetProperty, SymbolArg);
    m_instructions["sload"]        = makeTemplate(Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    m_instructions["sprint"]       = makeTemplate(Opcode::maSpecial, Opcode::miSpecialPrint, 0);
    m_instructions["saddhook"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialAddHook, IntegerArg | OptionalArg); // optional arg; might be used later, but isn't used now
    m_instructions["srunhook"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialRunHook, IntegerArg | OptionalArg); // optional arg; might be used later, but isn't used now
    m_instructions["sthrow"]       = makeTemplate(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    m_instructions["sterminate"]   = makeTemplate(Opcode::maSpecial, Opcode::miSpecialTerminate, 0);
    m_instructions["ssuspend"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    m_instructions["snewarray"]    = makeTemplate(Opcode::maSpecial, Opcode::miSpecialNewArray, IntegerArg);
    m_instructions["smakelist"]    = makeTemplate(Opcode::maSpecial, Opcode::miSpecialMakeList, IntegerArg);
    m_instructions["snewhash"]     = makeTemplate(Opcode::maSpecial, Opcode::miSpecialNewHash, IntegerArg | OptionalArg);
    m_instructions["sinstance"]    = makeTemplate(Opcode::maSpecial, Opcode::miSpecialInstance, 0);
    m_instructions["sresizearray"] = makeTemplate(Opcode::maSpecial, Opcode::miSpecialResizeArray, IntegerArg);
    m_instructions["sbind"]        = makeTemplate(Opcode::maSpecial, Opcode::miSpecialBind, IntegerArg);
    m_instructions["sfirst"]       = makeTemplate(Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    m_instructions["snext"]        = makeTemplate(Opcode::maSpecial, Opcode::miSpecialNext, 0);

    // Unary/Binary/Ternary
    // @change Unlike PCC2 version, we do not support the fused instructions on this interface
    addEnumeratedInstructions(m_instructions, Opcode::maUnary,   "u", getUnaryName);
    addEnumeratedInstructions(m_instructions, Opcode::maBinary,  "b", getBinaryName);
    addEnumeratedInstructions(m_instructions, Opcode::maTernary, "t", getTernaryName);

    // Scope
    addScopeInstruction(m_instructions, Opcode::maPush,  "push");
    addScopeInstruction(m_instructions, Opcode::maPop,   "pop");
    addScopeInstruction(m_instructions, Opcode::maStore, "store");

    m_instructions["dimloc"]  = makeTemplate(Opcode::maDim, Opcode::sLocal,  SymbolArg);
    m_instructions["dimtop"]  = makeTemplate(Opcode::maDim, Opcode::sStatic, SymbolArg);
    m_instructions["dimglob"] = makeTemplate(Opcode::maDim, Opcode::sShared, SymbolArg);

    // Indirect operation
    addIndirectInstruction(m_instructions, Opcode::maIndirect, "ind", IntegerArg);
    addIndirectInstruction(m_instructions, Opcode::maMemref,   "mem", SymbolArg);
}

inline uint32_t
interpreter::Assembler::findInstruction(const String_t& name) const
{
    // Check regular
    std::map<String_t, uint32_t>::const_iterator it = m_instructions.find(afl::string::strLCase(name));
    if (it != m_instructions.end()) {
        return it->second;
    }

    // Might be generic
    util::StringParser p(name);
    uint8_t type;
    if (p.parseString("GENINT")) {
        type = IntegerArg;
    } else if (p.parseString("GENSYM")) {
        type = SymbolArg;
    } else if (p.parseString("GENLOC")) {
        type = LocalArg;
    } else if (p.parseString("GENLIT")) {
        type = LiteralArg;
    } else if (p.parseString("GENLABEL")) {
        type = LabelArg;
    } else if (p.parseString("GEN")) {
        type = IntegerArg | OptionalArg;
    } else {
        throw Error(Format("invalid instruction '%s'", name));
    }

    int minor, major;
    if (p.parseInt(major) && p.parseString(".") && p.parseInt(minor) && p.parseEnd()) {
        return makeTemplate(static_cast<Opcode::Major>(major), static_cast<uint8_t>(minor), type);
    } else {
        throw Error(Format("invalid instruction '%s'", name));
    }
}
