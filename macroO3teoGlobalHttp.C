#include "TMath.h"
#include "include/funSolar.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// const int mMax      = 12;
// const int nMax      = 367;
// const int YYMin     = 1979;
// const int YYCur     = 2022;
// const int YYCurNext = 2022;
// const int YYMax     = 1978 + ;

float funDirect(float var, float sslope, float yycut) {
  return var * sslope + yycut;
}

void macroO3teoGlobalHttp(float lat, float lon, const char preLoc[10], int snT,
                          int optLinear, float rMin, float rMax, float rfmMin,
                          float rfmMax, float rErMin, float rErMax) {

  const int mMax = 12;
  const int nMax = 367;
  const int YYMin = 1979;
  const int YYCur = 2024;
  const int YYCurNext = 2024; // last year with complete data for predictions
  const int YYMax = 2024 + snT;     // extend to show future predictions

  //  bin size
  // o3 UD: rMin, rMax
  // fm: rfmMin, rfmMax
  // error plots: rErMin, rErMax

  gStyle->SetTitleFontSize(0.12);
  gStyle->SetNdivisions(505, "Y");
  gStyle->SetNdivisions(505, "X");
  gStyle->SetLabelSize(0.12, "X");
  gStyle->SetLabelSize(0.12, "Y");
  gStyle->SetTitleOffset(0.6);

  gStyle->SetTitleXOffset(0.8);
  gStyle->SetTitleYOffset(0.6);

  gStyle->SetTitleXSize(0.12);
  gStyle->SetTitleYSize(0.12);

  gStyle->SetOptStat("e");
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.3);

  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadLeftMargin(0.2);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetTitleFontSize(0.3);

  ifstream inFile, inFileSnSkim, inFitLinear;
  char fileName[500];
  strcpy(fileName, "skim_");
  strcat(fileName, preLoc);
  strcat(fileName, "/");
  strcat(fileName, preLoc);
  strcat(fileName, ".dat");

  char fitLinearName[500];
  strcpy(fitLinearName, "skim_");
  strcat(fitLinearName, preLoc);
  strcat(fitLinearName, "/");
  strcat(fitLinearName, preLoc);
  strcat(fitLinearName, "_fitlinear.dat");

  char fileSnSkimName[500];
  strcpy(fileSnSkimName, "snData/sndataskim.dat");

  cout << "o3 data file:\t\t" << fileName << endl;
  cout << "Sunspot skim file: \t" << fileSnSkimName << endl;

  float datLinear, ycut, slope, chi_NDF;
  if (optLinear == 0) {
    inFitLinear.open(fitLinearName);

    inFitLinear >> ycut;
    inFitLinear >> slope;
    inFitLinear >> chi_NDF;

    cout << "ycut:\t" << ycut << endl;
    cout << "slope:\t " << slope << endl;
    cout << "chi/NDF:\t " << chi_NDF << endl;

    inFitLinear.close();
  } else if (optLinear == 1) {
    inFitLinear.open(fitLinearName);

    // reads 8 lines to take the next three values of
    // the second row neccesary for chi value:
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;
    inFitLinear >> datLinear;

    inFitLinear >> ycut;
    inFitLinear >> slope;
    inFitLinear >> chi_NDF;

    cout << "ycut:t" << ycut << endl;
    cout << "slope:\t " << slope << endl;
    cout << "chi/NDF:\t " << chi_NDF << endl;

    inFitLinear.close();

  } else {
    cerr << " " << endl;
    cerr << "Fit parameters option configured wrong.. PLease check..." << endl;
    cerr << " " << endl;
    exit(8);
  }

  TFile *file = new TFile("global.root", "RECREATE");
  file->mkdir("ana");

  TCanvas *c11 = new TCanvas("Solar Calculus", "Solar Calculus", 500, 1000);
  c11->Divide(2, 3);

  TCanvas *c22 = new TCanvas("Form Factor f(m)", "Form Factor f(m)", 1000, 500);
  c22->Divide(snT, (YYCurNext - YYMin) / snT + 1, 0.001, 0.001);

  TCanvas *c33 = new TCanvas("history o3", "history o3", 1000, 500);
  c33->Divide(snT, (YYCurNext - YYMin) / snT + 1, 0.001, 0.001);

  TCanvas *c44 = new TCanvas("o3teo study", "o3teo study", 1000, 500);
  c44->Divide(snT, (YYMax - YYMin) / snT + 1, 0.001, 0.001);

  TCanvas *c55 = new TCanvas("o3teo error", "o3teo error", 1000, 500);
  c55->Divide(5, (YYCurNext - YYMin - snT) / 5 + 1, 0.001, 0.001);

  TLatex *tex = new TLatex(0.05, 0.00, preLoc);
  tex->SetTextColorAlpha(17, 0.90);
  tex->SetTextSize(0.1);
  tex->SetTextAngle(45);
  tex->SetNDC();

  TLatex *texfm = new TLatex(0.001, 0.91, "f(m)");
  texfm->SetTextSize(0.11);
  texfm->SetNDC();

  TLatex *texdu = new TLatex(0.001, 0.91, "(DU)");
  texdu->SetTextSize(0.1);
  texdu->SetNDC();

  // variables for solar calculations
  double AST[nMax], ws[nMax], dh[nMax], czh[nMax], RoR2[nMax], GIse[nMax],
      dofyear[nMax];
  double avGIse_m[mMax];
  int n = 0; // day of year

  for (int i = 0; i < 365; i++) {
    n++;
    dofyear[i] = n;
    dh[i] = fdh(n);
    AST[i] = fAST(n, 12.0, lon);
    ws[i] = fws(AST[i]);
    czh[i] = fczh(lat, dh[i], ws[i]);
    RoR2[i] = fRoR2(AST[i], n);
    GIse[i] = fGIse(RoR2[i], czh[i]);

    if (n <= 31)
      avGIse_m[0] += GIse[i];
    if ((n >= 32) && (n <= 59))
      avGIse_m[1] += GIse[i];
    if ((n >= 60) && (n <= 90))
      avGIse_m[2] += GIse[i];
    if ((n >= 91) && (n <= 120))
      avGIse_m[3] += GIse[i];
    if ((n >= 121) && (n <= 151))
      avGIse_m[4] += GIse[i];
    if ((n >= 152) && (n <= 181))
      avGIse_m[5] += GIse[i];
    if ((n >= 182) && (n <= 212))
      avGIse_m[6] += GIse[i];
    if ((n >= 213) && (n <= 243))
      avGIse_m[7] += GIse[i];
    if ((n >= 244) && (n <= 273))
      avGIse_m[8] += GIse[i];
    if ((n >= 274) && (n <= 304))
      avGIse_m[9] += GIse[i];
    if ((n >= 305) && (n <= 334))
      avGIse_m[10] += GIse[i];
    if ((n >= 335) && (n <= 365))
      avGIse_m[11] += GIse[i];
  }

  // average GIse from form factor calculation
  avGIse_m[0] = avGIse_m[0] / 31;
  avGIse_m[1] = avGIse_m[1] / 28;
  avGIse_m[2] = avGIse_m[2] / 31;
  avGIse_m[3] = avGIse_m[3] / 30;
  avGIse_m[4] = avGIse_m[4] / 31;
  avGIse_m[5] = avGIse_m[5] / 30;
  avGIse_m[6] = avGIse_m[6] / 31;
  avGIse_m[7] = avGIse_m[7] / 31;
  avGIse_m[8] = avGIse_m[8] / 30;
  avGIse_m[9] = avGIse_m[9] / 31;
  avGIse_m[10] = avGIse_m[10] / 30;
  avGIse_m[11] = avGIse_m[11] / 31;

  // variables for reading <location>.dat and sunspot data
  float dat, datSn, datUd, aaSn, aaUd, ddSn, ddUd;
  int mmSn, mmUd;
  float snExp[nMax], udExp[nMax], snx[nMax], udx[nMax];
  int nSn, nUd;

  // variables for computing form factor and o3 teoretical predictions
  int dy = 0, dm[mMax];
  float avAs_dy = 0, avAs_dm[mMax], fm[mMax], mx[mMax];
  float o3_teo[nMax];

  // x axis (mX months) for ploting form factor fm
  for (int jj = 0; jj < mMax; jj++) {
    mx[jj] = jj + 1;
  }

  char fmName[500];

  // error variables
  float o3ExpYY[nMax][40], erx[nMax], ery[nMax];
  float av_er;
  int nVal, nIndex;

  // ERROR loop for pointing predictions to a snT years back
  for (int ii = 0; ii < nMax; ii++) {
    for (int jj = 0; jj < 40; jj++) {
      o3ExpYY[ii][jj] = 0;
    }
  }

  for (int YY = YYMin; YY <= YYMax; YY++) {

    inFile.open(fileName);
    nIndex = 0;
    while (!inFile.eof()) {
      inFile >> ddUd;
      inFile >> mmUd;
      inFile >> aaUd;
      inFile >> datUd;

      if (aaUd == YY) {
        nIndex++;
        if (datUd > 0) {
          o3ExpYY[nIndex - 1][YY - YYMin] =
              datUd; // track o3 vals for every year
        }
      }

    } // while infile
    inFile.close();
  }
  //====END===== ERROR loop for pointing predictions to a snT years back

  //========LOOP OVER years of history ================

  char dirRoot[100];
  char writeRoot[100];

  for (int YY = YYMin; YY <= YYMax; YY++) {

    // making directories for root file
    sprintf(dirRoot, "%d/fm", YY);
    file->mkdir(dirRoot);
    sprintf(dirRoot, "%d/history", YY);
    file->mkdir(dirRoot);
    sprintf(dirRoot, "%d/comp", YY);
    file->mkdir(dirRoot);
    sprintf(dirRoot, "%d/error", YY);
    file->mkdir(dirRoot);

    cout << "procesing year: " << YY << " for location: " << preLoc << endl;
    for (int gg = 0; gg < nMax; gg++) {
      snExp[gg] = 0;
      udExp[gg] = 0;
      o3_teo[gg] = 0;
    }

    for (int q = 0; q < mMax; q++) {
      dm[q] = 0; // days of month to average from
      avAs_dm[q] = 0;
      fm[q] = 0;
    }

    inFileSnSkim.open(fileSnSkimName);
    nSn = -1;
    while (!inFileSnSkim.eof()) {
      inFileSnSkim >> aaSn;
      inFileSnSkim >> mmSn;
      inFileSnSkim >> ddSn;
      inFileSnSkim >> dat;
      inFileSnSkim >> datSn;
      inFileSnSkim >> dat;
      inFileSnSkim >> dat;

      if (aaSn == YY) {
        nSn++;
        snExp[nSn] = datSn;
        snx[nSn] = nSn + 1;
      }
    }
    inFileSnSkim.close();

    avAs_dy = 0; // yearly average
    dy = 0;      // days of year to average from
    inFile.open(fileName);
    nUd = -1;
    while (!inFile.eof()) {
      inFile >> ddUd;
      inFile >> mmUd;
      inFile >> aaUd;
      inFile >> datUd;

      if (aaUd == YY) {
        nUd++;
        udExp[nUd] = datUd;
        udx[nUd] = nUd + 1;

        if (datUd > 0) {
          dy++;
          avAs_dy += datUd; // taking o3 average by year

          if (mmUd < mMax + 1) {
            dm[mmUd - 1]++;
            avAs_dm[mmUd - 1] += datUd; // taking o3 average by month
          }
        }
      }

    } // while infile
    inFile.close();

    if (dy != 0)
      avAs_dy = avAs_dy / dy;
    else
      avAs_dy = 0;

    // computing form factor by month
    for (int m = 0; m < mMax; m++) {

      if (dm[m] > 0) {
        avAs_dm[m] = avAs_dm[m] / dm[m];
        fm[m] = 1.0 / (avGIse_m[m] * avAs_dy) * avAs_dm[m];
      } else {
        avAs_dm[m] = 0;
        fm[m] = 0;
      }

    } // for m

    TGraph *grUdExp = new TGraph(nUd, udx, udExp);

    grUdExp->GetXaxis()->SetLimits(0, 366);
    grUdExp->GetYaxis()->SetRangeUser(rMin, rMax);
    grUdExp->SetTitle();
    grUdExp->GetXaxis()->SetTitle("Time (days)");
    //    grUdExp->GetYaxis()->SetTitle("DU ");
    grUdExp->GetXaxis()->CenterTitle();
    grUdExp->SetMarkerStyle(6);
    grUdExp->SetMarkerColor(1);

    // getting predictions from snT years back in time

    TGraph *gro3teo;
    TGraph *gro3_delta;
    TF1 *f1;

    if (YY <= (YYMax - snT)) {
      for (int ddt = 0; ddt < 366; ddt++) {
        if (ddt <= 31)
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[0] * GIse[ddt];
        if ((ddt >= 32) && (ddt <= 59))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[1] * GIse[ddt];
        if ((ddt >= 60) && (ddt <= 90))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[2] * GIse[ddt];
        if ((ddt >= 91) && (ddt <= 120))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[3] * GIse[ddt];
        if ((ddt >= 121) && (ddt <= 151))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[4] * GIse[ddt];
        if ((ddt >= 152) && (ddt <= 181))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[5] * GIse[ddt];
        if ((ddt >= 182) && (ddt <= 212))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[6] * GIse[ddt];
        if ((ddt >= 213) && (ddt <= 243))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[7] * GIse[ddt];
        if ((ddt >= 244) && (ddt <= 273))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[8] * GIse[ddt];
        if ((ddt >= 274) && (ddt <= 304))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[9] * GIse[ddt];
        if ((ddt >= 305) && (ddt <= 334))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[10] * GIse[ddt];
        if ((ddt >= 335) && (ddt <= 365))
          o3_teo[ddt] = funDirect(snExp[ddt], slope, ycut) * fm[11] * GIse[ddt];
      }

      gro3teo = new TGraph(nUd, udx, o3_teo);
      gro3teo->GetXaxis()->SetLimits(0, 366);
      gro3teo->GetYaxis()->SetRangeUser(rMin, rMax);
      gro3teo->SetTitle();
      //      gro3teo->GetYaxis()->SetTitle("DU ");
      gro3teo->GetXaxis()->SetTitle("Time (days)");
      gro3teo->GetXaxis()->CenterTitle();
      gro3teo->SetMarkerStyle(6);
      gro3teo->SetMarkerColor(4);

      // error calculation. Comparison with o3teo preditions
      av_er = 0;
      nVal = 0;
      for (int eri = 0; eri < 365; eri++) {
        if (o3_teo[eri] > 0) {
          if (o3ExpYY[eri][YY - YYMin + snT] > 0) {
            nVal++;
            erx[eri] = eri;
            ery[eri] = TMath::Abs(o3_teo[eri] - o3ExpYY[eri][YY - YYMin + snT]);
            ery[eri] = ery[eri] / o3ExpYY[eri][YY - YYMin + snT];
            av_er += ery[eri];
          } else {
            erx[eri] = 0;
            ery[eri] = 0;
            av_er += ery[eri];
          }
        } else {
          erx[eri] = 0;
          ery[eri] = 0;
          av_er += ery[eri];
        }
      }
      if (nVal > 0)
        av_er = av_er / ((float)nVal);
      else
        av_er = 0;

      gro3_delta = new TGraph(365, erx, ery);
      gro3_delta->GetXaxis()->SetLimits(0, 366);
      gro3_delta->GetYaxis()->SetRangeUser(rErMin, rErMax);
      gro3_delta->SetTitle();
      gro3_delta->GetXaxis()->SetTitle("Time (days)");
      gro3_delta->GetYaxis()->SetTitle("Error");
      gro3_delta->GetXaxis()->CenterTitle();
      gro3_delta->GetYaxis()->CenterTitle();
      gro3_delta->SetMarkerStyle(25);
      gro3_delta->SetMarkerSize(0.2);

      f1 = new TF1("f1", "x*[0]+[1]", 0, 365);
      f1->SetParameters(0, av_er);
      f1->SetLineColor(4);

      //========= END ERROR calculations=============

    } // if( YY <= (YYMax - snT)){

    //=c22===FORM=FACTOR============

    if (YY <= YYCur) {
      TGraph *grfm = new TGraph(mMax, mx, fm);

      c22->cd(YY - YYMin + 1);
      c22->cd(YY - YYMin + 1)->SetGridx();
      c22->cd(YY - YYMin + 1)->SetGridy();
      sprintf(fmName, "%d", YY);
      grfm->SetTitle("");
      grfm->GetXaxis()->SetTitle("Time (months)");
      grfm->GetXaxis()->SetLimits(1, 13);
      grfm->GetYaxis()->SetRangeUser(rfmMin, rfmMax);
      grfm->SetMarkerStyle(6);

      TLatex *textfm = new TLatex(0.6, 0.85, fmName);
      textfm->SetTextSize(0.18);
      textfm->SetNDC();

      grfm->DrawClone("AC*");
      tex->Draw();
      texfm->Draw();
      textfm->Draw();

      sprintf(writeRoot, "%d/fm", YY);
      file->cd(writeRoot);
      grfm->Write("", TObject::kOverwrite);
      delete grfm;
    }
    //===END====c22===FORM=FACTOR============

    //====c33===History============
    TString s1 = "";
    s1.Form("%d", YY);
    TString s2 = "";
    s2.Form(" #color[4]{#diamond} from %d", YY);
    TString s3 = "";
    s3.Form("%d(from %d)", YY + snT, YY);
    TString s4 = "";
    s4.Form("#color[4]{#bullet} Mean: %0.1f %%", av_er * 100);

    TLatex *t1 = new TLatex(0.6, 0.25, s1.Data());
    t1->SetTextColorAlpha(1, 0.90);
    t1->SetTextSize(0.17);
    t1->SetNDC();

    TLatex *t11 = new TLatex(0.6, 0.25, s1.Data());
    t11->SetTextColorAlpha(1, 0.90);
    t11->SetTextSize(0.17);
    t11->SetNDC();

    TLatex *t2 = new TLatex(0.15, 0.85, s2.Data());
    t2->SetTextColorAlpha(1, 0.90);
    t2->SetTextSize(0.14);
    t2->SetNDC();

    TLatex *t3 = new TLatex(0.25, 0.80, s3.Data());
    t3->SetTextColorAlpha(1, 0.90);
    t3->SetTextSize(0.18);
    t3->SetNDC();

    TLatex *t4 = new TLatex(0.25, 0.65, s4.Data());
    t4->SetTextColorAlpha(1, 0.90);
    t4->SetTextSize(0.2);
    t4->SetNDC();

    c33->cd(YY - YYMin + 1);
    c33->cd(YY - YYMin + 1)->SetGridx();
    c33->cd(YY - YYMin + 1)->SetGridy();
    if (YY <= YYCur) {
      grUdExp->DrawClone("AP");
      t1->Draw();
      tex->Draw();
      texdu->Draw();
    }
    //===END====c33===History============

    //====c44===o3teo==STUDY============
    if (YY < (YYMin + snT)) {
      c44->cd(YY - YYMin + 1);
      c44->cd(YY - YYMin + 1)->SetGridx();
      c44->cd(YY - YYMin + 1)->SetGridy();
      grUdExp->DrawClone("AP");
      tex->Draw();
      texdu->Draw();
      t1->Draw();

      c44->cd(YY - YYMin + 1 + snT);
      c44->cd(YY - YYMin + 1 + snT)->SetGridx();
      c44->cd(YY - YYMin + 1 + snT)->SetGridy();
      gro3teo->DrawClone("AP");
      tex->Draw();
      texdu->Draw();
      t2->Draw();

    } else if ((YY >= (YYMin + snT)) && (YY <= (YYMax - snT))) {
      c44->cd(YY - YYMin + snT + 1);
      c44->cd(YY - YYMin + snT + 1)->SetGridx();
      c44->cd(YY - YYMin + snT + 1)->SetGridy();
      gro3teo->DrawClone("AP");
      t2->Draw();
      texdu->Draw();

      c44->cd(YY - YYMin + 1);
      c44->cd(YY - YYMin + 1)->SetGridx();
      c44->cd(YY - YYMin + 1)->SetGridy();
      grUdExp->DrawClone("P");
      tex->Draw();
      t1->Draw();
    }

    else if ((YY > YYMax - snT) && (YY <= YYMax)) {
      // else {
      c44->cd(YY - YYMin + 1);
      c44->cd(YY - YYMin + 1)->SetGridx();
      c44->cd(YY - YYMin + 1)->SetGridy();

      if (YY <= YYCur) {
        grUdExp->DrawClone("P");
      }
      tex->Draw();
      t1->Draw();
    }

    sprintf(writeRoot, "%d/history", YY);
    file->cd(writeRoot);
    grUdExp->Write("", TObject::kOverwrite);

    sprintf(writeRoot, "%d/comp", YY);
    file->cd(writeRoot);
    gro3teo->Write("", TObject::kOverwrite);

    //====c44===o3teo==STUDY============

    //==c55===eroor o3teo STUDY============

    if (YY <= (YYCur - snT)) {

      c55->cd(YY - YYMin + 1);
      c55->cd(YY - YYMin + 1)->SetGridx();
      c55->cd(YY - YYMin + 1)->SetGridy();
      gro3_delta->DrawClone("A*");
      f1->DrawClone("sames");
      t3->Draw();
      t4->Draw();
      tex->Draw();
    }
    sprintf(writeRoot, "%d/error", YY);
    file->cd(writeRoot);
    gro3_delta->Write("", TObject::kOverwrite);

    // delete grUdExp;
    // delete gro3teo;
    // delete gro3_delta;
    // delete f1;

    //==END==c55===eroor o3teo STUDY============

  } // for YY

  //=c11===SOLAR=CALULATIONS==================
  TGraph *grdh = new TGraph(n, dofyear, dh);
  grdh->SetTitle("#delta");
  TGraph *grAST = new TGraph(n, dofyear, AST);
  grAST->SetTitle("AST");
  TGraph *grws = new TGraph(n, dofyear, ws);
  grws->SetTitle("#omega_{s}");
  TGraph *grczh = new TGraph(n, dofyear, czh);
  grczh->SetTitle("cos(z)");
  TGraph *grRoR2 = new TGraph(n, dofyear, RoR2);
  grRoR2->SetTitle("(R_{0}/R)^{2}");
  TGraph *grGIse = new TGraph(n, dofyear, GIse);
  grGIse->SetTitle("G(Ise)");

  c11->cd(1);
  c11->cd(1)->SetGridx();
  c11->cd(1)->SetGridy();

  grdh->DrawClone("A*");

  c11->cd(2);
  c11->cd(2)->SetGridx();
  c11->cd(2)->SetGridy();
  grAST->Draw("A*");

  c11->cd(3);
  c11->cd(3)->SetGridx();
  c11->cd(3)->SetGridy();
  grws->Draw("A*");

  c11->cd(4);
  c11->cd(4)->SetGridx();
  c11->cd(4)->SetGridy();
  grczh->Draw("A*");

  c11->cd(5);
  c11->cd(5)->SetGridx();
  c11->cd(5)->SetGridy();
  grRoR2->Draw("A*");

  c11->cd(6);
  c11->cd(6)->SetGridx();
  c11->cd(6)->SetGridy();
  grGIse->Draw("A*");
  for (int p = 0; p < 6; p++) {
    c11->cd(p + 1);
    tex->Draw();
  }

  //======END=c11===SOLAR=CALULATIONS==================

  file->cd("ana");
  c11->Write();
  c22->Write();
  c33->Write();
  c44->Write();
  c55->Write();
  grdh->Write();
  grAST->Write();
  grws->Write();
  grczh->Write();
  grRoR2->Write();
  grGIse->Write();

  char cmdMv[200];
  strcpy(cmdMv, "mv global.root skim_");
  strcat(cmdMv, preLoc);
  strcat(cmdMv, "/");
  strcat(cmdMv, preLoc);
  strcat(cmdMv, "_global.root");

  system(cmdMv);

  file->Close();

} // void macroHistory
