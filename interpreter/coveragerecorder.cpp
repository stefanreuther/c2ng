/**
  *  \file interpreter/coveragerecorder.cpp
  *  \brief Class interpreter::CoverageRecorder
  *
  *  File format:
  *  - per file:
  *        TN:<test name>
  *        SF:<path>
  *  - per function:
  *        FN:<line nr>,<function name>
  *        FNDA:<count>,<function name>
  *  - per file:
  *        FNF:<# function>
  *        FNH:<# functions hit>
  *        DA:<line>,<count>
  *        LH:<# lines hit>
  *        LF:<# lines found>
  *        end_of_record
  */

#include <vector>
#include "interpreter/coveragerecorder.hpp"

#include "afl/string/format.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/process.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "util/math.hpp"

using afl::string::Format;

namespace {
    const char*const ANON_NAME = "anon";

    /* Generate a unique, non-empty name.
       We do generate elements with empty names (e.g. main entry point); geninfo would ignore those. */
    String_t generateUniqueName(String_t name, std::set<String_t>& usedNames)
    {
        if (name.empty()) {
            name = ANON_NAME;
        }
        String_t result = name;
        int counter = 0;
        while (!usedNames.insert(result).second) {
            ++counter;
            result = Format("%s_%d", name, counter);
        }
        return result;
    }
}


/*
 *  Representation of a function (BytecodeObject).
 */

struct interpreter::CoverageRecorder::Function {
    /** Link to object to prevent it from being deleted while we're running. */
    afl::base::Ref<const BytecodeObject> code;

    /** Flag whether this function was executed. */
    bool hit;

    Function(const BytecodeObject& bco)
        : code(bco),
          hit(false)
        { }
};


/*
 *  Representation of a source file
 */

struct interpreter::CoverageRecorder::File {
    /** Set of all functions.
        Indexed by BytecodeObject address. */
    FunctionMap_t functions;

    /** Coverage status for all source code lines. */
    LineMap_t lines;
};


/*
 *  Comparator for Function pointers.
 *
 *  For determinism, we sort functions by name for output.
 *  If names are not unique, we sort by line number.
 */

struct interpreter::CoverageRecorder::CompareFunctions {
    bool operator()(const Function* a, const Function* b) const
        {
            int nameCompare = util::compare3(a->code->getSubroutineName(), b->code->getSubroutineName());
            if (nameCompare != 0) {
                return nameCompare < 0;
            }

            return a->code->getLineNumber(0) < b->code->getLineNumber(0);
        }
};


/*
 *  CoverageRecorder
 */

interpreter::CoverageRecorder::CoverageRecorder()
{ }

interpreter::CoverageRecorder::~CoverageRecorder()
{ }

void
interpreter::CoverageRecorder::checkProcess(Process& p)
{
    addProcessState(p);
}

void
interpreter::CoverageRecorder::addBCO(const BytecodeObject& bco)
{
    SeenSet_t seen;
    addFunction(bco, seen);
}

void
interpreter::CoverageRecorder::addProcessState(const Process& proc)
{
    if (size_t n = proc.getNumActiveFrames()) {
        if (const Process::Frame* f = proc.getFrame(n-1)) {
            SeenSet_t seen;
            if (Function* fcn = addFunction(*f->bco, seen)) {
                fcn->hit = true;
            }

            if (File* file = addFile(*f->bco)) {
                file->lines[f->bco->getLineNumber(f->pc)] = true;
            }
        }
    }
}

void
interpreter::CoverageRecorder::save(afl::io::Stream& out, String_t testName)
{
    afl::io::TextFile tf(out);
    tf.setSystemNewline(false);
    for (FileMap_t::const_iterator it = m_files.begin(); it != m_files.end(); ++it) {
        tf.writeLine(Format("TN:%s", testName));
        tf.writeLine(Format("SF:%s", it->first));
        saveFile(tf, *it->second);
        tf.writeLine("end_of_record");
    }
    tf.flush();
}

void
interpreter::CoverageRecorder::saveFile(afl::io::TextFile& tf, const File& file)
{
    // Sort functions for determinism
    std::vector<const Function*> funcs;
    for (FunctionMap_t::const_iterator it = file.functions.begin(); it != file.functions.end(); ++it) {
        funcs.push_back(it->second);
    }
    std::sort(funcs.begin(), funcs.end(), CompareFunctions());

    // Output functions
    size_t functionsHit = 0;
    size_t functionsFound = 0;
    std::set<String_t> usedNames;
    usedNames.insert(ANON_NAME);   // block suffix-less version; we want 'anon_1' to be the first
    for (size_t i = 0; i < funcs.size(); ++i) {
        const BytecodeObject& bco = *funcs[i]->code;
        uint32_t lineNr = bco.getLineNumber(0);
        if (lineNr != 0) {
            // Output
            String_t name = generateUniqueName(bco.getSubroutineName(), usedNames);
            tf.writeLine(Format("FN:%d,%s", lineNr, name));
            tf.writeLine(Format("FNDA:%d,%s", int(funcs[i]->hit), name));

            // Count
            ++functionsFound;
            if (funcs[i]->hit) {
                ++functionsHit;
            }
        }
    }
    tf.writeLine(Format("FNF:%d", functionsFound));
    tf.writeLine(Format("FNH:%d", functionsHit));

    // Output lines
    size_t linesHit = 0;
    size_t linesFound = 0;
    for (LineMap_t::const_iterator it = file.lines.begin(); it != file.lines.end(); ++it) {
        // Output
        tf.writeLine(Format("DA:%d,%d", it->first, int(it->second)));

        // Count
        ++linesFound;
        if (it->second) {
            ++linesHit;
        }
    }
}

interpreter::CoverageRecorder::Function*
interpreter::CoverageRecorder::addFunction(const BytecodeObject& bco, SeenSet_t& seen)
{
    // Add, if it has source code
    Function* func = 0;
    bool recurse = true;
    if (File* file = addFile(bco)) {
        // Create function
        func = file->functions[&bco];
        if (func == 0) {
            func = file->functions.insertNew(&bco, new Function(bco));

            // Create all lines
            for (size_t i = 1; i < bco.lineNumbers().size(); i += 2) {
                file->lines.insert(std::make_pair(bco.lineNumbers()[i], false));
            }
        } else {
            recurse = false;
        }
    }

    // Recurse into children
    if (recurse) {
        if (seen.insert(&bco).second) {
            for (size_t i = 0; i < bco.literals().size(); ++i) {
                if (SubroutineValue* sv = dynamic_cast<SubroutineValue*>(bco.literals()[i])) {
                    addFunction(*sv->getBytecodeObject(), seen);
                }
            }
        }
    }
    return func;
}

interpreter::CoverageRecorder::File*
interpreter::CoverageRecorder::addFile(const BytecodeObject& bco)
{
    String_t fn = bco.getFileName();
    if (fn.empty()) {
        return 0;
    }

    File* f = m_files[fn];
    if (f == 0) {
        f = m_files.insertNew(fn, new File());
    }
    return f;
}
