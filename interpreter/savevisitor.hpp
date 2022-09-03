/**
  *  \file interpreter/savevisitor.hpp
  *  \brief Class interpreter::SaveVisitor
  */
#ifndef C2NG_INTERPRETER_SAVEVISITOR_HPP
#define C2NG_INTERPRETER_SAVEVISITOR_HPP

#include "afl/charset/charset.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/visitor.hpp"
#include "afl/io/datasink.hpp"

namespace interpreter {

    class SaveContext;
    class Context;
    class TagNode;

    /** Visitor to save a value.
        Values are serialized to an 48-bit tag node (see TagNode) plus an optional block of auxiliary information.
        A data segment consists of a sequence of tag nodes, followed by the concatenated auxiliary information.

        This class implements a visitor to save values and segments in this format.

        SaveVisitor().visit() saves one object.
        Structured data (e.g. hashes) may contain multiple objects to resolve possibly-shared links between this data.
        To resolve that, we need a SaveContext. */
    class SaveVisitor : public afl::data::Visitor {
     public:
        /** Constructor.
            Make a visitor to produce TagNode/aux data.
            \param out [out] TagNode goes here
            \param aux [out] Auxiliary data appended here
            \param cs  [in] Character set
            \param ctx [in] Save context to save structured data */
        SaveVisitor(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx);

        // Visitor methods:
        virtual void visitString(const String_t& str);
        virtual void visitInteger(int32_t iv);
        virtual void visitFloat(double fv);
        virtual void visitBoolean(bool bv);
        virtual void visitHash(const afl::data::Hash& hv);
        virtual void visitVector(const afl::data::Vector& vv);
        virtual void visitOther(const afl::data::Value& other);
        virtual void visitNull();
        virtual void visitError(const String_t& source, const String_t& str);

        /** Save segment.
            \param out [out] Data goes here
            \param data [in] Data segment to save
            \param slots [in] Number of slots to save (can be more or less than the number of elements in the segment)
            \param cs [in] Character set
            \param ctx [in,optional] Save context to save structured data */
        static void save(afl::io::DataSink& out,
                         const afl::data::Segment& data, size_t slots,
                         afl::charset::Charset& cs, SaveContext& ctx);

        /** Save contexts.
            \param out [out] Data goes here
            \param contexts [in] Contexts to save
            \param ctx [in] Save context to save structured data

            This is a stripped-down version of save().
            It assumes that contexts are never null. */
        static void saveContexts(afl::io::DataSink& out,
                                 const afl::container::PtrVector<interpreter::Context>& contexts,
                                 SaveContext& ctx);

        /** Save name list.
            \param out Data goes here
            \param names Names to save
            \param slots Number of names to save. Must be <= getNumNames().
            \param cs Character set */
        static void saveNames(afl::io::DataSink& out, const afl::data::NameMap& names, size_t slots, afl::charset::Charset& cs);

     private:
        TagNode& m_out;
        afl::io::DataSink& m_aux;
        afl::charset::Charset& m_charset;
        SaveContext& m_context;
    };

}

#endif
