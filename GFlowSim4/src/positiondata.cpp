#include "positiondata.hpp"
#include "simdata.hpp"

namespace GFlowSimulation {
  // Constructor
  PositionData::PositionData(GFlow *gflow) : DataObject(gflow, "Pos") {};

  // Destructor
  PositionData::~PositionData() {
    // Erase data
    for (auto &pd : positions)
      if (pd) delete [] pd;
    positions.clear();
  }

  void PositionData::post_step() {
    // Only record if enough time has gone by
    RealType time = Base::gflow->getElapsedTime();
    if (time-lastRecording<delay) return;

    // Record what time it was
    timeStamps.push_back(time);

    // --- Record all positions

    // Get the number of particles
    int number = Base::simData->number;
    numbers.push_back(number);
    // Don't do anything if there are 0 or fewer particles
    if (number<=0) return;
    // Create an array for the data
    RealType *array = new RealType[number*DIMENSIONS];
    positions.push_back(array);
    // Fill the array of positions
    for (int i=0; i<number; ++i)
      for (int d=0; d<DIMENSIONS; ++d) {
        array[DIMENSIONS*i+d] = Base::simData->x[i][d];
      }

    // Set last recording
    lastRecording = time;
  }

  bool PositionData::writeToFile(string fileName, bool useName) {
    // The name of the directory for this data
    string dirName = fileName;
    if (*fileName.rbegin()=='/') // Make sure there is a /
      dirName += dataName+"/";
    else 
      dirName += ("/"+dataName+"/");

    // Create a directory for all the data
    mkdir(dirName.c_str(), 0777);

    // Create a file stream
    ofstream fout;

    // Write all the data to the file in the format { x[0], x[1], ... } as a csv
    for (int iter=0; iter<timeStamps.size(); ++iter) {
      // Open the file stream
      fout.open(dirName+dataName+toStr(iter)+".csv");
      if (fout.fail()) return false;

      // Write all the particle data
      for (int n=0; n<numbers[iter]; ++n) {
        // Print out:  x[0], x[1], ...  \n
        for (int d=0; d<DIMENSIONS; ++d) {
          fout << positions[iter][DIMENSIONS*n+d];
          if (d!=DIMENSIONS-1) fout << ",";
        }
        fout << "\n";
      }

      // Close the file stream
      fout.close();
    }

    // Write out all the time stamps
    fout.open(dirName+"times.csv");
    if (fout.fail()) return false;
    for (auto ts : timeStamps) 
      fout << ts << endl;
    fout.close();

    // Return success
    return true;
  }

}
