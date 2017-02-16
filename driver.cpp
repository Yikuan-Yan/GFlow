/// driver.cpp - The program that creates and runs simulations
/// For a bacteria simulation, use the option " -bacteria " or " -bacteria=1 "
///
///
/// Author: Nathaniel Rupprecht

#include "Simulator.h"

string printAsTime(double seconds) {
  stringstream stream;
  string str;
  int hours = floor(seconds/3600.);
  seconds -= 3600*hours;
  int minutes = floor(seconds/60.);
  seconds -= 60*minutes;
  if (hours<10) stream << "0" << hours << ":";
  else stream << hours << ":";
  if (minutes<10) stream << "0" << minutes << ":";
  else stream << minutes << ":";
  if (seconds<10) stream << "0" << seconds;
  else stream << seconds;
  stream >> str;
  return str;
}

int main(int argc, char** argv) {
  // Start the clock
  auto start_t = clock();

  // Parameters
  double width = 4.;     // Length of the pipe
  double height = 2.;    // 2*Radius of the pipe
  double drop = 0.;      // How far above the surface we should drop things
  double epsilon = 1e-4; // Default epsilon
  double radius = 0.05;  // Disc radius
  double dispersion = 0; // Disc radius dispersion
  double bR = 0.5;        // Radius of buoyant particle
  double density = 5;    // Density of buoyant particle
  double velocity = 0.5; // Fluid velocity (at center or for Couette flow)
  double temperature = -1; // Temperature
  double dissipation = -1; // Dissipation
  double friction = -1;    // Friction
  double frequency = 0.; // Shake frequency
  double amplitude = 0.; // Shake Amplitude
  double phi = 0.5;      // Packing density
  double time = 10.;     // How long the entire simulation should last
  double start = 0;      // At what time we start recording data
  double pA = 0.;        // What percent of the particles are active
  double activeF = default_run_force;  // Active force (default is 5)
  double maxV = 1.5;     // Max velocity to bin
  double minVx = -0.1;
  double maxVx = 0.6;
  double minVy = -0.1;
  double maxVy = 0.1;
  double percent = 0.5;  // What percent of the jamPipe is closed off
  int bins = -1;         // How many bins we should use when making a density profile
  int vbins = -1;        // How many velocity bins we should use
  int number = -1;       // Override usual automatic particle numbers and use prescribed number
  string atype = "";     // What type of active particle to use
  bool interact = true;  // Whether particles should interact or not

// bacteria parameters: (default values. pass arguments through command line)
  double replenish = 0.0;  // Replenish rate
  double d1 = 50.0;
  double d2 = 50.0;
  double a1 = 1.0;
  double a2 = 1.0;
  double b1 = 0.0;
  double k1 = 1.0;
  double k2 = 1.0;
  double L1 = 0.0;
  double L2 = 0.0;
  double s1 = 1.0; 
  double s2 = 1.0;
  double mu = 0.0; // mutation rate
  double ds = 0.0; // mutation amount 

  // Run Type
  bool bacteria = false;      // Bacteria box
  bool sedimentation = false; // Sedimentation box
  bool sphereFluid = false;   // Sphere fluid
  bool couetteFlow = false;   // Use pure Couette conditions for the sphere fluid
  bool buoyancy = false;      // Create buoyancy box

  // Display parameters
  int animationSortChoice = -1;
  bool mmpreproc = true;
  bool animate = false;
  bool bulk = false;
  bool pressure = false;
  bool dispKE = false;
  bool dispAveKE = false;
  bool dispFlow = false;
  bool dispAveFlow = false;
  bool dispProfile = false;
  bool dispAveProfile = false;
  bool dispVelProfile = false;
  bool dispFlowXProfile = false;
  bool dispVelDist = false;
  bool totalDist = false;
  bool projDist = false;
  bool useVelDiff = false;
  bool clustering = false;
  bool everything = false;
  bool trackHeight = false;
  bool rcFlds = false;
  //----------------------------------------
  // Parse command line arguments
  //----------------------------------------
  ArgParse parser(argc, argv);
  parser.get("width", width);
  parser.get("height", height);
  parser.get("drop", drop);
  parser.get("epsilon", epsilon);
  parser.get("radius", radius);
  parser.get("dispersion", dispersion);
  parser.get("bR", bR);
  parser.get("density", density);
  parser.get("velocity", velocity);
  parser.get("temperature", temperature);
  parser.get("dissipation", dissipation);
  parser.get("friction", friction);
  parser.get("frequency", frequency);
  parser.get("amplitude", amplitude);
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
  parser.get("atype", atype);
  parser.get("interact", interact);
  parser.get("replenish", replenish);
  parser.get("bacteria", bacteria);
  parser.get("sedimentation", sedimentation);
  parser.get("sphereFluid", sphereFluid);
  parser.get("couetteFlow", couetteFlow);
  parser.get("buoyancy", buoyancy);
  parser.get("animationSortChoice", animationSortChoice);
  parser.get("mmpreproc", mmpreproc);
  parser.get("animate", animate);
  parser.get("bulk", bulk);
  parser.get("pressure", pressure);
  parser.get("KE", dispKE);
  parser.get("aveKE", dispAveKE);
  parser.get("flow", dispFlow);
  parser.get("aveFlow", dispAveFlow);
  parser.get("profileMap", dispProfile);
  parser.get("profile", dispAveProfile);
  parser.get("velProfile", dispVelProfile);
  parser.get("flowXProfile", dispFlowXProfile);
  parser.get("velDist", dispVelDist);
  parser.get("projDist", projDist);
  parser.get("totalDist", totalDist);
  parser.get("useVelDiff", useVelDiff);
  parser.get("clustering", clustering);
  parser.get("everything", everything);
  parser.get("trackHeight", trackHeight);

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
  parser.get("recFields", rcFlds);
  parser.get("dSec", ds);
  
  //----------------------------------------

  // Dependent variables
  double Vol = width*height;
  int num = Vol/(PI*sqr(radius))*phi;
  if (number<0) number = num;
  int NA = number*pA, NP = number-NA;

  // Seed random number generators
  srand48( std::time(0) );
  srand( std::time(0) );
  seedNormalDistribution();
  //----------------------------------------

  Simulator simulation;
  int statLBP = 0;
  if (dispKE || dispFlow || dispAveFlow || dispAveKE) {
    simulation.addStatistic(statKE);
    simulation.addStatistic(statPassiveKE); //** <==
    simulation.addStatistic(statPassiveFlow);
    simulation.addStatistic(statActiveFlow);
    simulation.addStatistic(statFlowRatio);
    statLBP = 5;
  }
  if (trackHeight) simulation.addStatistic(statLargeBallPosition);
  
  if (totalDist || projDist) simulation.setRecordDist(true);
  simulation.setStartRecording(start);
  if (dispProfile || dispAveProfile) simulation.setCaptureProfile(true);
  if (dispVelProfile || dispFlowXProfile) simulation.setCaptureVelProfile(true);
  if (animate) simulation.setCapturePositions(true);
  if (bulk) simulation.setRecordBulk(true);
  if (pressure) simulation.setRecordPressure(true);
  if (dispVelDist) simulation.setCaptureVelocity(true);
  // Set active particle type
  PType type = getType(atype);
  if (type!=ERROR) simulation.setActiveType(type);
  // Set up the simulation
  if (bacteria) {
    simulation.setEatRate(s1);
    simulation.setReplenish(replenish);
    simulation.createBacteriaBox(number, radius, width, height, velocity);
  }
  else if (sedimentation) simulation.createSedimentationBox(NP+NA, radius, width, height, activeF);
  else if (sphereFluid) simulation.createSphereFluid(NP, NA, radius, activeF, radius, width, height, velocity, couetteFlow);
  else if (buoyancy) simulation.createBuoyancyBox(radius, bR, density, width, height, drop, dispersion, frequency, amplitude);
  else simulation.createControlPipe(NP, NA, radius, velocity, activeF, radius, width, height);
  simulation.setParticleInteraction(interact);
  
  if (animationSortChoice>=0) simulation.setAnimationSortChoice(animationSortChoice);
  if (bins>0) simulation.setBins(bins);
  if (vbins>0) simulation.setVBins(vbins);
  if (everything) simulation.setRecordDist(true);
  simulation.setMaxV(maxV);
  simulation.setMinVx(minVx);
  simulation.setMaxVx(maxVx);
  simulation.setMinVy(minVy);
  simulation.setMaxVy(maxVy);
  simulation.setUseVelocityDiff(useVelDiff);

  if (temperature>=0) simulation.setTemperature(temperature);
  if (dissipation>=0) {
    simulation.setParticleDissipation(dissipation);
    simulation.setWallDissipation(dissipation);
  }
  if (friction>=0) {
    simulation.setParticleCoeff(friction);
    simulation.setWallCoeff(friction);
  }

  // simulation.setResourceDiffusion(d1);
  // simulation.setWasteDiffusion(d2);
  simulation.setResourceBenefit(a1);
  simulation.setWasteHarm(a2);
  simulation.setSecretionCost(b1);
  simulation.setResourceSaturation(k1);
  simulation.setWasteSaturation(k2);
  simulation.setResourceDecay(L1);
  simulation.setWasteDecay(L2);
 // simulation.setEatRate(s1); // moved to line above create box
  simulation.setSecretionRate(s2);
  simulation.setMutationRate(mu);
  simulation.setMutationAmount(ds);
  simulation.setRecFields(rcFlds);

  /// Print condition summary
  cout << "----------------------- RUN SUMMARY -----------------------\n\n";
  cout << "Command: ";
  for (int i=0; i<argc; i++) cout << argv[i] << " ";
  cout << endl << endl; // Line break
  cout << "Dimensions: " << width << " x " << height << " (Volume: " << Vol << ")\n";
  cout << "Radius: " << radius << ", Dispersion: " << dispersion << "\n";
  cout << "Characteristic Fluid Velocity: " << velocity << "\n";
  cout << "Phi: " << phi << ", Number: " << simulation.getNumber() << ", (Actual Phi: " << simulation.getNumber()*PI*sqr(radius)/(width*height) << ")\n";
  cout << "Percent Active: " << pA*100 << "%, (Actual %: " << 100.*NA/(double)(NA+NP) << ")\n";
  if (NA>0) {
    cout << "Active Force: " << activeF << ", Active Type: " << getString(simulation.getActiveType()) << "\n";
  }
  cout << "N Active: " << simulation.getASize() << ", N Passive: " << simulation.getPSize() << "\n";
  if (dispVelDist || totalDist || projDist) {
    cout << endl; // Line break
    cout << "Max V: " << simulation.getMaxV() << ", Min/Max Vx: " << simulation.getMinVx() << ", " << simulation.getMaxVx() << ", Min/Max Vy: " << simulation.getMinVy() << ", " << simulation.getMaxVy() << endl;
    cout << "Bin X: " << simulation.getBinXWidth() << ", Bin Y: " << simulation.getBinYWidth() << ", vBin X: " << simulation.getVBinXWidth() << ", vBin Y: " << simulation.getVBinYWidth() << endl;
    cout << "vBin X Zero: " << simulation.getVBinXZero() << ", vBin Y Zero: " << simulation.getVBinYZero() << endl;
    cout << "Using relative velocity: " << (useVelDiff ? "True" : "False") << endl;
  }

  cout << "\n...........................................................\n\n";
  
  /// Run the actual program
  try {
    simulation.setDefaultEpsilon(epsilon);
    if (bacteria) simulation.bacteriaRun(time);
    else simulation.run(time);
  }
  catch (...) {
    cout << "An error has occured. Exiting.\n";
    throw;
  }
  auto end_t = clock(); // End timing
  double realTime = (double)(end_t-start_t)/CLOCKS_PER_SEC;

  /// Print the run information
  double runTime = simulation.getRunTime();
  cout << "Sim Time: " << time << ", Run time: " << runTime << " s (" << printAsTime(runTime) << "), Ratio: " << time/runTime << ", (" << runTime/time << ")" <<endl;
  cout << "Setup Time: " << realTime - runTime << "\n";
  cout << "Start Time: " << start << ", Record Time: " << max(0., time-start) << "\n";
  cout << "Actual (total) program run time: " << realTime << "\n";
  cout << "Iterations: " << simulation.getIter() << ", Default Epsilon: " << simulation.getDefaultEpsilon() << endl;
  cout << "Sectors: X: " << simulation.getSecX() << ", Y: " << simulation.getSecY();
  cout << "\n\n----------------------- END SUMMARY -----------------------\n\n";
  
  /// Print data
  if (animate) {
    if (bacteria) { 
      simulation.printBacteriaToFile(); // print particle positions at each time
      simulation.printResourceToFile();
      simulation.printWasteToFile();
      cout << simulation.printWalls() << endl; // for now - later print geometry in different format
    }
    else cout << simulation.printAnimationCommand() << endl;
  }  
  if (bulk) cout << simulation.printBulkAnimationCommand() << endl;
  if (pressure) cout << simulation.printPressureAnimationCommand() << endl;
  if (dispKE) {
    cout << "KE=" << simulation.getStatistic(0) << ";\n";
    cout << "Print[\"Average kinetic energy\"]\nListLinePlot[KE,PlotRange->All,PlotStyle->Black]\n";
    if (simulation.getASize()>0) {
      cout << "passKE=" << simulation.getStatistic(1) << ";\n";
      cout << "Print[\"Average passive particle kinetic energy\"]\nListLinePlot[passKE,PlotRange->All,PlotStyle->Black]\n";
    }
  }
  if (dispAveKE) {
    cout << "aveKE=" << simulation.getAverage(0) << ";\n";
    cout << "avePassKE=" << simulation.getAverage(1) << ";\n";
  }
  if (dispFlow) {
    cout << "passflow=" << simulation.getStatistic(2) << ";\n";
    cout << "Print[\"Passive Flow\"]\nListLinePlot[passflow,PlotRange->All,PlotStyle->Black]\n";
    cout << "actflow=" << simulation.getStatistic(3) << ";\n";
    cout << "Print[\"Active Flow\"]\nListLinePlot[actflow,PlotRange->All,PlotStyle->Black]\n";
    cout << "xi=" << simulation.getStatistic(4) << ";\n";
    cout << "Print[\"Xi\"]\nListLinePlot[xi,PlotRange->All,PlotStyle->Black]\n";
  }
  if (trackHeight) {
    cout << "bheight=" << simulation.getStatistic(statLBP) << ";\n";
    cout << "Print[\"Large Ball Height\"]\nListLinePlot[bheight,PlotRange->All,PlotStyle->Black]\n";
  }
  if (dispAveFlow) {
    cout << "avePassFlow=" << simulation.getAverage(2) << ";\n";
    cout << "aveActFlow=" << simulation.getAverage(3) << ";\n";
    cout << "aveXi=" << simulation.getAverage(4) << ";\n";
  }
  if (dispProfile) {
    cout << "prof=" << simulation.getProfile() << ";\n";
    cout << "Print[\"Profile\"]\nMatrixPlot[prof,ImageSize->Large]\n"; 
  }
  if (dispAveProfile) {
    cout << "aveProf=" << simulation.getAveProfile() << ";\n";
    cout << "Print[\"Average Profile\"]\nListLinePlot[aveProf,PlotStyle->Black,ImageSize->Large]\n";
  }
  if (dispVelProfile) {
    cout << "velProf=" << simulation.getVelProfile() << ";\n";
    cout << "Print[\"(x) Velocity Profile\"]\nListLinePlot[velProf,PlotStyle->Black,ImageSize->Large]\n";
  }
  if (dispFlowXProfile) {
    cout << "flowXProf=" << simulation.getFlowXProfile() << ";\n";
    cout << "Print[\"(x) Velocity flow (as a function of x)\"]\nListLinePlot[flowXProf,PlotStyle->Black,ImageSize->Large]\n";
  }
  if (dispVelDist) {
    cout << "velDist=" << simulation.getVelocityDistribution() << ";\n";
    cout << "Print[\"Velocity Distribution\"]\nListLinePlot[velDist,PlotStyle->Black,ImageSize->Large,PlotRange->All]\n";
    cout << "aVelDist=" << simulation.getAuxVelocityDistribution() << ";\n";
    cout << "Print[\"Auxilary Velocity Distribution\"]\nListLinePlot[aVelDist,PlotStyle->Black,ImageSize->Large,PlotRange->All]\n";
  }
  if (projDist) {
    stringstream stream;
    string str;
    stream << simulation.getCollapsedProjectedDistribution();
    stream >> str;
    if (mmpreproc) cout << "projDist=" << mmPreproc(str) << ";\n";
    else cout << "projDist=" << str << ";\n";
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
