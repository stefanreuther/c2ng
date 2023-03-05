/**
  *  \file interpreter/vmio/assemblersavecontext.cpp
  *  \brief Class interpreter::vmio::AssemblerSaveContext
  */

#include "interpreter/vmio/assemblersavecontext.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/string/format.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savevisitor.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

namespace {
    String_t quoteName(const String_t& s)
    {
        if (s.empty()) {
            return "-";
        } else {
            return s;
        }
    }

    void addTab(String_t& result)
    {
        if (result.size() < 20) {
            result.append(20 - result.size(), ' ');
        } else if (result.size() < 40) {
            result.append(40 - result.size(), ' ');
        } else {
            result.append(3, ' ');
        }
    }
}


interpreter::vmio::AssemblerSaveContext::AssemblerSaveContext()
    : m_metadata(),
      m_sequence(),
      m_usedNames(),
      m_counter(0),
      m_debugInformationEnabled(true)
{ }

interpreter::vmio::AssemblerSaveContext::~AssemblerSaveContext()
{ }

uint32_t
interpreter::vmio::AssemblerSaveContext::addBCO(const interpreter::BytecodeObject& bco)
{
    struct BytecodeMetaObject : public MetaObject {
        BytecodeMetaObject(const interpreter::BytecodeObject& bco)
            : m_bco(bco)
            { }
        virtual void writeDeclaration(AssemblerSaveContext& /*asc*/, afl::io::TextWriter& out)
            {
                out.writeLine(afl::string::Format("Declare Sub %s", name));
            }
        virtual void writeBody(AssemblerSaveContext& asc, afl::io::TextWriter& out)
            {
                // Fetch references
                const afl::data::NameMap& locals = m_bco.localVariables();

                // Prototype
                String_t keyword = (m_bco.isProcedure() ? "Sub" : "Function");
                size_t declareFrom;
                bool declareArgs;
                if (m_bco.getMinArgs() == 0 && m_bco.getMaxArgs() == 0 && !m_bco.isVarargs()) {
                    // Nullary function
                    out.writeLine(afl::string::Format("%s %s", keyword, name));
                    declareFrom = 0;
                    declareArgs = false;
                } else if (m_bco.getMaxArgs() < m_bco.getMinArgs() || locals.getNumNames() < m_bco.getMaxArgs()) {
                    // Invalid
                    out.writeLine(afl::string::Format("%s %s", keyword, name));
                    declareFrom = 0;
                    declareArgs = true;
                } else {
                    // Regular parameterized function
                    out.writeText(afl::string::Format("%s %s (", keyword, name));
                    for (size_t i = 0, n = m_bco.getMaxArgs(); i < n; ++i) {
                        // Separator
                        if (i != 0) {
                            out.writeText(", ");
                        }

                        // Optional?
                        if (i == m_bco.getMinArgs()) {
                            out.writeText("Optional ");
                        }

                        // Name
                        out.writeText(locals.getNameByIndex(i));
                    }
                    declareFrom = m_bco.getMaxArgs();
                    declareArgs = false;

                    // FIXME: deal with varargs

                    out.writeLine(")");

                }
                if (name != m_bco.getSubroutineName()) {
                    out.writeLine(afl::string::Format("  .name %s", quoteName(m_bco.getSubroutineName())));
                }

                // Locals
                for (size_t i = declareFrom, n = locals.getNumNames(); i < n; ++i) {
                    out.writeLine(afl::string::Format("  .local %s", quoteName(locals.getNameByIndex(i))));
                }
                if (m_bco.isVarargs()) {
                    out.writeLine("  .varargs");
                }

                // Argument limits
                if (declareArgs) {
                    out.writeLine(afl::string::Format("  .min_args %d", m_bco.getMinArgs()));
                    out.writeLine(afl::string::Format("  .max_args %d", m_bco.getMaxArgs()));
                }

                // Debug information: File name
                if (asc.isDebugInformationEnabled() && !m_bco.getFileName().empty()) {
                    out.writeLine(afl::string::Format("  .file %s", m_bco.getFileName()));
                }

                // Debug information: Line numbers
                // Write out line numbers as we're going through the code.
                // If the debug information is well-formed, this will Just Work[tm].
                // If the debug information is not well-formed (i.e. non-continguous), write out the excess at the end.
                // If debug information is disabled, just pretend that we already processed everything.
                const std::vector<uint32_t>& lineNumbers = m_bco.lineNumbers();
                size_t lineLimit = lineNumbers.size() & ~1;
                size_t lineIndex = (asc.isDebugInformationEnabled() ? 0 : lineLimit);

                // Find labels
                // Labels are for the benefit of the user only and not needed for re-assembling,
                // thus it's not an error if we see an out-of-bounds label (and it's not the end of the world if we miss one).
                std::set<size_t> labels;
                for (size_t i = 0, n = m_bco.getNumInstructions(); i < n; ++i) {
                    const Opcode& insn = m_bco(i);
                    if (insn.isJumpOrCatch() && (insn.minor & Opcode::jSymbolic) == 0 && insn.arg <= n) {
                        labels.insert(insn.arg);
                    }
                }

                // Assembler code
                std::set<size_t>::const_iterator labelIt = labels.begin();
                for (size_t i = 0, n = m_bco.getNumInstructions(); i < n; ++i) {
                    // Label
                    if (labelIt != labels.end() && *labelIt == i) {
                        out.writeLine(afl::string::Format("  label%d:", i));
                        ++labelIt;
                    }
                    // Line number
                    while (lineIndex < lineLimit && lineNumbers[lineIndex] == i) {
                        out.writeLine(afl::string::Format("    .line %d", lineNumbers[lineIndex+1]));
                        lineIndex += 2;
                    }
                    // Instruction
                    Opcode opc = m_bco(i);
                    opc.major = opc.getExternalMajor();
                    out.writeLine(asc.formatInstruction(opc, m_bco));
                }

                // Potential label at end of subroutine
                if (labelIt != labels.end()) {
                    out.writeLine(afl::string::Format("  label%d:", *labelIt));
                }

                // Potential non-wellformed line number records
                while (lineIndex < lineLimit) {
                    out.writeLine(afl::string::Format("    .line %d, %d", lineNumbers[lineIndex+1], lineNumbers[lineIndex]));
                    lineIndex += 2;
                }

                out.writeLine(afl::string::Format("End%s", keyword));
                out.writeLine();
            }
        const interpreter::BytecodeObject& m_bco;
    };


    Map_t::iterator it = m_metadata.find(&bco);
    if (it == m_metadata.end()) {
        MetaObject* p = new BytecodeMetaObject(bco);
        m_metadata.insertNew(&bco, p);

        // Save preconditions
        {
            afl::io::NullStream null;
            afl::charset::Utf8Charset cs;
            SaveVisitor::save(null, bco.literals(), bco.literals().size(), cs, *this);
        }

        // Sequence it
        m_sequence.push_back(p);
        p->isSequenced = true;

        // Assign a name
        if (Tokenizer::isValidUppercaseIdentifier(bco.getSubroutineName()) && m_usedNames.find(bco.getSubroutineName()) == m_usedNames.end()) {
            p->name = bco.getSubroutineName();
        } else {
            do {
                p->name = afl::string::Format("BCO%d", ++m_counter);
            } while (m_usedNames.find(p->name) != m_usedNames.end());
        }
        m_usedNames.insert(p->name);
    } else {
        if (!it->second->isSequenced) {
            it->second->needDeclaration = true;
        }
    }
    return 0;
}

uint32_t
interpreter::vmio::AssemblerSaveContext::addHash(const afl::data::Hash& hash)
{
    (void) hash;
    return 0;
}

uint32_t
interpreter::vmio::AssemblerSaveContext::addArray(const interpreter::ArrayData& array)
{
    (void) array;
    return 0;
}

uint32_t
interpreter::vmio::AssemblerSaveContext::addStructureType(const interpreter::StructureTypeData& type)
{
    struct StructureTypeMetaObject : public MetaObject {
        StructureTypeMetaObject(const interpreter::StructureTypeData& type)
            : m_type(type)
            { }
        virtual void writeDeclaration(AssemblerSaveContext& /*asc*/, afl::io::TextWriter& out)
            {
                out.writeLine(afl::string::Format("Declare Struct %s", name));
            }
        virtual void writeBody(AssemblerSaveContext& /*asc*/, afl::io::TextWriter& out)
            {
                // Header
                out.writeLine(afl::string::Format("Struct %s", name));

                // Content
                const afl::data::NameMap& names = m_type.names();
                for (afl::data::NameMap::Index_t i = 0, n = names.getNumNames(); i < n; ++i) {
                    out.writeLine(afl::string::Format("    .field %s", names.getNameByIndex(i)));
                }

                // End
                out.writeLine("EndStruct");
                out.writeLine();
            }
        const interpreter::StructureTypeData& m_type;
    };

    Map_t::iterator it = m_metadata.find(&type);
    if (it == m_metadata.end()) {
        MetaObject* p = new StructureTypeMetaObject(type);
        m_metadata.insertNew(&type, p);

        // Sequence it
        m_sequence.push_back(p);
        p->isSequenced = true;

        // Assign a name
        do {
            p->name = afl::string::Format("TYPE%d", ++m_counter);
        } while (m_usedNames.find(p->name) != m_usedNames.end());
        m_usedNames.insert(p->name);
    } else {
        if (!it->second->isSequenced) {
            it->second->needDeclaration = true;
        }
    }
    return 0;
}

uint32_t
interpreter::vmio::AssemblerSaveContext::addStructureValue(const interpreter::StructureValueData& value)
{
    (void) value;
    return 0;
}

bool
interpreter::vmio::AssemblerSaveContext::isCurrentProcess(const interpreter::Process* /*p*/)
{
    return false;
}

void
interpreter::vmio::AssemblerSaveContext::save(afl::io::TextWriter& out)
{
    for (size_t i = 0, n = m_sequence.size(); i < n; ++i) {
        if (m_sequence[i]->needDeclaration) {
            m_sequence[i]->writeDeclaration(*this, out);
        }
    }
    for (size_t i = 0, n = m_sequence.size(); i < n; ++i) {
        m_sequence[i]->writeBody(*this, out);
    }
}

void
interpreter::vmio::AssemblerSaveContext::setDebugInformation(bool flag)
{
    m_debugInformationEnabled = flag;
}

bool
interpreter::vmio::AssemblerSaveContext::isDebugInformationEnabled() const
{
    return m_debugInformationEnabled;
}

String_t
interpreter::vmio::AssemblerSaveContext::formatLiteral(const afl::data::Value* value)
{
    class Visitor : public afl::data::Visitor {
     public:
        Visitor(AssemblerSaveContext& parent)
            : m_parent(parent),
              m_result()
            { }
        virtual void visitString(const String_t& str)
            {
                m_result = interpreter::quoteString(str);
            }
        virtual void visitInteger(int32_t iv)
            {
                m_result = afl::string::Format("%d", iv);
            }
        virtual void visitFloat(double fv)
            {
                m_result = interpreter::formatFloat(fv);
            }
        virtual void visitBoolean(bool bv)
            {
                m_result = bv ? "true" : "false";
            }
        virtual void visitHash(const afl::data::Hash& /*hv*/)
            { m_result = "FIXME-hash"; }
        virtual void visitVector(const afl::data::Vector& /*vv*/)
            { m_result = "FIXME-vector"; }
        virtual void visitOther(const afl::data::Value& other)
            {
                if (const SubroutineValue* sv = dynamic_cast<const SubroutineValue*>(&other)) {
                    // Subroutine value
                    m_result = m_parent.formatSubroutineReference(*sv->getBytecodeObject());
                } else if (const StructureType* st = dynamic_cast<const StructureType*>(&other)) {
                    // Structure
                    m_result = m_parent.formatStructureTypeReference(*st->getType());
                } else if (const BaseValue* bv = dynamic_cast<const BaseValue*>(&other)) {
                    // Other
                    try {
                        TagNode tag;
                        afl::io::NullStream aux;
                        NullSaveContext ctx;
                        bv->store(tag, aux, ctx);
                        if (aux.getSize() != 0) {
                            // FIXME: log: tag with aux value
                        }

                        // First value is uppermost bits of tag; if those bits were nonzero,
                        // this would be a float value, and we'd entered the visitFloat() case.
                        m_result = afl::string::Format("(%d,%d)", tag.tag >> 8, tag.value);
                    }
                    catch (interpreter::Error&) {
                        // FIXME: log
                        m_result = "#<unknown>";
                    }
                } else {
                    m_result = "#<unknown>";
                }
            }
        virtual void visitNull()
            {
                m_result = "null";
            }
        virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
            {
                m_result = "#<error>";
            }
        const String_t& get() const
            { return m_result; }
     private:
        AssemblerSaveContext& m_parent;
        String_t m_result;
    };
    Visitor v(*this);
    v.visit(value);
    return v.get();
}

String_t
interpreter::vmio::AssemblerSaveContext::formatInstruction(const Opcode& opc, const BytecodeObject& bco)
{
    String_t tpl = opc.getDisassemblyTemplate();
    if (tpl.find('?') != String_t::npos) {
        if (opc.arg != 0) {
            return afl::string::Format("    genint%d.%d %d", opc.major, opc.minor, opc.arg);
        } else {
            return afl::string::Format("    gen%d.%d", opc.major, opc.minor, opc.arg);
        }
    } else {
        String_t result = "    ";
        for (String_t::size_type i = 0; i < tpl.size(); ++i) {
            if (tpl[i] == '\t') {
                addTab(result);
            } else if (tpl[i] == '%' && i+1 < tpl.size()) {
                char mode = tpl[++i];
                uint16_t arg = opc.arg;

                switch (mode) {
                 case 'n':
                    // Name
                    if (arg < bco.names().getNumNames()) {
                        result += bco.names().getNameByIndex(arg);
                        addTab(result);
                        result += "% name ";
                    }
                    result += afl::string::Format("#%d", arg);
                    break;

                 case 'l':
                    // Literal
                    result += formatLiteral(bco.getLiteral(arg));
                    break;

                 case 'L':
                    // Local
                    if (arg < bco.localVariables().getNumNames()) {
                        result += bco.localVariables().getNameByIndex(arg);
                        addTab(result);
                        result += "% local ";
                    }
                    result += afl::string::Format("#%d", arg);
                    break;

                 case 'd':
                    // Decimal integer
                    result += afl::string::Format("%d", int16_t(arg));
                    break;

                 case 'u':
                    // Unsigned integer
                    result += afl::string::Format("%d", arg);
                    break;

                 case 'T':  // Static by address
                 case 'G':  // Shared by address
                 default:
                    result += afl::string::Format("#%d", arg);
                    break;
                }
            } else {
                result += tpl[i];
            }
        }
        return result;
    }
}

String_t
interpreter::vmio::AssemblerSaveContext::formatSubroutineReference(BytecodeObject& bco)
{
    if (MetaObject* p = find(&bco)) {
        return p->name;
    } else {
        // FIXME: log
        return "#<error>";
    }
}

String_t
interpreter::vmio::AssemblerSaveContext::formatStructureTypeReference(StructureTypeData& type)
{
    if (MetaObject* p = find(&type)) {
        return p->name;
    } else {
        // FIXME: log
        return "#<error>";
    }
}

interpreter::vmio::AssemblerSaveContext::MetaObject*
interpreter::vmio::AssemblerSaveContext::find(void* p) const
{
    return m_metadata[p];
}
