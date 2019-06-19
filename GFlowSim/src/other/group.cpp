#include "group.hpp"
// Other files
#include "../base/simdata.hpp"

namespace GFlowSimulation {

  Group::Group(const Group &g) {
    global_ids = g.global_ids;
    local_ids = g.local_ids;
  }

  Group& Group::operator=(const Group &g) {
    global_ids = g.global_ids;
    local_ids = g.local_ids;
    // Return
    return *this;
  }

  int Group::size() const {
    return global_ids.size();
  }

  bool Group::empty() const {
    return global_ids.empty();
  }

  int Group::at(int i) const {
    return local_ids.at(i);
  }
    
  int Group::g_at(int i) const {
    return global_ids.at(i);
  }

  void Group::findCenterOfMass(RealType *rcm, SimData *simData) const {
    if (size()==0 || simData==nullptr) return;
    // Get the dimensionaliry
    int sim_dimensions = simData->getSimDimensions();
    // Dispacement vector
    Vec dr(sim_dimensions), r0(sim_dimensions), Rcm(sim_dimensions);

    // Get the bounds and boundary conditions
    GFlow *gflow = simData->getGFlow();
    Bounds bounds = gflow->getBounds(); // Simulation bounds

    // Get position, mass arrays
    RealType **x = simData->X();
    RealType *im = simData->Im();
    RealType mass = 0;

    // Set reference point
    int mid = floor(local_ids.size()/2);
    copyVec(x[local_ids[mid]], r0.data, sim_dimensions);
    // Go through all particles
    for (auto id : local_ids) {
      // Get displacement between the particle and the reference point.
      gflow->getDisplacement(x[id], r0.data, dr.data);
      // Get the mass of the particle - assumes none of the particles have infinite mass.
      RealType m = 1./im[id];
      mass += m;
      // Update Rcm
      plusEqVecScaled(Rcm.data, dr.data, m, sim_dimensions);
    }
    // Divide by total mass and add to reference position to get the actual com position.
    scalarMultVec(1./mass, Rcm.data, sim_dimensions);
    // Add to reference point, and do wrapping
    Rcm += r0;

    // Harmonic boundary conditions.
    for (int d=0; d<sim_dimensions; ++d) {
      if (Rcm[d]<bounds.min[d]) Rcm[d] += bounds.wd(d);
      else if (bounds.max[d]<Rcm[d]) Rcm[d] -= bounds.wd(d);
    }

    copyVec(Rcm, rcm);
  }

  void Group::findCOMVelocity(RealType *v, SimData *simData) const {
    if (size()==0) return;
    // Get the dimensionaliry
    int sim_dimensions = simData->getSimDimensions();
    // Zero vector
    zeroVec(v, sim_dimensions);
    // Compute new momentum
    RealType mass = 0;
    for (auto id : local_ids) {
      RealType m = 1./simData->Im(id);
      plusEqVecScaled(v, simData->V(id), m, sim_dimensions);
      mass += m;
    }
    // Normalize
    scalarMultVec(1./mass, v, sim_dimensions);
  }

  void Group::addAcceleration(RealType *A, SimData *simData) const {
    if (size()==0) return;
    // Get the dimensionaliry
    int sim_dimensions = simData->getSimDimensions();

    // Get arrays
    RealType **f = simData->F();
    RealType *im = simData->Im();

    for (auto id : local_ids) {
      if (im[id]>0) {
        RealType mass = 1./im[id];
        plusEqVecScaled(f[id], A, mass, sim_dimensions);
      }
    }
  }

  int Group::getIndex(int id) const {
    if (correspondence.empty()) return -1;
    auto it = correspondence.find(id);
    if (it==correspondence.end()) return -1;
    return it->second;
  }

  void Group::add(int id) {
    if (use_correspondence) correspondence.insert(pair<int, int>(id, local_ids.size()));
    global_ids.push_back(id);
    local_ids.push_back(id);
  }

  void Group::findNetForce(RealType *frc, SimData *simData) const {
    if (size()==0) return;
    // Get the dimensionaliry
    int sim_dimensions = simData->getSimDimensions();
    // Zero vector
    zeroVec(frc, sim_dimensions);
    // Force array
    RealType **f = simData->F();
    // Compute net force
    for (auto id : local_ids) {
      plusEqVec(frc, f[id], sim_dimensions);
    }
  }

  void Group::findClosestObject(const RealType *point, RealType *displacement, SimData *simData) const {
    if (size()==0) return;
    // Force array
    int sim_dimensions = simData->getSimDimensions();
    RealType **x = simData->X();
    Vec disp(sim_dimensions), maxDisp(sim_dimensions);
    RealType maxD = 0;
    // Compute net force
    for (auto id : local_ids) {
      simData->getGFlow()->getDisplacement(x[id], point, disp.data);
      RealType d = sqr(disp);
      if (d>maxD) {
        maxD = d;
        maxDisp = disp;
      }
    }
    copyVec(maxDisp, displacement);
  }

  void Group::update_local_ids(SimData *simData) const {
    // Make sure sizes are the same
    int _size = size();
    // Update local ids
    for (int i=0; i<_size; ++i) {
      int gid = global_ids[i];;
      int lid = simData->getLocalID(gid);
      local_ids[i] = lid;
    }
    // Redo the correspondence set.
    redo_correspondence();
  }

  void Group::setUseCorrespondence(bool u) {
    if (u) redo_correspondence();
    else correspondence.clear();
    use_correspondence = u;
  }

  void Group::redo_correspondence() const {
    if (!use_correspondence) return;
    // Redo the correspondence set.
    for (int i=0; i<local_ids.size(); ++i) {
      auto it = correspondence.find(local_ids[i]);
      if (it==correspondence.end()) correspondence.insert(pair<int, int>(local_ids[i], i));
      else it->second = i;
    }
  }

}