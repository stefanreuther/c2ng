/**
  *  \file interpreter/exporter/jsonexporter.cpp
  *  \brief Class interpreter::exporter::JsonExporter
  *
  *  FIXME: this uses PCC2's ad-hoc JSON formatter. Use the JSON infrastructure from afl.
  */

#include "interpreter/exporter/jsonexporter.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/string/format.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/scalarvalue.hpp"

namespace {
    void writeValue(afl::io::TextWriter& tf, afl::data::Value* value, int depth);

    void writeQuotedString(afl::io::TextWriter& tf, String_t s)
    {
        tf.writeText("\"");
        afl::charset::Utf8Reader rdr(afl::string::toBytes(s), 0);
        while (rdr.hasMore()) {
            afl::charset::Unichar_t ch = rdr.eat();
            if (ch < 32 || ch >= 127) {
                tf.writeText(afl::string::Format("\\u%04X", ch));
            } else if (ch == '\\' || ch == '\"') {
                tf.writeText(afl::string::Format("\\%c", ch));
            } else {
                tf.writeText(afl::string::Format("%c", ch));
            }
        }
        tf.writeText("\"");
    }

    bool tryWriteArray(afl::io::TextWriter& tf, afl::data::Value* value, int depth)
    {
        // Sufficient depth to recurse?
        --depth;
        if (depth <= 0) {
            return false;
        }

        // Correct type?
        interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(value);
        if (!iv) {
            return false;
        }

        // Has any dimensions?
        if (iv->getDimension(0) == 0) {
            return false;
        }

        // OK, looks like an array. Write as one.
        int32_t dim = iv->getDimension(1);
        tf.writeText("[");
        for (int32_t i = 1; i < dim; ++i) {
            if (i != 1) {
                tf.writeText(",");
            }
            try {
                afl::data::Segment argSeg;
                argSeg.pushBackNew(interpreter::makeIntegerValue(i));
                interpreter::Arguments argBlock(argSeg, 0, 1);

                std::auto_ptr<afl::data::Value> element(iv->get(argBlock));
                writeValue(tf, element.get(), depth);
            }
            catch (...) {
                writeQuotedString(tf, "#<error>");
            }
        }
        tf.writeText("]");
        return true;
    }

    void writeValue(afl::io::TextWriter& tf, afl::data::Value* value, int depth)
    {
        if (value == 0) {
            tf.writeText("null");
        } else if (afl::data::BooleanValue* bv = dynamic_cast<afl::data::BooleanValue*>(value)) {
            tf.writeText(bv->getValue() ? "true" : "false");
        } else if (afl::data::ScalarValue* sv = dynamic_cast<afl::data::ScalarValue*>(value)) {
            tf.writeText(afl::string::Format("%d", sv->getValue()));
        } else if (tryWriteArray(tf, value, depth)) {
            // ok
        } else {
            writeQuotedString(tf, interpreter::toString(value, false));
        }
    }
}


interpreter::exporter::JsonExporter::JsonExporter(afl::io::TextWriter& file)
    : m_file(file),
      m_firstField(true),
      m_firstRecord(true)
{ }

void
interpreter::exporter::JsonExporter::startTable(const FieldList& /*fields*/, afl::base::Memory<const TypeHint> /*types*/)
{
    // Start an array
    m_file.writeText("[");
    m_firstRecord = true;
}

void
interpreter::exporter::JsonExporter::startRecord()
{
    // Start a hash
    if (!m_firstRecord) {
        m_file.writeText(",\n");
    }
    m_file.writeText("{");
    m_firstField = true;
    m_firstRecord = false;
}

void
interpreter::exporter::JsonExporter::addField(afl::data::Value* value, const String_t& name, TypeHint /*type*/)
{
    // Write a value
    if (!m_firstField) {
        m_file.writeText(",\n");
    }
    writeQuotedString(m_file, name);
    m_file.writeText(":");
    writeValue(m_file, value, 3);
    m_firstField = false;
}

void
interpreter::exporter::JsonExporter::endRecord()
{
    m_file.writeText("}");
}

void
interpreter::exporter::JsonExporter::endTable()
{
    m_file.writeText("]\n");
}
