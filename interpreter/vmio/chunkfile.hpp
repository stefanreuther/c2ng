/**
  *  \file interpreter/vmio/chunkfile.hpp
  *  \brief Class interpreter::vmio::ChunkFile
  */
#ifndef C2NG_INTERPRETER_VMIO_CHUNKFILE_HPP
#define C2NG_INTERPRETER_VMIO_CHUNKFILE_HPP

#include "afl/base/growablememory.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/vmio/structures.hpp"

namespace interpreter { namespace vmio {

    /** Reading and writing chunk-based virtual machine state.
        A virtual machine state file consists of
        - a file-type specific header.
          This class provides the "object file" (*.qc) format (interpreter::vmio::structures::ObjectFileHeader,
          methods loadObjectFileHeader, saveObjectFile).
          The game integration provides a different header for live VM state files.
        - a sequence of objects (e.g. BytecodeObject, Process, etc.)

        Each object consist of a header (interpreter::vmio::structures::ObjectHeader),
        followed by a list of properties.
        The set of properties is specific to each object type.

        This class provides inner classes Loader and Writer. */
    class ChunkFile {
     public:
        typedef interpreter::vmio::structures::UInt32_t UInt32_t;

        /** Chunk file loader.
            To use,
            - read file header
            - construct Loader
            - repeatedly call readObject() to read objects
              - for each object, repeatedly call readProperty() to read properties
                - for each property, read content from the provided stream */
        class Loader {
         public:
            /** Constructor.
                @param s   Stream to read from. Must be Ref<> to allow creation of child streams.
                @param tx  Translator (for error messages) */
            Loader(const afl::base::Ref<afl::io::Stream>& s, afl::string::Translator& tx);

            /** Read an object.
                @param [out] type Object type
                @param [out] id   Object Id
                @return true on success; false on EOF */
            bool readObject(uint32_t& type, uint32_t& id);

            /** Read a property.
                @param [out] id    Property Id
                @param [out] count Number of elements (property-specific)
                @return stream (usable with afl::base::Ref) to read property content; null if no more properties */
            afl::io::Stream* readProperty(uint32_t& id, uint32_t& count);

            /** Get number of properties of this object.
                @return Number */
            uint32_t getNumProperties() const;

            /** Get size of property value in bytes.
                @param id Property Id [1,getNumProperties()]
                @return size, 0 if id is out of range */
            uint32_t getPropertySize(uint32_t id) const;

            /** Get number of elements of a property.
                @param id Property Id [1,getNumProperties()]
                @return property-specific count, 0 if id is out of range */
            uint32_t getPropertyCount(uint32_t id) const;

         private:
            afl::base::Ref<afl::io::Stream> m_stream;
            afl::string::Translator& m_translator;
            uint32_t m_objectSize;
            afl::base::Ptr<afl::io::Stream> m_propertyStream;
            afl::io::Stream::FileSize_t m_nextProperty;
            uint32_t m_propertyId;
            afl::io::Stream::FileSize_t m_nextObject;
            afl::base::GrowableMemory<UInt32_t> m_properties;

            void consumeObjectSize(uint32_t needed);
        };

        /** Chunk file write.
            To use,
            - write file header
            - for each object, call start()
              - for each property, call startProperty(), writing the properties in numerical order starting from 1;
                write the property, finish with endProperty()
              - end the object using end() */
        class Writer {
         public:
            /** Constructor.
                @param s Stream to write into */
            explicit Writer(afl::io::Stream& s);

            /** Start an object.
                @param type  Object type (otyp_xxx)
                @param id    Object Id
                @param nprop Number of properties */
            void start(uint32_t type, uint32_t id, uint32_t nprop);

            /** Finish a property. */
            void end();

            /** Start a property.
                @param count Number of elements (property-specific) */
            void startProperty(uint32_t count);

            /** Finish a property. */
            void endProperty();

         private:
            afl::io::Stream& m_stream;

            interpreter::vmio::structures::ObjectHeader m_header;
            afl::io::Stream::FileSize_t m_headerPosition;

            uint32_t m_propertyIndex;
            afl::io::Stream::FileSize_t m_thisPropertyPosition;
            afl::base::GrowableMemory<UInt32_t> m_properties;

            void writeHeader();
        };

        /** Load header of an object file (*.qc).
            @param s  Stream. Must be Ref<> to match Loader.
            @param tx Translator (for error messages)
            @return Id of entry-point object
            @throw FileProblemException on error */
        static uint32_t loadObjectFileHeader(afl::base::Ref<afl::io::Stream> s, afl::string::Translator& tx);

        /** Write header of an object file (*.qc).
            @param s     Stream
            @param entry of entry-point object
            @throw FileProblemException on error */
        static void writeObjectFileHeader(afl::io::Stream& s, uint32_t entry);
    };

} }

#endif
