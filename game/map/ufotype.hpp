/**
  *  \file game/map/ufotype.hpp
  *  \brief Class game::map::UfoType
  */
#ifndef C2NG_GAME_MAP_UFOTYPE_HPP
#define C2NG_GAME_MAP_UFOTYPE_HPP

#include "afl/container/ptrmap.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/typedobjecttype.hpp"
#include "game/map/ufo.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace map {

    class Configuration;

    /** Container and ObjectType for Ufos.

        This manages Ufos (Host 3.2/UFO.HST), General Objects (PHost Util33GO), and Wormholes (PHost Util14Wormhole).

        Ufos and GOs are closely related; Ids 1..1000 are defined to be the same object.
        We ought to identify GOs with larger Ids by Id/Type-Code, as we cannot require add-ons to coordinate Ids.
        For now, we rely on the Id only (like PCC2, PCC1).
        We merge received information; last seen instance survives.

        For wormholes, we have three possible sources:
        - the Ufo from KORE.DAT
        - the UTIL.DAT entry
        - the WORMHOLE.TXT file
        Unfortunately, there is no 1:1 mapping between wormhole Ids and Ufo Ids
        (each WH consumes two WH Id slots, but whereas a bidirectional WH consumes two Ufo slots,
        an unidirectional one consumes only one).
        We therefore queue all UTIL.DAT wormholes first, and merge them later upon postprocess() time; see there.

        PCC 1.x also reads WORMHOLE.TXT to fill in the blanks.
        As of 20201012, this is not supported by PCC2 nor c2ng.

        <b>Implementation Details</b>

        For now, I am (almost) re-using the PCC2 implementation here; replacing PCC2's list by a sorted PtrVector.
        ObjectType indexes are 1-based indexes into that PtrVector (possibly different from Ufo Ids).
        Efficient Ufo iteration can use indexes, there is no other custom iteration mechanism required.

        This means
        - possibly O(n**2) creation (but O(n) iteration, where PCC2 has O(n**2))
        - cannot deal with multiple ufos with same Id, different type (but that could probably be added easily).

        Since we address Ufos by index, indexes change when Ufos are added or removed.
        Should that be required, we need to make some stable-reference mechanism
        (low-fi version: just validate that Id/index still match) and use that for at least UfoContext, possibly also Reference.

        Because Ufos can link to each other, be careful to not delete them lightly.

        In addition to managing Ufos, UfoType also manages a temporary list of wormhole reports,
        which are merged into Ufos using postprocess(). */
    class UfoType : public TypedObjectType<Ufo> {
     public:
        /** Constructor. */
        UfoType();

        /** Destructor. */
        ~UfoType();

        /** Add an Ufo.
            Use this function to create Ufos loaded from host data.
            \param id    Ufo Id
            \param type  Type code
            \param color Color code. Must not be 0 (0 indicates unused slot in host file).
            \return Newly-allocated or existing Ufo object; null on error */
        Ufo* addUfo(int id, int type, int color);

        /** Add message information.
            The information must be addressed to a Ufo or Wormhole and will be routed internally.
            \param info Information */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Postprocess after load.
            This merges Ufo and Wormhole information, and updates Ufo history data for the current turn.
            \param turn Current turn number
            \param mapConfig Map configuration
            \param config host configuration
            \param tx translator
            \param log logger */
        void postprocess(int turn, const Configuration& mapConfig, const game::config::HostConfiguration& config, afl::string::Translator& tx, afl::sys::LogListener& log);


        /*
         *  ObjectType methods
         */

        virtual Ufo* getObjectByIndex(Id_t index);
        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;

        /** Find index for an Ufo, given an Id.
            If an Ufo with the given Id exists, returns the (1-based) index such that getUfoByIndex() will return that Ufo.
            If an Ufo with the given Id does not exist, returns the (1-based) index where it would have to be inserted in the sequence.
            \param id Desired Id
            \return 1-based index, compatible with getUfoByIndex(), getObjectByIndex(). */
        Id_t findUfoIndexById(int id) const;

        /** Get Ufo by index.
            Unlike the getObjectByIndex() method, this may return an object whose isValid() is false.
            \param index 1-based index
            \return Ufo, or null */
        Ufo* getUfoByIndex(Id_t index);

     private:
        struct Wormhole {
            Point pos;
            IntegerProperty_t mass;
            IntegerProperty_t stabilityCode;
            IntegerProperty_t ufoId;
            IntegerProperty_t bidirFlag;
        };

        typedef afl::container::PtrVector<Ufo> UfoList_t;
        typedef afl::container::PtrMap<int,Wormhole> WormholeMap_t;

        Ufo* getUfoById(int id);

        /** Ufo storage.
            Valid for entire lifetime. */
        UfoList_t m_ufos;

        /** Wormhole storage.
            Populated only during setup (until postprocess() is called). */
        WormholeMap_t m_wormholes;

        /** Get/add wormhole.
            \param id Wormhole Id
            \return Newly-allocated or existing wormhole */
        Wormhole& addWormhole(int id);

        /** Merge wormhole data.
            \param ufo        Ufo
            \param wormholeId Wormhole Id
            \param data       Wormhole data
            \param isNew      true if this is a new Ufo created from scratch; false if there is existing host data
                              (controls how aggressively we overwrite)
            \param turnNumber Turn number
            \param config     Host configuration
            \param tx         Translator
            \param log        Log */
        void mergeWormhole(Ufo& ufo, int wormholeId, const Wormhole& data, bool isNew, int turnNumber, const game::config::HostConfiguration& config, afl::string::Translator& tx, afl::sys::LogListener& log);
    };

} }

#endif
