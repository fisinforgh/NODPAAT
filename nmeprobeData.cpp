#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h> // for getpid()

namespace fs = std::filesystem;
using namespace std;

constexpr float XMin = -179.375f;
constexpr float step = 1.25f;
constexpr int bpLine = 25; // bins per line

void usage() {
  cout << "-A<latitude> -B<longitude> -P<prefix i.e BOG> -D</path/to/data> "
          "-S<opt>"
       << endl;
  cout << "In this case, please use opt = 1 for nimbus" << endl;
  cout << "                         opt = 2 for meteor" << endl;
  cout << "                         opt = 3 for  earth" << endl;
  exit(8);
}

float lat{};
float lon{};
string prefix{};
string pathtodata{};
int opt{};

int main(int argc, char *argv[]) {

  if (argc != 6) {
    usage();
  }

  while ((argc > 1) && (argv[1][0] == '-')) {
    switch (argv[1][1]) {

      // latitude
    case 'A':
      lat = strtof(&argv[1][2], nullptr);
      break;

      // longitude
    case 'B':
      lon = strtof(&argv[1][2], nullptr);
      break;

      // International airport code (most representative if there are more
      // than one i.e BOG
    case 'P':
      prefix = string(&argv[1][2]);
      break;

      // path to download data from NASA
      //  files must have in a directory by year. Each must start with
      //  aura_<YEAR>
    case 'D':
      pathtodata = string(&argv[1][2]);
      break;

    case 'S':
      opt = strtol(&argv[1][2], nullptr, 10);
      break;

    default:
      cerr << "Please check options ... " << '\n' << '\n';
      usage();
    }

    ++argv;
    --argc;
  }

  cout << "Lat: " << lat << " Lon: " << lon << "location: " << prefix << endl;

  // Get process ID to make temporary files unique
  pid_t process_id = getpid();

  // Check if directory exists using std::filesystem
  if (!fs::exists(pathtodata) || !fs::is_directory(pathtodata)) {
    cerr << "path to data does not exist. Please check.." << endl;
    exit(8);
  }

  cout << "Path to data exists!! " << endl;
  cout << pathtodata << endl;

  cout << " location : " << prefix << endl;

  const float latHalf = (lat >= 0) ? ceil(lat) - 0.5f : ceil(lat) + 0.5f;

  if (abs(latHalf) > 89.5) {
    cerr << "latitude not valid.." << endl;
    cerr << "Please check." << endl;
    exit(8);
  }

  const float lonBin = (lon - XMin) / step + 1;
  const int rLonBin = static_cast<int>(round(lonBin));

  // in order to check if rLonBin - (rLonBin/bpLine)*25 is equal to cero
  // or bigger. If cero, it must take line before it is calculated to
  // obtain the string data and take the las 3 characteres of it.
  // Otherwise everithing is ok
  const int chkLonLine = rLonBin - (rLonBin / bpLine) * 25;

  // reference line between 1 and 25
  // find the line number from 1 to 12 (25 bins times 12 = 300)
  // last bin has 13 bins plus " lat =  ### " reference
  const int rnLine = rLonBin / bpLine + 1;
  int nLine; // line to read from file

  size_t found;

  string strFName;
  string str3e;
  string strYY;
  string strMM;
  string strDD;

  string sFileName;
  string fileName;

  ifstream inList;
  ifstream inFile;
  ifstream inLog;
  ifstream inLogLatLine;
  ofstream outFile;

  int latLine;
  int YMIN, YMAX;

  float ud;

  // Unique temporary file names using process ID
  const string listFile = "list_" + to_string(process_id) + ".txt";
  const string logLatLineFile = "logLatLine_" + to_string(process_id) + ".txt";
  const string logLonLineFile = "logLonLine_" + to_string(process_id) + ".txt";

  string sat;
  if (opt == 1) {
    YMIN = 1979;
    YMAX = 1993;
    sat = "nimbus_";
  } else if (opt == 2) {
    YMIN = 1994;
    YMAX = 1994;
    sat = "meteor_";
  } else if (opt == 3) {
    YMIN = 1996;
    YMAX = 2004;
    sat = "earth_";
  } else {
    cerr << "Check options opt ?? .. " << endl;
    exit(9);
  }

  cout << " sat: " << sat << endl;

  for (int i = YMIN; i <= YMAX; i++) {
    const string outData = prefix + "_" + to_string(i) + ".dat";
    outFile.open(outData);

    const string dirFileName = pathtodata + sat + to_string(i);

    cout << "dir: " << dirFileName << endl;

    const string cmdName = "ls -1 " + dirFileName + "/L3*.txt > " + listFile;

    system(cmdName.c_str());

    cout << "PROCESSING YEAR:  " << i << " ..." << endl;

    inList.open(listFile);
    int dd = 0;
    while (!inList.eof()) {
      dd++;
      inList >> fileName;
      cout << "fileName: " << fileName << endl;

      inFile.open(fileName);

      // Build latitude string with proper formatting
      string cLatHalf;
      char buffer[20];
      snprintf(buffer, sizeof(buffer), "%0.1f", latHalf);
      cLatHalf = buffer;

      string cmdSed = "sed -n '/lat =  ";
      if (latHalf < -10) {
        cmdSed += cLatHalf;
      } else if ((latHalf > -10) && (latHalf < 0)) {
        cmdSed += " " + cLatHalf;
      } else if ((latHalf > 0) && (latHalf < 10)) {
        cmdSed += "  " + cLatHalf;
      } else {
        cmdSed += " " + cLatHalf;
      }

      cmdSed += "/=' " + fileName + " > " + logLatLineFile;
      system(cmdSed.c_str());

      // latLine = 0;
      inLogLatLine.open(logLatLineFile);
      inLogLatLine >> latLine;
      inLogLatLine.close();
      fs::remove(logLatLineFile);

      nLine = latLine - 12 + rnLine;

      if (chkLonLine == 0)
        nLine = nLine - 1;

      const string cmdAwk = "awk 'FNR==" + to_string(nLine) + "' " + fileName +
                            " > " + logLonLineFile;

      system(cmdAwk.c_str());

      string sLine;
      string strBin;

      ifstream inLogLonLine;
      inLogLonLine.open(logLonLineFile);
      getline(inLogLonLine, sLine);
      inLogLonLine.close();
      fs::remove(logLonLineFile);

      if (chkLonLine == 0) {
        strBin = sLine.substr(73, 3);
      } else {
        // there are 25*3 characters. sLine has 76 = 75 * a space from
        // getline. We must add one position from getline and substract
        // 3 positions because it will read the next value fro latitude
        strBin =
            sLine.substr((rLonBin - (rLonBin / bpLine) * 25) * 3 - 3 + 1, 3);
      }
      cout << "ud: " << strBin << endl;

      ud = strtof(strBin.c_str(), nullptr);

      sFileName = fileName;

      if (opt == 1) {
        str3e = "n7t";
        found = sFileName.find(str3e);
      } else if (opt == 2) {
        str3e = "m3t";
        found = sFileName.find(str3e);
      } else {
        str3e = "epc";
        found = sFileName.find(str3e);
      }

      strYY = sFileName.substr(found + 4, 4);
      strMM = sFileName.substr(found + 4 + 4, 2);
      strDD = sFileName.substr(found + 4 + 4 + 2, 2);

      if (ud <= 0)
        ud = -1;

      outFile << strDD << "\t" << strMM << "\t" << strYY << "\t" << ud << endl;
      // cout << strDD << "\t" << strMM << "\t" << strYY << "\t" << ud << endl;

    } // while
    inList.close();
    fs::remove(listFile);
    outFile.close();

  } // for

  return 0;
}
