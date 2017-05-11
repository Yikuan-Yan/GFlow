#include "DataRecord.hpp"

namespace GFlow {
  DataRecord::DataRecord() : delay(1./15.), delayTimer(0) {};

  void DataRecord::update(RealType dt) {
    delayTimer += dt;
  }

  void DataRecord::record(SimData* simData) {
    if (delayTimer<delay) return;
    // Record data
    
  }
  
}
