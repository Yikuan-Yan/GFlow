#ifndef __LENNARD_JONES__GFLOW__
#define __LENNARD_JONES__GFLOW__

#include "../base/interaction.hpp"

namespace GFlowSimulation {

  /**
  *  \brief LennardJones where all particles have the same force strength.
  *
  *  Lennard Jones force. The particle sigma will represent the force cutoff,
  *  generally 2.5*sig, where sig is the inter-particle distance where V=0.
  *  We use cutoff=2.5 by default, but it can be changed. Strength is the
  *  "epsilon" parameter in LJ.
  *
  *  The parameters for LJ are the LJ strength (parameters[0]), and the cuttoff (parameters[1]).
  */
  class LennardJones : public Interaction {
  public:
    //! @brief Constructor
    LennardJones(GFlow *);

    //! @brief Initialize the force, check if all the special data (dataF, dataI) the force needs exists, make
    //! sure parameter packs are up to date.
    virtual void initialize() override;

    //! @brief Set the lennard jones interaction strength. Must be non-negative.
    void setStrength(RealType);

    //! @brief Set the lennard jones cutoff range. Must be at least 1.
    void setCutoff(RealType);

    //! @param[in] normal
    //! @param[in] distance
    //! @param[in] id1
    //! @param[in] id2
    //! @param[in] simData
    //! @param[in] param_pack A parameter pack, passed in from force. Contains characteristic 
    //! constants of the force, and extra data the force needs. Should be of the form { strength, cutoff } (length 2).
    //! @param[in,out] data_pack Data to be updated by the function. Should be of the form  { virial } (length 1). 
    static void force(simd_float*, simd_float*, const simd_float, const simd_float, const simd_float*, const RealType*, RealType*);
  };

}
#endif // __LENNARD_JONES__GFLOW__