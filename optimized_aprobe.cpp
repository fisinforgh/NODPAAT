// optimized_aprobe.cpp
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

class OptimizedAprobe {
private:
  float lat, lon;
  string prefix;
  string pathToData;

  // Optimized coordinate conversion using compile-time constants
  static constexpr float STEP_A = 0.25f;
  static constexpr float XMIN_LAT_A = -89.875f;
  static constexpr float XMIN_LON_A = -179.875f;
  static constexpr int YMIN = 2024;
  static constexpr int YMAX = 2024;

  inline int calculateLatBin(float latitude) const noexcept {
    int bin = static_cast<int>(round((latitude - XMIN_LAT_A) / STEP_A));
    return max(0, bin);
  }

  inline int calculateLonBin(float longitude) const noexcept {
    int bin = static_cast<int>(round((longitude - XMIN_LON_A) / STEP_A));
    return max(0, bin);
  }

  // Check if directory exists without system calls
  bool directoryExists(const string &path) const {
    return fs::exists(path) && fs::is_directory(path);
  }

  // Get list of HE5 files efficiently
  vector<string> getHE5Files(int year) const {
    vector<string> files;
    string dirPath = pathToData + "aura_" + to_string(year);

    if (!directoryExists(dirPath)) {
      cerr << "Directory does not exist: " << dirPath << endl;
      return files;
    }

    try {
      files.reserve(400); // Reserve space for typical year
      for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".he5") {
          files.push_back(entry.path().string());
        }
      }
    } catch (const fs::filesystem_error &e) {
      cerr << "Error reading directory: " << e.what() << endl;
    }

    sort(files.begin(), files.end());
    return files;
  }

  // Extract date from filename more efficiently
  struct DateInfo {
    string day, month, year;
    bool valid = false;
  };

  DateInfo extractDate(const string &filename) const {
    DateInfo info;

    size_t pos = filename.find("3e");
    if (pos == string::npos)
      return info;

    pos += 3; // Skip "3e"
    if (pos + 8 > filename.length())
      return info;

    info.year = filename.substr(pos, 4);
    info.month = filename.substr(pos + 5, 2);
    info.day = filename.substr(pos + 7, 2);
    info.valid = true;

    return info;
  }

  // Execute h5dump command and get result more efficiently
  float executeH5Dump(const string &filename, int binLat, int binLon) const {
    // Build h5dump command
    string command = "h5dump -d \"/HDFEOS/GRIDS/OMI Column Amount O3/Data "
                     "Fields/ColumnAmountO3[" +
                     to_string(binLat) + "," + to_string(binLon) +
                     ";,;,;,]\" -y \"" + filename +
                     "\" 2>/dev/null | awk 'FNR == 11 {print $1}'";

    // Execute command and capture output directly
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      cerr << "Failed to execute h5dump command" << endl;
      return -1;
    }

    float value = 0;
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe)) {
      value = strtof(buffer, nullptr);
    }
    pclose(pipe);

    return (value <= 0) ? -1 : value;
  }

public:
  OptimizedAprobe(float lat, float lon, const string &prefix,
                  const string &pathToData)
      : lat(lat), lon(lon), prefix(prefix), pathToData(pathToData) {

    // Ensure path ends with '/'
    if (!pathToData.empty() && pathToData.back() != '/') {
      this->pathToData += '/';
    }
  }

  bool process() {
    cout << "Processing Aprobe for location: " << prefix << " (Lat: " << lat
         << ", Lon: " << lon << ")" << endl;

    if (!directoryExists(pathToData)) {
      cerr << "Data path does not exist: " << pathToData << endl;
      return false;
    }

    int binLat = calculateLatBin(lat);
    int binLon = calculateLonBin(lon);

    cout << "Calculated bins - Lat: " << binLat << ", Lon: " << binLon << endl;

    // Process years
    for (int year = YMIN; year <= YMAX; ++year) {
      cout << "Processing year: " << year << endl;

      string outputFile = prefix + "_" + to_string(year) + ".dat";
      ofstream outFile(outputFile);

      if (!outFile.is_open()) {
        cerr << "Cannot create output file: " << outputFile << endl;
        continue;
      }

      // Use larger buffer for better performance
      outFile.rdbuf()->pubsetbuf(nullptr, 8192);

      vector<string> he5Files = getHE5Files(year);
      cout << "Found " << he5Files.size() << " HE5 files" << endl;

      if (he5Files.empty()) {
        cout << "No HE5 files found for year " << year << endl;
        continue;
      }

      for (const string &filename : he5Files) {
        cout << "Processing: " << fs::path(filename).filename().string()
             << endl;

        DateInfo dateInfo = extractDate(filename);
        if (!dateInfo.valid) {
          cerr << "Could not extract date from: " << filename << endl;
          continue;
        }

        float value = executeH5Dump(filename, binLat, binLon);

        outFile << dateInfo.day << '\t' << dateInfo.month << '\t'
                << dateInfo.year << '\t' << value << '\n';

        cout << "Date: " << dateInfo.day << "/" << dateInfo.month << "/"
             << dateInfo.year << ", Value: " << value << endl;
      }

      cout << "Completed processing year " << year << endl;
    }

    cout << "Aprobe processing completed" << endl;
    return true;
  }

  // Debug function to print coordinate calculations
  void debugCoordinates() const {
    cout << "=== Coordinate Debug Information ===" << endl;
    cout << "Input coordinates: Lat=" << lat << ", Lon=" << lon << endl;
    cout << "Latitude calculation:" << endl;
    cout << "  (lat - XMIN_LAT_A) / STEP_A = (" << lat << " - " << XMIN_LAT_A
         << ") / " << STEP_A << endl;
    cout << "  = " << (lat - XMIN_LAT_A) / STEP_A << endl;
    cout << "  Rounded bin: " << calculateLatBin(lat) << endl;

    cout << "Longitude calculation:" << endl;
    cout << "  (lon - XMIN_LON_A) / STEP_A = (" << lon << " - " << XMIN_LON_A
         << ") / " << STEP_A << endl;
    cout << "  = " << (lon - XMIN_LON_A) / STEP_A << endl;
    cout << "  Rounded bin: " << calculateLonBin(lon) << endl;
    cout << "===================================" << endl;
  }
};

void printUsage() {
  cout << "Usage: optimized_aprobe -A<latitude> -B<longitude> -P<prefix> "
          "-D<path_to_data>"
       << endl;
  cout
      << "Example: optimized_aprobe -A4.36 -B-74.04 -PBOG -D/path/to/nasa/data/"
      << endl;
  cout << "Options:" << endl;
  cout << "  -A<lat>    Latitude (e.g., -A4.36)" << endl;
  cout << "  -B<lon>    Longitude (e.g., -B-74.04)" << endl;
  cout << "  -P<prefix> Location prefix (e.g., -PBOG)" << endl;
  cout << "  -D<path>   Path to NASA data directory" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printUsage();
    return 1;
  }

  float lat = 0, lon = 0;
  string prefix, pathToData;
  bool hasLat = false, hasLon = false, hasPrefix = false, hasPath = false;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      cerr << "Error: Invalid argument format: " << argv[i] << endl;
      printUsage();
      return 1;
    }

    switch (argv[i][1]) {
    case 'A':
      lat = strtof(&argv[i][2], nullptr);
      hasLat = true;
      break;
    case 'B':
      lon = strtof(&argv[i][2], nullptr);
      hasLon = true;
      break;
    case 'P':
      prefix = string(&argv[i][2]);
      hasPrefix = true;
      break;
    case 'D':
      pathToData = string(&argv[i][2]);
      hasPath = true;
      break;
    default:
      cerr << "Error: Unknown option: " << argv[i] << endl;
      printUsage();
      return 1;
    }
  }

  // Validate all required parameters
  if (!hasLat || !hasLon || !hasPrefix || !hasPath) {
    cerr << "Error: Missing required parameters" << endl;
    printUsage();
    return 1;
  }

  if (prefix.empty()) {
    cerr << "Error: Empty prefix provided" << endl;
    return 1;
  }

  if (pathToData.empty()) {
    cerr << "Error: Empty data path provided" << endl;
    return 1;
  }

  cout << "Parameters:" << endl;
  cout << "  Latitude: " << lat << endl;
  cout << "  Longitude: " << lon << endl;
  cout << "  Prefix: " << prefix << endl;
  cout << "  Data Path: " << pathToData << endl;

  auto start = chrono::high_resolution_clock::now();

  OptimizedAprobe aprobe(lat, lon, prefix, pathToData);

  // Enable debug output for coordinate calculations
  aprobe.debugCoordinates();

  bool success = aprobe.process();

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

  cout << "Processing time: " << duration.count() << " ms" << endl;

  return success ? 0 : 1;
}
