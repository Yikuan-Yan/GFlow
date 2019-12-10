#ifndef __ATOMS_HPP_GFLOW__
#define __ATOMS_HPP_GFLOW__

#include "../gflow.hpp"
#include "../utility/vectormath.hpp"
#include "../other/timedobject.hpp"

#include <set> // For storing sets of holes in the arrays
#include <unordered_map> // For mapping owned particles to halo particles

#include "../test/d-vec.hpp"

namespace GFlowSimulation {

  //! \brief Class that holds all the atom data for a domain.
  //!
  //! SimDataTimer: The timer should be used to record the time taken by non-mpi related data update tasks, such as particle
  //! coordinate wrapping, removing particles, updating halo particles, etc. 
  //!
  //! Summary of numbers:
  //!
  //! The following inequalities hold:  _number <= _size <= _capacity. 
  //! The reason why _number <= _size is that there may be some invalid (type=-1) particles in the middle of the number array. 
  //! The reason why _size <= _capacity is that we may have allocated more space than necessary for particles.
  class SimData : public Base, public TimedObject {
  public:
    //! \brief Default constructor.
    SimData(GFlow*);

    //! \brief Destructor.
    ~SimData();

    //! \brief Initialize the atom container.
    virtual void initialize() override;

    //! \brief Resets timers.
    virtual void pre_integrate() override;

    //! \brief Remove all halo and ghost particles.
    virtual void post_integrate() override;

    //! \brief Reserve space for particles, extending the lengths of all arrays to the requested size.
    void reserve(int);

    //! \brief Add a default particle, the properties of this particle should be set from the outside after this is called. Returns
    //! the id of the added particle.
    int addParticle();

    //! \brief Add several default particles. Same as calling addParticle multiple times. Returns the id of the first particle added.
    int addParticle(int);

    //! \brief Add a particle to the simdata. Returns the id of the added particle.
    //!
    //! Add a particle to the simdata. This is the public version of the function, so we can only add owned particles. 
    //! Depending on how many particles are in the array, and the array capacities, it may be necessary to resize the array 
    //! to add the particle.
    int addParticle(const RealType*, const RealType*, const RealType, const RealType, const int);

    // \brief Mark a particle for removal.
    void markForRemoval(const int);

    //! \brief Remove all the particles that need to be removed, consolidate data.
    void doParticleRemoval();

    //! \brief Do a quick sort based on the particle's positions.
    void sortParticles();

    //! \brief Do a quick sort based on particle's position, projected onto a vector.
    void sortParticles(Vec&);

    //! \brief Remove particles that have NAN positions or velocities. Return true if any were removed. If the flag is set to true, do particle removal if any particles were marked for removal.
    bool removeBadParticles(bool=true);

    //! \brief Update the primary particle that halo particles correspond to.
    void updateHaloParticles();

    //! \brief Update particles on other processors.
    void startGhostParticleUpdates();

    //! \brief Receive ghost particle data from other processors.
    void finishGhostParticleUpdates();

    // --- Accessors

    struct vec_access;
    struct scalar_access;
    struct integer_access;

    // --- Get vector data
    vec_access X();
    RealType*  X(int);
    RealType&  X(int, int);
    vec_access V();
    RealType*  V(int);
    RealType&  V(int, int);
    vec_access F();
    RealType*  F(int);
    RealType&  F(int, int);

    vec_access VectorData(int);
    vec_access VectorData(const string&);

    // --- Get scalar data
    scalar_access Sg();
    RealType& Sg(int);
    scalar_access Im();
    RealType& Im(int);

    scalar_access ScalarData(int);
    scalar_access ScalarData(const string&);

    // --- Get integer data
    integer_access Type();
    int& Type(int);
    integer_access Id();
    int& Id(int);

    integer_access IntegerData(int);
    integer_access IntegerData(const string&);

    // --- Constant accessors

    // const RealType** X() const;
    // const RealType*  X(int) const;
    // const RealType&  X(int, int) const;
    // const RealType** V() const;
    // const RealType*  V(int) const;
    // const RealType&  V(int, int) const;
    // const RealType** F() const;
    // const RealType*  F(int) const;
    // const RealType&  F(int, int) const;

    // const RealType* Sg() const;
    // const RealType& Sg(int) const;
    // const RealType* Im() const;
    // const RealType& Im(int) const;

    // const int* Type() const;
    // const int& Type(int) const;
    // const int* Id() const;
    // const int& Id(int) const;

    bool Valid(int) const;

    // --- Data creation and request

    // The request versions get the data entry's place if it exists, and creates it if it doesn't.
    int requestVectorData(string);
    int requestScalarData(string);
    int requestIntegerData(string);
    
    // The get versions do not *create* the data entries, they just look for them.
    int getVectorData(string);
    int getScalarData(string);
    int getIntegerData(string);

    // --- Halo and Ghost particles

    //! \brief Create a halo particle of a certain particle, with a certain displacement from the original particle.
    void createHaloOf(int, const Vec&);

    //! \brief Mark all halo particles for removal and clear halo particle data.
    void removeHaloParticles();

    //! \brief Mark all ghost particles for removal and clear ghost particle data.
    void removeGhostParticles();

    //! \brief Remove both halo and ghost particles.
    //!
    //! This simply calls remove_halo_particles and remove_ghost_particles.
    void removeHaloAndGhostParticles();

    //! \brief Update simdata, migrate particles to other processors, handle assignment and initialization of ghost particles.
    void update();

    // --- Particle size information

    //! \brief The size of the part of the arrays that may contain valid particles.
    int size() const;

    //! \brief The size of the part of the arrays that may contained valid owned (non-ghost) particles.
    int size_owned() const;

    //! \brief Return the number of particles on the processor.
    int number() const;

    //! \brief Returns the number of owned particles on this processor (does not count halo or ghost particles).
    int number_owned() const;

    //! \brief Return the number of ghost particles on this processor.
    int number_ghosts() const;

    //! \brief Get the index of the first ghost particle.
    int first_ghost() const;

    //! \brief Return the number of types of particles.
    int ntypes() const;

    // --- Clear entries

    //! \brief Set all velocities to zero.
    void clearV();

    //! \brief Set all forces to zero.
    void clearF();

    //! \brief Set all entries in a scalar data list to zero, if the entry exists.
    void clearScalar(const string);

    // --- Other accessors

    //! \brief Get the local id of a particle given the global id.
    //!
    //! The local id is where in the array is the particle stored. The global id is a unique identifier for
    //! every particle that has ever existed in the simulation.
    int getLocalID(int) const;

    //! \brief Get what will be the global id of the next particle added to the system.
    int getNextGlobalID() const;

    //! \brief Get the boundary conditions.
    //!
    //! These are actually stored in the gflow object, so we pass them on from gflow.
    const BCFlag* getBCs() const;

    //! \brief Get the simulation bounds
    //!
    //! These are actually stored in the gflow object, so we pass them on from gflow.
    Bounds getBounds() const;

    //! \brief Get the needs remake flag.
    bool getNeedsRemake();

    //! \brief Get the position of the first halo particle in the array.
    int getFirstHalo();

    //! \brief Get the position of the first ghost particle in the array.
    int getFirstGhost();

    //! \brief Get the last number of ghosts that we sent to other processors.
    int getLastNGhostsSent();

    //! \brief Get the last number of ghosts that we received from other processors.
    int getLastNGhostsRecv();

    //! \brief Get the last number of exchange particles that we sent to other processors.
    int getLastNExchangeSent();

    //! \brief Get the last number of exchange particles that we received from other processors.
    int getLastNExchangeRecv();

    //! \brief Is this the local id of an owned particle.
    //!
    //! If isReal is true, isHalo and isGhost will be false.
    bool isReal(int);

    //! \brief Is this the local id of a halo particle.
    bool isHalo(int);

    //! \brief Is this the local id of a ghost particle.
    bool isGhost(int);

    // --- Mutators

    //! \brief Set the position of the first halo particle in the array.
    void setFirstHalo(int);

    //! \brief Set the position of the first ghost particle in the array.
    void setFirstGhost(int);

    //! \brief Set the needs remake flag.
    void setNeedsRemake(bool=true);

    //! \brief Add a vector data entry.
    void addVectorData(string);

    //! \brief Add a scalar data entry.
    void addScalarData(string);

    //! \brief Add an integer data entry.
    void addIntegerData(string);

    //! \brief Set the send ghost velocity flag.
    void setSendGhostVelocity(bool b) { 
      #if USE_MPI==1
      send_ghost_velocity = b; 
      #endif
    }
    //! \brief Set the send ghost omega flag.
    void setSendGhostOmega(bool b) { 
      #if USE_MPI==1
      send_ghost_omega = b; 
      #endif
    }

    friend class ForceMaster;
    friend class DataMaster;

    // --- MPI related timers.

    TimedObject barrier_timer;
    TimedObject send_timer;
    TimedObject recv_timer;
    TimedObject ghost_send_timer;
    TimedObject ghost_recv_timer;
    TimedObject ghost_wait_timer;
    TimedObject exchange_search_timer;
    TimedObject ghost_search_timer;

  private:
    // --- Helper functions.

    //! \brief Allocate more space to hold owned particles.
    void resize_owned(int);

    //! \brief Set all values for a particle to default values
    void reset_particle(int);

    //! \brief Swap two particle's data.
    void swap_particle(int, int);

    //! \brief Partitions the sublist, then calls sort on each sublist.
    void quick_sort(int, int, int);

    //! \brief The partition step for quicksort
    int quick_sort_partition(int, int, int);

    //! \brief Recursively sorts by dimension.
    void recursion_help(int, int, int);

    // --- Data

    //! \brief A flag that can be set to true whenever something happens that might invalidate the current data.
    bool needs_remake = false;

    // -*-*-*- Particle data -*-*-*-

    //! \brief Vector data.
    //!
    //! Contains postion (0), velocity (1), and force (2).
    vector<RealType**> vdata;

    //! \brief Scalar data.
    //! 
    //! Contains sigma (0), inverse mass (1). Can also contain repulsion, dissipation, coefficient of friction, etc.
    vector<RealType*> sdata;

    //! \brief Integer data.
    //!
    //! Contains type (0), global id (1). Can also contain body membership information, etc.
    vector<int*> idata;

    // -*-*-*- Data mapping -*-*-*-

    // Map names to entries in the data vectors.
    std::map<string, int> vector_data_map;
    std::map<string, int> scalar_data_map;
    std::map<string, int> integer_data_map;

    // -*-*-*- Ids -*-*-*-

    //! \brief The next global id a particle will be given.
    int next_global_id = 0;

    //! \brief A map between global and local ids, <global, local>.
    std::unordered_map<int, int> id_map;

    //! \brief Whether to use the id_map.
    bool use_id_map = true;

    //! \brief A map between local halo particle ids and primary (local) IDs.
    std::vector<int> halo_map;

    //! \brief Record where "holes" are in the particle array
    std::set<int> remove_list;

    // -*-*-*- Numbers -*-*-*-

    //! \brief Number of particles on this processor. Counts halo or ghost particle.
    int _number = 0; 
    //! \brief Number of halo particle on this processor.
    int _number_halo = 0;
    //! \brief Number of ghost particles on this processor.
    int _number_ghost = 0;

    //! \brief The last part of the array that might contain valid particles.
    //!
    //! Often, this might be the entry after the last valid particle on the processor. However, if the last valid particle was deleted, 
    //! this might not be the case.
    int _size = 0;

    //! \brief The total capacity of the particle data arrays.
    int _capacity = 0;

    //! \brief The position of the first halo particle. This assumes that halo particles are stored contiguously.
    //!
    //! If this value is == to _size, then there are no halo particles.
    int _first_halo = 0;

    //! \brief The position of the first ghost particle. This assumes that ghost particles are stored contiguously.
    //!
    //! If this value is == to _size, then there are no ghost particles.
    int _first_ghost = 0;

    //! \brief Copy this data from force master.
    int _ntypes = 0;

    // -*-*-*- MPI/Parallel related -*-*-*-
    #if USE_MPI == 1

    // Tags
    const int send_size_tag = 0;
    const int send_particle_tag = 1;
    const int send_ghost_tag = 2;
    const int update_ghost_tag = 3;

    //! \brief Exchange particles that belong to other processors.
    //!
    //! Move particles that belong to other domains to those domains, and delete them from here. Then recieve
    //! particles from other domains that belong to this domain.
    inline void exchange_particles();

    //! \brief Figure out which particles should be ghost particles, and send copies of them to neighboring processors.
    inline void create_ghost_particles();

    //! \brief Send ghost particle updates to other processors.
    inline void send_ghost_updates();
    //! \brief Start receiving ghost particle updates.
    inline void start_ghost_recv();
    //! \brief Receive ghost particle updates from other processors and store data.
    inline void recv_ghost_updates();

    //! \brief Pack up the particle data for the specified ids and send it to another processor, optionally deleting the original particles
    //! from this processor.
    inline void send_particle_data(const vector<int>&, int, vector<RealType>&, MPI_Request*, int, bool=false);

    //! \brief Send particles, so that their position relative to the other processor is minimal. Used for sending ghost particles.
    inline void send_particle_data_relative(const vector<int>&, int, vector<RealType>&, MPI_Request*, int, int);

    //! \brief Recieve particle information, and use it to create new particles. Return the number of particles recieved.
    inline int recv_new_particle_data(int, vector<RealType>&, int);

    //! \brief Width of data to send when migrating particles.
    int data_width = 0;

    //! \brief Width of data to send when sending ghost particle information.
    int ghost_data_width = 0;

    //! \brief All the processors that are neighbors.
    vector<int> neighbor_ranks;

    //! \brief Maps rank to position in neighbor_ranks.
    std::map<int, int> neighbor_map;

    /// Sending data

    //! \brief The i-th vector in the array contains ids of the particles that should be sent from this processor
    //! to the i-th neighboring processor.
    vector<vector<int> > send_ids;

    /// Recieving data.

    //! \brief The i-th entry in the vector contains the number of particles (NOT data size) that we need to send to 
    //! the i-th neighboring processor.
    vector<int> send_size;
    
    //! \brief The i-th entry in the vector contains the number of particles (NOT data size) that the i-th neighboring 
    //! processor will send to this processor.
    vector<int> recv_size;

    //! \brief Record the last number of ghost particles we had to send (so we can keep track).
    int _last_n_ghosts_sent = 0;
    //! \brief Record the last number of ghost particles we had to receive (so we can keep track).
    int _last_n_ghosts_recv = 0;
    //! \brief Record the last number of particles exchanged to other processors.
    int _last_n_exchange_sent = 0;
    //! \brief Record the last number of particles exchanged from other processors.
    int _last_n_exchange_recv = 0;

    //! \brief List of particles to send as ghosts to neighboring processors. The i-th entry is for the i-th neighbor. The processor id
    //! can be found by asking the topology (for now this only applies to KD tree, but it probably should be changed to apply to all types
    //! of topologies).
    vector<vector<int> > send_ghost_list;

    //! \brief How many ghost particles we will recieve from each neighbor processor.
    vector<int> recv_ghost_sizes;

    /// Sending and receiving data.

    //! \brief A buffer for sending data.
    vector<vector<RealType>> buffer_list;

    //! \brief A buffer for receiving data.
    vector<vector<RealType> > recv_buffer;

    vector<MPI_Request> recv_request_list;
    vector<MPI_Request> send_request_list;

    // --- What types of data to send to adjacent processors.

    //! \brief Whether ghost particles' velocity should be sent.
    bool send_ghost_velocity = false;
    //! \brief Whether ghost particles' angular velocity should be sent.
    bool send_ghost_omega = false;
    #endif
  };


  //! \brief Struct for accessing the underlying vector data of SimData in a layout agnostic manner.
  struct SimData::vec_access {
    //! \brief Create a null vec_access object, to be assigned later.
    vec_access() {};

    //! \brief Access a vector.
    real* operator() (const int id) { return data_ptr[id]; }
    const real* operator() (const int id) const { return data_ptr[id]; }
    //! \brief Access a component of a vector.
    real& operator() (const int id, const int d) { return data_ptr[id][d]; }
    real operator() (const int id, const int d) const { return data_ptr[id][d]; }
    //! \brief Access a component via contiguous index.
    real& operator[] (const int contiguous_index) { return (*data_ptr)[contiguous_index]; }
    real operator[] (const int contiguous_index) const { return (*data_ptr)[contiguous_index]; }

    //! \brief Load contiguous entries into simd vector
    simd_float load_to_simd(const int contiguous_index) { return simd_load_u(&data_ptr[0][contiguous_index]); }

    //! \brief Store from a simd vector into contiguous entries.
    void store_simd(const int contiguous_index, const simd_float value) { simd_store_u(value, &data_ptr[0][contiguous_index]); }

    //! \brief Return whether the underlying pointer is null.
    bool isnull() { return data_ptr==nullptr; }

    friend SimData;

  private:
    //! \brief Private constructor, so only ParticleContainer can create a vec_access.
    vec_access(real** data) : data_ptr(data) {};

    //! \brief Pointer to the underlying data.
    real **data_ptr = nullptr;
  };

  //! \brief Struct for accessing the underlying scalar data of SimData in a layout agnostic manner.
  struct SimData::scalar_access {
    //! \brief Create a null scalar_access object, to be assigned later.
    scalar_access() {};

    real& operator() (const int i) { return data_ptr[i]; }
    real operator() (const int i) const { return data_ptr[i]; }

    real& operator[] (const int i) { return data_ptr[i]; }
    real operator[] (const int i) const { return data_ptr[i]; }

    //! \brief Load data to a simd vector such that the data aligns with the dimensionality of vectors.
    //!
    //! For example, if sim_dimensions==2, and we are using SSE, and 2 | k, im.valign_load_to_simd(k) -> { im[2*k], im[k], im[k+1], im[k+1] }.
    template<int dims> 
    simd_float valign_load_to_simd(const int contiguous_index) {
      #if SIMD_TYPE==SIMD_NONE
      return data_ptr[contiguous_index];
      #elif SIMD_TYPE==SIMD_SSE3
      return _mm_set_ps(
        data_ptr[(contiguous_index+3)/dims], 
        data_ptr[(contiguous_index+2)/dims],
        data_ptr[(contiguous_index+1)/dims], 
        data_ptr[contiguous_index/dims]
      );
      #elif SIMD_TYPE==AVX || SIMD_TYPE==AVX2
      return _mm256_set_ps(
        data_ptr[(contiguous_index+7)/dims],
        data_ptr[(contiguous_index+6)/dims],
        data_ptr[(contiguous_index+5)/dims],
        data_ptr[(contiguous_index+4)/dims],
        data_ptr[(contiguous_index+3)/dims],
        data_ptr[(contiguous_index+2)/dims],
        data_ptr[(contiguous_index+1)/dims],
        data_ptr[contiguous_index/dims],
      );
      #elif SIMD_TYPE==SIMD_MIC
      return _mm512_set_ps(
        data_ptr[(contiguous_index+15)/dims],
        data_ptr[(contiguous_index+14)/dims],
        data_ptr[(contiguous_index+13)/dims],
        data_ptr[(contiguous_index+12)/dims],
        data_ptr[(contiguous_index+11)/dims],
        data_ptr[(contiguous_index+10)/dims],
        data_ptr[(contiguous_index+9)/dims],
        data_ptr[(contiguous_index+8)/dims]
        data_ptr[(contiguous_index+7)/dims],
        data_ptr[(contiguous_index+6)/dims],
        data_ptr[(contiguous_index+5)/dims],
        data_ptr[(contiguous_index+4)/dims],
        data_ptr[(contiguous_index+3)/dims],
        data_ptr[(contiguous_index+2)/dims],
        data_ptr[(contiguous_index+1)/dims],
        data_ptr[contiguous_index/dims]
      );
      #endif
    }

    //! \brief Load a series of contiguous data to a simd vector.
    simd_float load_to_simd(const int contiguous_index) { return simd_load_u(&data_ptr[contiguous_index]); }

    //! \brief Store a simd vector to contiguous data.
    void store_simd(const int contiguous_index, simd_float value) { simd_store_u(value, &data_ptr[contiguous_index]); }

    //! \brief Return whether the underlying pointer is null.
    bool isnull() { return data_ptr==nullptr; }

    friend SimData;

  private:
    //! \brief Private constructor, so only ParticleContainer can create a scalar_access.
    scalar_access(real* data) : data_ptr(data) {};

    //! \brief Pointer to the underlying data.
    real *data_ptr = nullptr;
  };

  //! \brief Struct for accessing the underlying integer data of SimData in a layout agnostic manner.
  struct SimData::integer_access {
    //! \brief Create a null integer_access object, to be assigned later.
    integer_access() {};

    int& operator() (const int i) { return data_ptr[i]; }
    int operator() (const int i) const { return data_ptr[i]; }

    int& operator[] (const int i) { return data_ptr[i]; }
    int operator[] (const int i) const { return data_ptr[i]; }

    //! \brief Return whether the underlying pointer is null.
    bool isnull() { return data_ptr==nullptr; }

    friend SimData;

  private:
    //! \brief Private constructor, so only ParticleContainer can create a scalar_access.
    integer_access(int* data) : data_ptr(data) {};

    //! \brief Pointer to the underlying data.
    int *data_ptr = nullptr;
  };

  typedef SimData::vec_access vec_access;
  typedef SimData::scalar_access scalar_access;
  typedef SimData::integer_access integer_access;

}


#endif // __ATOMS_HPP_GFLOW__
