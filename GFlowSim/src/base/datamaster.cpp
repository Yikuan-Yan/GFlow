#include "datamaster.hpp"
#include "simdata.hpp"
#include "integrator.hpp"
#include "domainbase.hpp"
#include "interaction.hpp"

namespace GFlowSimulation {

  DataMaster::DataMaster(GFlow *gflow) : Base(gflow) {};

  DataMaster::~DataMaster() {
    for (auto& dob : dataObjects)
      if (dob) delete dob;
  }

  void DataMaster::initialize() {
    Base::initialize();
    for (auto& dob : dataObjects)
      if (dob) dob->initialize();
  }

  void DataMaster::addDataObject(DataObject *dob) {
    dataObjects.push_back(dob);
  }

  void DataMaster::setCommand(int ac, char **av) {
    argc = ac; argv = av;
  }

  void DataMaster::setInitializationTime(RealType t) {
    initialization_time = t;
  }

  void DataMaster::startTimer() {
    start_time = current_time();
    timing = true;
  }

    // End the timer and add the new time to the record
  void DataMaster::endTimer() {
    if (timing) {
      auto end_time = current_time();
      run_time += time_span(end_time, start_time);
      timing = false;
    }
  }

  void DataMaster::pre_integrate() {
    startTimer();
    // Always allow preintegrate step to happen.
    for (auto& dob : dataObjects)
      if (dob) dob->pre_integrate();
  }

  void DataMaster::pre_step() {
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->pre_step();
  }
  
  void DataMaster::pre_exchange() {
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->pre_exchange();
  }

  void DataMaster::pre_forces() {
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->pre_forces();
  }

  void DataMaster::post_forces() {
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->post_forces();
  }

  void DataMaster::post_step() {
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->post_step();
  }

  void DataMaster::post_integrate() {
    endTimer();
    if (Base::gflow->getElapsedTime()<startRecTime) return;
    for (auto& dob : dataObjects)
      if (dob) dob->post_integrate();
  }

  bool DataMaster::writeToDirectory(string writeDirectory) {
    // --- Do file related things
    bool success = true;

    // Remove previously existing files if they exist
    system(("rm -rf "+writeDirectory).c_str());

    // Create the directory
    mkdir(writeDirectory.c_str(), 0777);

    // --- Write a summary
    writeSummary(writeDirectory);

    // --- Write the bounds and dimensions to a info file
    if (!dataObjects.empty()) {
      ofstream fout(writeDirectory+"/info.csv");
      if (fout.fail()) success = false;
      else {
        // Write the number of dimensions
        fout << sim_dimensions << endl;
        // Write the bounds
        for (int d=0; d<sim_dimensions; ++d) {
          fout << Base::gflow->getBounds().min[d] << "," << Base::gflow->getBounds().max[d];
          fout << endl;
        }
        fout.close();
      }
    }

    // --- Have all the data objects write their data
    for (auto& dob : dataObjects)
      if (dob) success &= dob->writeToFile(writeDirectory, true);

    // --- Write all files 
    for (auto& f : files) {
      ofstream fout(writeDirectory+"/"+f.first);
      if (fout.fail()) success = false;
      else fout << f.second;
      fout.close();
    }

    // Return true if all writes were successful
    return success;
  }

  void DataMaster::resetTimer() {
    run_time = 0;
  }

  void DataMaster::setStartRecTime(RealType t) {
    startRecTime = t;
  }

  // Set the fps of all the data objects
  void DataMaster::setFPS(RealType fps) {
    for (auto &dob : dataObjects) 
      if (dob) dob->setFPS(fps);
  }

  void DataMaster::giveFile(string filename, string file_contents) {
    files.push_back(pair<string, string>(filename, file_contents));
  }

  // Set the fps of particular data objects
  void DataMaster::setFPS(int obj_id, RealType fps) {
    if (-1<obj_id && obj_id<dataObjects.size())
      dataObjects.at(obj_id)->setFPS(fps);
  }

  RealType DataMaster::getRatio() const {
    return Base::gflow->getTotalRequestedTime()/run_time;
  }

  inline bool DataMaster::writeSummary(string writeDirectory) {
    std::ofstream fout(writeDirectory+"/run_summary.txt");
    if (fout.fail()) {
      // Write error message
      std::cerr << "Failed to open file [" << writeDirectory << "/run_summary.txt]." << endl;
      return false;
    }
    // End timer - in case the run failed, and therefore didn't end the timer
    endTimer();
    // Print Header
    fout << "**********          SUMMARY          **********\n";
    fout << "*******  GFlow Granular Simulator v 4.0 *******\n";
    fout << "********** 2018, Nathaniel Rupprecht **********\n";
    fout << "***********************************************\n\n";
    // Print command
    if (argc>0) {
      for (int c=0; c<argc; ++c) fout << argv[c] << " ";
    }
    else { // Try to get command from gflow
      pair<int, char**> command = gflow->getCommand();
      if (command.second) for (int c=0; c<command.first; ++c) fout << command.second[c] << " ";
    }
    fout << "\n\n";
    // --- Print timing summary
    RealType requestedTime = Base::gflow ? Base::gflow->getTotalRequestedTime() : 0;
    RealType ratio = Base::gflow->getTotalTime()/run_time;
    int iterations = Base::gflow->getIter(), particles = Base::simData->number();
    // Helper lambda - checks whether run_time was non-zero
    auto toStrRT = [&] (RealType x) -> string {
      return (run_time>0 ? toStr(x) : "--");
    };
    // Print data
    fout << "Timing and performance:\n";
    if (initialization_time>0) fout << "  - Initialization time:      " << initialization_time << "\n"; 
    fout << "  - Time simulated:           " << Base::gflow->getTotalTime() << "\n";
    fout << "  - Requested Time:           " << requestedTime << "\n";
    fout << "  - Run Time:                 " << run_time;
    if (run_time>60) fout << " ( h:m:s - "   << printAsTime(run_time) << " )";
    fout << "\n";
    fout << "  - Ratio x Particles:        " << toStrRT(ratio*particles) << "\n";
    fout << "  - Iter x Particles / s:     " << toStrRT(iterations*(particles/run_time)) << "\n";
    fout << "  - Ratio:                    " << toStrRT(ratio) << "\n";
    fout << "  - Inverse Ratio:            " << toStrRT(1./ratio) << "\n";
    fout << "\n";

    fout << "Timing breakdown:\n";
    const int entries = 4;
    double timing[entries], total = 0;
    fout << "  -- Pre-forces, integrator:  " << toStrRT(timing[0] = gflow->fhs_timer.time()/run_time*100) << "%,\t" << gflow->fhs_timer.time() << "\n";
    fout << "  -- Post-forces, integrator: " << toStrRT(timing[1] = gflow->shs_timer.time()/run_time*100) << "%,\t" << gflow->shs_timer.time() << "\n";
    fout << "  -- Pre-forces, domain:      " << toStrRT(timing[2] = gflow->domain_timer.time()/run_time*100) << "%,\t" << gflow->domain_timer.time() << "\n";
    fout << "  -- Interactions:            " << toStrRT(timing[3] = gflow->forces_timer.time()/run_time*100) << "%,\t" << gflow->forces_timer.time() << "\n";
    for (int i=0; i<entries; ++i) total += timing[i];
    fout << "  - Uncounted:                " << toStrRT((100. - total)) << "%,\t" << run_time*(100. - total)*0.01 << "\n";
    fout << "\n";

    // --- Print simulation summary
    fout << "Simulation and space:\n";
    fout << "  - Dimensions:               " << sim_dimensions << "\n";
    fout << "  - Boundaries:               ";
    for (int d=0; d<sim_dimensions; ++d) {
      fout << "{" << Base::gflow->bounds.min[d] << "," << Base::gflow->bounds.max[d] << "}";
      if (d!=sim_dimensions-1) fout << ", ";
    }
    fout << "\n";
    fout << "  - Boundaries:               ";
    for (int d=0; d<sim_dimensions; ++d) {
      switch (Base::gflow->getBC(d)) {
        case BCFlag::OPEN: {
          fout << "Open";
          break;
        }
        case BCFlag::WRAP: {
          fout << "Wrap";
          break;
        }
        case BCFlag::REFL: {
          fout << "Reflect";
          break;
        }
        case BCFlag::REPL: {
          fout << "Repulse";
          break;
        }
        default: {
          fout << "Other";
          break;
        }
      }
      if (d!=sim_dimensions-1) fout << ", ";
    }
    fout << "\n";
    fout << "  - Number of particles:      " << Base::simData->number() << "\n";
    int types = Base::simData->ntypes();
    if (types>1) {
      int *count = new int[types];
      for (int ty=0; ty<types; ++ty) count[ty] = 0;
      for (int n=0; n<Base::simData->number(); ++n) ++count[Base::simData->Type(n)];
      for (int ty=0; ty<types; ++ty)
        fout << "     Type " << toStr(ty) << ":                  " << count[ty] << " (" << 
          count[ty] / static_cast<RealType>(Base::simData->number()) << "%)\n";
      delete [] count;
    }
    RealType vol = 0;
    for (int n=0; n<Base::simData->number(); ++n) vol += pow(simData->Sg(n), sim_dimensions);
    vol *= pow(PI, sim_dimensions/2.) / tgamma(sim_dimensions/2. + 1.);
    RealType phi = vol/Base::gflow->getBounds().vol();
    fout << "  - Packing fraction:         " << phi << "\n";
    fout << "\n";

    // --- Print integration summary
    fout << "Integration:\n";
    fout << "  - Iterations:               " << iterations << "\n";
    fout << "  - Time per iteration:       " << toStrRT(run_time / static_cast<RealType>(iterations)) << "\n";
    if (integrator) fout << "  - Time step (at end):       " << integrator->getTimeStep() << "\n";
    fout << "  - Average dt:               " << Base::gflow->getTotalTime()/iterations << "\n";
    fout << "\n";
    
    // --- Print the domain summary
    fout << "Domain summary (as of end of simulation):\n";
    fout << "  - Grid dimensions:          ";
    for (int d=0; d<sim_dimensions; ++d) {
      fout << Base::domain->getDims()[d];
      if (d!=sim_dimensions-1) fout << ", ";
    }
    fout << "\n";
    fout << "  - Total sectors:            " << Base::domain->getNumCells() << "\n";
    fout << "  - Grid lengths:             ";
    for (int d=0; d<sim_dimensions; ++d) {
      fout << Base::domain->getWidths()[d];
      if (d!=sim_dimensions-1) fout << ", ";
    }
    fout << "\n";
    if (Base::domain) {
      fout << "  - Cutoff:                   " << Base::domain->getCutoff() << "\n";
      fout << "  - Skin depth:               " << Base::domain->getSkinDepth() << "\n";
      fout << "  - Move ratio tolerance:     " << Base::domain->getMvRatioTolerance() << "\n";
      fout << "  - Delay missed target:      " << Base::domain->getMissedTarget() << "\n";
      fout << "  - Average miss:             " << Base::domain->getAverageMiss() << "\n";
      if (run_time>0) {
        fout << "  - Sector remakes:           " << Base::domain->getNumberOfRemakes() << "\n";
        RealType re_ps = Base::domain->getNumberOfRemakes() / Base::gflow->getTotalTime();
        fout << "  - Remakes per second:       " << re_ps << "\n";
        fout << "  - Average remake delay:     " << 1./re_ps << "\n";
        fout << "  - Average iters per delay:  " << static_cast<RealType>(iterations) / Base::domain->getNumberOfRemakes() <<"\n";
      }
    }
    fout << "\n";

    // --- Interactions
    fout << "Interactions:\n";
    int c=0, inters=0;
    for (auto &it : gflow->interactions) {
      fout << "     Interaction " << c << ":           length " << it->size() << "\n";
      ++c;
      inters += it->size();
    }
    fout << "  - Inter.s per particle:     " << static_cast<double>(inters) / particles << "\n";
    fout << "\n";

    // --- Print particle summary
    writeParticleData(fout);
    fout << "\n";
    
    // Close the stream
    fout.close();

    // Write the log file
    writeLogFile(writeDirectory);

    // Return success
    return true;
  }

  inline void DataMaster::writeParticleData(std::ostream& out) {
    // If there are no particles
    if (Base::simData->number()==0) {
      cout << "No particles.\n";
    }
    // If there are particles, record data about them.
    RealType asigma(0), amass(0), aden(0), aspeed(0), ake(0);
    RealType maxsigma = Base::simData->Sg(0), maxmass = 1./Base::simData->Im(0), 
      maxden = 1./(Base::simData->Im(0)*sphere_volume(maxsigma, sim_dimensions)),
      maxspeed = magnitudeVec(Base::simData->V(0), sim_dimensions), 
      maxke = sqr(magnitudeVec(Base::simData->V(0), sim_dimensions))*(1./Base::simData->Im(0));
    RealType minsigma = maxsigma, minmass = maxmass, minden = maxden, minspeed = maxspeed, minke = maxke;
    for (int n=0; n<Base::simData->number(); ++n) {
      if (Base::simData->Type(n)<0) continue;
      // Cutoff
      RealType sig = Base::simData->Sg(n);
      asigma += sig; 
      if (sig<minsigma) minsigma = sig;
      if (sig>maxsigma) maxsigma = sig;
      // Speed
      RealType speed = magnitudeVec(Base::simData->V(n), sim_dimensions);
      aspeed += speed;
      if (speed<minspeed) minspeed = speed;
      if (speed>maxspeed) maxspeed = speed;
      // Only evaluate the following if the mass is not infinite
      if (Base::simData->Im(n)>0) {
        // Mass
        RealType mass = 1./Base::simData->Im(n);
        amass += mass;
        if (mass<minmass) minmass = mass;
        if (mass>maxmass) maxmass = mass;
        // Density
        RealType den = 1./(Base::simData->Im(n)*sphere_volume(sig, sim_dimensions));
        aden += den;
        if (den<minden) minden = den;
        if (den>maxden) maxden = den;
        // Kinetic energy
        RealType ke = sqr(magnitudeVec(Base::simData->V(n), sim_dimensions))*(1./Base::simData->Im(n));
        ake += ke;
        if (ke<minke) minke = ke;
        if (ke>maxke) maxke = ke;
      }
    }
    // Normalize
    RealType invN = 1./static_cast<RealType>(Base::simData->number());
    asigma *= invN;
    amass  *= invN;
    aden   *= invN;
    aspeed *= invN;
    ake    *= (0.5*invN);
    // Print data
    out << "Particle Average Data (at finish): (Ave, [Min, Max])\n";
    out << "  - Sigma:                    " << asigma << "\n";
    out << "  - Mass:                     " << amass << "\n";
    out << "  - Density:                  " << aden << "\n";
    out << "  - Speed:                    " << aspeed << "\n";
    out << "  - Kinetic energy:           " << ake << "\n";
    out << "\n";
    out << "Particle min/max data (at finish):\n";
    out << "  - Sigma:                    [ " << minsigma << ", " << maxsigma << " ]\n";
    out << "  - Mass:                     [ " << minmass << ", " << maxmass << " ]\n";
    out << "  - Density:                  [ " << minden << ", " << maxden << " ]\n";
    out << "  - Speed:                    [ " << minspeed << ", " << maxspeed << " ]\n";
    out << "  - Kinetic energy:           [ " << minke << ", " << maxke << " ]\n";
    out << "\n";
  }

  inline void DataMaster::writeDomainData(std::ostream&) {
    
  }

  inline bool DataMaster::writeLogFile(string writeDirectory) {
    std::ofstream fout(writeDirectory+"/log.txt");
    if (fout.fail()) {
      // Write error message
      std::cerr << "Failed to open file [" << writeDirectory << "/run_log.txt]." << endl;
      return false;
    }
    // Print Header
    fout << "**********          RUN LOG          **********\n";
    fout << "*******  GFlow Granular Simulator v 4.0 *******\n";
    fout << "********** 2018, Nathaniel Rupprecht **********\n";
    fout << "***********************************************\n\n";

    // --- Print version related data
    fout << "Github and version info:\n";
    // Get the github version. 
    std::ifstream fin("../.git/refs/heads/master");
    if (fin.fail()) {
      fout << "  - Github version:            --\n";
    }
    else {
      string commit_hash;
      fin >> commit_hash;
      fin.close();
      fout << "  - Github version:           " << commit_hash << "\n";
    }
    fout << "\n";

    fout << "SIMD info:\n";
    fout << "  - SIMD type:                ";
    switch (SIMD_TYPE) {
      case 0: {
        fout << "No SIMD\n";
        break;
      }
      case 1: {
        fout << "SSE\n";
        break;
      }
      case 2: {
        fout << "AVX\n";
        break;
      }
      case 3: {
        fout << "AVX2\n";
        break;
      }
      case 4: {
        fout << "MIC\n";
        break;
      }
      default: {
        fout << "Unrecognized\n";
        break;
      }
    }

    // Close file stream
    fout.close();

    // Return success
    return true;
  }

}
