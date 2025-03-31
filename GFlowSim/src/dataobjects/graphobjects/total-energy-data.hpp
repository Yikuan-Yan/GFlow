#ifndef __TOTAL_ENERGY_DATA_HPP__GFLOW__
#include <functional>
#define __TOTAL_ENERGY_DATA_HPP__GFLOW__

#include "../dataobjecttypes/graphobject.hpp"

namespace GFlowSimulation {

  class TotalEnergyData : public GraphObject {
  public:
    //! \brief Default constructor.
    TotalEnergyData(GFlow*, bool=true);

    //! \brief Make sure potential energy data is being collected.
    virtual void pre_integrate() override;

    //! \brief Collect the position data from simdata --- happens during the post-step phase
    virtual void post_step() override;

  private:
    //! \brief Whether to use the average energy per particle (as opposed to the total energy).
    bool useAve;

    //! \brief If restrict_energy = true and the total energy ever reaches a level larger than this fraction of the initial energy, the program ends.
    RealType fraction;
  };

}
#endif // __TOTAL_ENERGY_DATA_HPP__GFLOW__