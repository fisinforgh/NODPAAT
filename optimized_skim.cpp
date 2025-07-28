// optimized_skim.cpp
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

class OptimizedSkim {
private:
  string prefix;

  // Pre-calculated lookup tables for date conversions

  static constexpr array<int, 12> days_in_month_normal = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  static constexpr array<int, 12> days_in_month_leap = {31, 29, 31, 30, 31, 30,
                                                        31, 31, 30, 31, 30, 31};
  static constexpr array<int, 13> cumulative_days_normal = {
      0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  static constexpr array<int, 13> cumulative_days_leap = {
      0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

  struct DateData {
    int day, month, year;
    float value;

    DateData() : day(0), month(0), year(0), value(0) {}
    DateData(int d, int m, int y, float v)
        : day(d), month(m), year(y), value(v) {}
  };

  // Fast leap year check using bitwise operations
  inline bool isLeapYear(int year) const noexcept {
    return (year & 3) == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  // day-of-year calculation using lookup tables
  inline int dayOfYear(int day, int month, int year) const noexcept {
    if (month < 1 || month > 12 || day < 1)
      return -1;

    const auto &cumulative =
        isLeapYear(year) ? cumulative_days_leap : cumulative_days_normal;
    const auto &days_in_month =
        isLeapYear(year) ? days_in_month_leap : days_in_month_normal;

    if (day > days_in_month[month - 1])
      return -1;

    return cumulative[month - 1] + day;
  }

  // conversion from day-of-year to calendar date
  pair<int, int> dayOfYearToDate(int doy, int year) const noexcept {
    if (doy < 1)
      return {0, 0};

    const auto &cumulative =
        isLeapYear(year) ? cumulative_days_leap : cumulative_days_normal;

    // Binary search for month (faster than linear search)
    int month = lower_bound(cumulative.begin() + 1, cumulative.end(), doy) -
                cumulative.begin();
    int day = doy - cumulative[month - 1];

    return {day, month};
  }

  // Get all .dat files in directory efficiently using filesystem
  vector<string> getDataFiles() const {
    vector<string> files;
    string dirPath = prefix + "/";

    try {
      files.reserve(50); // Reserve space to avoid reallocations
      for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dat") {
          files.push_back(entry.path().string());
        }
      }
    } catch (const fs::filesystem_error &e) {
      cerr << "Error reading directory: " << e.what() << endl;
    }

    sort(files.begin(), files.end()); // Ensure consistent ordering
    return files;
  }

  // Fast file reading with better I/O buffer
  vector<DateData> readFileData(const string &filename) const {
    vector<DateData> data;
    ifstream file(filename);

    if (!file.is_open()) {
      cerr << "Cannot open file: " << filename << endl;
      return data;
    }

    // Use larger buffer for better I/O performance
    file.rdbuf()->pubsetbuf(nullptr, 8192);

    data.reserve(400); // Reserve space based on original code

    int day, month, year;
    float value;

    while (file >> day >> month >> year >> value) {
      data.emplace_back(day, month, year, value);
    }

    return data;
  }

public:
  explicit OptimizedSkim(const string &prefix) : prefix(prefix) {}

  bool process() {
    cout << "Processing location: " << prefix << endl;

    // Create output directory
    string outputDir = "skim_" + prefix;
    fs::create_directories(outputDir);

    string outputFile = outputDir + "/" + prefix + ".dat";
    ofstream outFile(outputFile);

    if (!outFile.is_open()) {
      cerr << "Cannot create output file: " << outputFile << endl;
      return false;
    }

    // Use larger buffer for output
    outFile.rdbuf()->pubsetbuf(nullptr, 8192);

    // Get all data files
    vector<string> files = getDataFiles();

    cout << "Found " << files.size() << " files to process" << endl;

    for (const string &filename : files) {
      cout << "Processing file: " << filename << endl;

      vector<DateData> fileData = readFileData(filename);
      if (fileData.empty())
        continue;

      // Get year from first entry
      int year = fileData[0].year;
      int numDays = isLeapYear(year) ? 366 : 365;

      // Create lookup map for faster access (hash map is O(1) vs O(n) array
      // search)
      unordered_map<int, size_t> dayToIndex;
      dayToIndex.reserve(fileData.size());

      for (size_t i = 0; i < fileData.size(); ++i) {
        if (fileData[i].day > 0 && fileData[i].month > 0) {
          int doy = dayOfYear(fileData[i].day, fileData[i].month, year);
          if (doy > 0) {
            dayToIndex[doy] = i;
          }
        }
      }

      // Process each day of the year
      for (int doy = 1; doy <= numDays; ++doy) {
        auto it = dayToIndex.find(doy);
        if (it != dayToIndex.end()) {
          // Data exists for this day
          const DateData &data = fileData[it->second];
          outFile << data.day << '\t' << data.month << '\t' << data.year << '\t'
                  << data.value << '\n';
        } else {
          // No data for this day, generate calendar date
          auto [day, month] = dayOfYearToDate(doy, year);
          outFile << day << '\t' << month << '\t' << year << '\t' << -3 << '\n';
        }
      }
    }

    cout << "Processing completed successfully" << endl;
    return true;
  }
};

void printUsage() {
  cout << "Usage: optimized_skim -P<prefix>" << endl;
  cout << "Example: optimized_skim -PBOG" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printUsage();
    return 1;
  }

  string prefix;
  if (argv[1][0] == '-' && argv[1][1] == 'P') {
    prefix = string(&argv[1][2]);
  } else {
    printUsage();
    return 1;
  }

  if (prefix.empty()) {
    cerr << "Error: Empty prefix provided" << endl;
    printUsage();
    return 1;
  }

  auto start = chrono::high_resolution_clock::now();

  OptimizedSkim skim(prefix);
  bool success = skim.process();

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

  cout << "Processing time: " << duration.count() << " ms" << endl;

  return success ? 0 : 1;
}
