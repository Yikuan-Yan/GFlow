#include "Integrator.hpp"

namespace GFlow {
  Integrator::Integrator() : dt(1e-4), time(0), runTime(0), iter(0), maxIter(0), simData(0), dataRecord(0) {};

  void Integrator::initialize() {
    // Set run time
    if (simData) runTime = simData->getRunTime();
    else runTime = 0;

    // Set max iter
    if (dt>0) maxIter = runTime/dt;
    else maxIter = 0;
  }
  
}
