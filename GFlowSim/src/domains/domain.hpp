
#ifndef __DOMAIN_TEST_HPP__GFLOW__
#define __DOMAIN_TEST_HPP__GFLOW__

#include "../base/domainbase.hpp"
#include "cell.hpp"

namespace GFlowSimulation {

  class Domain : public DomainBase {
  public:
    //! \brief Default constructor.
    Domain(GFlow*);

    //! \brief Destructor.
    virtual ~Domain() override;

    virtual void initialize() override;

    virtual void pre_integrate() override;

    //! \brief Exchange particles between processors
    virtual void exchange_particles() override;

    //! \brief Get all the particles within a radius of another particle
    //!
    //! Fills a passed in vector with the ids of all the particles that lie within
    //! a specified distance of a given particle.\n
    //! This function must be overloaded by all children of DomainBase.
    virtual void getAllWithin(int, RealType, vector<int>&) override;

    //! \brief Remove all particles that are overlapping by more than a certain fraction.
    //! 
    //! Only the smaller particle is removed. Removal happens at the end of the function,
    //! i.e. particles are not removed until no overlaps occur, but are simply removed if they
    //! overlap too much with any other particle, even if the other particle will be removed.
    virtual void removeOverlapping(RealType) override;

    //! \brief Construct neighbor lists.
    //!
    //! Resets cells, then creates all the neighbor lists from the cells.
    virtual void construct() override;

    //! \brief Set the minimum cell size. 
    //!
    //! Causes a rebuild of all the cells.
    virtual void setCellSize(RealType) override;

  private:

    //! \brief Create the cells.
    inline void create_cells();

    //! \brief Updates the cells - this just clears the cells, fills them, and records how many particles there were.
    //!
    //! \see clear_cells
    //! \see fill_cells
    inline void update_cells();

    //! \brief Clear all the particle IDs from all the cells
    inline void clear_cells();

    //! \brief Put all particles in the domain into cells.
    inline void fill_cells();

    //! \brief Turns a linear cell index into a (DIMENSIONS)-dimensional index
    inline void linear_to_tuple(const int, int*);

    //! \brief Turns a (DIMENSIONS)-dimensional index into a linear cell index.
    inline void tuple_to_linear(int&, const int*);

    //! \brief Correct a linear index for wrapping. Returns true if the index is a valid cell.
    inline bool correct_index(int*);

    //! \brief Get the tuple index of a cell that a position lies in.
    inline void get_cell_index_tuple(const RealType*, int*);

    //! \brief Get the linear index of the cell that a position lies within.
    inline int get_cell_index(const RealType*);

    //! \brief Calculates the maximum "small sigma."
    //!
    //! Particles that are larger than max_small_sigma are "large particles," and must search more than
    //! one sector around them.
    inline void calculate_max_small_sigma();

    //! \brief Array of products, used to compute linear indices from vectors or tuple indices.
    int *products = nullptr;

    //! \brief What type of borders there are in the "up" directions.
    //!
    //! 0 - None, 1 - Halo, 2 - Wrap.
    int *border_type_up   = nullptr;

    //! \brief What type of borders there are in the "down" directions.
    //!
    //! 0 - None, 1 - Halo, 2 - Wrap.
    int *border_type_down = nullptr;

    //! \brief Whether the domain has been initialized or not.
    bool initialized = false;

    //! \brief The number of particles last time we built the cells.
    int number = 0;

    //! \brief A vector holding all the cells in the domain.
    vector<Cell> cells;
  };

}
#endif // __DOMAIN_TEST_HPP__GFLOW__
