/**
  *  \file ui/res/resourcefile.hpp
  *  \brief Class ui::res::ResourceFile
  */
#ifndef C2NG_UI_RES_RESOURCEFILE_HPP
#define C2NG_UI_RES_RESOURCEFILE_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"

namespace ui { namespace res {

    /** PCC 1.x Resource File.
        Provides access to the individual files contained in a PCC 1.x ".res" file.

        A PCC 1.x resource file contains multiple sub-streams identified by a number each.
        You can use openMember() to obtain a stream object that allows you to read a member.
        Any number of these streams can be active at any given time. */
    class ResourceFile {
     public:
        /** Constructor.
            \param file File
            \param tx Translator (for error messages) */
        ResourceFile(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx);

        /** Destructor. */
        ~ResourceFile();

        /** Open a resource file member as stream.
            \param id Member Id
            \return stream object if a member with this Id exists; null if no such member exists */
        afl::base::Ptr<afl::io::Stream> openMember(uint16_t id);

     private:
        /// Underlying file.
        afl::base::Ref<afl::io::Stream> m_file;

        /// Index entry (cooked).
        struct Entry {
            uint16_t id;
            uint32_t position;
            uint32_t length;
        };

        /// Index.
        std::vector<Entry> m_index;

        void init(afl::string::Translator& tx);
    };

} }

#endif
