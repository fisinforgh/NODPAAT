// This program shows relationship between total o3 and sunspot number
// Author: Julian Salamanca
// Version: 1.0
// root -l "macroEveChi2.C(<step>, <palette number>,<event cutoff>)"

#include <iostream>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<fstream>
#include "TMath.h"

Double_t fitLinear(Double_t *x, Double_t *par) {
  return par[0]+par[1]*x[0];
}

void statsMacroChi2LRSO3Sn(int h, int pal, int eve){
  
  //=====STYLE====================================
  
  gStyle->SetTitleFontSize(0.08);
  //  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(510,"Y");
  gStyle->SetNdivisions(510,"X");
  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");
  gStyle->SetTitleOffset(0.6);
  
  gStyle->SetTitleXOffset(0.7);
  gStyle->SetTitleYOffset(0.8);
  
  gStyle->SetLabelOffset(0.002,"X");
  gStyle->SetLabelOffset(0.002,"Y");
  
  gStyle->SetTitleXSize(0.06);
  gStyle->SetTitleYSize(0.06);

  gStyle->SetOptFit(1);
  gStyle->SetOptStat("e");
  gStyle->SetStatW(0.30);
  gStyle->SetStatH(0.2);

  gStyle->SetGridColor(1);
  gStyle->SetGridWidth(2);
  gStyle->SetGridStyle(7);
  
  gStyle->SetCanvasPreferGL(1);
  gStyle->SetPalette(pal);

  
  char fileROOTName[50];
  sprintf(fileROOTName, "linChi2_%dx%d_eveCut_%d.root", h,h,eve);
  char fileStatsName[50];
  sprintf(fileStatsName, "linChi2_%dx%d_eveCut_%d.dat", h,h,eve);

  ofstream outFile;
  outFile.open(fileStatsName);

  //=====END===STYLE===============

  TFile* file = new TFile(fileROOTName, "RECREATE");

  TH2F *hEarth = new TH2F("hEarth","Earth", 180, -180, 180, 90, -90, 90);
  // TH2F *hChi2 = new TH2F("hChi2", "hChi2", 180, -180, 180, 90, -90, 90);
  // TH2F *hChi2Cut = new TH2F("hChi2Cut", "hChi2Cut", 180, -180, 180, 180, -90, 90);

  // TH2F *hSlope = new TH2F("hSlope", "hSlope", 180, -180, 180, 90, -90, 90);
  // TH2F *hSlopeCut = new TH2F("hSlopeCut", "hSlopeCut", 180, -180, 180, 90, -90, 90);

  // TH2F *hb = new TH2F("hb", "hb", 180, -180, 180, 90, -90, 90);
  // TH2F *hbCut = new TH2F("hbCut", "hbCut", 180, -180, 180, 90, -90, 90);

  
  ifstream inChi2File, inEarthData;
  ofstream outChi2FileCut;
  ofstream outChi2FileNoCut;

  char dirPathName[500];
  
  const int lonMin = -180;
  const int lonMax = 180;
  const int latMin = -90;
  const int latMax = 90;

  const int sizeGrid=16500;
  //  int h = 2;

  
  Double_t b = 0;
  Double_t slope = 0;
  Double_t chi2NDF = 0;
  Double_t chi2 = 0;
  Double_t NDF = 0;
  Double_t errb = 0;
  Double_t errSlope = 0;
  Double_t prob = 0;

  Double_t bCut = 0;
  Double_t slopeCut = 0;
  Double_t chi2NDFCut = 0;
  Double_t chi2Cut = 0;
  Double_t NDFCut = 0;
  Double_t errbCut = 0;
  Double_t errSlopeCut = 0;
  Double_t probCut = 0;

  
  Double_t vLon[sizeGrid];
  Double_t vLat[sizeGrid];
  Double_t vSlope[sizeGrid];
  Double_t vSlopeCut[sizeGrid];
  Double_t vb[sizeGrid];
  Double_t vbCut[sizeGrid];
  Double_t vChi2NDF[sizeGrid];
  Double_t vChi2NDFCut[sizeGrid];

  Double_t vProbChi2NDF[sizeGrid];
  Double_t vProbChi2NDFCut[sizeGrid];
  Double_t vNDFCut[sizeGrid];

  Double_t avChi2NDFCut=0;
  Double_t avChi2Cut=0;
  Double_t avNDFCut=0;
  Double_t avNDF=0;
  Double_t avProb=0;
  Double_t avProbCut=0;

  int chi2Count0    = 0;
  int chi2Count0_5  = 0;
  int chi2Count0_75 = 0;
  int chi2Count1    = 0;
  int chi2Count1_25 = 0;
  int chi2Count1_5  = 0;
  int chi2Count1_75 = 0;
  int chi2Count2    = 0;

  int probCont0_2p5 = 0;
  int probCont2p5_5 = 0;
  int probCont2p5_97p5 = 0;
  int probCont5_95 = 0;
  int probCont95_1 = 0;
  int probCont97p5_1 = 0;
  int probCont1 = 0;
  int probCont2 = 0;
  int probCont3 = 0;
  int probCont4 = 0;
  int probCont5 = 0;
  int probCont6 = 0;
  int probCont7 = 0;
  int probCont8 = 0;
  int probCont9 = 0;
  int probCont10 = 0;
    
  int nCont=-1; // warning: if starts from 0, there is o point in all graphics
  //IMPORTANT: It seems LON180 jas cero values on TCO3 values so we MUST draw
  // upto strictly LESS THAN 180 (NOT equal to)
  for(int i=lonMin; i < lonMax; i=i+h ){
    for(int j=latMin; j <= latMax; j=j+h ){
      
      //if((j!=0)&&(i!=0)){
      nCont++;
      
      sprintf(dirPathName,"skim_LAT%dLON%d/LAT%dLON%d_fitlinear.dat", j, i, j, i);
      //cout << dirPathName << endl;
      inChi2File.open(dirPathName);
      
      
      //	while(!inChi2File.eof()){
      for(int rows = 0; rows < 1; rows++){
	inChi2File >> b;
	inChi2File >> slope;
	inChi2File >> chi2NDF;
	inChi2File >> chi2;
	inChi2File >> NDF;
	inChi2File >> errb;
	inChi2File >> errSlope;
	inChi2File >> prob;
	
	inChi2File >> bCut;
	inChi2File >> slopeCut;
	inChi2File >> chi2NDFCut;
	inChi2File >> chi2Cut;
	inChi2File >> NDFCut;
	inChi2File >> errbCut;
	inChi2File >> errSlopeCut;
	inChi2File >> probCut;
	
	vLon[nCont]     = i;
	vLat[nCont]     = j;
	vSlopeCut[nCont]= slopeCut;
	vbCut[nCont]    = bCut;
	
	vChi2NDF[nCont]    = chi2NDF;
	vChi2NDFCut[nCont] = chi2NDFCut;
	
	//	hChi2->Fill(i, j, chi2NDF);
	//hChi2Cut->Fill(i, j, chi2NDFCut);
	
	//cout << chi2NDFCut << endl;
	
	// This to lines were used to check a constant values for a LON180, thas why LON was taken "less than" 180
	//if((bCut > 346.91)&&(bCut < 346.92))
	//cout << "bCut: " << bCut << " slope: " << slopeCut << " lat: " << j << " lon: " << i << endl;
	
	//vProbChi2NDFCut[nCont] = TMath::Prob(chi2Cut, NDFCut);
	vProbChi2NDF[nCont] = prob;
	vProbChi2NDFCut[nCont] = probCut;
	
	vNDFCut[nCont] = NDFCut;
	
	avNDF        += NDF ;
	avNDFCut     += NDFCut ;
	avChi2Cut    += chi2Cut ;
	avChi2NDFCut += chi2NDFCut ;
	avProbCut    += probCut ;
	
	if((chi2NDFCut >= 0)   &&(chi2NDFCut < 0.5))
	  chi2Count0++;
	if((chi2NDFCut >= 0.5) &&(chi2NDFCut < 0.75))
	  chi2Count0_5++;
	if((chi2NDFCut >= 0.75)&&(chi2NDFCut < 1.0))
	  chi2Count0_75++;
	if((chi2NDFCut >= 1.0) &&(chi2NDFCut < 1.25))
	  chi2Count1++;
	if((chi2NDFCut >= 1.25)&&(chi2NDFCut < 1.5))
	  chi2Count1_25++;
	if((chi2NDFCut >= 1.5) &&(chi2NDFCut < 1.75))
	  chi2Count1_5++;
	if((chi2NDFCut >= 1.75)&&(chi2NDFCut < 2.0))
	  chi2Count1_75++;
	if(chi2NDFCut >= 2.0)
	  chi2Count2++;
	
	
	//cout << "probCut: " << probCut << endl;
	
	if(probCut < 0.025)
	  probCont0_2p5++;
	if ((probCut >= 0.025)&&(probCut < 0.05))
	  probCont2p5_5++;
	if ((probCut >= 0.025)&&(probCut <= 0.975))
	  probCont2p5_97p5++;
	if ((probCut >= 0.05)&&(probCut <= 0.95))
	  probCont5_95++;
	if ((probCut > 0.95)&&(probCut <= 1.0))
	  probCont95_1++;
	if ((probCut > 0.975)&&(probCut <= 1.0))
	  probCont97p5_1++;


	if(probCut < 0.1)
	  probCont1++;
	if((probCut >= 0.1)&&(probCut < 0.2))
	  probCont2++;
	if((probCut >= 0.2)&&(probCut < 0.3))
	  probCont3++;
	if((probCut >= 0.3)&&(probCut < 0.4))
	  probCont4++;
	if((probCut >= 0.4)&&(probCut < 0.5))
	  probCont5++;
	if((probCut >= 0.5)&&(probCut < 0.6))
	  probCont6++;
	if((probCut >= 0.6)&&(probCut < 0.7))
	  probCont7++;
	if((probCut >= 0.7)&&(probCut < 0.8))
	  probCont8++;
	if((probCut >= 0.8)&&(probCut < 0.9))
	  probCont9++;
	if((probCut >= 0.9)&&(probCut <= 1.0 ))
	  probCont10++;
	
	
	
	//} if
	inChi2File.close();
      }
    }
  }

  
  cout << nCont << endl;

  Int_t x,y;
  int z;
  
  inEarthData.open("earth.dat");
  
  int nn=-1;
  while(!inEarthData.eof()){
    nn++;
    inEarthData >> x;
    inEarthData >> y;
    z = 1.0;

    //    hEarth->SetBinContent(x,y,z);
    hEarth->Fill(x,y,z);
    
    
    
  }
  inEarthData.close();
  
  
  //filling linChi2 Data
  outFile << "Grilla: " << h <<"x" << h << " eveCut: " << eve << endl;
  outFile << endl;
  
  outFile << "chi entre 0 y 0.5: "     << chi2Count0     << endl;
  outFile << "chi entre 0.5 y 0.75: "  << chi2Count0_5   << endl;
  outFile << "chi entre 0.75 y 1.0: "  << chi2Count0_75  << endl;
  outFile << "chi entre 1 y 1.25: "    << chi2Count1     << endl;
  outFile << "chi entre 1.25 y 1.5: "  << chi2Count1_25  << endl;
  outFile << "chi entre 1.5 y 1.75: "  << chi2Count1_5   << endl;
  outFile << "chi entre 1.75 y 2: "    << chi2Count1_75  << endl;
  outFile << "chi entre 2 y mas "      << chi2Count2     << endl;

  outFile << endl;

  outFile << "suma: " << chi2Count0 +chi2Count0_5 +chi2Count0_75 +chi2Count1 +chi2Count1_25 +chi2Count1_5 + chi2Count1_75 + chi2Count2 << endl;
  outFile << "nCont: " << nCont << endl;

  int press = 4;
  outFile << endl;

  outFile << "chi entre 0 y 0.5: "    << setprecision(press) << (float)chi2Count0   /(float)nCont*100  << "%" << endl;
  outFile << "chi entre 0.5 y 0.75: " << setprecision(press) << (float)chi2Count0_5 /(float)nCont*100  << "%" << endl;
  outFile << "chi entre 0.75 y 1.0: " << setprecision(press) << (float)chi2Count0_75/(float)nCont*100  << "%" << endl;
  outFile << "chi entre 1.0 y 1.25: " << setprecision(press) << (float)chi2Count1   /(float)nCont*100  << "%" << endl;
  outFile << "chi entre 1.25 y 1.5: " << setprecision(press) << (float)chi2Count1_25/(float)nCont*100  << "%" << endl;
  outFile << "chi entre 1.5 y 1.75: " << setprecision(press) << (float)chi2Count1_5 /(float)nCont*100  << "%" << endl;
  outFile << "chi entre 1.75 y 2 "    << setprecision(press) << (float)chi2Count1_75/(float)nCont*100  << "%" << endl;
  outFile << "chi entre 2 y mas "     << setprecision(press) << (float)chi2Count2   /(float)nCont*100  << "%" << endl;

  outFile << endl;
  
  outFile << " avNDF: "        << setprecision(press+2) << avNDF/nCont     << endl;
  outFile << " avNDFCut: "     << avNDFCut/nCont     << endl;
  outFile << " avChi2Cut: "    << avChi2Cut/nCont    << endl;
  outFile << " avChi2NDFCut: " << avChi2NDFCut/nCont << endl;
  
  outFile << endl;
  
  outFile << "ProbCut entre 0 y 0.025: "    << setprecision(press) << (float)probCont0_2p5   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.025 y 0.05: " << setprecision(press) << (float)probCont2p5_5   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.025 y 0.975: "<< setprecision(press) << (float)probCont2p5_97p5/(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.05 y 0.95: "  << setprecision(press) << (float)probCont5_95    /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.95 y 1: "     << setprecision(press) << (float)probCont95_1    /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.975 y 1: "    << setprecision(press) << (float)probCont97p5_1  /(float)nCont*100  << "%" << endl;

  outFile << endl;
  
  outFile << "ProbCut entre 0 y 0.025: "     << probCont0_2p5    << endl;
  outFile << "ProbCut entre 0.025 y 0.05: "  << probCont2p5_5    << endl;
  outFile << "ProbCut entre 0.025 y 0.975: " << probCont2p5_97p5 << endl;
  outFile << "ProbCut entre 0.05 y 0.95: "   << probCont5_95     << endl;
  outFile << "ProbCut entre 0.95 y 1: "      << probCont95_1     << endl;
  outFile << "ProbCut entre 0.975 y 1: "     << probCont97p5_1   << endl;

  outFile << endl;

  outFile << "ProbCut entre 0 y 0.1:   " << probCont1  << " " << (float)probCont1   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.1 y 0.2: " << probCont2  << " " << (float)probCont2   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.2 y 0.3: " << probCont3  << " " << (float)probCont3   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.3 y 0.4: " << probCont4  << " " << (float)probCont4   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.4 y 0.5: " << probCont5  << " " << (float)probCont5   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.5 y 0.6: " << probCont6  << " " << (float)probCont6   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.6 y 0.7: " << probCont7  << " " << (float)probCont7   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.7 y 0.8: " << probCont8  << " " << (float)probCont8   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.8 y 0.9: " << probCont9  << " " << (float)probCont9   /(float)nCont*100  << "%" << endl;
  outFile << "ProbCut entre 0.9 y 1.0: " << probCont10 << " " << (float)probCont10  /(float)nCont*100  << "%" << endl;
  
  outFile << endl;
  
  outFile << "avProbCut: " << avProbCut/nCont << endl;
  outFile << "Total probCur: " <<
    probCont1 +  probCont2 +  probCont3 +  probCont4 +  probCont5
    + probCont6 +  probCont7 +  probCont8 +  probCont9 +  probCont10 << endl;

  outFile.close();
  
  cout << "Dat file ready: " << fileStatsName << endl;
  //________________________________________________

  

  //------------- CHI2
  TCanvas *cchi2 = new TCanvas("chi2", "chi2",1500,1000);
  cchi2->Divide(2,2);
  
  TGraph2D *gChi2 = new TGraph2D(nCont, vLon, vLat, vChi2NDF);
  gChi2->SetName("Chi2");
  TGraph *gChi2XZ = new TGraph(nCont, vLon, vChi2NDF);
  gChi2XZ->SetName("Chi2XZ");
  TGraph *gChi2YZ = new TGraph(nCont, vLat, vChi2NDF);
  gChi2YZ->SetName("Chi2YZ");
  
  cchi2->cd(1);
  gChi2->Draw("surf1z");
  
  cchi2->cd(2);
  gChi2XZ->Draw("ap");
  
  cchi2->cd(3);
  gChi2YZ->Draw("ap");
  
  cchi2->cd(4);
  //gChi2Cut->DrawClone("cont1");
  //  hEarth->DrawClone("colz same");
  //hEarth->Draw("text same");
  gChi2->Draw("colz");

  //_________________________________

  //------------- CHI2 CUT
  
  TCanvas *cchi2cut = new TCanvas("chi2cut", "chi2cut",1500,1000);
  TGraph2D *gChi2Cut = new TGraph2D(nCont, vLon, vLat, vChi2NDFCut);
  gChi2Cut->SetName("Chi2Cut");
  
  cchi2cut->cd();
  gChi2Cut->GetHistogram()->GetYaxis()->CenterTitle();
  gChi2Cut->GetHistogram()->GetXaxis()->CenterTitle();
  gChi2Cut->GetHistogram()->GetYaxis()->CenterTitle();
  gChi2Cut->GetHistogram()->GetXaxis()->CenterTitle();
  gChi2Cut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gChi2Cut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gChi2Cut->GetHistogram()->GetXaxis()->SetTitleOffset(0.7);
  gChi2Cut->GetHistogram()->GetYaxis()->SetTitleOffset(0.7);
  gChi2Cut->Draw("colz");
  hEarth->DrawClone("same");
  // cchi2cut->Update();
  //cchi2cut->Modified();
  
  //_________________________________

  //------------- CHI2 Projections
  
  TCanvas *cchi2cutxy = new TCanvas("chi2cutxy", "chi2cutxy",1500,500);
  cchi2cutxy->Divide(2,1);
  
  TGraph *gChi2CutXZ = new TGraph(nCont, vLon, vChi2NDFCut);
  gChi2CutXZ->SetName("Chi2CutXZ");
  TGraph *gChi2CutYZ = new TGraph(nCont, vLat, vChi2NDFCut);
  gChi2CutYZ->SetName("gChi2CutYZ");
  
  cchi2cutxy->cd(2)->SetGridx();
  cchi2cutxy->cd(2)->SetGridy();
  gChi2CutXZ->SetMarkerStyle(24);
  gChi2CutXZ->SetMarkerSize(0.8);
  gChi2CutXZ->GetXaxis()->SetRangeUser(-182,182);
  gChi2CutXZ->GetYaxis()->SetRangeUser(0,3);
  gChi2CutXZ->GetXaxis()->SetTitle("Longitud");
  gChi2CutXZ->GetYaxis()->SetTitle("#chi^{2}_{Red}");
  gChi2CutXZ->GetXaxis()->CenterTitle();
  gChi2CutXZ->GetYaxis()->CenterTitle();
  gChi2CutXZ->DrawClone("ap");

  cchi2cutxy->cd(1)->SetGridx();
  cchi2cutxy->cd(1)->SetGridy();
  gChi2CutYZ->SetMarkerStyle(24);
  gChi2CutYZ->SetMarkerSize(0.8);
  gChi2CutYZ->GetXaxis()->SetRangeUser(-92,92);
  gChi2CutYZ->GetYaxis()->SetRangeUser(0,3);
  gChi2CutYZ->GetXaxis()->SetTitle("Latitud");
  gChi2CutYZ->GetYaxis()->SetTitle("#chi^{2}_{Red}");
  gChi2CutYZ->GetXaxis()->CenterTitle();
  gChi2CutYZ->GetYaxis()->CenterTitle();
  gChi2CutYZ->DrawClone("ap");
  
  //_________________________________


  //------------- slope
  
  TCanvas *cslopecut = new TCanvas("slopecut", "slopecut",1500,1000);
  cslopecut->Divide(2,2);


  TGraph2D *gSlopeCut = new TGraph2D(nCont, vLon, vLat, vSlopeCut);
  gSlopeCut->SetName("SlopeCut");
  TGraph *gSlopeCutXZ = new TGraph(nCont, vLon, vSlopeCut);
  gSlopeCutXZ->SetName("SlopeCutXZ");
  TGraph *gSlopeCutYZ = new TGraph(nCont, vLat, vSlopeCut);
  gSlopeCutYZ->SetName("SlopeCutYZ");
  
  cslopecut->cd(1);
  //gSlopeCut->GetXaxis()->SetRangeUser(-180,180);
  //gSlopeCut->GetYaxis()->SetRangeUser(-90,90);
  gSlopeCut->DrawClone("surf1z");
  
  cslopecut->cd(2)->SetGridx();
  cslopecut->cd(2)->SetGridy();
  gSlopeCutXZ->SetMarkerStyle(24);
  gSlopeCutXZ->SetMarkerSize(0.8);
  gSlopeCutXZ->GetXaxis()->SetRangeUser(-182, 182);
  gSlopeCutXZ->GetYaxis()->SetRangeUser(-0.15, 0.15);
  gSlopeCutXZ->GetXaxis()->SetTitle("Longitud");
  gSlopeCutXZ->GetYaxis()->SetTitle("a_{1}");
  gSlopeCutXZ->GetXaxis()->CenterTitle();
  gSlopeCutXZ->GetYaxis()->CenterTitle();
  gSlopeCutXZ->Draw("ap");

  cslopecut->cd(3)->SetGridx();
  cslopecut->cd(3)->SetGridy();
  gSlopeCutYZ->SetMarkerStyle(24);
  gSlopeCutYZ->SetMarkerSize(0.8);
  gSlopeCutYZ->GetXaxis()->SetRangeUser(-92,92);
  gSlopeCutYZ->GetYaxis()->SetRangeUser(-0.15,0.15);
  gSlopeCutYZ->GetXaxis()->SetTitle("Latitud");
  gSlopeCutYZ->GetYaxis()->SetTitle("a_{1}");
  gSlopeCutYZ->GetXaxis()->CenterTitle();
  gSlopeCutYZ->GetYaxis()->CenterTitle();
  gSlopeCutYZ->Draw("ap");
  

  cout << "stage9" << endl;

  
  cslopecut->cd(4);
  gSlopeCut->GetHistogram()->GetYaxis()->CenterTitle();
  gSlopeCut->GetHistogram()->GetXaxis()->CenterTitle();
  gSlopeCut->GetHistogram()->GetYaxis()->CenterTitle();
  gSlopeCut->GetHistogram()->GetXaxis()->CenterTitle();
  gSlopeCut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gSlopeCut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gSlopeCut->GetHistogram()->GetXaxis()->SetTitleOffset(0.7);
  gSlopeCut->GetHistogram()->GetYaxis()->SetTitleOffset(0.7);

  gSlopeCut->Draw("colz");
  hEarth->DrawClone("sames");
  
  //_________________________________

  //------------- intercept b 

  TCanvas *cbcut = new TCanvas("bcut", "bcut",1500,1000);
  cbcut->Divide(2,2);
  
  TGraph2D *gbCut = new TGraph2D(nCont, vLon, vLat, vbCut);
  gbCut->SetName("bCut");
  TGraph *gbCutXZ = new TGraph(nCont, vLon, vbCut);
  TGraph *gbCutYZ = new TGraph(nCont, vLat, vbCut);
  
  cbcut->cd(1);
  gbCut->GetHistogram()->GetYaxis()->CenterTitle();
  gbCut->GetHistogram()->GetXaxis()->CenterTitle();
  gbCut->GetHistogram()->GetYaxis()->CenterTitle();
  gbCut->GetHistogram()->GetXaxis()->CenterTitle();
  gbCut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gbCut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gbCut->GetHistogram()->GetXaxis()->SetTitleOffset(1.2);
  gbCut->GetHistogram()->GetYaxis()->SetTitleOffset(1.2);
  gbCut->DrawClone("surf1z");

  cbcut->cd(2)->SetGridx();
  cbcut->cd(2)->SetGridy();
  gbCutXZ->SetMarkerStyle(24);
  gbCutXZ->SetMarkerSize(0.8);
  gbCutXZ->GetXaxis()->SetRangeUser(-182, 182);
  gbCutXZ->GetYaxis()->SetRangeUser(0, 420);
  gbCutXZ->GetXaxis()->SetTitle("Longitud");
  gbCutXZ->GetYaxis()->SetTitle("a_{0}");
  gbCutXZ->GetXaxis()->CenterTitle();
  gbCutXZ->GetYaxis()->CenterTitle();
  gbCutXZ->Draw("ap");

  cbcut->cd(3)->SetGridx();
  cbcut->cd(3)->SetGridy();
  gbCutYZ->SetMarkerStyle(24);
  gbCutYZ->SetMarkerSize(0.8);
  gbCutYZ->GetXaxis()->SetRangeUser(-92,92);
  gbCutYZ->GetYaxis()->SetRangeUser(0, 420);
  gbCutYZ->GetXaxis()->SetTitle("Latitud");
  gbCutYZ->GetYaxis()->SetTitle("a_{0}");
  gbCutYZ->GetXaxis()->CenterTitle();
  gbCutYZ->GetYaxis()->CenterTitle();
  gbCutYZ->Draw("ap");
  

  cbcut->cd(4);
  gbCut->GetHistogram()->GetYaxis()->CenterTitle();
  gbCut->GetHistogram()->GetXaxis()->CenterTitle();
  gbCut->GetHistogram()->GetYaxis()->CenterTitle();
  gbCut->GetHistogram()->GetXaxis()->CenterTitle();
  gbCut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gbCut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gbCut->GetHistogram()->GetXaxis()->SetTitleOffset(0.7);
  gbCut->GetHistogram()->GetYaxis()->SetTitleOffset(0.7);
  gbCut->Draw("colz");
  hEarth->DrawClone("same");
  

  
  //_________________________________

  //------------- prob cut 

  TCanvas *cprobCut = new TCanvas("probchi2cut", "probchi2cut",1500,500);
  cprobCut->Divide(2,1);
  
  TGraph2D *gProbChi2Cut = new TGraph2D(nCont, vLon, vLat, vProbChi2NDFCut);
  gProbChi2Cut->SetName("Prob Cut");
  TGraph2D *gNDFCut = new TGraph2D(nCont, vLon, vLat, vNDFCut);
  gNDFCut->SetName("NDF Cut");
 
  cprobCut->cd(1);
  gProbChi2Cut->GetHistogram()->GetYaxis()->CenterTitle();
  gProbChi2Cut->GetHistogram()->GetXaxis()->CenterTitle();
  gProbChi2Cut->GetHistogram()->GetYaxis()->CenterTitle();
  gProbChi2Cut->GetHistogram()->GetXaxis()->CenterTitle();
  gProbChi2Cut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gProbChi2Cut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gProbChi2Cut->GetHistogram()->GetXaxis()->SetTitleOffset(0.7);
  gProbChi2Cut->GetHistogram()->GetYaxis()->SetTitleOffset(0.7);
  gProbChi2Cut->Draw("colz");
  hEarth->Draw("same");
  

  cprobCut->cd(2);
  gNDFCut->GetHistogram()->GetYaxis()->CenterTitle();
  gNDFCut->GetHistogram()->GetXaxis()->CenterTitle();
  gNDFCut->GetHistogram()->GetYaxis()->CenterTitle();
  gNDFCut->GetHistogram()->GetXaxis()->CenterTitle();
  gNDFCut->GetHistogram()->GetYaxis()->SetTitle("Latitud");
  gNDFCut->GetHistogram()->GetXaxis()->SetTitle("Longitud");
  gNDFCut->GetHistogram()->GetXaxis()->SetTitleOffset(0.7);
  gNDFCut->GetHistogram()->GetYaxis()->SetTitleOffset(0.7);
  gNDFCut->Draw("colz");
  hEarth->DrawClone("same");

  file->WriteObject(hEarth, "earth");
  file->WriteObject(cchi2, "chi2");
  file->WriteObject(cchi2cut,"chi2cut");
  file->WriteObject(cchi2cutxy,"chi2cutxy");
  file->WriteObject(cslopecut,"slopecut");
  file->WriteObject(cbcut,"bcut");
  file->WriteObject(cprobCut,"probCut");

}
