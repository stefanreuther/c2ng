/**
  *  \file game/map/planeteffectors.cpp
  *  \brief Class game::map::PlanetEffectors
  */

#include "game/map/planeteffectors.hpp"
#include "game/map/planetformula.hpp"
#include "afl/string/format.hpp"
#include "afl/base/memory.hpp"

typedef afl::base::Memory<int> Ints_t;
typedef afl::base::Memory<const int> ConstInts_t;

game::map::PlanetEffectors::PlanetEffectors()
{
    clear();
}

bool
game::map::PlanetEffectors::operator==(const PlanetEffectors& other) const
{
    return ConstInts_t(m_effectors).equalContent(other.m_effectors);
}

bool
game::map::PlanetEffectors::operator!=(const PlanetEffectors& other) const
{
    return !operator==(other);
}

void
game::map::PlanetEffectors::clear()
{
    // ex GPlanetEffectors::clear
    Ints_t(m_effectors).fill(0);
}

void
game::map::PlanetEffectors::add(Kind eff, int count)
{
    // ex GPlanetEffectors::add
    m_effectors[eff] += count;
}

void
game::map::PlanetEffectors::set(Kind eff, int count)
{
    // ex GPlanetEffectors::set
    m_effectors[eff] = count;
}

int
game::map::PlanetEffectors::get(Kind eff) const
{
    // ex GPlanetEffectors::get
    return m_effectors[eff];
}

int
game::map::PlanetEffectors::getNumTerraformers() const
{
    // ex GPlanetEffectors::getNumTerraformers
    return m_effectors[HeatsTo50] + m_effectors[CoolsTo50] + m_effectors[HeatsTo100];
}

String_t
game::map::PlanetEffectors::describe(afl::string::Translator& tx, int shipOwner, const game::config::HostConfiguration& config, const HostVersion& host) const
{
    // ex WPlanetGrowthTile::drawData (part)
    using afl::string::Format;
    if (int numTerraformers = getNumTerraformers()) {
        if (int numHissers = get(Hiss)) {
            return Format(tx("%d ship%!1{s%} hissing, %d ship%!1{s%} terraforming"), numHissers, numTerraformers);
        } else {
            return Format(tx("%d ship%!1{s%} terraforming"), numTerraformers);
        }
    } else {
        if (int numHissers = get(Hiss)) {
            int effect = getHissEffect(shipOwner, numHissers, config, host);
            String_t fmt = (effect > 0
                            ? tx("%d ship%!1{s%} hissing (+%d)")
                            : tx("%d ship%!1{s%} hissing (no effect)"));
            return Format(fmt, numHissers, effect);
        } else {
            return tx("No ship effects considered");
        }
    }
}
