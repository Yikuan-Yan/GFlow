#include "simdata.hpp"
// Other files
#include "../utility/memory.hpp"

namespace GFlowSimulation {

  SimData::SimData(GFlow *gflow) : Base(gflow), bounds(Bounds(2)) {
    // Initialize vdata array
    vdata = vector<RealType**>(3, nullptr);
    // Initialize sdata array
    sdata = vector<RealType*>(2, nullptr);
    // Initialize idata array
    idata = vector<int*>(2, nullptr);
    // Set up bounds to have the propper dimensions
    bounds = Bounds(sim_dimensions);
  }

  SimData::~SimData() {
    for (auto &v : vdata)
      dealloc_array_2d(v);
    for (auto &s : sdata)
      if (s) delete [] s;
    for (auto &i : idata)
      if (i) delete [] i;
  }

  //! @brief Initialize the atom container.
  void SimData::initialize() {
    Base::initialize();

    // For now
    bounds = gflow->getBounds();
  }

  //! @brief Reserve space for particles, extending the lengths of all arrays to the requested size.
  void SimData::reserve(int num) {
    for (auto &v : vdata) {
      if (v) dealloc_array_2d(v);
      v = alloc_array_2d<RealType>(num, sim_dimensions);
    }
    for (auto &s : sdata) {
      if (s) delete [] s;
      s = new RealType[num];
    }
    for (auto &i : idata) {
      if (i) delete [] i;
      i = new int[num];
    }
    // Reset numbers
    _number = 0;
    _size = 0;
    _capacity = num;
  }

  void SimData::addParticle() {
    if (_size+1 > _capacity) resize_owned(32);
    // Zero all data
    for (auto v : vdata)
      zeroVec(v[_size], sim_dimensions);
    for (auto s : sdata)
      s[_size] = 0;
    // Set type, give a global id
    Type(_size) = 0;
    Id(_size) = next_global_id++;
    ++_number;
    ++_size;
  }

  void SimData::addParticle(int num) {
    if (num<=0) return;
    if (_size+num > _capacity) resize_owned(num+_size-_capacity+32);
    for (int i=0; i<num; ++i) {
      // Zero all data
      for (auto v : vdata)
        zeroVec(v[_size], sim_dimensions);
      for (auto s : sdata)
        s[_size] = 0;
      // Set type, give a global id
      Type(_size) = 0;
      Id(_size) = next_global_id++;
      ++_number;
      ++_size;
    }
  }

  void SimData::addParticle(const RealType *x, const RealType *v, const RealType sg, const RealType im, const int type) {
    // If not enough spots to add a new owned particle, create more
    if (_size>=_capacity) resize_owned(32);
    copyVec(x, X(_size), sim_dimensions);
    copyVec(v, V(_size), sim_dimensions);
    zeroVec(F(_size), sim_dimensions);
    Sg(_size) = sg;
    Im(_size) = im;
    Type(_size) = type;
    Id(_size) = next_global_id++;
    ++_number;
    ++_size;
  }

  void SimData::markForRemoval(const int id) {
    if (Type(id)<0) return;
    // Mark for removal, clear some data
    remove_list.insert(id);
    Type(id) = -1;
    id_map.erase(Id(id));
    Id(id) = -1;
    zeroVec(V(id), sim_dimensions);
    zeroVec(F(id), sim_dimensions);
  }

  void SimData::doParticleRemoval() {
    // If there is nothing to remove, we're done
    if (remove_list.empty() || _number==0) return;
    // Variables
    int count = 0, count_back = _size, need_removal = 0;
    // Set all types to -1, remove global ids
    for (auto id : remove_list) {
      if (Type(id)!=-1) ++need_removal;
      Type(id) = -1;
    }
    // Fill in all holes
    int removed = 0;
    for(auto id : remove_list) {
      // We either need to start at (_size-1), or moved the particle that was at count_back. Either way, decrement.
      --count_back;
      // We have removed a particle
      ++removed;

      // Find the next valid particle (counting back from the end) to fill for the particle we want to remove
      // C++ 20 has a std::set contains() function.
      while ( contains(remove_list, count_back) && count_back>id) --count_back;

      // Move the particle to fill the particle we want to remove - moving the good particle to the hole erases the hole's 
      // global id.
      if (count_back>id) move_particle(count_back, id);
      else break;
    }
    // Decrease number
    _number -= need_removal;

    // Clear list
    remove_list.clear();
    // We need to update
    if (removed>0) needs_remake = true;
  }

  void SimData::exchangeParticles() {
    // @todo Implement
  }

  void SimData::updateHaloParticles() {
    for (int i=0; i<halo_map.size(); i+=2) {
      int hid = halo_map[i];
      int pid = halo_map[i+1];
      // Update force
      plusEqVec(vdata[2][pid], vdata[2][hid], sim_dimensions);
      // Clear halo particle force record
      zeroVec(vdata[2][hid], sim_dimensions);
    }
  }

  RealType** SimData::X() {
    return vdata[0];
  }

  RealType* SimData::X_arr() {
    return *vdata[0];
  }

  RealType* SimData::X(int i) {
    return vdata[0][i];
  }

  RealType& SimData::X(int i, int d) {
    return vdata[0][i][d];
  }

  RealType** SimData::V() {
    return vdata[1];
  }

  RealType* SimData::V_arr() {
    return *vdata[1];
  }

  RealType* SimData::V(int i) {
    return vdata[1][i];
  }

  RealType& SimData::V(int i, int d) {
    return vdata[1][i][d];
  }

  RealType** SimData::F() {
    return vdata[2];
  }

  RealType* SimData::F_arr() {
    return *vdata[2];
  }

  RealType* SimData::F(int i) {
    return vdata[2][i];
  }

  RealType& SimData::F(int i, int d) {
    return vdata[2][i][d];
  }

  RealType* SimData::Sg() {
    return sdata[0];
  }

  RealType& SimData::Sg(int i) {
    return sdata[0][i];
  }

  RealType* SimData::Im() {
    return sdata[1];
  }

  RealType& SimData::Im(int i) {
    return sdata[1][i];
  }

  int* SimData::Type() {
    return idata[0];
  }

  int& SimData::Type(int i) {
    return idata[0][i];
  }

  int* SimData::Id() {
    return idata[1];
  }

  int& SimData::Id(int i) {
    return idata[1][i];
  }

  int SimData::size() {
    return _number;
  }

  int SimData::number() {
    return _number;
  }

  int SimData::ntypes() {
    return _ntypes;
  }

  void SimData::clearF() {
    for (int i=0; i<_size*sim_dimensions; ++i)
      vdata[1][0][i] = 0;
  }

  int SimData::getLocalID(int global) const {
    auto it = id_map.find(global);
    // Return the global iterator. We use -1 to mean "no such particle."
    return it==id_map.end() ? -1 : it->second;
  }

  int SimData::getNextGlobalID() const {
    return next_global_id;
  }

  const BCFlag* SimData::getBCs() const {
    return Base::gflow->getBCs();
  }

  Bounds SimData::getBounds() const {
    return bounds;
  }

  bool SimData::getNeedsRemake() {
    return needs_remake;
  }

  void SimData::setNeedsRemake(bool r) {
    needs_remake = r;
  }

  void SimData::resize_owned(int num) {
    // Compute new capacity
    int new_capacity = _capacity + num;
    // Allocate new vector data arrays
    for (auto &v : vdata) {
      RealType **nv = alloc_array_2d<RealType>(new_capacity, sim_dimensions);
      // Transfer data
      copyVec(v, nv, _size);
      // Delete old array, set new
      if(v) delete [] v;
      v = nv;
    }
    // Allocate new scalar data arrays
    for (auto &s : sdata) {
      RealType *ns = new RealType[new_capacity];
      // Transfer data
      copyVec(s, ns, _size);
      // Delete old array, set new
      if (s) delete [] s;
      s = ns;
    }
    // Allodate new integer data
    // Allocate new scalar data arrays
    for (auto &i : idata) {
      int *ni = new int[new_capacity];
      // Transfer data
      copyVec(i, ni, _size);
      // Delete old array, set new
      if (i) delete [] i;
      i = ni;
    }

    // Set new sizes
    _capacity += num;
  }

  void SimData::move_particle(int src, int dst) {
    // Get global IDs
    int gs = Id(src);
    int gd = Id(dst);
    // If we are overwriting a valid particle
    if (Type(dst)>-1) {
      --_number;
      id_map.erase(gd);
    }
    // Transfer data
    for (auto v : vdata)
      copyVec(v[src], v[dst], sim_dimensions);
    for (auto s : sdata)
      s[dst] = s[src];
    for (auto i : idata)
      i[dst] = i[src];

    // Remap global id
    auto it = id_map.find(gs);
    if (id_map.end()!=it)
      it->second = dst;
    else throw false; // \todo Make an exception for this case.

    // We have invalidated the local id
    needs_remake = true;
  }

}