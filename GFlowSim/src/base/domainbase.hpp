#ifndef __DOMAIN_BASE_HPP__GFLOW__
#define __DOMAIN_BASE_HPP__GFLOW__

#include "../gflow.hpp"

namespace GFlowSimulation {

  /** 
  *  \brief The base class for domain decomposition and sectorization classes.
  *
  *  DomainBase classes are responsible for a subvolume (possibly the whole volume)
  *  of the simulation, keeping track of where the particles are and where they need 
  *  to be transfered to (if using MPI parallelization).\n
  *
  *  They are responsible for creating verlet lists for all the forces (if deemed 
  *  necessary) during the pre-forces step of the simulation.\n
  *  
  *  The natural way to do this is to divide the domain into sectors, or cells, and 
  *  slot particles into these cells so we can easily lookup potential neighbors of 
  *  each particle.\n
  *
  *  MPI tutorials: <http://mpitutorial.com/>
  *
  */
  class DomainBase : public Base {
  public:
    //! Constructor
    DomainBase(GFlow*);

    //! Destructor
    virtual ~DomainBase();

    virtual void pre_integrate() override;

    virtual void pre_forces() override;

    // --- Accessors

    //! Get the array of the number of cells in each dimension
    const int* getDims() const;

    //! Get the array of the width of the cells in each dimension
    const RealType* getWidths() const;

    //! Get the total number of cells in the domain
    int getNumCells() const;

    //! Get the skin depth the domain is using
    RealType getSkinDepth() const;

    //! Get the cutoff
    RealType getCutoff() const;

    //! Get the move ratio tollerance
    RealType getMvRatioTolerance() const;

    //! Get the number of times the domain has made verlet lists.
    int getNumberOfRemakes() const;

    //! Get the number of times the update delay was too long.
    int getMissedTarget() const;

    //! Get what the average amount by which we missed our target particle 
    //! displacement was.
    RealType getAverageMiss() const;

    //! Get the sample size variable
    int getSampleSize() const;

    // --- Locator functions

    //! \brief Get all the particles within a radius of another particle
    //! Fills a passed in vector with the ids of all the particles that lie within
    //! a specified distance of a given particle.\n
    //! This function must be overloaded by all children of DomainBase.
    virtual void getAllWithin(int, RealType, vector<int>&)=0;

    //! \brief Remove particles that are overlapping by more than some fraction.
    virtual void removeOverlapping(RealType)=0;

    // --- Mutators

    //! \brief Set the skin depth. This function is virtual, as the inheriting class
    //! may need to remake itself after doing this.
    virtual void setSkinDepth(RealType);

    //! \brief Set the cell size. 
    //!
    //! Really, this suggests a cell size. It must be larger than the minimum possible cell size, 
    //! and must evenly divide the size of the domain.
    virtual void setCellSize(RealType)=0;

    //! \brief Set the sample size variable.
    void setSampleSize(int);

    //! \brief Set the maximum update delay.
    void setMaxUpdateDelay(RealType);

    //! \brief Remakes interactionhandlers (if neccessary).
    //!
    //! This function should be overridden by each child to remake the interaction handlers of the forces as they
    //! see fit. It should, however, be called first in each child function (DomainBase::construct()). This function
    //! will only be called if the handlers need to be constructed.
    //!
    //! This function:
    //! (1) Wraps particle positions to their cannonical form.
    //! (2) Sets the lastUpdate timer
    //! (3) Increments the number_of_remakes counter
    //! (5) Calls the ForceMaster clear() function
    //! (6) Calls fillXVL() to record the positions of the particles
    virtual void construct();

    // GFlow is a friend class
    friend class GFlow;

  protected:
    // --- Helper functions

    //! Delete and set as null the xVL array
    void nullXVL();

    //! Set up xVL array
    void setupXVL(int);

    //! Fill the xVL array with the positions
    void fillXVL();

    void pair_interaction(int, int);

    //! Find the maximum amount two particles might have moved closer to one another
    virtual RealType maxMotion();

    //! Check whether particles might have move far enough to warrant verlet list remake.
    virtual bool check_needs_remake();

    //! \brief Calculates the maximum "small sigma."
    //!
    //! Particles that are larger than max_small_sigma are "large particles," and must search more than
    //! one sector around them.
    virtual void calculate_max_small_sigma();

    // --- Data

    //! \brief The bounds of the domain
    Bounds domain_bounds;
    
    //! \brief The bounds of the entire simulation
    Bounds bounds;

    //! \brief The number of times the domain has remade the sectors
    int number_of_remakes = 0;

    //! \brief Number of cells in each dimension
    int *dims;

    //! \brief The widths of a cell in each dimension
    RealType *widths;

    //! \brief The inverse widths of a cell in each dimension
    RealType *inverseW;

    // --- Sectorization constants

    //! \brief The skin depth.
    //!
    //! The extra amount around a particle that the domain should count as being a particle neighbor. So if
    //! d(x, y) < rx + ry + skin_depth, the particles are neighbors.
    RealType skin_depth = DEFAULT_SKIN_DEPTH;
    
    //! \brief The maximum cutoff radius a particle can have and be guarenteed to only have to look in 
    //! adjacent cells for neighbors.
    //!
    //! How domains decide to determine max_small_sigma is up to them.
    RealType max_small_sigma = 0.;

    //! \brief The target cell size. 
    //!
    //! Cells will be at least this wide in each dimension, but since an integral number of them have
    //! to fit in the domain in each dimension, the actual widths will be different. They will be at
    //! least this large though.
    RealType target_cell_size = 0.;

    //! \brief The minimum allowable cutoff for small particles, 2*max_small_sigma + skin_depth
    RealType min_small_cutoff = 0.; 

    // --- Timers

    // Update timers and related
    RealType last_check = -1.;
    RealType last_update = -1.;
    RealType update_delay = 1.0e-4;
    RealType max_update_delay = DEFAULT_MAX_UPDATE_DELAY;

    //! \brief What criteria the domain should.
    //!
    //! 0 - Use an update delay.
    //! 1 - update every fixed number of steps.
    int update_decision_type = 0;

    //! \brief How many steps the domain should wait between domain redos.
    int update_delay_steps = 8;

    //! \brief How many steps since the last time the domain was remade.
    int steps_since_last_remake = 0;

    //! \brief What fraction of the skin depth should particles move before the domain remake.
    //!
    //! If 
    RealType motion_factor = DEFAULT_MOTION_FACTOR;

    //! \brief The target move ratio for remake
    RealType mv_ratio_tolerance = DEFAULT_MV_RATIO_TOLERANCE;

    //! \brief The number of times max_motion / skinDepth was > 1
    int missed_target = 0;

    //! \brief The average (once divided by [missed_target]) amount the delay missed by
    RealType ave_miss = 0.;

    //! \brief An array storing the positions of the particles at the last verlet list creation
    RealType **xVL = nullptr;

    //! \brief The size of the xVL array
    int sizeXVL = 0;
    
    //! \brief How many particles the domain should sample to estimate the maximum displacement of 
    //! particles. An important parameter when "used for good."
    //!
    //! If [sample_size]>0, the domain should sample a subset of the particles for calculating the 
    //! max and second largest displacements. Otherwise, use all the particles. For 
    //! homogenous mixtures of particles, e.g. gasses, you only need to look at a few particle
    //! to find a good representation of the maximum displacement of any particle. \n
    //! Of course, it is likely that you didn't find the true maximum displacement, and so the domain 
    //! should estimate that the true maximum dispacement is larger. How much larger should
    //! depend on the total number of particles compared to how many the domain sampled. \n
    //! If there is a non-homogenous mixture of particles, then there may be local regions
    //! where some particles are moving very quickly, like an explosion, or a ball dropping
    //! into particles, which means that the domain may miss this if the domain only sample a subset of the 
    //! particles. In this case, set sample_size to zero. Or risk it. Your choice.
    int sample_size = -1;
  };

}
#endif // __DOMAIN_BASE_HPP__GFLOW__