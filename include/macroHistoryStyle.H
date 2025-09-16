void mStyle(){
  gStyle->SetTitleFontSize(0.12);
  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(510,"Y");
  gStyle->SetNdivisions(510,"X");
  gStyle->SetLabelSize(0.075,"X");
  gStyle->SetLabelSize(0.075,"Y");
  gStyle->SetTitleOffset(0.6);
  gStyle->SetTitleXOffset(0.6);
  gStyle->SetTitleYOffset(0.6);
  gStyle->SetTitleXSize(0.08);
  gStyle->SetTitleYSize(0.08);
  gStyle->SetGridColor(4);
  gStyle->SetOptStat("e");
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.3);
}
