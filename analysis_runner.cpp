#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class GridAnalysis {
private:
  int gridPrecision;
  double alpha;
  int latMin = -90;
  int latMax = 90;
  int lonMin = -180;
  int lonMax = 180;

  void printEndMessages(const std::string &location) {
    for (int i = 0; i < 21; i++) {
      std::cout << "============================================== END "
                << location << " == " << std::endl;
    }
    std::cout << std::endl;
  }

  void runChi2Analysis(int nEveOffSet, const std::string &location,
                       double alpha) {
    std::cout << "== START ./chi2LRSO3vsSnRunApp " << nEveOffSet << " "
              << location << " ===" << std::endl;

    // Construct the command
    std::stringstream cmd;
    cmd << "./chi2LRSO3vsSnRunApp -E" << nEveOffSet << " -N" << location
        << " -I" << alpha;

    // Execute the command
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
      std::cerr << "Warning: Command failed with return code " << result
                << std::endl;
    }

    printEndMessages(location);
  }

  void runRootMacro(int gridPrecision, int param2, int nEveOffSet) {
    std::stringstream cmd;
    cmd << "root -b -q statsMacroChi2LRSO3Sn.C\\(" << gridPrecision << ","
        << param2 << "," << nEveOffSet << "\\)";

    std::cout << cmd.str() << std::endl;
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
      std::cerr << "Warning: ROOT macro failed with return code " << result
                << std::endl;
    }
    std::cout << std::endl;
  }

public:
  GridAnalysis(int precision, double alphaValue)
      : gridPrecision(precision), alpha(alphaValue) {}

  void runBulkAnalysis(int nEveOffSet) {
    std::cout << "Starting bulk Chi2 analysis with parameters:" << std::endl;
    std::cout << "Event Offset: " << nEveOffSet << std::endl;
    std::cout << "Grid Precision: " << gridPrecision << std::endl;
    std::cout << "Alpha: " << alpha << std::endl;
    std::cout << "Grid bounds: LAT[" << latMin << "," << latMax << "], LON["
              << lonMin << "," << lonMax << "]" << std::endl;
    std::cout << std::endl;

    for (int lon = lonMin; lon <= lonMax; lon += gridPrecision) {
      for (int lat = latMin; lat <= latMax; lat += gridPrecision) {
        std::string location =
            "LAT" + std::to_string(lat) + "LON" + std::to_string(lon);
        runChi2Analysis(nEveOffSet, location, alpha);
      }
    }
  }

  void runOccurrencesStudy() {
    std::cout << "=== OCCURRENCES STUDY ===" << std::endl;
    std::cout << "Running analysis for multiple event offset values"
              << std::endl;
    std::cout << std::endl;

    // Define the event offset values from the original script
    std::vector<int> eventOffsets = {3,  4,  5,  6,  7,  8,  9,  10,
                                     11, 12, 13, 14, 15, 17, 19, 21};

    for (int offset : eventOffsets) {
      std::cout << "Processing event offset: " << offset << std::endl;

      // Run bulk analysis
      runBulkAnalysis(offset);

      // Run ROOT macro
      runRootMacro(10, 1, offset);

      std::cout << "Completed processing for offset " << offset << std::endl;
      std::cout << "================================================"
                << std::endl;
    }
  }

  // Method to run individual analysis (equivalent to calling
  // bulkChi2LRSO3vsSnRunApp.sh directly)
  void runSingleAnalysis(int nEveOffSet, int precision, double alphaValue) {
    gridPrecision = precision;
    alpha = alphaValue;
    runBulkAnalysis(nEveOffSet);
  }
};

// Usage and command line argument handling
void printUsage(const char *programName) {
  std::cout << "Usage:" << std::endl;
  std::cout << "1. Run full occurrences study:" << std::endl;
  std::cout << "   " << programName << " --study" << std::endl;
  std::cout << std::endl;
  std::cout << "2. Run single bulk analysis:" << std::endl;
  std::cout << "   " << programName << " <nEveOffSet> <grid_precision> <alpha>"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Parameters:" << std::endl;
  std::cout << "   nEveOffSet     : Number of events to cutoff" << std::endl;
  std::cout << "   grid_precision : Grid precision (e.g., 10, 5, or 2)"
            << std::endl;
  std::cout << "   alpha          : Alpha parameter (e.g., 1.0)" << std::endl;
  std::cout << std::endl;
  std::cout << "Examples:" << std::endl;
  std::cout << "   " << programName << " --study" << std::endl;
  std::cout << "   " << programName << " 10 10 1.0" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc == 2 && std::string(argv[1]) == "--study") {
    // Run the full occurrences study
    GridAnalysis analysis(10, 1.0); // Default values from original script
    analysis.runOccurrencesStudy();
    return 0;
  }

  if (argc == 4) {
    // Run single bulk analysis
    try {
      int nEveOffSet = std::stoi(argv[1]);
      int gridPrecision = std::stoi(argv[2]);
      double alpha = std::stod(argv[3]);

      GridAnalysis analysis(gridPrecision, alpha);
      analysis.runSingleAnalysis(nEveOffSet, gridPrecision, alpha);
      return 0;
    } catch (const std::exception &e) {
      std::cerr << "Error parsing arguments: " << e.what() << std::endl;
      printUsage(argv[0]);
      return 1;
    }
  }

  std::cerr << "Invalid number of arguments." << std::endl;
  printUsage(argv[0]);
  return 1;
}

// Compilation instructions:
// g++ -o grid_analysis grid_analysis.cpp -std=c++11
