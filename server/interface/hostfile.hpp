/**
  *  \file server/interface/hostfile.hpp
  *  \brief Interface server::interface::HostFile
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTFILE_HPP
#define C2NG_SERVER_INTERFACE_HOSTFILE_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "server/interface/filebase.hpp"

namespace server { namespace interface {

    /** Host file server interface.
        This interface allows to access files from the host service, using host's directory abstractions. */
    class HostFile : public afl::base::Deletable {
     public:
        typedef FileBase::Type Type_t;

        /** Directory label definition.
            In addition to their path names, directories can have internationalized names built from other properties of the node.
            For example, a game node will have the game Id as name, but will be labeled with the game name.

            Files normally use NameLabel, i.e. their correct name. */
        enum Label {
            NameLabel,          ///< Use name (default).
            TurnLabel,          ///< Use "Turn %{turn}".
            SlotLabel,          ///< Use "Files for %{slotName}".
            GameLabel,          ///< Use "%{gameName} Files".
            ToolLabel,          ///< Use "%{toolName} Files".
            NoLabel,            ///< This is a virtual, unlabeled node. Used for root nodes.
            HistoryLabel        ///< Use "History".
        };

        /** File/directory information. */
        struct Info : public FileBase::Info {
            /** Default constructor. */
            Info()
                : name(), label(NameLabel), turnNumber(), slotId(), slotName(), gameId(), gameName(), toolName()
                { }

            // FileBase::Info provides type, visibility, size, contentId.

            /** Name. */
            String_t name;

            /** Label definition.
                Determines how the user-visible name is built from the node's other information. */
            Label label;

            /** Turn number, if any. */
            afl::base::Optional<int32_t> turnNumber;

            /** Slot number, if any. */
            afl::base::Optional<int32_t> slotId;

            /** Slot name (race name), if any. */
            afl::base::Optional<String_t> slotName;

            /** Game Id, if any. */
            afl::base::Optional<int32_t> gameId;

            /** Game name, if any. */
            afl::base::Optional<String_t> gameName;

            /** Tool name, if any. */
            afl::base::Optional<String_t> toolName;
        };

        /** Vector of node information. */
        typedef std::vector<Info> InfoVector_t;


        /** Get file content (GET).
            \param fileName File name.
            \return file content */
        virtual String_t getFile(String_t fileName) = 0;

        /** Get directory content (LS).
            \param dirName [in] Directory name.
            \param result [out] Directory content */
        virtual void getDirectoryContent(String_t dirName, InfoVector_t& result) = 0;

        /** Get file information (STAT).
            \param fileName File name.
            \return file info */
        virtual Info getFileInformation(String_t fileName) = 0;

        /** Get path description (PSTAT).
            Provides information for all path components.
            \param dirName [in] Directory name.
            \param result [out] Path component descriptions */
        virtual void getPathDescription(String_t dirName, InfoVector_t& result) = 0;


        /** Format Label to string.
            \param label Label
            \return String representation */
        static String_t formatLabel(Label label);

        /** Parse string into Label.
            \param str [in] String
            \param result [out] Result
            \retval true String was valid, \c result has been set
            \retval false String not recognized, \c result unchanged */
        static bool parseLabel(const String_t& str, Label& result);

        /** Merge information.
            Generally, a directory's context will propagate down to its content.
            For example, all files in a game's directory will belong to that game.

            This function is intended to simplify the implementation of this convention
            by requiring child nodes only provide the information they have, and propagate parent information downwards.

            Node that users of a HostFile instance need not use mergeInfo();
            information provided by getPathDescription() or getFileInformation() will be fully-populated.

            \param i [in/out] Child node information to update
            \param parent [in] Parent node information */
        static void mergeInfo(Info& i, const Info& parent);
    };

} }

#endif
