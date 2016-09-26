/**
  *  \file interpreter/exporter/dbfexporter.cpp
  *
  *  PCC2 Comment:
  *
  *  Unlike the text exporters, which use the type hints only as a guide to assign
  *  field widths, this one uses them to assign field types, and therefore require
  *  correctly-typed values to create a well-formatted file. So far, this restriction
  *  is implemented for bool, i.e. receiving a string or int instead of a bool will
  *  treat that with the getBoolValue() function, just like CCScript does when such
  *  a value is used in a boolean context.
  *
  *  This module is modelled after PCC 1.x export.pas::CDbfExporter.
  */

#include "interpreter/exporter/dbfexporter.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/base/staticassert.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/bits/fixedstring.hpp"

namespace {
    struct FileHeader {
        uint8_t signature;
        uint8_t year;
        uint8_t month;
        uint8_t day;
        afl::bits::Value<afl::bits::UInt32LE> numRecords;
        afl::bits::Value<afl::bits::UInt16LE> headerSize;
        afl::bits::Value<afl::bits::UInt16LE> recordSize;
        uint8_t reserved[20];
    };
    static_assert(sizeof(FileHeader) == 32, "FileHeader");

    struct FieldDescriptor {
        uint8_t name[11];
        uint8_t type;
        uint8_t address[4];
        uint8_t length;
        uint8_t decimals;
        uint8_t reserved[14];
    };
    static_assert(sizeof(FieldDescriptor) == 32, "FieldDescriptor");
}

interpreter::exporter::DbfExporter::DbfExporter(afl::io::Stream& file)
    : m_file(file),
      m_widths(),
      m_startPosition(0),
      m_numRecords(0),
      m_recordSize(0),
      m_record(),
      m_recordPosition(),
      m_fieldNumber(0)
{ }

void
interpreter::exporter::DbfExporter::startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types)
{
    // ex IntDbfExporter::startTable

    // Initialize and write dummy header
    m_recordSize = 1;   /* for deletion marker */
    m_numRecords = 0;
    writeFileHeader();

    for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
        // Parse field definition
        int userWidth = std::abs(fields.getFieldWidth(i));
        int width;
        int decim;
        char typ;
        const TypeHint* pth = types.eat();
        switch (pth ? *pth : thNone) {
         case thBool:
            width = 1;
            decim = 0;
            typ = 'L';
            break;

         case thInt:
            width = userWidth ? userWidth : 10;
            decim = 0;
            typ = 'N';
            break;

         case thFloat:
            width = userWidth ? userWidth : 10;
            decim = 2;
            typ = 'N';
            break;

         case thString:
            width = userWidth ? userWidth : 30;
            decim = 0;
            typ = 'C';
            break;

         default:
            width = userWidth ? userWidth : 100;
            decim = 0;
            typ = 'C';
            break;
        }

        // Build field descriptor
        if (width > 255) {
            width = 255;
        }

        FieldDescriptor fieldDesc;
        afl::base::fromObject(fieldDesc).fill(0);
        afl::base::fromObject(fieldDesc).trim(11).copyFrom(afl::string::toBytes(fields.getFieldName(i)));
        fieldDesc.type = typ;
        fieldDesc.length = width;
        fieldDesc.decimals = decim;
        m_file.fullWrite(afl::base::fromObject(fieldDesc));

        m_widths.push_back(width);
        m_recordSize += width;

        // Validate. Refuse record sizes that cannot be represented in a 16-bit integer.
        // Refuse more than 1020 fields (which would mean the header size cannot be
        // represented in a 16-bit integer).
        if (m_recordSize >= 0x8000 || i >= 1020) {
            throw Error("Too many fields for DBF export");
        }
    }

    // Write terminator
    static const uint8_t terminator[] = {13};
    m_file.fullWrite(terminator);

    m_startPosition = m_file.getPos();
    m_record.resize(m_recordSize);
}

void
interpreter::exporter::DbfExporter::startRecord()
{
    // ex IntDbfExporter::startRecord
    m_recordPosition = m_record;
    m_recordPosition.split(1).fill(' ');
    m_fieldNumber = 0;
}

void
interpreter::exporter::DbfExporter::addField(afl::data::Value* value, const String_t& /*name*/, TypeHint type)
{
    // ex IntDbfExporter::addField
    if (type == thBool) {
        int bv = getBooleanValue(value);
        if (bv == 0) {
            m_recordPosition.split(1).fill('N');
        } else if (bv > 0) {
            m_recordPosition.split(1).fill('Y');
        } else {
            m_recordPosition.split(1).fill('?');
        }
    } else {
        String_t s;
        bool rightAlign;
        if (type == thFloat) {
            if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(value)) {
                s = afl::string::Format("%.2f", fv->getValue());
            } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(value)) {
                s = afl::string::Format("%d.00", iv->getValue());
            } else {
                /* treat as null */
            }
            rightAlign = true;
        } else if (type == thInt) {
            if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(value)) {
                s = afl::string::Format("%.0f", fv->getValue());
            } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(value)) {
                s = afl::string::Format("%d", iv->getValue());
            } else {
                /* treat as null */
            }
            rightAlign = true;
        } else {
            /* anything else */
            s = toString(value, false);
            rightAlign = false;
        }
        if (rightAlign && s.size() < m_widths[m_fieldNumber]) {
            s.insert(s.begin(), m_widths[m_fieldNumber] - s.size(), ' ');
        }
        afl::bits::packFixedString(m_recordPosition.split(m_widths[m_fieldNumber]), afl::string::toMemory(s));
    }
    ++m_fieldNumber;
}

void
interpreter::exporter::DbfExporter::endRecord()
{
    // ex IntDbfExporter::endRecord
    m_file.fullWrite(m_record);
    ++m_numRecords;
}

void
interpreter::exporter::DbfExporter::endTable()
{
    // ex IntDbfExporter::endTable()
    // Write one additional byte. My specs don't say this is needed,
    // but dbview.exe doesn't show the last record without it. This
    // may as well be a bug in dbview, but it doesn't hurt to have
    // this byte.
    static const uint8_t eof[] = {0};
    m_file.fullWrite(eof);
    writeFileHeader();
}

// /** Write dBASE file header. */
void
interpreter::exporter::DbfExporter::writeFileHeader()
{
    // ex IntDbfExporter::writeFileHeader
    FileHeader header;
    afl::base::fromObject(header).fill(0);
    header.signature = 3;        // dBASE III file
    // ignore year/month/day fields
    header.numRecords = m_numRecords;
    header.headerSize = m_startPosition;
    header.recordSize = m_recordSize;

    m_file.setPos(0);
    m_file.fullWrite(afl::base::fromObject(header));
}
