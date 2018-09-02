/**
  *  \file game/map/ufotype.hpp
  */
#ifndef C2NG_GAME_MAP_UFOTYPE_HPP
#define C2NG_GAME_MAP_UFOTYPE_HPP

#include <list>
#include "game/map/ufo.hpp"
#include "game/map/objecttype.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace map {

    // FIXME: For now, I am (almost) re-using the PCC2 implementation here despite its drawbacks.
    // Since Ufo is uncopyable, we cannot make a list like PCC2 does; thus, we use a PtrVector.
    // This means
    // - possibly O(n**2) creation (but O(n) iteration, where PCC2 has O(n**2))
    // - cannot deal with multiple ufos with same Id, different type

    class Universe;

    class UfoType : public ObjectType {
     public:
        //    typedef std::list<TUtil14Wormhole> wormlist_t;
        //    typedef ufolist_t::iterator iterator;
        //    typedef ufolist_t::const_iterator const_iterator;

        // Construction:
        UfoType(Universe& univ);
        ~UfoType();

        //    // Loading:
        //    void addWormholeData(const TUtil14Wormhole& wh);
        //    void addObjectData(const TUtil33GO& obj);
        //    void addHistoryData(const TDbUfo& ufo);
        Ufo* addUfo(int id, int type, int color);
        void addMessageInformation(const game::parser::MessageInformation& info);
        void postprocess(int turn);

        //    // GObjectType:
        virtual Ufo* getObjectByIndex(Id_t index);
        virtual Universe* getUniverseByIndex(Id_t index);
        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;

        //    // Container access:
        //    iterator begin();
        //    iterator end();
        //    const_iterator begin() const;
        //    const_iterator end() const;

        //    iterator getFirstAt(int id);

     private:
        typedef afl::container::PtrVector<Ufo> UfoList_t;

        Ufo* getUfoById(int id);

        //    GUfo& getUfoByIndex(int32_t index);

        // Universe link
        Universe& m_universe;

        // Ufo storage
        UfoList_t m_ufos;

        //    // Wormhole storage
        //    wormlist_t wormholes;
    };

} }

#endif
