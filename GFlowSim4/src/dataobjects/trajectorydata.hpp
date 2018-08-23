#ifndef __TRAJECTORY_DATA_HPP__GFLOW__
#define __TRAJECTORY_DATA_HPP__GFLOW__

#include "../base/dataobject.hpp"

#include <map>

template<int dimensions> struct PData {
  RealType X[dimensions];
  RealType V[dimensions];
  RealType sg;
  int type;
};

namespace GFlowSimulation {

  class TrajectoryData : public DataObject {
    //! @brief Constructor
    TrajectoryData(GFlow*);

    //! @brief Destructor
    ~TrajectoryData();

    //! @brief Collect the position data from simdata --- happens during the post-step phase
    virtual void post_step();

    //! @brief Write data to a file - if true, the string is a path, and you should use your own name as the file name.
    // Returns true for success.
    virtual bool writeToFile(string, bool=true);

  private:

    //! @brief Map of particle id, data
    std::map<int, vector<PData> > data;

  };

}
#endif // __TRAJECTORY_DATA_HPP__GFLOW__