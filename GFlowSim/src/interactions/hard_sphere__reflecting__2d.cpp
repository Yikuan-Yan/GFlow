#include "hard_sphere__reflecting__2d.hpp"
// Other files
#include "../interactionhandlers/verletlist-pairs.hpp"

namespace GFlowSimulation {

  HardSphere_Reflecting_2d::HardSphere_Reflecting_2d(GFlow *gflow) : Interaction(gflow, new VerletListPairs(gflow)) {};

  void HardSphere_Reflecting_2d::interact() const {
    // Do dimensional check.
    // \todo Should probably have some sort of global error message system.
    if (sim_dimensions!=2) return;

    // Get the data pointers.
    RealType **x = Base::simData->X();
    RealType **v = Base::simData->V();
    RealType *sg = Base::simData->Sg();
    RealType *im = Base::simData->Im();
    int    *type = Base::simData->Type();

    // Make sure all needed pointers are non null.
    // \todo Should probably have some sort of global error message system.
    if (x==nullptr || v==nullptr || sg==nullptr || im==nullptr || type==nullptr) return;

    // Get the bounds and boundary conditions
    Bounds bounds = Base::gflow->getBounds(); // Simulation bounds
    BCFlag boundaryConditions[2];
    copyVec(Base::gflow->getBCs(), boundaryConditions, 2); // Keep a local copy of the bcs
    // Extract bounds related data
    RealType bnd_x = bounds.wd(0);
    RealType bnd_y = bounds.wd(1);

    // Needed constants
    RealType sg1, sg2, dx, dy, rsqr, r, invr, magnitude;
    // Point to the actual list from the verlet list object. Since we set the handler at initialization to 
    // be of type VerletListPairs, this cast should always succeed.
    vector<int> &verlet = dynamic_cast<VerletListPairs*>(handler)->verlet;

    // --- Go through all particles
    for (int i=0; i<verlet.size(); i+=2) {
      int id1 = verlet[i];
      int id2 = verlet[i+1];
      // Check if the types are good
      if (type[id1]<0 || type[id2]<0) continue;
      // Calculate displacement.
      dx = x[id1][0] - x[id2][0];
      dy = x[id1][1] - x[id2][1];
      // Harmonic corrections to distance.
      if (boundaryConditions[0]==BCFlag::WRAP) {
        RealType dX = bnd_x - fabs(dx);
        if (dX<fabs(dx)) dx = dx>0 ? -dX : dX;
      }  
      if (boundaryConditions[1]==BCFlag::WRAP) {
        RealType dY = bnd_y - fabs(dy);
        if (dY<fabs(dy)) dy = dy>0 ? -dY : dY;
      }  
      // Calculate squared distance
      rsqr = dx*dx + dy*dy;
      // Get radii
      sg1 = sg[id1];
      sg2 = sg[id2];
      // If close, interact.
      if (rsqr < sqr(sg1 + sg2)) {
        // Calculate distance, inverse distance.
        r = sqrt(rsqr);
        invr = 1./r;
        // Create a normal vector
        dx *= invr;
        dy *= invr;

        // Initial projected velocities
        RealType v1 = v[id1][0]*dx + v[id1][1]*dy;
        RealType v2 = v[id2][0]*dx + v[id2][1]*dy;

        // Relative velocities
        RealType dvx = v[id1][0] - v[id2][0];
        RealType dvy = v[id1][1] - v[id2][1];

        // Only interact if particles are going towards each other.
        if (dx*dvx+dy*dvy>0) continue;

        // \todo Correctly calculate final velocities based on momentum.

        // Elastic collision results in normal momenta reflecting
        v[id1][0] -= 2*v1*dx;
        v[id1][1] -= 2*v1*dy;

        v[id2][0] -= 2*v2*dx;
        v[id2][1] -= 2*v2*dy;
      }
    }
  }

}