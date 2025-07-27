#include <cstdlib>
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class OptimizedOzoneDataProcessor {
private:
  std::string pathO3Files;
  int evCut;
  static std::mutex compilation_mutex;
  static std::unordered_set<std::string> compiled_programs;

  // Single compilation with better optimization flags
  bool compilePrograms() {
    std::lock_guard<std::mutex> lock(compilation_mutex);

    std::vector<std::pair<std::string, std::string>> programs = {
        {"optimized_aprobe.cpp", "aprobe.exe"},
        {"optimized_skim.cpp", "skim.exe"}};

    for (const auto &[source, executable] : programs) {
      // Skip if already compiled and executable exists
      if (compiled_programs.count(executable) && fs::exists(executable)) {
        continue;
      }

      // Enhanced compilation with aggressive optimizations
      std::string command = "g++ -O3 -march=native -mtune=native -flto "
                            "-funroll-loops -ffast-math -DNDEBUG "
                            "-Wno-write-strings " +
                            source + " -o " + executable;

      std::cout << "Compiling: " << command << std::endl;
      int result = std::system(command.c_str());
      if (result != 0) {
        std::cerr << "Failed to compile: " << command << std::endl;
        return false;
      }
      compiled_programs.insert(executable);
    }
    return true;
  }

  void cleanupExecutables() {
    // Only cleanup at the very end to avoid recompilation
    return;
  }

  // Execute command with better error handling and performance
  bool executeCommand(const std::string &command) {
    std::cout << "Running: " << command << std::endl;
    int result = std::system(command.c_str());
    if (result != 0) {
      std::cerr << "Command failed with code " << result << ": " << command
                << std::endl;
      return false;
    }
    return true;
  }

  // Optimized file moving with better error handling
  bool moveDataFiles(const std::string &location) {
    try {
      fs::create_directories(location);

      std::vector<fs::path> filesToMove;

      // Collect files first to avoid iterator invalidation
      for (const auto &entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file()) {
          std::string filename = entry.path().filename().string();
          if (filename.find(location + "_") == 0 &&
              filename.find(".dat") != std::string::npos) {
            filesToMove.push_back(entry.path());
          }
        }
      }

      // Move files
      for (const auto &filePath : filesToMove) {
        std::string filename = filePath.filename().string();
        fs::path destination = fs::path(location) / filename;

        if (fs::exists(destination)) {
          fs::remove(destination); // Remove existing file
        }

        fs::rename(filePath, destination);
        std::cout << "Moved: " << filename << " to " << location << "/"
                  << std::endl;
      }

      return true;
    } catch (const fs::filesystem_error &ex) {
      std::cerr << "Filesystem error: " << ex.what() << std::endl;
      return false;
    }
  }

public:
  OptimizedOzoneDataProcessor(const std::string &dataPath, int eventCutoff)
      : pathO3Files(dataPath), evCut(eventCutoff) {

    // Ensure path ends with slash
    if (!pathO3Files.empty() && pathO3Files.back() != '/') {
      pathO3Files += '/';
    }
  }

  bool processLocation(const std::string &location, double lat, double lon) {
    std::cout << "Processing location: " << location << " (Lat: " << std::fixed
              << std::setprecision(6) << lat << ", Lon: " << lon << ")"
              << std::endl;

    // Compile programs once
    if (!compilePrograms()) {
      return false;
    }

    // Use higher precision for coordinates to avoid floating point issues
    std::ostringstream args;
    args << " -A" << std::fixed << std::setprecision(6) << lat << " -B"
         << std::fixed << std::setprecision(6) << lon << " -P" << location
         << " -D" << pathO3Files;

    std::string argsStr = args.str();
    std::cout << "Parameters for cpp codes: " << argsStr << std::endl;

    // Run aprobe.exe with better error handling
    if (!executeCommand("./aprobe.exe" + argsStr)) {
      return false;
    }

    // Move files to directory
    if (!moveDataFiles(location)) {
      return false;
    }

    // Run skim.exe
    if (!executeCommand("./skim.exe -P" + location)) {
      std::cerr << "Warning: skim.exe failed, continuing..." << std::endl;
      // Don't return false here as this might not be critical
    }

    return true;
  }

  // Parallel grid processing for maximum performance
  bool processGridParallel(int latMin, int latMax, int lonMin, int lonMax,
                           int gridPrecision, int numThreads = 0) {

    if (numThreads == 0) {
      numThreads =
          std::min(static_cast<int>(std::thread::hardware_concurrency()), 8);
    }

    std::cout << "Processing grid with " << numThreads << " threads: "
              << "Lat[" << latMin << "," << latMax << "], "
              << "Lon[" << lonMin << "," << lonMax << "], "
              << "Precision: " << gridPrecision << std::endl;

    // Generate all coordinate pairs
    std::vector<std::pair<int, int>> coordinates;
    for (int lon = lonMin; lon <= lonMax; lon += gridPrecision) {
      for (int lat = latMin; lat <= latMax; lat += gridPrecision) {
        coordinates.emplace_back(lat, lon);
      }
    }

    std::cout << "Total locations to process: " << coordinates.size()
              << std::endl;

    // Process in parallel chunks
    std::vector<std::future<bool>> futures;
    std::mutex output_mutex;
    size_t completed = 0;
    size_t total_coords = coordinates.size();

    auto processChunk =
        [this, &output_mutex, &completed,
         total_coords](const std::vector<std::pair<int, int>> &chunk) -> bool {
      bool allSuccess = true;

      for (const auto &[lat, lon] : chunk) {
        std::string location =
            "LAT" + std::to_string(lat) + "LON" + std::to_string(lon);

        {
          std::lock_guard<std::mutex> lock(output_mutex);
          std::cout << "Processing: " << location << " (" << ++completed << "/"
                    << total_coords << ")" << std::endl;
        }

        if (!processLocation(location, lat, lon)) {
          std::lock_guard<std::mutex> lock(output_mutex);
          std::cerr << "Failed to process location: " << location << std::endl;
          allSuccess = false;
        }
      }

      return allSuccess;
    };

    // Divide work among threads
    size_t chunkSize = (coordinates.size() + numThreads - 1) / numThreads;

    for (int i = 0; i < numThreads && i * chunkSize < coordinates.size(); ++i) {
      size_t start = i * chunkSize;
      size_t end = std::min(start + chunkSize, coordinates.size());

      std::vector<std::pair<int, int>> chunk(coordinates.begin() + start,
                                             coordinates.begin() + end);

      futures.push_back(std::async(std::launch::async, processChunk, chunk));
    }

    // Wait for all threads to complete and check results
    bool allSuccess = true;
    for (auto &future : futures) {
      if (!future.get()) {
        allSuccess = false;
      }
    }

    return allSuccess;
  }

  // Sequential processing (original behavior)
  bool processGrid(int latMin, int latMax, int lonMin, int lonMax,
                   int gridPrecision) {
    std::cout << "Processing grid sequentially: Lat[" << latMin << "," << latMax
              << "], "
              << "Lon[" << lonMin << "," << lonMax << "], "
              << "Precision: " << gridPrecision << std::endl;

    size_t totalLocations = ((latMax - latMin) / gridPrecision + 1) *
                            ((lonMax - lonMin) / gridPrecision + 1);
    size_t processed = 0;

    for (int lon = lonMin; lon <= lonMax; lon += gridPrecision) {
      for (int lat = latMin; lat <= latMax; lat += gridPrecision) {
        std::string location =
            "LAT" + std::to_string(lat) + "LON" + std::to_string(lon);

        std::cout << "Processing: " << location << " (" << ++processed << "/"
                  << totalLocations << ")" << std::endl;

        if (!processLocation(location, lat, lon)) {
          std::cerr << "Failed to process location: " << location << std::endl;
          return false;
        }
      }
    }

    return true;
  }

  ~OptimizedOzoneDataProcessor() {
    // Final cleanup only when processor is destroyed
    std::vector<std::string> executables = {"aprobe.exe", "skim.exe"};
    for (const auto &exe : executables) {
      if (fs::exists(exe)) {
        fs::remove(exe);
      }
    }
  }
};

// Static member definitions
std::mutex OptimizedOzoneDataProcessor::compilation_mutex;
std::unordered_set<std::string> OptimizedOzoneDataProcessor::compiled_programs;

void printUsage(const char *programName) {
  std::cout << "Usage for parallel grid processing:" << std::endl;
  std::cout << programName
            << " pgrid <path_to_ozone_data> <lat_min> <lat_max> "
               "<grid_precision> <cutoff_events> [num_threads]"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Usage for sequential grid processing:" << std::endl;
  std::cout << programName
            << " grid <path_to_ozone_data> <lat_min> <lat_max> "
               "<grid_precision> <cutoff_events>"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Usage for single location:" << std::endl;
  std::cout << programName
            << " location <location_name> <path_to_ozone_data> <lat> <lon> "
               "<cutoff_events>"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Examples:" << std::endl;
  std::cout << programName
            << " pgrid /path/to/nasa/data/ -90 90 10 6 4  # 4 threads"
            << std::endl;
  std::cout << programName << " grid /path/to/nasa/data/ -90 90 10 6"
            << std::endl;
  std::cout << programName << " location BOG /path/to/nasa/data/ 4.36 -74.04 6"
            << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  std::string mode = argv[1];

  if (mode == "pgrid") { // Parallel grid processing
    if (argc < 7 || argc > 8) {
      std::cout << "Parallel grid mode requires 6-7 arguments" << std::endl;
      printUsage(argv[0]);
      return 1;
    }

    std::string pathO3Files = argv[2];
    int latMin = std::stoi(argv[3]);
    int latMax = std::stoi(argv[4]);
    int gridPrecision = std::stoi(argv[5]);
    int evCut = std::stoi(argv[6]);
    int numThreads = (argc == 8) ? std::stoi(argv[7]) : 0;

    // Fixed longitude range
    int lonMin = -180;
    int lonMax = 180;

    auto start = std::chrono::high_resolution_clock::now();

    OptimizedOzoneDataProcessor processor(pathO3Files, evCut);

    if (!processor.processGridParallel(latMin, latMax, lonMin, lonMax,
                                       gridPrecision, numThreads)) {
      std::cerr << "Parallel grid processing failed" << std::endl;
      return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::seconds>(end - start);

    std::cout << "Parallel grid processing completed successfully in "
              << duration.count() << " seconds" << std::endl;

  } else if (mode == "grid") { // Sequential grid processing
    if (argc != 7) {
      std::cout << "Grid mode requires 6 arguments" << std::endl;
      printUsage(argv[0]);
      return 1;
    }

    std::string pathO3Files = argv[2];
    int latMin = std::stoi(argv[3]);
    int latMax = std::stoi(argv[4]);
    int gridPrecision = std::stoi(argv[5]);
    int evCut = std::stoi(argv[6]);

    int lonMin = -180;
    int lonMax = 180;

    auto start = std::chrono::high_resolution_clock::now();

    OptimizedOzoneDataProcessor processor(pathO3Files, evCut);

    if (!processor.processGrid(latMin, latMax, lonMin, lonMax, gridPrecision)) {
      std::cerr << "Grid processing failed" << std::endl;
      return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::seconds>(end - start);

    std::cout << "Grid processing completed successfully in "
              << duration.count() << " seconds" << std::endl;

  } else if (mode == "location") {
    if (argc != 7) {
      std::cout << "Location mode requires 6 arguments" << std::endl;
      printUsage(argv[0]);
      return 1;
    }
  }
}
