#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class OzoneDataProcessor {
private:
    std::string pathO3Files;
    int evCut;
    
    // Compile and run external programs
    bool compilePrograms() {
        std::vector<std::string> compileCommands = {
            // "g++ -Wno-write-strings nmeprobeData.cpp -o nmeprobe.exe",
            "g++ -Wno-write-strings optimized_aprobe.cpp -o aprobe.exe",
            // "g++ -Wno-write-strings make_1995.cpp -o make_1995.exe",
            "g++ -Wno-write-strings optimized_skim.cpp -o skim.exe"
        };
        
        for (const auto& command : compileCommands) {
            std::cout << "Compiling: " << command << std::endl;
            int result = std::system(command.c_str());
            if (result != 0) {
                std::cerr << "Failed to compile: " << command << std::endl;
                return false;
            }
        }
        return true;
    }
    
    void cleanupExecutables() {
        std::vector<std::string> executables = {
            "nmeprobe.exe", "aprobe.exe", "make_1995.exe", "skim.exe"
        };
        
        for (const auto& exe : executables) {
            if (fs::exists(exe)) {
                fs::remove(exe);
            }
        }
    }
    
public:
    OzoneDataProcessor(const std::string& dataPath, int eventCutoff) 
        : pathO3Files(dataPath), evCut(eventCutoff) {}
    
    bool processLocation(const std::string& location, double lat, double lon) {
        std::cout << "Processing location: " << location 
                  << " (Lat: " << lat << ", Lon: " << lon << ")" << std::endl;
        
        // Compile programs
        if (!compilePrograms()) {
            return false;
        }
        
        // Prepare command arguments
        std::string args = " -A" + std::to_string(lat) + 
                          " -B" + std::to_string(lon) + 
                          " -P" + location + 
                          " -D" + pathO3Files;
        
        std::cout << "Parameters for cpp codes: " << args << std::endl;
        
        // Run nmeprobe.exe with different S values
        // nmeprobe appears to process NASA ozone data with different sensor/source parameters (S=1,2,3)
        // This likely extracts ozone measurements from different satellite instruments or data sources
        /*
        std::vector<int> sValues = {1, 2, 3};
        for (int s : sValues) {
            std::string command = "./nmeprobe.exe" + args + " -S" + std::to_string(s);
            std::cout << "Running: " << command << std::endl;
            int result = std::system(command.c_str());
            if (result != 0) {
                std::cerr << "Failed to run nmeprobe.exe with S=" << s << std::endl;
                cleanupExecutables();
                return false;
            }
        }
        */
        
        // Run aprobe.exe
        std::string aprobeCommand = "./aprobe.exe" + args;
        std::cout << "Running: " << aprobeCommand << std::endl;
        int result = std::system(aprobeCommand.c_str());
        if (result != 0) {
            std::cerr << "Failed to run aprobe.exe" << std::endl;
            cleanupExecutables();
            return false;
        }
        
        // Run make_1995.exe
        // make_1995 likely processes or normalizes ozone data to a baseline year (1995)
        // This could be for temporal standardization or creating reference measurements
        /*
        std::string make1995Command = "./make_1995.exe -P" + location;
        std::cout << "Running: " << make1995Command << std::endl;
        result = std::system(make1995Command.c_str());
        if (result != 0) {
            std::cerr << "Failed to run make_1995.exe" << std::endl;
            cleanupExecutables();
            return false;
        }
        */
        
        // Create directory and move files
        try {
            fs::create_directory(location);
            
            // Move all files matching pattern location_*.dat to the directory
            for (const auto& entry : fs::directory_iterator(".")) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find(location + "_") == 0 && filename.find(".dat") != std::string::npos) {
                        fs::rename(entry.path(), location + "/" + filename);
                        std::cout << "Moved: " << filename << " to " << location << "/" << std::endl;
                    }
                }
            }
        } catch (const fs::filesystem_error& ex) {
            std::cerr << "Filesystem error: " << ex.what() << std::endl;
            cleanupExecutables();
            return false;
        }
        
        // Run skim.exe
        std::string skimCommand = "./skim.exe -P" + location;
        std::cout << "Running: " << skimCommand << std::endl;
        result = std::system(skimCommand.c_str());
        if (result != 0) {
            std::cerr << "Failed to run skim.exe" << std::endl;
        }
        
        // Cleanup
        cleanupExecutables();
        
        return true;
    }
    
    bool processGrid(int latMin, int latMax, int lonMin, int lonMax, int gridPrecision) {
        std::cout << "Processing grid: Lat[" << latMin << "," << latMax << "], "
                  << "Lon[" << lonMin << "," << lonMax << "], "
                  << "Precision: " << gridPrecision << std::endl;
        
        for (int lon = lonMin; lon <= lonMax; lon += gridPrecision) {
            for (int lat = latMin; lat <= latMax; lat += gridPrecision) {
                std::cout << "===============================================================" << std::endl;
                std::cout << " " << std::endl;
                
                // Generate location string
                std::string location = "LAT" + std::to_string(lat) + "LON" + std::to_string(lon);
                
                std::cout << "Starting: processLocation(" << location 
                          << ", " << lat << ", " << lon << ")" << std::endl;
                
                if (!processLocation(location, lat, lon)) {
                    std::cerr << "Failed to process location: " << location << std::endl;
                    return false;
                }
                
                std::cout << "===============================================================" << std::endl;
            }
        }
        
        return true;
    }
};

void printUsage(const char* programName) {
    std::cout << "Usage for grid processing:" << std::endl;
    std::cout << programName << " grid <path_to_ozone_data> <lat_min> <lat_max> <grid_precision> <cutoff_events>" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage for single location:" << std::endl;
    std::cout << programName << " location <location_name> <path_to_ozone_data> <lat> <lon> <cutoff_events>" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << programName << " grid /path/to/nasa/data/ -90 90 10 6" << std::endl;
    std::cout << programName << " location BOG /path/to/nasa/data/ 4.36 -74.04 6" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "grid") {
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
        
        // Fixed longitude range as in original script
        int lonMin = -180;
        int lonMax = 180;
        
        OzoneDataProcessor processor(pathO3Files, evCut);
        
        if (!processor.processGrid(latMin, latMax, lonMin, lonMax, gridPrecision)) {
            std::cerr << "Grid processing failed" << std::endl;
            return 1;
        }
        
        std::cout << "Grid processing completed successfully" << std::endl;
        
    } else if (mode == "location") {
        if (argc != 7) {
            std::cout << "Location mode requires 6 arguments" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        std::string location = argv[2];
        std::string pathO3Files = argv[3];
        double lat = std::stod(argv[4]);
        double lon = std::stod(argv[5]);
        int evCut = std::stoi(argv[6]);
        
        OzoneDataProcessor processor(pathO3Files, evCut);
        
        if (!processor.processLocation(location, lat, lon)) {
            std::cerr << "Location processing failed" << std::endl;
            return 1;
        }
        
        std::cout << "Location processing completed successfully" << std::endl;
        
    } else {
        std::cout << "Invalid mode. Use 'grid' or 'location'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
