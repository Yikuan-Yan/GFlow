#include "force.hpp"
// Other files
#include "simdata.hpp"
#include "vectormath.hpp"

namespace GFlowSimulation {

  Force::Force(GFlow *gflow) : Base(gflow) {};

  Force::~Force() {}

  int Force::vlSize() const {
    return verletList.vlSize();
  }

  const VerletList& Force::getVerletList() const {
    return verletList;
  }

  void Force::clearVerletList() {
    verletList.clear();
  }

  void Force::addVerletPair(int id1, int id2) {
    // Add the head if it is new
    verletList.addPair(id1, id2);
  }

}