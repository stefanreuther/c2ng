/**
  *  \file game/test/cargocontainer.hpp
  *  \brief Class game::test::CargoContainer
  */
#ifndef C2NG_GAME_TEST_CARGOCONTAINER_HPP
#define C2NG_GAME_TEST_CARGOCONTAINER_HPP

#include "game/cargocontainer.hpp"

namespace game { namespace test {

    /** Test support: simple cargo container.
        This container reports to be able to contain everything, and has 5000/10000 in storage.
        It does not change on commit. */
    class CargoContainer : public game::CargoContainer {
     public:
        virtual String_t getName(afl::string::Translator& tx) const;
        virtual String_t getInfo1(afl::string::Translator& tx) const;
        virtual String_t getInfo2(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();
    };

} }

#endif
