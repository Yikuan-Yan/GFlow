#ifndef __PARTICLE_TEMPLATE_HPP__GFLOW__
#include <functional>
#define __PARTICLE_TEMPLATE_HPP__GFLOW__

#include "../utility/randomengines.hpp"

namespace GFlowSimulation {

  struct ParticleTemplate {
    //! @brief Default constructor. Voids pointers.
    ParticleTemplate();

    //! @brief Destructor.
    ~ParticleTemplate();

    ParticleTemplate(const ParticleTemplate& p);

    ParticleTemplate(ParticleTemplate&& p);

    ParticleTemplate& operator=(const ParticleTemplate& p);

    //! @brief Create particle data.
    void createParticle(RealType*, RealType&, RealType&, int&, int, int);

    struct NullEngine {};
  
    // Random engines
    RandomEngine *type_engine, *radius_engine, *mass_engine, *velocity_engine;
    string type_string, radius_string, mass_string, velocity_string;
  };

}

 #endif // __PARTICLE_TEMPLATE_HPP__GFLOW__