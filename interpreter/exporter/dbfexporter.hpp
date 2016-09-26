/**
  *  \file interpreter/exporter/dbfexporter.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_DBFEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_DBFEXPORTER_HPP

#include "interpreter/exporter/exporter.hpp"
#include "afl/io/stream.hpp"
#include "afl/base/growablememory.hpp"

namespace interpreter { namespace exporter {

    class DbfExporter : public Exporter {
     public:
        DbfExporter(afl::io::Stream& file);

        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::Stream& m_file;             ///< Stream to write to.

        std::vector<std::size_t> m_widths;   ///< Field widths.

        size_t m_startPosition;              ///< Position of first data record in file.
        uint32_t m_numRecords;               ///< Number of data records in file.
        size_t m_recordSize;                 ///< Size of data record.

        afl::base::GrowableMemory<uint8_t> m_record;       ///< Buffer for one data record. Size is recordSize.
        afl::base::Memory<uint8_t> m_recordPosition;       ///< Next byte to write in data record.
        size_t m_fieldNumber;                ///< Next field to write in data record.

        void writeFileHeader();
    };

} }

#endif
