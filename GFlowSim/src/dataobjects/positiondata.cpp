#include "positiondata.hpp"
// Other files
#include "../utility/printingutility.hpp"
#include "../base/simdata.hpp"

#include "../visualization/visualization.hpp"

namespace GFlowSimulation {
  // Constructor
  PositionData::PositionData(GFlow *gflow) : DataObject(gflow, "Pos") {
    // The data to gather
    vector_data_entries.push_back("X");
    vector_data_entries.push_back("V");
    scalar_data_entries.push_back("Sg");
    scalar_data_entries.push_back("StripeX");
    integer_data_entries.push_back("Type");
    integer_data_entries.push_back("ID"); 
  };

  void PositionData::pre_integrate() {
    storeData.set_vector_data(vector_data_entries);
    storeData.set_scalar_data(scalar_data_entries);
    storeData.set_integer_data(integer_data_entries);
    storeData.initialize(simData);
    // Store initial data
    storeData.store(initial_data);
  }

  void PositionData::post_step() {
    // Only record if enough time has gone by
    if (!DataObject::_check()) return;

    // Record what time it was
    float time = Base::gflow->getElapsedTime();
    timeStamps.push_back(time);

    // Record all the data
    vector<float> data;
    storeData.store(data);
    // Store this timestep's data
    positions.push_back(data);
  }

  bool PositionData::writeToFile(string fileName, bool useName) {
    // The name of the directory for this data
    string dirName = fileName;
    if (*fileName.rbegin()=='/') // Make sure there is a /
      dirName += dataName+"/";
    else 
      dirName += ("/"+dataName+"/");
    // Make a directory for the data
    mkdir(dirName.c_str(), 0777);

    return storeData.write(dirName+"data.csv", positions);
  }

}
