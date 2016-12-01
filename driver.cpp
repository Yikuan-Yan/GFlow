/// driver.cpp - The program that creates and runs simulations
/// For a bacteria simulation, use the option -bacteria=1
///
///
/// Author: Nathaniel Rupprecht

#include "Simulator.h"

int main(int argc, char** argv) {
  // Start the clock
  auto start_t = clock();

  // Parameters
  double width = 4.;     // Length of the pipe
  double height = 2.;    // 2*Radius of the pipe
  double radius = 0.05;  // Disc radius
  double velocity = 0.5; // Fluid velocity (at center)
  double temperature = 0; // Temperature
  double phi = 0.5;      // Packing density
  double time = 600.;    // How long the entire simulation should last
  double start = 30;     // At what time we start recording data
  double pA = 0.;        // What percent of the particles are active
  double activeF = 1.5;  // Active force (default is 5)
  double maxV = 1.5;     // Max velocity to bin
  double minVx = -0.1;
  double maxVx = 0.6;
  double minVy = -0.1;
  double maxVy = 0.1;
  double percent = 0.5;  // What percent of the jamPipe is closed off
  int bins = -1;         // How many bins we should use when making a density profile
  int vbins = -1;        // How many velocity bins we should use
  int number = -1;       // Override usual automatic particle numbers and use prescribed number

// bacteria parameters: (default values. pass arguments through command line)
  double replenish = 0;  // Replenish rate
  double d1 = 50.0;
  double d2 = 50.0;
  double a1 = 1.0;
  double a2 = 1.0;
  double b1 = 0.0;
  double k1 = 1.0;
  double k2 = 1.0;
  double L1 = 0.0;
  double L2 = 0.0;
  double s1 = -1.0; // to be eating
  double s2 = 1.0;
  double mu = 0.0; // mutation rate
  double ds = 0.0; // mutation amount 

  // Run Type
  bool bacteria = false;      // Bacteria box
  bool sedimentation = false; // Sedimentation box

  // Display parameters
  bool mmpreproc = true;
  bool animate = false;
  bool dispKE = false;
  bool aveKE = false;
  bool dispFlow = false;
  bool dispProfile = false;
  bool dispAveProfile = false;
  bool dispVelDist = false;
  bool totalDist = false;
  bool useVelDiff = false;
  bool clustering = false;
  bool everything = false;

  //----------------------------------------
  // Parse command line arguments
  //----------------------------------------
  ArgParse parser(argc, argv);
  parser.get("width", width);
  parser.get("height", height);
  parser.get("radius", radius);
  parser.get("velocity", velocity);
  parser.get("temperature", temperature);
  parser.get("phi", phi);
  parser.get("time", time);
  parser.get("start", start);
  parser.get("active", pA);
  parser.get("force", activeF);
  parser.get("bins", bins);
  parser.get("vbins", vbins);
  parser.get("maxV", maxV);
  parser.get("maxVx", maxVx);
  parser.get("minVx", minVx);
  parser.get("maxVy", maxVy);
  parser.get("minVy", minVy);
  parser.get("percent", percent);
  parser.get("number", number);
  parser.get("replenish", replenish);
  parser.get("bacteria", bacteria);
  parser.get("sedimentation", sedimentation);
  parser.get("mmpreproc", mmpreproc);
  parser.get("animate", animate);
  parser.get("dispKE", dispKE);
  parser.get("aveKE", aveKE);
  parser.get("flow", dispFlow);
  parser.get("profileMap", dispProfile);
  parser.get("profile", dispAveProfile);
  parser.get("velDist", dispVelDist);
  parser.get("totalDist", totalDist);
  parser.get("useVelDiff", useVelDiff);
  parser.get("clustering", clustering);
  parser.get("everything", everything);
  // Bacteria parameters from command line
  parser.get("rDiff", d1);
  parser.get("wDiff", d2);
  parser.get("benefit", a1);
  parser.get("harm", a2);
  parser.get("cost", b1);
  parser.get("rSat", k1);
  parser.get("wSat", k2);
  parser.get("rDec", L1);
  parser.get("wDec", L2);
  parser.get("rSec", s1);
  parser.get("wSec", s2);
  parser.get("mutProb", mu);
  parser.get("dSec", ds);
  //----------------------------------------

  // Dependent variables
  double Vol = width*height;
  double rA = radius;
  int num = Vol/(PI*sqr(radius))*phi;
  if (number<0) number = num;
  int NA = number*pA, NP = number-NA;

  // Seed random number generators
  srand48( std::time(0) );
  srand( std::time(0) );
  //----------------------------------------

  Simulator simulation;
  if (dispKE || dispFlow) {
    simulation.addStatistic(statKE);
    simulation.addStatistic(statPassiveFlow);
    simulation.addStatistic(statActiveFlow);
    simulation.addStatistic(statFlowRatio);
  }
  simulation.setStartRecording(start);
  if (dispProfile || dispAveProfile) simulation.setCaptureProfile(true);
  if (animate) simulation.setCapturePositions(true);
  if (dispVelDist) simulation.setCaptureVelocity(true);

  // Set up the simulation
  if (bacteria) {
    simulation.setReplenish(replenish);
    simulation.createBacteriaBox(number, radius, width, height, velocity);
  }
  else if (sedimentation) simulation.createSedimentationBox(NP+NA, radius, width, height, activeF);
  else simulation.createControlPipe(NP, NA, radius, velocity, activeF, rA, width, height);
  
  if (bins>0) simulation.setBins(bins);
  if (vbins>0) simulation.setVBins(vbins);
  if (everything) simulation.setRecordDist(true);
  simulation.setMaxV(maxV);
  simulation.setMinVx(minVx);
  simulation.setMaxVx(maxVx);
  simulation.setMinVy(minVy);
  simulation.setMaxVy(maxVy);
  simulation.setUseVelocityDiff(useVelDiff);

  // simulation.setResourceDiffusion(d1);
  // simulation.setWasteDiffusion(d2);
  simulation.setResourceBenefit(a1);
  simulation.setWasteHarm(a2);
  simulation.setSecretionCost(b1);
  simulation.setResourceSaturation(k1);
  simulation.setWasteSaturation(k2);
  simulation.setResourceDecay(L1);
  simulation.setWasteDecay(L2);
  simulation.setEatRate(s1);
  simulation.setSecretionRate(s2);
  simulation.setMutationRate(mu);
  simulation.setMutationAmount(ds);

  /// Print condition summary
  cout << "----------------------- RUN SUMMARY -----------------------\n\n";
  cout << "Command: ";
  for (int i=0; i<argc; i++) cout << argv[i] << " ";
  cout << endl << endl; // Line break
  cout << "Dimensions: " << width << " x " << height << " (Volume: " << Vol << ")\n";
  cout << "Radius: " << radius << "\n";
  cout << "Characteristic Fluid Velocity: " << velocity << "\n";
  cout << "Phi: " << phi << ", Number: " << number << ", (Actual Phi: " << number*PI*sqr(radius)/(width*height) << ")\n";
  cout << "Percent Active: " << pA*100 << "%, (Actual %: " << 100.*NA/(double)(NA+NP) << ")\n";
  if (NA>0) cout << "Active Force: " << activeF << endl;
  cout << "N Active: " << NA << ", N Passive: " << NP << "\n";
  cout << endl; // Line break
  cout << "Max V: " << simulation.getMaxV() << ", Min/Max Vx: " << simulation.getMinVx() << ", " << simulation.getMaxVx() << ", Min/Max Vy: " << simulation.getMinVy() << ", " << simulation.getMaxVy() << endl;
  cout << "Bin X: " << simulation.getBinXWidth() << ", Bin Y: " << simulation.getBinYWidth() << ", vBin X: " << simulation.getVBinXWidth() << ", vBin Y: " << simulation.getVBinYWidth() << endl;
  cout << "vBin X Zero: " << simulation.getVBinXZero() << ", vBin Y Zero: " << simulation.getVBinYZero() << endl;
  cout << "Sectors: " << simulation.getSecX() << ", " << simulation.getSecY() << endl;
  cout << "\n...........................................................\n\n";
  
  /// Run the actual program
  if (bacteria) simulation.bacteriaRun(time);
  else simulation.run(time);
  auto end_t = clock(); // End timing

  /// Print the run information
  cout << "Sim Time: " << time << ", Run time: " << simulation.getRunTime() << ", Ratio: " << time/simulation.getRunTime() << endl;
  cout << "Start Time: " << start << ", Record Time: " << max(0., time-start) << "\n";
  cout << "Actual (total) program run time: " << (double)(end_t-start_t)/CLOCKS_PER_SEC << "\n";
  cout << "Iterations: " << simulation.getIter();
  cout << "\n\n----------------------- END SUMMARY -----------------------\n\n";
  
  /// Print data
  if (animate) {
    if (bacteria) { 
        simulation.printBacteriaToFile(); // print particle positions at each time
        simulation.printResourceToFile();
        simulation.printWasteToFile();
 	cout << simulation.printWalls() << endl; // for now - later print geometry in different format
    }
    else {
	cout << simulation.printWatchList() << endl;
    	cout << simulation.printWalls() << endl;
    	cout << simulation.printAnimationCommand() << endl;
    }
  }  
  if (dispKE) {
    cout << "aveKE=" << simulation.getStatistic(0) << ";\n";
    cout << "Print[\"Average kinetic energy\"]\nListLinePlot[aveKE,PlotRange->All,PlotStyle->Black]\n";
  }
  if (aveKE) {
    cout << "KE=" << average(simulation.getStatistic(0)) << ";\n";
  }
  if (dispFlow) {
    cout << "passflow=" << simulation.getStatistic(1) << ";\n";
    cout << "Print[\"Passive Flow\"]\nListLinePlot[passflow,PlotRange->All,PlotStyle->Black]\n";
    cout << "actflow=" << simulation.getStatistic(2) << ";\n";
    cout << "Print[\"Active Flow\"]\nListLinePlot[actflow,PlotRange->All,PlotStyle->Black]\n";
    cout << "xi=" << simulation.getStatistic(3) << ";\n";
    cout << "Print[\"Xi\"]\nListLinePlot[xi,PlotRange->All,PlotStyle->Black]\n";
  }
  if (dispProfile) {
    cout << "prof=" << simulation.getProfile() << ";\n";
    cout << "Print[\"Profile\"]\nMatrixPlot[prof,ImageSize->Large]\n"; 
  }
  if (dispAveProfile) {
    cout << "aveProf=" << simulation.getAveProfile() << ";\n";
    cout << "Print[\"Average Profile\"]\nListLinePlot[aveProf,PlotStyle->Black,ImageSize->Large]\n";
  }
  if (dispVelDist) {
    cout << "velDist=" << simulation.getVelocityDistribution() << ";\n";
    cout << "Print[\"Velocity Distribution\"]\nListLinePlot[velDist,PlotStyle->Black,ImageSize->Large,PlotRange->All]\n";
    //cout << "aVelDist=" << simulation.getAuxVelocityDistribution() << ";\n";
    //cout << "Print[\"Auxilary Velocity Distribution\"]\nListLinePlot[aVelDist,PlotStyle->Black,ImageSize->Large,PlotRange->All]\n";
  }
  if (totalDist) {
    stringstream stream;
    string str;
    stream << simulation.getCollapsedDistribution(0);
    stream >> str;
    if (mmpreproc) cout << "dist=" << mmPreproc(str) << ";\n";
    else cout << "dist=" << str << ";\n";
  }
  if (clustering) {
    stringstream stream;
    string str;
    stream << simulation.getClusteringRec();
    stream >> str;
    if (mmpreproc) cout << "clust=" << mmPreproc(str) << ";\n";
    else cout << "clust=" << str << ";\n";
    cout << "Print[\"Clustering\"]\nListLinePlot[clust,PlotStyle->Black,ImageSize->Large,PlotRange->All]\n";
  }
  if (everything) {
    stringstream stream;
    string str;
    stream << simulation.getDistribution();
    stream >> str;
    if (mmpreproc) cout << "fullDist=" << mmPreproc(str) << ";\n";
    else cout << "fullDist=" << str << ";\n";
    stream.clear(); str.clear();
  }
  return 0;
}
