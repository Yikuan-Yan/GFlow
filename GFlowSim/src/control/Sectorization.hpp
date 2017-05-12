/*
 * Author: Nathaniel Rupprecht
 * Start Data: May 11, 2017
 *
 */

#ifndef __SECTORIZATION_HPP__
#define __SECTORIZATION_HPP__

// Includes
#include "../../include/Utility.hpp"
#include "SimData.hpp"

namespace GFlow {

  /*
   * @class Sectorization
   * Uses cell lists to create verlet lists so we can easily calculate forces
   */
  class Sectorization {
  public:
    // Default constructor
    Sectorization();
    
    // Set the simulation data to manage
    void setSim(SimData* sd);
    
    // Update sectorization
    void sectorize();

#ifdef USE_MPI
    void atom_move();
    void atom_copy();
#endif

    // Create verlet lists
    void createVerletLists();

    // Create wall lists
    void createWallLists();

    // Returns the particle verlet list
    auto& getVerletList() { return verletList; }

    // Returns the wall neighbor list
    auto& getWallList() { return wallList; }

    // Returns the move list
    auto& getMoveList() { return moveList; }

    // Returns the copy list
    auto& getCopyList() { return copyList; }

  private:
    // Private helper functions
    int getSec(const RealType, const RealType);

    // Pointer to the simulation data we manage
    SimData *simData;
    
    // The sectors: a list of the id's of particles that are in these sectors
    list<int> *sectors;

    // Particle neighbor list. First int is the id of the main particle, the other ints are particles that are withing the cutoff region of the main particle.
    list<list<int> > verletList;

    // Wall neighbor list. First int is the id of the wall, the other ints are the id's of the particles within the cutoff region.
    list<list<int> > wallList;

    // List of particles that have migrated out of the simulation domain. List of { int, list<int> } specifies the processor to move to and the id's of the particles that need to move there.
    list<pair<int, list<int> > > moveList;

    // List of particles that are near the edge of the simulation domain and need to be copied to another processor. List of { int, list<int> } specifies the processor to move to and the id's of the particles that need to move there.
    list<pair<int, list<int> > > copyList;

    // Sectorization Data
    RealType cutoff;     // The cutoff radius
    int nsx, nsy;        // The number of sectors
    RealType sdx, sdy;   // Sector width and height
    RealType isdx, isdy; // The inverse of the sector widths and heights

    Bounds bounds;       // Domain bounds

    // Rank and number of processors - for MPI 
    int rank, numProc;

  };

}
#endif // __SECTORIZATION_HPP__
