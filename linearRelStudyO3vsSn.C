// This program shows relationship between total o3 and sunspot number
// Author: Julian Salamanca
// Version: 1.0
// root -l "macroAsnvsn.C(1,\"BOG\")"

#include <iostream>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<fstream>

const Int_t YMin = 1979;
const Int_t YMax = 2024; // change this year according to the sn number year

const Int_t maxData = 100000;
const Int_t grYnEvOffSet = -100;
const Int_t grYOffSet = 100;
const Int_t grYMinOffSet = 100;
const Int_t arrMax = (YMax - YMin + 2)*365;
const Int_t binSnMax  = (YMax - 1818 + 2)*365;


Double_t fitLinear(Double_t *x, Double_t *par) {
  return par[0]+par[1]*x[0];
}

void linearRelStudyO3vsSn(Int_t nEvOffSet, const char preLoc[10], double alpha){

  //=====STYLE====================================

  gStyle->SetTitleFontSize(0.08);
  //  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(505,"Y");
  gStyle->SetNdivisions(505,"X");
  gStyle->SetLabelSize(0.08,"X");
  gStyle->SetLabelSize(0.08,"Y");
  gStyle->SetTitleOffset(0.6);
  
  gStyle->SetTitleXOffset(0.70);
  gStyle->SetTitleYOffset(0.43);

  gStyle->SetLabelOffset(0.001,"X");
  gStyle->SetLabelOffset(0.0,"Y");

  gStyle->SetTitleXSize(0.15);
  gStyle->SetTitleYSize(0.16);

  gStyle->SetGridColor(4); 
  gStyle->SetOptFit(1111); //https://root.cern.ch/doc/master/classTPaveStats.html
  gStyle->SetOptStat("e");
  gStyle->SetStatW(0.30);
  gStyle->SetStatH(0.2);

  gStyle->SetCanvasPreferGL(1);
  
  //=====END===STYLE===============

  cout << "Locaton: " << preLoc << endl;

  TFile* file = new TFile("asvssn.root", "RECREATE");
  TCanvas *c1 = new TCanvas("Sunspot Number, o3", preLoc,1000,1000);
  TCanvas *c2 = new TCanvas("Sunspot Number, o3 analysis",preLoc,1000,1000);
  //  TH1F *sn_vs_dd     = new TH1F("sn_vs_dd","Sunspot Number",73730,0,73730);
  TH1F *sn_vs_dd     = new TH1F("sn_vs_dd","Sunspot Number",binSnMax,0,binSnMax);
  TH1F *snskim_vs_dd = new TH1F("snskim_vs_dd","Sunspot Number",arrMax,0,arrMax);
  TH1F *ud_vs_dd     = new TH1F("ud_vs_dd","o3",arrMax,0,arrMax);

  c1->Divide(2,2);

  c2->Divide(1,2);
  c2->cd(1)->Divide(2,1);
  c2->cd(2)->Divide(2,1);
  c2->cd(2)->cd(1)->Divide(1,2);

  //skimed from 1979. With no skimed is data form 1818
  ifstream inSnFile, inSnFileSkim, inFile, inSortFile,inSortLines;
  ofstream outFile, outFileErr, outFileErrSkim, outFitLinear;

  char dirName[500];
  strcpy(dirName,"skim_");
  strcat(dirName,preLoc);
  strcat(dirName,"/");
  
  char dirSnName[500];
  strcpy(dirSnName,"snData/");
    

  char pathFileName[50];
  strcpy(pathFileName,dirName);
  strcat(pathFileName,preLoc);
  strcat(pathFileName,".dat");
  
  char outFileName[500];
  strcpy(outFileName,dirName);
  strcat(outFileName,preLoc);
  strcat(outFileName,"_snavud.dat");

  char outFileErrName[500];
  strcpy(outFileErrName,dirName);
  strcat(outFileErrName,preLoc);
  strcat(outFileErrName,"_snavuderr.dat");

  char outFileErrSkimName[500];
  strcpy(outFileErrSkimName,dirName);
  strcat(outFileErrSkimName,preLoc);
  strcat(outFileErrSkimName,"_snavuderrskim.dat");

  char outFitLinearName[500];
  strcpy(outFitLinearName,dirName);
  strcat(outFitLinearName,preLoc);
  strcat(outFitLinearName,"_fitlinear.dat");

  Int_t udDD, udMM, udYY;
  Double_t ud[arrMax], udSkim[arrMax];

  Int_t snDD, snMM, snYY;
  Double_t sn[maxData], snSkim[arrMax], dat, datSn;
  Double_t snSD[maxData], snSkimSD[arrMax];
  
  char pathSnFileName[500];
  strcpy(pathSnFileName,dirSnName);
  strcat(pathSnFileName,"sndata.dat");

  char pathSnFileNameSkim[500];
  strcpy(pathSnFileNameSkim,dirSnName);
  strcat(pathSnFileNameSkim,"sndataskim.dat");
  
  Int_t nSn=-1;
  Int_t nSnSkim=-1;
  Int_t nUd=-1;

  char lYY[20]; // label year 

  inSnFile.open(pathSnFileName);
  cout << "inSnFile: " << pathSnFileName << endl;
  while(!inSnFile.eof()){
    nSn++;
    
    inSnFile >> snYY;
    inSnFile >> snMM;
    inSnFile >> snDD;
    inSnFile >> dat;
    inSnFile >> sn[nSn];
    inSnFile >> snSD[nSn];
    inSnFile >> dat;
    inSnFile >> dat;

    
    if(nSn==73779)
      cout << nSn << " " << sn[nSn] << endl;

    
    if(nSn%1000 == 0)
      cout << nSn << endl;
    
    if(sn[nSn]>0){
      sn_vs_dd->SetBinContent(nSn + 1,sn[nSn]);
    }
    
    if(snYY%10 == 0){ // this is for setting labels everi 10 years, for 01JAN
      sprintf(lYY, "%d", snYY);
      if((snMM==1)&&(snDD==1)){
	sn_vs_dd->GetXaxis()->SetBinLabel(nSn,lYY);
	cout << "lyy " << lYY << "" << nSn << " " << sn[nSn] << endl;
      }
    }

  }
  inSnFile.close();
  
  cout <<  nSn << " " << sn[nSn] << endl;
  cout << "arrMax: " << arrMax << " " << "size sn: " << binSnMax << endl;
  
  inSnFileSkim.open(pathSnFileNameSkim);
  cout << "inSnFileSkim: " << pathSnFileNameSkim << endl;
  
  
  while(!inSnFileSkim.eof()){
    nSnSkim++;

    inSnFileSkim >> snYY;
    inSnFileSkim >> snMM;
    inSnFileSkim >> snDD;
    inSnFileSkim >> dat;
    inSnFileSkim >> snSkim[nSnSkim];
    inSnFileSkim >> snSkimSD[nSnSkim];
    inSnFileSkim >> dat;
    inSnFileSkim >> dat;


    if(snSkim[nSnSkim]==270){
      cout << snYY << snMM << snDD << " ------------>SD 270: " << snSkimSD[nSnSkim] << endl;
    }
    
    if(nSnSkim%500 == 0)
      cout << nSnSkim << endl;

    if(snSkim[nSnSkim]>0){
      snskim_vs_dd->SetBinContent(nSnSkim + 1,snSkim[nSnSkim]); 
    }
    
    if(snYY%10 == 0){ // this is for setting labels everi 10 years, for 01JAN
      sprintf(lYY, "%d", snYY);
      if((snMM==1)&&(snDD==1)){
	snskim_vs_dd->GetXaxis()->SetBinLabel(nSnSkim,lYY);
      }
    }
        
    snskim_vs_dd->SetBinContent(nSnSkim + 1,snSkim[nSnSkim]);
  }
  inSnFileSkim.close();

  cout <<  nSnSkim << " " << snSkim[nSnSkim] << endl;
  cout << "arrMax: " << arrMax << " " << "size sn: " << binSnMax << endl;
  
  
  inFile.open(pathFileName);
  cout << "inFile: " << pathFileName << endl;
  while(!inFile.eof()){
    nUd++;
    inFile >> udDD;
    inFile >> udMM;
    inFile >> udYY;
    inFile >> ud[nUd];

    if(ud[nUd] > 200)
      udSkim[nUd] = ud[nUd];

    if(udYY%10 == 0){ // this is for setting labels everi 10 years, for 01JAN
      sprintf(lYY, "%d", udYY);
      if((udMM==1)&&(udDD==1)){
	ud_vs_dd->GetXaxis()->SetBinLabel(nUd,lYY);
      }
    }
    ud_vs_dd->SetBinContent(nUd,ud[nUd]);
    
  }
  inFile.close();

  outFile.open(outFileName);

  cout << "outFileName: " << outFileName << endl;
  cout << "nUd: " << nUd << endl;
  cout << snSkim[0] << " " << ud[0] << endl;

  for(Int_t id=0; id <= nSnSkim; id++){
    if(snSkim[id]>0.0){
      if(ud[id]>0.0){
	//outFile << snSkim[id] << "\t" << ud[id] << endl;
      	outFile << snSkim[id] << "\t" << ud[id] << "\t" << snSkimSD[id] << endl;
      }
    }
  }
  outFile.close();

  

  char cmdSort[500];
  strcpy(cmdSort,"sort -n ");
  strcat(cmdSort,outFileName);
  strcat(cmdSort," > sortfile.dat");
  
  system(cmdSort);
  //system("cp sortfile.dat cplogSort.dat");
  system("head -1 sortfile.dat > logSort.txt");
  system("tail -1 sortfile.dat >> logSort.txt");
  system("wc -l sortfile.dat >> logSort.txt");
  
  Int_t csortLines;
  Int_t clastSn;

  Int_t initSn;
  Double_t auxDat;
  inSortLines.open("logSort.txt");
  inSortLines >> initSn;
  inSortLines >> auxDat;
  inSortLines >> auxDat; // from thrid columm on SD of sortfile.dat
  inSortLines >> clastSn;
  inSortLines >> auxDat;
  inSortLines >> auxDat; // from thrid columm on SD of sortfile.dat
  inSortLines >> csortLines;

  inSortLines.close();
  system("rm logSort.txt");

  const Int_t sortLines = csortLines;
  const Int_t lastSn = clastSn;

  // from skim Sn values
  Int_t sortSn, nEv=0;
  Double_t sortUd, sortSnSD;
  
  Double_t avud=0, avsnsd=0;

  cout << "here1"<< endl;
  cout << lastSn << endl;
  cout << sortLines << endl;
  cout << "here2"<< endl;
  //  Double_t arrSortUd[lastSn+1][sortLines+1];

  Double_t erSn[lastSn+1], erUd[lastSn+1], erX[lastSn+1], erY[lastSn+1];
  Double_t udMax[lastSn+1], udMin[lastSn+1], absDS[lastSn+1], ev[lastSn+1];
  Int_t contEr=-1;

  // If 2d array is large, it is recomended to use the heap or vectors
  //double arrSortUd[lastSn+1][sortLines+1]; // this line works in 5.34 but
  // does not work in 6.02.. It seems ROOT 6.02 is cheking for memory

  //I change to this line but it is not corect because we dont know the second
  // dimension
  Double_t arrSortUd[lastSn+1][lastSn+1];
  Double_t arrSortSnSD[lastSn+1][lastSn+1];
  
  cout << "here3"<< endl;


  Double_t erSnSkim[lastSn+1], erUdSkim[lastSn+1], erXSkim[lastSn+1], erYSkim[lastSn+1];
  Double_t udMaxSkim[lastSn+1], udMinSkim[lastSn+1], absDSSkim[lastSn+1], evSkim[lastSn+1];
  Int_t contErSkim=-1;

  inSortFile.open("sortfile.dat");
  cout << "loop over inSortFile: " << endl;

  outFileErr.open(outFileErrName);
  outFileErrSkim.open(outFileErrSkimName);
  while(!inSortFile.eof()){
    inSortFile >> sortSn;
    inSortFile >> sortUd;
    inSortFile >> sortSnSD; // from sn Standar Deviation (SD) in sortfile.dat

    //cout << sortSn << " " << sortUd << " " << sortSnSD << endl;
    
    if(sortSn == initSn){
      nEv++;
      avud+=sortUd;
      avsnsd+=sortSnSD;  // from sn Standar Deviation (SD) in sortfile.dat
      
      arrSortUd[initSn][nEv -1] = sortUd;
      arrSortSnSD[initSn][nEv -1] = sortSnSD;
    }
    else{
      contEr++;
      //contErSkim++;
      ev[contEr] = nEv;
      Double_t *p = new Double_t[nEv];
      Double_t *p_SD = new Double_t[nEv];
      for(Int_t k=0; k<nEv; k++){
	p[k]=arrSortUd[initSn][k];
	p_SD[k]=arrSortSnSD[initSn][k];
	
      }
      
      erSn[contEr] = initSn;
      erUd[contEr] = TMath::Mean(nEv,p);


      //erY[contEr]  = 1.0/(TMath::Sqrt((Double_t)nEv))*TMath::SSE(nEv,p); 
      //erY[contEr]  = 1.0/(TMath::Sqrt((Double_t)nEv))*TMath::StdDev(nEv,p);
      if(nEv > 1){
	
	// SSE: sum of squared of errors Variance(Sn)= SSE/(n-1) SD= sqrt(Variance)
	//Var_s2: variance for TCO (Ud)
	Double_t var_s2=0;
	Double_t SSE_arr_SD = 0;
	for(Int_t k=0; k<nEv; k++){
	  var_s2 += TMath::Power(p[k] - TMath::Mean(nEv,p),2);
	  SSE_arr_SD += TMath::Power(p_SD[k],2);
	}
	//	cout << "S2. " << var_s2 << nEv << endl;
	
	erY[contEr]  = alpha*1.0/(TMath::Sqrt((Double_t)nEv))*TMath::Sqrt(1.0/((Double_t)nEv-1.0)*var_s2);
	//cout << "S2. " << var_s2 << nEv << " ey: " << erY[contEr] << endl;
	//cout << 1.0/(TMath::Sqrt((Double_t)nEv)) << " " << 1.0/(nEv-1.0) << endl;

	erX[contEr]  = TMath::Sqrt(SSE_arr_SD/((Double_t)nEv-1.0));
	//erX[contEr]  = TMath::Sqrt(SSE_arr_SD/((Double_t)nEv));
	//erX[contEr]  = 1.0/((Double_t)nEv)*TMath::Sqrt(SSE_arr_SD);
	//erX[contEr] = 0;
	
	if(sortSn==270){
	  cout << sortSn << "---------SSE " << erX[contEr] << endl;
	}
	
	
      }
      else if(nEv == 1){
	erY[contEr]  = 0;
	erX[contEr]  = 0;
	
	//cout << "Events: " << nEv << " ey: " << erY[contEr] << endl;
      }
      else{
	erY[contEr]  = 0;
	erX[contEr]  = 0;
	
	//cout << "Found errY = 0" << nEv << endl;
      }
      udMax[contEr]=TMath::MaxElement(nEv,p);
      udMin[contEr]=TMath::MinElement(nEv,p);


      //if( erY[contEr] == 0)
      //cout << "Found errY = 0" << endl;

	
      //standar deviation per sn bin
      absDS[contEr] = 0.0;
      for(Int_t q=0; q < nEv; q++){
	absDS[contEr]+=TMath::Abs(p[q]-TMath::Mean(nEv,p));
      }

      if(nEv!=0)
	absDS[contEr]=absDS[contEr]/nEv;
      else
	absDS[contEr]=0.0;

      outFileErr << erSn[contEr]  << "\t" << erUd[contEr]  << "\t"
		 << erX[contEr]   << "\t" << erY[contEr]   << "\t\t"
		 << udMax[contEr] << "\t" << udMin[contEr] << "\t" 
		 << ev[contEr]    << "\t" << absDS[contEr] << endl;
      

      if(nEv > nEvOffSet){
	if(erY[contEr]>0){
	  contErSkim++;
	  
	  erUdSkim[contErSkim]  = erUd[contEr];
	  erSnSkim[contErSkim]  = erSn[contEr];
	  erXSkim[contErSkim]   = erX[contEr];
      	  erYSkim[contErSkim]   = erY[contEr];
	  udMaxSkim[contErSkim] = udMax[contEr];
	  udMinSkim[contErSkim] = udMin[contEr];
	  evSkim[contErSkim]    = ev[contEr];
	  absDSSkim[contErSkim] = absDS[contEr];
	  
	  outFileErrSkim << erSnSkim[contErSkim] << "\t" << erUdSkim[contErSkim] << "\t"
			 << erXSkim[contErSkim]  << "\t" << erYSkim[contErSkim]   << "\t"
			 << udMaxSkim[contErSkim]<< "\t" << udMinSkim[contErSkim] << "\t"
			 << evSkim[contErSkim]   << "\t" << absDSSkim[contErSkim] << endl;
	   
	  }
      }
      
      nEv=1;
      initSn = sortSn;
      avud=sortUd;
      arrSortUd[initSn][nEv -1] = sortUd;
      delete[] p;
    }//else
    
  }//while inSortFile
  inSortFile.close();
  outFileErr.close();
  outFileErrSkim.close();

  cout << "contEr: " << contEr << endl;
  cout << "contErSkim: " << contErSkim << endl;

  strcpy(cmdSort,"mv sortfile.dat skim_");
  strcat(cmdSort,preLoc);
  strcat(cmdSort,"/");
  strcat(cmdSort,preLoc);
  strcat(cmdSort,"_sortSno3.dat");
  
  system(cmdSort);
  
  
  //============== c1 =================
  c1->cd(1);
  c1->cd(1)->SetGridx();
  c1->cd(1)->SetGridy();
  sn_vs_dd->DrawClone("P");
  sn_vs_dd->Rebin(365);
  sn_vs_dd->Scale(365.0*1.0/sn_vs_dd->GetEntries());
  sn_vs_dd->Draw("same");
  
  c1->cd(2);
  c1->cd(2)->SetGridx();
  c1->cd(2)->SetGridy();
  snskim_vs_dd->Draw("P");
  
  c1->cd(3);
  c1->cd(3)->SetGridx();
  c1->cd(3)->SetGridy();
  ud_vs_dd->Draw("P");

  cout << "nUd: " << nUd << endl;
  cout << "nSn: " << nSn << endl;
  cout << "nSnSkim: " << nSnSkim << endl;
  if(nSnSkim != nUd){
    cerr << "Please check nUd MUST have the same lines as nSnSkim" << endl;
    cerr << "Sunspot and TCO data MUST have SAME number of entries" << endl;
    return;
  }
  
  TGraph *grSnUd = new TGraph(nSnSkim,snSkim,ud);
  grSnUd->SetTitle("o3 vs Sunspot Number");
  c1->cd(4);
  c1->cd(4)->SetGridx();
  c1->cd(4)->SetGridy();

  grSnUd->DrawClone("AP");

  //============== c2 =================

  TGraph *grSnUdSkim = new TGraph(nSnSkim,snSkim,udSkim);
  grSnUdSkim->SetTitle("o3 vs Sunspot Number (skim)");
    
  TGraph *grSnUdMax = new TGraph(contEr,erSn,udMax);
  grSnUdMax->SetTitle("o3 vs Sunspot Number fluctuations (Max)");
  grSnUdMax->GetYaxis()->SetRangeUser(0,600);
  grSnUdMax->SetMarkerStyle(4);
  grSnUdMax->SetMarkerSize(0.3);
  grSnUdMax->SetMarkerColor(15);
  
  TGraph *grSnUdMin = new TGraph(contEr,erSn,udMin);
  grSnUdMin->SetTitle("o3 vs Sunspot Number fluctuations (Min)");
  grSnUdMin->GetYaxis()->SetRangeUser(0,600);
  grSnUdMin->SetMarkerStyle(4);
  grSnUdMin->SetMarkerSize(0.3);
  grSnUdMin->SetMarkerColor(23);

  TGraph *grSnUdAv = new TGraph(contEr,erSn,erUd);
  grSnUdAv->SetTitle("o3 average vs Sunspot Number");
  grSnUdAv->GetYaxis()->SetRangeUser(0,600);
  grSnUdAv->SetMarkerStyle(2);
  grSnUdAv->SetMarkerColor(1);

  c2->cd(1)->cd(1);
  c2->cd(1)->cd(1)->SetGridx();
  c2->cd(1)->cd(1)->SetGridy();
  grSnUdSkim->Draw("AP");
  
  c2->cd(1)->cd(2);
  c2->cd(1)->cd(2)->SetGridx();
  c2->cd(1)->cd(2)->SetGridy();
  grSnUdMax->Draw("AP");
  grSnUdMin->DrawClone("P");
  grSnUdAv->DrawClone("P");
  
  //=================Panel of all data with an without nEvOffset==========
  
  TGraphErrors *grSnUdAvEr = new TGraphErrors(contEr,erSn,erUd,erX,erY);
  grSnUdAvEr->SetTitle("o3 average vs Sunspot Number(errors)");
  grSnUdAvEr->GetYaxis()->SetRangeUser(0,600);
  grSnUdAvEr->SetMarkerStyle(2);
  grSnUdAvEr->SetMarkerColor(1);
 
  TGraphErrors *grSnUdAvErSkim = new TGraphErrors(contErSkim,erSnSkim,erUdSkim,erXSkim,erYSkim);
  grSnUdAvErSkim->SetTitle("o3 average vs Sunspot Number (nEvOffset)");
  grSnUdAvErSkim->GetYaxis()->SetRangeUser(0,600);
  grSnUdAvErSkim->SetMarkerStyle(2);
  grSnUdAvErSkim->SetMarkerColor(3);

  TGraph *grErYEv = new TGraph(contEr,erY,ev);
  grErYEv->SetTitle("o3 ocurrences per Sunspot number vs erY");
  grErYEv->GetYaxis()->SetRangeUser(0,lastSn);
  grErYEv->SetMarkerStyle(2);
  grErYEv->SetMarkerColor(1);
  
  TF1 *fitFcn = new TF1("fitFcn",fitLinear,0,lastSn,2);
  //TF1 *fitFcn = new TF1("fitFcn",fitLinear,0,erSnSkim[contErSkim]+2,2);
  fitFcn->SetLineWidth(4);
  fitFcn->SetLineColor(kRed);
  fitFcn->SetParameters(1,1);
  fitFcn->SetParName(0,"a_{0} (UD)");
  fitFcn->SetParName(1,"a_{1} (UD)");
  
  c2->cd(2)->cd(1);
  c2->cd(2)->cd(1)->cd(1)->SetGridx();
  c2->cd(2)->cd(1)->cd(1)->SetGridy();
  grSnUdAvEr->GetYaxis()->SetRangeUser(grYMinOffSet,lastSn + grYOffSet);
  //grSnUdAvEr->SetMarkerColorAlpha(16, .5);
  //grSnUdAvEr->SetMarkerColor(16);
  
  grSnUdAvEr->SetLineWidth(0);
  grSnUdAvEr->SetLineStyle(1);
  grSnUdAvEr->SetLineColorAlpha(1, .45);

  grSnUdAvEr->SetMarkerStyle(105);
  grSnUdAvEr->SetMarkerSize(0.8);
  grSnUdAvEr->SetMarkerColorAlpha(4, 0.6);

  grSnUdAvEr->Fit(fitFcn,"","",2,lastSn);
  grSnUdAvEr->Draw("AP");
  //grSnUdAvEr->SetFillColor(4);
  //grSnUdAvEr->SetFillStyle(3010);
  // grSnUdAvEr->Draw("AP3");
   
  //fit stats for all events
  outFitLinear.open(outFitLinearName);
  cout << "name: -->" << outFitLinearName << endl;
  
  outFitLinear << fixed << showpoint << setprecision(4)
	       << fitFcn->GetParameter(0) << "\t" << fitFcn->GetParameter(1) << "\t"
	       << (fitFcn->GetChisquare())/(fitFcn->GetNDF())              << "\t"
	       << fitFcn->GetChisquare()  << "\t" << fitFcn->GetNDF()      << "\t" 
	       << fitFcn->GetParError(0)  << "\t" << fitFcn->GetParError(1)<< "\t" 
	       << fitFcn->GetProb() << "\t" << endl;

  cout << "All events: chi2/ndf "
       << fitFcn->GetChisquare()/fitFcn->GetNDF() << endl;
  
  c2->cd(2)->cd(1);
  c2->cd(2)->cd(1)->cd(2)->SetGridx();
  c2->cd(2)->cd(1)->cd(2)->SetGridy();
  grErYEv->GetYaxis()->SetRangeUser(0,lastSn + grYnEvOffSet);
  grErYEv->SetMarkerStyle(4);
  grErYEv->SetMarkerSize(0.3);
  grErYEv->Draw("AP");
  
  c2->cd(2)->cd(2);
  c2->cd(2)->cd(2)->SetGridx();
  c2->cd(2)->cd(2)->SetGridy();
  grSnUdAvErSkim->GetYaxis()->SetRangeUser(grYMinOffSet,lastSn + grYOffSet);

  grSnUdAvErSkim->SetLineWidth(1);
  grSnUdAvErSkim->SetLineStyle(1);
  grSnUdAvErSkim->SetLineColorAlpha(4, .9);

  grSnUdAvErSkim->SetMarkerStyle(105);
  grSnUdAvErSkim->SetMarkerSize(0.8);
  grSnUdAvErSkim->SetMarkerColorAlpha(1, 0.6);

  grSnUdAvErSkim->Fit("fitFcn","","",0,erSnSkim[contErSkim]+1);
  grSnUdAvErSkim->Draw("AP");

  //fit stats for events greater than nEvOffSet
  outFitLinear << fixed << showpoint << setprecision(4)
	       << fitFcn->GetParameter(0) << "\t" << fitFcn->GetParameter(1) << "\t"
	       << (fitFcn->GetChisquare())/(fitFcn->GetNDF())              << "\t"
	       << fitFcn->GetChisquare()  << "\t" << fitFcn->GetNDF()      << "\t" 
	       << fitFcn->GetParError(0)  << "\t" << fitFcn->GetParError(1)<< "\t"
	       << fitFcn->GetProb()  << "\t" << endl;
  outFitLinear.close();

  cout << "Events cut off  " << nEvOffSet <<  " chi2/ndf: "
       << fitFcn->GetChisquare()/fitFcn->GetNDF() << endl;
    
  file->Write();
  grSnUd->Write();
  grSnUdSkim->Write();
  grSnUdMax->Write();
  grSnUdMin->Write();
  grSnUdAv->Write();
  grSnUdAvEr->Write();
  grSnUdAvErSkim->Write();
  grErYEv->Write();
  c1->Write();
  c2->Write();
  
  char cmdMv[200];
  strcpy(cmdMv,"mv asvssn.root skim_");
  strcat(cmdMv,preLoc);
  strcat(cmdMv,"/");
  strcat(cmdMv,preLoc);
  strcat(cmdMv,"_sno3av.root");
  
  system(cmdMv);
  cout << "lastSn: " << lastSn << endl;
}//void
