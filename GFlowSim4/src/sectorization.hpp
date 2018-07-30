#ifndef __SECTORIZATION_HPP__GFLOW__
#define __SECTORIZATION_HPP__GFLOW__

#include "domainbase.hpp"
#include "array.hpp"

namespace GFlowSimulation {

  class Sectorization : public DomainBase {
  public:
    //! Constructor
    Sectorization(GFlow *);

    // Pre-integrate calls sectorize
    virtual void pre_integrate();

    // --- Accessors

    // Get a sector
    const vector<int>& getSector(int *) const;

    // --- Mutators

    //! Set the skin depth, and remake the sectors
    virtual void setSkinDepth(RealType);

    //! Set the cutoff factor, and remake the sectors
    virtual void setCutoffFactor(RealType);
    
    // GFlow is a friend class
    friend class GFlow;
    friend class SectorizationData;
    
  private:
    // --- Helper functions

    virtual void remake_verlet();

    // Create sectors
    inline void makeSectors();

    // Create verlet lists for all forces
    inline void makeVerletLists();

    // Given two particles, take care of whether they should exert forces on one another
    inline void pairInteraction(int, int);
    
    // The id's of particles in each sector - these are the actual sectors
    Array< vector<int> > sectors;

    // Current head - for verlet list creation
    int currentHead;
  };

};

#endif // __SECTORIZATION_HPP__GFLOW__