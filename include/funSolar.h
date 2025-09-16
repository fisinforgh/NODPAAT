// sun's declination
double fdh(int nn){
  double ang = (360/365.0)*(284+nn);
  ang = ang*TMath::Pi()/180.0;
  return 1/180.0*TMath::Pi()*23.45*TMath::Sin(ang); // in radians
}

//Aparent solar time (AST) or true solar time
// tr: current time , nn day number
// ll: longitude at location (-74.04min para bogota)
// NOTE: Most of the calculations I saw were like this:
//       AST = ttr + 4.0*(LSTM-ll)/60.0+ET/60.0;
//      llon > 0 when is Western Longitude i.e for example for Bogota
//      is lomgitude 74.07 W = -74.07, this means llon = 74.07: llon = abs(ll)
//      For Estern longitudes llon=-llon
//
//      This algoritm will be implemented like this:
//       AST = ttr + 4.0*(LSTM + llon)/60.0+ET/60.0; 
//      be aware the "+" sign instead of "-" from before. In this case
//      longitude llon = llon i.e If lon= -74.07, then llon = -74.07 
//      corresponding to Western longitude; otherwise, for estern longitudes 
//      llon=+<value>
//        
double fAST(int nn, double ttr, double llon){
  
  double LSTM = 15.0*(llon/15.0);
  double D = 360.0/365.0*(nn-81); // this is in deg
  D = D/180.0*(TMath::Pi());
  double ET = 9.87*TMath::Sin(2.0*D) - 7.53*TMath::Cos(D)-1.5*TMath::Sin(D);
  double AST;
  AST = ttr + 4.0*(LSTM-llon)/60.0+ET/60.0;

  return AST;
}

//hour angle
double fws(double hh){
  double ang = 15*(12-hh);
  return ang*(1/180.0)*TMath::Pi(); // in radians
}

//cosine of solar zenith angle
double fczh(double tth, double ddh, double wws){
  
  tth = tth/180.0*TMath::Pi();
  
  return TMath::Cos(tth)*TMath::Cos(ddh)*TMath::Cos(wws)
    + TMath::Sin(tth)*TMath::Sin(ddh);
}

//(Ro/Rt)² = Isei/Isc  (sc:solar constant)
double fRoR2(double hh, int nn){
  double a0=1.00011;
  double a1=0.034221;
  double a2=0.00128;
  double a3=0.000719;
  double a4=0.000077;
  double ang = 360*nn/365.0;
  ang = ang*(1/180.0)*TMath::Pi(); // radianes
  //return ang;
  return 1 + 0.033*TMath::Cos(ang);
  //return a0 + a1*TMath::Cos(hh) + a2*TMath::Sin(hh) + a3*TMath::Cos(2*hh) + a4*TMath::Sin(2*hh);
}

//GI_se = Isei/Isc*cos(thz) [ cos(thz)=czh ]  (sc:solar constant)
double fGIse(double RRoR2, double cczh){
  return RRoR2*cczh;
}

