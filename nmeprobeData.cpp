#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h> // for getpid()

using namespace std;

const float XMin = -179.375;
const float step = 1.25;
const int bpLine = 25; // bins per line

void usage() {
  cout << "-A<latitude> -B<longitude> -P<prefix i.e BOG> -D</path/to/data> "
          "-S<opt>"
       << endl;
  cout << "In this case, please use opt = 1 for nimbus" << endl;
  cout << "                         opt = 2 for meteor" << endl;
  cout << "                         opt = 3 for  earth" << endl;
  exit(8);
}

float lat;
float lon;
char *prefix = "";
char *pathtodata = "";
int opt;

int main(int argc, char *argv[]) {

  if (argc != 6) {
    usage();
  }

  while ((argc > 1) && (argv[1][0] == '-')) {
    switch (argv[1][1]) {

      // latitude
    case 'A':
      lat = atof(&argv[1][2]);
      break;

      // longitude
    case 'B':
      lon = atof(&argv[1][2]);
      break;

      // International airport code (most representative if there are more
      // than one i.e BOG
    case 'P':
      prefix = &(argv[1][2]);
      break;

      // path to download data from NASA
      //  files must have in a directory by year. Each must start with
      //  aura_<YEAR>
    case 'D':
      pathtodata = &(argv[1][2]);
      break;

    case 'S':
      opt = atoi(&argv[1][2]);
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

  // checking if dir exists
  ifstream inLogChkDir;

  char cmdChkDir[200];
  char logChkDirFile[100];
  int chkDir;

  sprintf(logChkDirFile, "logChkDir_%d.txt", process_id);

  strcpy(cmdChkDir, "test -d ");
  strcat(cmdChkDir, pathtodata);
  strcat(cmdChkDir, " ; echo $? > ");
  strcat(cmdChkDir, logChkDirFile);

  system(cmdChkDir);

  inLogChkDir.open(logChkDirFile);
  inLogChkDir >> chkDir;
  inLogChkDir.close();
  if (chkDir != 0) {
    cerr << "path to data does not exist. Please check.." << endl;
    char rmCmd[150];
    sprintf(rmCmd, "rm %s", logChkDirFile);
    system(rmCmd);
    exit(8);
  } else {
    cout << "Path to data exists!! " << endl;
    cout << pathtodata << endl;
    char rmCmd[150];
    sprintf(rmCmd, "rm %s", logChkDirFile);
    system(rmCmd);
  }

  char preLoc[100];
  sprintf(preLoc, "%s", prefix);
  cout << " location : " << preLoc << endl;

  float latHalf = 0.0;

  if (lat >= 0) {
    latHalf = ceil(lat) - 0.5;
  } else {
    latHalf = ceil(lat) + 0.5;
  }

  if (abs(latHalf) > 89.5) {
    cerr << "latitude not valid.." << endl;
    cerr << "Please check." << endl;
    exit(8);
  }

  float lonBin;
  lonBin = (lon - XMin) / (step) + 1;
  int rLonBin;
  rLonBin = round(lonBin);

  // in order to check if rLonBin - (rLonBin/bpLine)*25 is equal to cero
  // or bigger. If cero, it must take line before it is calculated to
  // obtain the string data and take the las 3 characteres of it.
  // Otherwise everithing is ok
  int chkLonLine;
  chkLonLine = rLonBin - (rLonBin / bpLine) * 25;

  int rnLine; // reference line between 1 and 25
  int nLine;  // line to reaf from file
  char cnLine[100];
  rnLine = rLonBin / bpLine +
           1; // find the line number from 1 to 12 (25 bins times 12 = 300)
  // last bin has 13 bins plus " lat =  ### " reference

  stringstream ss;
  stringstream ss1;
  size_t found;

  string strFName;
  string str3e;
  string strYY;
  string strMM;
  string strDD;

  string sFileName;
  char fileName[1000];
  char dirFileName[1000];

  ifstream inList;
  ifstream inFile;
  ifstream inLog;
  ifstream inLogLatLine;
  ofstream outFile;

  int latLine;
  int YMIN, YMAX;

  char outData[100];
  char yy[100];
  float ud;

  char cmdName[1000];
  char sat[100];

  // Unique temporary file names using process ID
  char listFile[100];
  char logLatLineFile[100];
  char logLonLineFile[100];

  sprintf(listFile, "list_%d.txt", process_id);
  sprintf(logLatLineFile, "logLatLine_%d.txt", process_id);
  sprintf(logLonLineFile, "logLonLine_%d.txt", process_id);

  if (opt == 1) {
    YMIN = 1979;
    YMAX = 1993;
    sprintf(sat, "nimbus_");
  } else if (opt == 2) {
    YMIN = 1994;
    YMAX = 1994;
    sprintf(sat, "meteor_");
  } else if (opt == 3) {
    YMIN = 1996;
    YMAX = 2004;
    sprintf(sat, "earth_");
  } else {
    cerr << "Check options opt ?? .. " << endl;
    exit(9);
  }

  cout << " sat: " << sat << endl;

  for (int i = YMIN; i <= YMAX; i++) {
    sprintf(outData, "%s_%d.dat", preLoc, i);
    outFile.open(outData);

    strcpy(dirFileName, pathtodata);
    strcat(dirFileName, sat);
    sprintf(yy, "%d", i);
    strcat(dirFileName, yy);

    cout << "dir: " << dirFileName << endl;

    strcpy(cmdName, "ls -1 ");
    strcat(cmdName, dirFileName);

    // fix: before was /*.txt. From https server, a robots.txt file appears
    // so the list will have a file different from NASA data (starts with L3)
    // for nimbus, meteor and earth probe missions
    // strcat(cmdName, "/*.txt > list.txt");
    strcat(cmdName, "/L3*.txt > ");
    strcat(cmdName, listFile);

    system(cmdName);

    cout << "PROCESSING YEAR:  " << i << " ..." << endl;

    inList.open(listFile);
    int dd = 0;
    while (!inList.eof()) {
      dd++;
      inList >> fileName;
      cout << "fileName: " << fileName << endl;

      inFile.open(fileName);

      char cLatHalf[10];
      sprintf(cLatHalf, "%0.1f", latHalf);

      char cmdSed[500];
      strcpy(cmdSed, "sed -n '/lat =  ");
      if (latHalf < -10) {
        strcat(cmdSed, cLatHalf);
      } else if ((latHalf > -10) && (latHalf < 0)) {
        strcat(cmdSed, " ");
        strcat(cmdSed, cLatHalf);
      } else if ((latHalf > 0) && (latHalf < 10)) {
        strcat(cmdSed, "  ");
        strcat(cmdSed, cLatHalf);
      } else {
        strcat(cmdSed, " ");
        strcat(cmdSed, cLatHalf);
      }

      strcat(cmdSed, "/=' ");
      strcat(cmdSed, fileName);
      strcat(cmdSed, " > ");
      strcat(cmdSed, logLatLineFile);
      system(cmdSed);

      // latLine = 0;
      inLogLatLine.open(logLatLineFile);
      inLogLatLine >> latLine;
      inLogLatLine.close();
      char rmLatCmd[150];
      sprintf(rmLatCmd, "rm %s", logLatLineFile);
      system(rmLatCmd);

      nLine = latLine - 12 + rnLine;

      if (chkLonLine == 0)
        nLine = nLine - 1;

      sprintf(cnLine, "%d", nLine);

      char cmdAwk[500];
      strcpy(cmdAwk, "awk 'FNR==");
      strcat(cmdAwk, cnLine);
      strcat(cmdAwk, "' ");
      strcat(cmdAwk, fileName);
      strcat(cmdAwk, " > ");
      strcat(cmdAwk, logLonLineFile);

      system(cmdAwk);

      string sLine;
      string strBin;

      ifstream inLogLonLine;
      inLogLonLine.open(logLonLineFile);
      getline(inLogLonLine, sLine);
      inLogLonLine.close();
      char rmLonCmd[150];
      sprintf(rmLonCmd, "rm %s", logLonLineFile);
      system(rmLonCmd);

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

      ss1.str("");
      ss1.clear();

      ss1 << strBin;
      ss1 >> ud;

      // it is very important to clear the stringstram ss
      ss.str("");
      ss.clear();

      ss << fileName;
      ss >> sFileName;

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
    char rmListCmd[150];
    sprintf(rmListCmd, "rm %s", listFile);
    system(rmListCmd);
    outFile.close();

  } // for

  return 0;
}
