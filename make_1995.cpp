#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#include <cstring>
#include <sstream>

using namespace std;

void usage(){
  cerr << "-P<prefix>" << endl;
  exit(8);
}

float funDY(int n, int y){

  float cDay;
  
  if(y%4!=0){
    
    if((n>=1)&&(n<=31))
      cDay = n+0.01;
    else if((n>=32)&&(n<=59))
      cDay = n+0.02- 31;
    else if((n>=60)&&(n<=90))
      cDay = n+0.03- 59;
    else if((n>=91)&&(n<=120))
      cDay = n+0.04- 90;
    else if((n>=121)&&(n<=151))
      cDay = n+0.05- 120;
    else if((n>=152)&&(n<=181))
      cDay = n+0.06- 151;
    else if((n>=182)&&(n<=212))
      cDay = n+0.07- 181;
    else if((n>=213)&&(n<=243))
      cDay = n+0.08- 212;
    else if((n>=244)&&(n<=273))
      cDay = n+0.09- 243;
    else if((n>=274)&&(n<=304))
      cDay = n+0.10- 273;
    else if((n>=305)&&(n<=334))
      cDay = n+0.11- 304;
    else 
      cDay = n+0.12- 334;
  }
  else{
    
    if((n>=1)&&(n<=31))
      cDay = n+0.01;
    else if((n>=32)&&(n<=60))
      cDay = n+0.02- 31;
    else if((n>=61)&&(n<=91))
      cDay = n+0.03- 60;
    else if((n>=92)&&(n<=121))
      cDay = n+0.04- 91;
    else if((n>=122)&&(n<=152))
      cDay = n+0.05- 121;
    else if((n>=153)&&(n<=182))
      cDay = n+0.06- 152;
    else if((n>=183)&&(n<=213))
      cDay = n+0.07- 182;
    else if((n>=214)&&(n<=244))
      cDay = n+0.08- 213;
    else if((n>=245)&&(n<=274))
      cDay = n+0.09- 244;
    else if((n>=275)&&(n<=305))
      cDay = n+0.10- 274;
    else if((n>=306)&&(n<=335))
      cDay = n+0.11- 305;
    else 
      cDay = n+0.12- 335;
   
  }
  
  return cDay;
}


char *prefix = "";

int main(int argc, char *argv[])
{
  
  if(argc != 2){
    usage();
  }
  
  while((argc > 1) && (argv[1][0] == '-')){
    switch (argv[1][1]){
      
      //representative airport code i.e. BOG for Bogota
    case 'P':
      prefix = &(argv[1][2]);
      break;
      
    default:
      cerr << "Please check options .. " << '\n' <<'\n';
      usage();
    }
    
    ++argv;
    --argc;
  }
  
  char preLoc[100];
  sprintf(preLoc,"%s",prefix);
  cout << "Location : " << preLoc << endl;
  
  ofstream outFile;
  
  char cmdName[500];
  char outFileName[500];

  strcpy(outFileName,preLoc);
  strcat(outFileName,"_1995.dat");
  
  outFile.open(outFileName);
  cout << "output: " << outFileName << endl;
  
  int cDD, cMM, cYY, doy; //integers current day month
  float fcDD, fcMM, fcYY; //floats current day month
  float cDM;
  
  cYY=1995;
  for( doy = 1; doy <= 365; doy++){
    
    cDM = funDY(doy, cYY);
    fcDD = floor(cDM);
    fcMM = cDM*100 - fcDD*100;
    
    cDD = round(fcDD);
    cMM = round(fcMM);
    outFile << cDD <<  "\t" << cMM  << "\t" << cYY << "\t" << -2 << endl;
  }
  outFile.close();
}
