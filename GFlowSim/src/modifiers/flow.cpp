#include "flow.hpp"
// Other files

namespace GFlowSimulation {

  Flow::Flow(GFlow *gflow) : Modifier(gflow), drag(0.1) {};

  void Flow::pre_forces() {
    // Create widths array
    RealType *width = new RealType[sim_dimensions];
    for (int d=0; d<sim_dimensions; ++d)
      width[d] = Base::gflow->getBounds().wd(d);

    // Number of (real - non ghost) particles
    int size = simData->size();
    // Get arrays
    RealType *x = simData->X_arr(), *v = simData->V_arr(), *f = simData->F_arr(), *sg = simData->Sg();

    // Update velocities
    for (int i=0; i<size*sim_dimensions; ++i) {
      int d = i % sim_dimensions;
      int id = i/sim_dimensions;
      // Dimension 0 is the flow direction
      RealType mask = (d==0 ? 0. : 1.);
      // Find the target velocity 
      RealType target = 1.-4.*sqr((x[i] - 0.5*width[d])/width[d]);
      // Drag is proportional to difference between target and actual velocity
      f[i] -= drag*(mask*target - v[i])*sg[id];
    }

    // Clean up
    delete [] width;
  }

}