#include "TApplication.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGButton.h"
#include "TGClient.h"
#include "TGColorSelect.h"
#include "TGComboBox.h"
#include "TGFrame.h"
#include "TGLabel.h"
#include "TGraph.h"
#include "TH1.h"
#include "TKey.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TRootEmbeddedCanvas.h"
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

class O3ViewerGUI : public TGMainFrame {
private:
  TGComboBox *fLocationCombo;
  TGComboBox *fCategoryCombo;
  TGComboBox *fYearCombo;
  TGComboBox *fGraphCombo;
  TGTextButton *fLoadButton;
  TGTextButton *fRefreshButton;
  TGTextButton *fExitButton;
  TGTextButton *fExportButton;
  TGCheckButton *fMultiYearCheck;
  TGColorSelect *fHistoryColorSelect;
  TGColorSelect *fTeoColorSelect;
  TGLabel *fStatusLabel;
  TRootEmbeddedCanvas *fEmbCanvas;
  TFile *fRootFile;
  TString fBaseDir;
  Color_t fHistoryColor;
  Color_t fTeoColor;
  TGCheckButton *fConnectHistoryCheck; // New checkbox
  TGraph *fGrHistory;                  // Stored clone for redraw
  TGraph *fGrTeo;                      // Stored clone for redraw (for overlay)
  TGCheckButton *fConnectTeoCheck;     // New checkbox for Teo lines
  TGCheckButton *fExportAllYearsCheck;
  bool fConnectTeo;     // Toggle state for Teo lines
  bool fConnectHistory; // Toggle state

public:
  O3ViewerGUI(const TGWindow *p, UInt_t w, UInt_t h, const char *basedir = ".");
  virtual ~O3ViewerGUI();

  void ScanLocations();
  void LoadFile();
  void PopulateYears();
  void PopulateGraphs();
  void LoadGraph();
  void LoadMultiYearPanel();
  void LoadSuperposition();
  void DrawObject(TObject *obj, const char *path);
  void OnLocationSelected();
  void OnCategorySelected();
  void OnYearSelected();
  void OnMultiYearToggled();
  void OnHistoryColorSelected();
  void OnTeoColorSelected();
  void CloseWindow();
  void OnConnectHistoryToggled();
  void OnConnectTeoToggled();
  void ExportData();
  void RefreshData();

  ClassDef(O3ViewerGUI, 0)
};

O3ViewerGUI::O3ViewerGUI(const TGWindow *p, UInt_t w, UInt_t h,
                         const char *basedir)
    : TGMainFrame(p, w, h), fRootFile(nullptr), fBaseDir(basedir),
      fHistoryColor(kBlue), fTeoColor(kRed) {
  // Initialize new members after initializer list
  fConnectHistoryCheck = nullptr;
  fGrHistory = nullptr;
  fGrTeo = nullptr;
  fConnectHistory = true;
  fConnectTeoCheck = nullptr;
  fConnectTeo = true;
  fExportAllYearsCheck = nullptr;

  SetWindowName("O3 Global Analysis Viewer");
  SetCleanup(kDeepCleanup);

  // Main vertical frame
  TGVerticalFrame *vframe = new TGVerticalFrame(this, w, h);

  // Control panel frame - better organized
  TGHorizontalFrame *controlFrame = new TGHorizontalFrame(vframe, w, 180);

  // ========== LEFT PANEL: Data Selection ==========
  TGGroupFrame *selectionGroup = new TGGroupFrame(controlFrame, "Data Selection", kVerticalFrame);
  selectionGroup->SetTitlePos(TGGroupFrame::kLeft);

  // Location selection
  TGHorizontalFrame *locFrame = new TGHorizontalFrame(selectionGroup, 300, 30);
  TGLabel *locLabel = new TGLabel(locFrame, "Location:");
  locLabel->SetWidth(70);
  locFrame->AddFrame(locLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 10, 5, 5));
  fLocationCombo = new TGComboBox(locFrame, 99);
  fLocationCombo->Resize(200, 22);
  fLocationCombo->Connect("Selected(Int_t)", "O3ViewerGUI", this, "OnLocationSelected()");
  locFrame->AddFrame(fLocationCombo, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
  selectionGroup->AddFrame(locFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 2));

  // Category selection
  TGHorizontalFrame *catFrame = new TGHorizontalFrame(selectionGroup, 300, 30);
  TGLabel *catLabel = new TGLabel(catFrame, "Category:");
  catLabel->SetWidth(70);
  catFrame->AddFrame(catLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 10, 5, 5));
  fCategoryCombo = new TGComboBox(catFrame, 100);
  fCategoryCombo->AddEntry("Analysis graphs", 0);
  fCategoryCombo->AddEntry("Form Factor", 1);
  fCategoryCombo->AddEntry("History O3", 2);
  fCategoryCombo->AddEntry("O3 Teo Study", 3);
  fCategoryCombo->AddEntry("O3 Teo Error", 4);
  fCategoryCombo->AddEntry("Individual Graphs", 5);
  fCategoryCombo->AddEntry("Superposition (History + Teo)", 6);
  fCategoryCombo->Resize(200, 22);
  fCategoryCombo->Select(0);
  fCategoryCombo->Connect("Selected(Int_t)", "O3ViewerGUI", this, "OnCategorySelected()");
  catFrame->AddFrame(fCategoryCombo, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
  selectionGroup->AddFrame(catFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 2, 2));

  // Year selection
  TGHorizontalFrame *yearFrame = new TGHorizontalFrame(selectionGroup, 300, 30);
  TGLabel *yearLabel = new TGLabel(yearFrame, "Year:");
  yearLabel->SetWidth(70);
  yearFrame->AddFrame(yearLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 10, 5, 5));
  fYearCombo = new TGComboBox(yearFrame, 101);
  fYearCombo->Resize(200, 22);
  fYearCombo->Connect("Selected(Int_t)", "O3ViewerGUI", this, "OnYearSelected()");
  yearFrame->AddFrame(fYearCombo, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
  selectionGroup->AddFrame(yearFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 2, 2));

  // Graph selection
  TGHorizontalFrame *graphFrame = new TGHorizontalFrame(selectionGroup, 300, 30);
  TGLabel *graphLabel = new TGLabel(graphFrame, "Graph:");
  graphLabel->SetWidth(70);
  graphFrame->AddFrame(graphLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 10, 5, 5));
  fGraphCombo = new TGComboBox(graphFrame, 102);
  fGraphCombo->Resize(200, 22);
  graphFrame->AddFrame(fGraphCombo, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
  selectionGroup->AddFrame(graphFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 2, 5));

  controlFrame->AddFrame(selectionGroup, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5));

  // ========== CENTER PANEL: Display Options ==========
  TGGroupFrame *displayGroup = new TGGroupFrame(controlFrame, "Display Options", kVerticalFrame);
  displayGroup->SetTitlePos(TGGroupFrame::kLeft);

  // Multi-year checkbox
  fMultiYearCheck = new TGCheckButton(displayGroup, "Show All Years Panel");
  fMultiYearCheck->Connect("Toggled(Bool_t)", "O3ViewerGUI", this, "OnMultiYearToggled()");
  displayGroup->AddFrame(fMultiYearCheck, new TGLayoutHints(kLHintsLeft, 10, 5, 8, 5));

  // Superposition section
  TGLabel *superTitle = new TGLabel(displayGroup, "Superposition Settings:");
  TGFont *boldFont = gClient->GetFont("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");
  if (boldFont) superTitle->SetTextFont(boldFont);
  displayGroup->AddFrame(superTitle, new TGLayoutHints(kLHintsLeft, 10, 5, 8, 5));

  // Color selection - horizontal layout
  TGHorizontalFrame *colorFrame = new TGHorizontalFrame(displayGroup, 250, 50);

  // History color
  TGVerticalFrame *histFrame = new TGVerticalFrame(colorFrame, 100, 50);
  TGLabel *histColorLabel = new TGLabel(histFrame, "History O3");
  histFrame->AddFrame(histColorLabel, new TGLayoutHints(kLHintsCenterX, 2, 2, 3, 2));
  fHistoryColorSelect = new TGColorSelect(histFrame, kBlue, 300);
  fHistoryColorSelect->Resize(70, 22);
  fHistoryColorSelect->Connect("ColorSelected(Pixel_t)", "O3ViewerGUI", this, "OnHistoryColorSelected()");
  histFrame->AddFrame(fHistoryColorSelect, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 3));
  colorFrame->AddFrame(histFrame, new TGLayoutHints(kLHintsLeft, 5, 10, 2, 2));

  // Teo color
  TGVerticalFrame *teoFrame = new TGVerticalFrame(colorFrame, 100, 50);
  TGLabel *teoColorLabel = new TGLabel(teoFrame, "O3 Teo");
  teoFrame->AddFrame(teoColorLabel, new TGLayoutHints(kLHintsCenterX, 2, 2, 3, 2));
  fTeoColorSelect = new TGColorSelect(teoFrame, kRed, 301);
  fTeoColorSelect->Resize(70, 22);
  fTeoColorSelect->Connect("ColorSelected(Pixel_t)", "O3ViewerGUI", this, "OnTeoColorSelected()");
  teoFrame->AddFrame(fTeoColorSelect, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 3));
  colorFrame->AddFrame(teoFrame, new TGLayoutHints(kLHintsLeft, 10, 5, 2, 2));

  displayGroup->AddFrame(colorFrame, new TGLayoutHints(kLHintsLeft, 5, 5, 2, 5));

  // Connect lines checkboxes
  fConnectHistoryCheck = new TGCheckButton(displayGroup, "Connect History Lines");
  fConnectHistoryCheck->SetOn(kTRUE);
  fConnectHistoryCheck->Connect("Toggled(Bool_t)", "O3ViewerGUI", this, "OnConnectHistoryToggled()");
  displayGroup->AddFrame(fConnectHistoryCheck, new TGLayoutHints(kLHintsLeft, 10, 5, 3, 2));

  fConnectTeoCheck = new TGCheckButton(displayGroup, "Connect Teo Lines");
  fConnectTeoCheck->SetOn(kTRUE);
  fConnectTeoCheck->Connect("Toggled(Bool_t)", "O3ViewerGUI", this, "OnConnectTeoToggled()");
  displayGroup->AddFrame(fConnectTeoCheck, new TGLayoutHints(kLHintsLeft, 10, 5, 2, 5));

  controlFrame->AddFrame(displayGroup, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5));

  // ========== RIGHT PANEL: Actions ==========
  TGGroupFrame *actionsGroup = new TGGroupFrame(controlFrame, "Actions", kVerticalFrame);
  actionsGroup->SetTitlePos(TGGroupFrame::kLeft);

  // Load button
  fLoadButton = new TGTextButton(actionsGroup, "&Load Graph", 200);
  fLoadButton->SetHeight(28);
  fLoadButton->Connect("Clicked()", "O3ViewerGUI", this, "LoadGraph()");
  actionsGroup->AddFrame(fLoadButton, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 10, 10, 8, 5));

  // Refresh button
  fRefreshButton = new TGTextButton(actionsGroup, "&Refresh Locations", 203);
  fRefreshButton->SetHeight(28);
  fRefreshButton->Connect("Clicked()", "O3ViewerGUI", this, "RefreshData()");
  actionsGroup->AddFrame(fRefreshButton, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 10, 10, 5, 5));

  // Export section
  TGLabel *exportTitle = new TGLabel(actionsGroup, "Export Data:");
  if (boldFont) exportTitle->SetTextFont(boldFont);
  actionsGroup->AddFrame(exportTitle, new TGLayoutHints(kLHintsLeft, 10, 5, 8, 3));

  fExportAllYearsCheck = new TGCheckButton(actionsGroup, "Export All Years");
  fExportAllYearsCheck->SetOn(kFALSE);
  actionsGroup->AddFrame(fExportAllYearsCheck, new TGLayoutHints(kLHintsLeft, 10, 5, 3, 3));

  fExportButton = new TGTextButton(actionsGroup, "&Export Data", 202);
  fExportButton->SetHeight(28);
  fExportButton->Connect("Clicked()", "O3ViewerGUI", this, "ExportData()");
  actionsGroup->AddFrame(fExportButton, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 10, 10, 3, 5));

  // Exit button
  fExitButton = new TGTextButton(actionsGroup, "&Exit", 201);
  fExitButton->SetHeight(28);
  fExitButton->Connect("Clicked()", "O3ViewerGUI", this, "CloseWindow()");
  actionsGroup->AddFrame(fExitButton, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 10, 10, 8, 5));

  // Status label
  fStatusLabel = new TGLabel(actionsGroup, "Ready");
  actionsGroup->AddFrame(fStatusLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 8));

  controlFrame->AddFrame(actionsGroup, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 5, 10, 5, 5));

  vframe->AddFrame(controlFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

  // Embedded canvas
  fEmbCanvas = new TRootEmbeddedCanvas("EmbCanvas", vframe, w - 10, h - 160);
  vframe->AddFrame(
      fEmbCanvas,
      new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

  AddFrame(vframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();

  // Scan for locations
  ScanLocations();
}

O3ViewerGUI::~O3ViewerGUI() {
  // Cleanup stored graphs first
  if (fGrHistory) {
    delete fGrHistory;
    fGrHistory = nullptr;
  }
  if (fGrTeo) {
    delete fGrTeo;
    fGrTeo = nullptr;
  }

  if (fRootFile && fRootFile->IsOpen()) {
    fRootFile->Close();
    delete fRootFile;
  }
  Cleanup();
}

void O3ViewerGUI::ScanLocations() {
  fLocationCombo->RemoveAll();

  void *dirp = gSystem->OpenDirectory(fBaseDir.Data());
  if (!dirp) {
    fStatusLabel->SetText("Error: Cannot open base directory!");
    return;
  }

  const char *entry;
  int locCount = 0;

  while ((entry = gSystem->GetDirEntry(dirp))) {
    TString dirName = entry;

    if (dirName.Contains("skim") && !dirName.BeginsWith(".")) {
      TString fullPath = Form("%s/%s", fBaseDir.Data(), dirName.Data());

      if (gSystem->OpenDirectory(fullPath.Data())) {
        TString locName = dirName;
        locName.ReplaceAll("_skim", "");
        locName.ReplaceAll("skim_", "");

        TString rootFile =
            Form("%s/%s_global.root", fullPath.Data(), locName.Data());
        if (gSystem->AccessPathName(rootFile.Data()) == 0) {
          fLocationCombo->AddEntry(locName.Data(), locCount++);
        }
      }
    }
  }

  gSystem->FreeDirectory(dirp);

  if (locCount > 0) {
    fLocationCombo->Select(0);
    fStatusLabel->SetText(Form("Found %d location(s)", locCount));
    OnLocationSelected();
  } else {
    fStatusLabel->SetText("No locations found! Check directory structure.");
  }
}

void O3ViewerGUI::OnLocationSelected() {
  if (fRootFile && fRootFile->IsOpen()) {
    fRootFile->Close();
    delete fRootFile;
    fRootFile = nullptr;
  }

  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();
  canvas->Modified();
  canvas->Update();

  LoadFile();
}

void O3ViewerGUI::LoadFile() {
  TGTextLBEntry *locEntry = (TGTextLBEntry *)fLocationCombo->GetSelectedEntry();
  if (!locEntry) {
    fStatusLabel->SetText("Error: No location selected!");
    return;
  }

  TString locName = locEntry->GetText()->GetString();

  TString possiblePaths[] = {Form("%s/skim_%s/%s_global.root", fBaseDir.Data(),
                                  locName.Data(), locName.Data()),
                             Form("%s/%s_skim/%s_global.root", fBaseDir.Data(),
                                  locName.Data(), locName.Data()),
                             Form("%s/%s/%s_global.root", fBaseDir.Data(),
                                  locName.Data(), locName.Data())};

  TString fileName = "";
  for (int i = 0; i < 3; i++) {
    if (gSystem->AccessPathName(possiblePaths[i].Data()) == 0) {
      fileName = possiblePaths[i];
      break;
    }
  }

  if (fileName == "") {
    fStatusLabel->SetText(
        Form("Error: Cannot find ROOT file for %s", locName.Data()));
    return;
  }

  fRootFile = TFile::Open(fileName.Data());
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: Cannot open file!");
    std::cerr << "Error: Cannot open file " << fileName.Data() << std::endl;
    return;
  }

  fStatusLabel->SetText(Form("Loaded: %s", locName.Data()));
  OnCategorySelected();
}

void O3ViewerGUI::PopulateYears() {
  fYearCombo->RemoveAll();

  if (!fRootFile || fRootFile->IsZombie())
    return;

  TIter next(fRootFile->GetListOfKeys());
  TKey *key;
  int entry = 0;

  while ((key = (TKey *)next())) {
    TString name = key->GetName();
    if (name.IsDigit()) {
      fYearCombo->AddEntry(name.Data(), entry++);
    }
  }

  if (entry > 0) {
    fYearCombo->Select(0);
    OnYearSelected();
  }
}

void O3ViewerGUI::PopulateGraphs() {
  fGraphCombo->RemoveAll();

  if (!fRootFile || fRootFile->IsZombie())
    return;

  Int_t catId = fCategoryCombo->GetSelected();

  if (catId == 0) {
    fGraphCombo->AddEntry("delta", 0);
    fGraphCombo->AddEntry("AST", 1);
    fGraphCombo->AddEntry("omega_s", 2);
    fGraphCombo->AddEntry("cos(z)", 3);
    fGraphCombo->AddEntry("(R0/R)^2", 4);
    fGraphCombo->AddEntry("G(Ise)", 5);
    fGraphCombo->AddEntry("Solar Calculus", 6);
    fGraphCombo->AddEntry("Form Factor f(m)", 7);
    fGraphCombo->AddEntry("history o3", 8);
    fGraphCombo->AddEntry("o3teo study", 9);
    fGraphCombo->AddEntry("o3teo error", 10);
  } else if (catId == 6) {
    // Superposition mode - get unique graphs from both history and comp
    // directories
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    if (!yearEntry)
      return;

    TString year = yearEntry->GetText()->GetString();
    std::set<TString> graphNames; // Use set for uniqueness

    // Scan history dir
    TString historyPath = Form("%s/history", year.Data());
    TDirectory *historyDir = (TDirectory *)fRootFile->Get(historyPath.Data());
    if (historyDir) {
      TIter nextHist(historyDir->GetListOfKeys());
      TKey *histKey;
      while ((histKey = (TKey *)nextHist())) {
        graphNames.insert(histKey->GetName());
      }
    }

    // Scan comp dir
    TString compPath = Form("%s/comp", year.Data());
    TDirectory *compDir = (TDirectory *)fRootFile->Get(compPath.Data());
    if (compDir) {
      TIter nextComp(compDir->GetListOfKeys());
      TKey *compKey;
      while ((compKey = (TKey *)nextComp())) {
        graphNames.insert(compKey->GetName());
      }
    }

    // Add unique graphs to combo
    int entry = 0;
    for (const auto &graphName : graphNames) {
      fGraphCombo->AddEntry(graphName.Data(), entry++);
    }
  } else if (catId >= 1 && catId <= 4) {
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    if (!yearEntry)
      return;

    TString year = yearEntry->GetText()->GetString();
    TString subdir;

    switch (catId) {
    case 1:
      subdir = "fm";
      break;
    case 2:
      subdir = "history";
      break;
    case 3:
      subdir = "comp";
      break;
    case 4:
      subdir = "error";
      break;
    }

    TString dirPath = Form("%s/%s", year.Data(), subdir.Data());
    TDirectory *dir = (TDirectory *)fRootFile->Get(dirPath.Data());

    if (dir) {
      TIter next(dir->GetListOfKeys());
      TKey *key;
      int entry = 0;
      while ((key = (TKey *)next())) {
        fGraphCombo->AddEntry(key->GetName(), entry++);
      }
    }
  } else if (catId == 5) {
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    if (!yearEntry)
      return;

    TString year = yearEntry->GetText()->GetString();
    TDirectory *yearDir = (TDirectory *)fRootFile->Get(year.Data());

    if (yearDir) {
      TIter nextSubdir(yearDir->GetListOfKeys());
      TKey *subdirKey;
      int entry = 0;

      while ((subdirKey = (TKey *)nextSubdir())) {
        TDirectory *subdir = (TDirectory *)yearDir->Get(subdirKey->GetName());
        if (subdir && subdir->InheritsFrom("TDirectory")) {
          TIter nextGraph(subdir->GetListOfKeys());
          TKey *graphKey;

          while ((graphKey = (TKey *)nextGraph())) {
            TString fullName =
                Form("%s/%s", subdirKey->GetName(), graphKey->GetName());
            fGraphCombo->AddEntry(fullName.Data(), entry++);
          }
        }
      }
    }
  }

  if (fGraphCombo->GetNumberOfEntries() > 0) {
    fGraphCombo->Select(0);
  }
}

void O3ViewerGUI::OnConnectHistoryToggled() {
  fConnectHistory = fConnectHistoryCheck->IsOn();

  if (fGrHistory && fGrTeo) { // Only if in superposition and graphs exist
    Int_t catId = fCategoryCombo->GetSelected();
    if (catId == 6) { // Superposition mode
      TCanvas *canvas = fEmbCanvas->GetCanvas();
      canvas->Clear();
      canvas->cd();
      canvas->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
      canvas->SetGrid();

      bool drawnFirst = false;
      TString drawOptHistory = fConnectHistory ? "APL" : "AP";
      TString drawOptTeo = fConnectTeo ? "PL" : "P";

      // Redraw History
      if (fGrHistory->GetN() > 0) {
        fGrHistory->Draw(drawOptHistory.Data());
        drawnFirst = true;
      }

      // Redraw Teo on top
      if (fGrTeo->GetN() > 0) {
        if (drawnFirst) {
          fGrTeo->Draw(Form("%s SAME", drawOptTeo.Data()));
        } else {
          fGrTeo->Draw(drawOptTeo.Data());
        }
      }

      // Legend
      TLegend *legend = new TLegend(0.7, 0.75, 0.9, 0.9);
      legend->SetBorderSize(1);
      legend->SetFillColor(0);
      legend->AddEntry(fGrHistory, "History O3", "lp");
      legend->AddEntry(fGrTeo, "O3 Teo Study", "lp");
      legend->Draw();

      canvas->Modified();
      canvas->Update();
    }
  }
  // If in multi-year mode, reload the panel
  if (fMultiYearCheck->IsOn()) {
    Int_t catId = fCategoryCombo->GetSelected();
    if (catId == 6 || catId == 2) {
      LoadMultiYearPanel();
    }
  }
}

void O3ViewerGUI::OnConnectTeoToggled() {
  fConnectTeo = fConnectTeoCheck->IsOn();

  // If in superposition mode with both graphs
  if (fGrHistory && fGrTeo) {
    Int_t catId = fCategoryCombo->GetSelected();
    if (catId == 6) { // Superposition mode
      TCanvas *canvas = fEmbCanvas->GetCanvas();
      canvas->Clear();
      canvas->cd();
      canvas->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
      canvas->SetGrid();

      bool drawnFirst = false;
      TString drawOptHistory = fConnectHistory ? "APL" : "AP";
      TString drawOptTeo = fConnectTeo ? "PL" : "P";

      // Redraw History
      if (fGrHistory->GetN() > 0) {
        fGrHistory->Draw(drawOptHistory.Data());
        drawnFirst = true;
      }

      // Redraw Teo on top
      if (fGrTeo->GetN() > 0) {
        if (drawnFirst) {
          fGrTeo->Draw(Form("%s SAME", drawOptTeo.Data()));
        } else {
          fGrTeo->Draw(drawOptTeo.Data());
        }
      }

      // Legend
      TLegend *legend = new TLegend(0.7, 0.75, 0.9, 0.9);
      legend->SetBorderSize(1);
      legend->SetFillColor(0);
      legend->AddEntry(fGrHistory, "History O3", "lp");
      legend->AddEntry(fGrTeo, "O3 Teo Study", "lp");
      legend->Draw();

      canvas->Modified();
      canvas->Update();
    }
  }

  // If in multi-year mode, reload the panel
  if (fMultiYearCheck->IsOn()) {
    Int_t catId = fCategoryCombo->GetSelected();
    if (catId == 6 || catId == 2 || catId == 3) {
      LoadMultiYearPanel();
    }
  }
}
void O3ViewerGUI::OnCategorySelected() {
  Int_t catId = fCategoryCombo->GetSelected();

  if (catId == 0) {
    fYearCombo->SetEnabled(kFALSE);
    fMultiYearCheck->SetEnabled(kFALSE);
    fMultiYearCheck->SetOn(kFALSE);
    PopulateGraphs();
  } else {
    fYearCombo->SetEnabled(kTRUE);
    fMultiYearCheck->SetEnabled(kTRUE);
    PopulateYears();
  }
}

void O3ViewerGUI::OnYearSelected() { PopulateGraphs(); }

void O3ViewerGUI::OnMultiYearToggled() {
  if (fMultiYearCheck->IsOn()) {
    fYearCombo->SetEnabled(kFALSE);
  } else {
    fYearCombo->SetEnabled(kTRUE);
  }
}

void O3ViewerGUI::OnHistoryColorSelected() {
  Pixel_t pixel = fHistoryColorSelect->GetColor();
  fHistoryColor = TColor::GetColor(pixel);
}

void O3ViewerGUI::OnTeoColorSelected() {
  Pixel_t pixel = fTeoColorSelect->GetColor();
  fTeoColor = TColor::GetColor(pixel);
}

void O3ViewerGUI::DrawObject(TObject *obj, const char *path) {
  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->cd();
  canvas->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
  canvas->SetGrid();

  if (obj->InheritsFrom("TGraph")) {
    TGraph *gr = (TGraph *)obj;
    TGraph *grClone = (TGraph *)gr->Clone();
    grClone->Draw("AP");
  } else if (obj->InheritsFrom("TH1")) {
    TH1 *h = (TH1 *)obj;
    TH1 *hClone = (TH1 *)h->Clone();
    hClone->Draw("hist");
  } else if (obj->InheritsFrom("TCanvas")) {
    TCanvas *storedCanvas = (TCanvas *)obj;
    storedCanvas->DrawClonePad();
  } else {
    fStatusLabel->SetText(Form("Error: Unsupported object type at %s", path));
    return;
  }

  canvas->Modified();
  canvas->Update();
}

void O3ViewerGUI::LoadSuperposition() {
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: File not loaded!");
    return;
  }

  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();
  canvas->cd();
  canvas->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
  canvas->SetGrid();

  TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
  TGTextLBEntry *graphEntry = (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

  if (!yearEntry || !graphEntry) {
    fStatusLabel->SetText("Error: Please select year and graph!");
    canvas->Modified();
    canvas->Update();
    return;
  }

  TString year = yearEntry->GetText()->GetString();
  TString graphName = graphEntry->GetText()->GetString();

  // Load from History O3
  TString historyPath = Form("%s/history/%s", year.Data(), graphName.Data());
  TObject *historyObj = fRootFile->Get(historyPath.Data());

  // Load from O3 Teo Study
  TString teoPath = Form("%s/comp/%s", year.Data(), graphName.Data());
  TObject *teoObj = fRootFile->Get(teoPath.Data());

  bool drawnFirst = false;
  bool historyDrawn = false;
  bool teoDrawn = false;

  // Cleanup old graphs
  if (fGrHistory) {
    delete fGrHistory;
    fGrHistory = nullptr;
  }
  if (fGrTeo) {
    delete fGrTeo;
    fGrTeo = nullptr;
  }

  // Draw/Store History O3 first (in selected color) - only if it has points
  if (historyObj && historyObj->InheritsFrom("TGraph")) {
    fGrHistory = (TGraph *)historyObj->Clone("gr_history");
    if (fGrHistory->GetN() > 0) {
      fGrHistory->SetLineColor(fHistoryColor);
      fGrHistory->SetMarkerColor(fHistoryColor);
      fGrHistory->SetMarkerStyle(20);
      fGrHistory->SetMarkerSize(0.8);
      fGrHistory->SetTitle(Form("%s - Year %s", graphName.Data(), year.Data()));
      TString drawOpt = fConnectHistory ? "APL" : "AP";
      fGrHistory->Draw(drawOpt.Data());
      drawnFirst = true;
      historyDrawn = true;
    } else {
      delete fGrHistory;
      fGrHistory = nullptr;
      std::cout << "Warning: Empty History O3 graph for " << graphName.Data()
                << " in year " << year.Data() << std::endl;
    }
  } else if (historyObj && historyObj->InheritsFrom("TH1")) {
    // Handle TH1 similarly if needed, but assuming TGraph for superposition
    TH1 *hHistory = (TH1 *)historyObj->Clone("h_history");
    if (hHistory->GetEntries() > 0) {
      hHistory->SetLineColor(fHistoryColor);
      hHistory->SetLineWidth(2);
      hHistory->SetTitle(Form("%s - Year %s", graphName.Data(), year.Data()));
      hHistory->Draw("hist");
      drawnFirst = true;
      historyDrawn = true;
    } else {
      delete hHistory;
      std::cout << "Warning: Empty History O3 histogram for "
                << graphName.Data() << " in year " << year.Data() << std::endl;
    }
  }

  // Draw/Store O3 Teo Study (in selected color) - only if it has points
  if (teoObj && teoObj->InheritsFrom("TGraph")) {
    fGrTeo = (TGraph *)teoObj->Clone("gr_teo");
    if (fGrTeo->GetN() > 0) {
      fGrTeo->SetLineColor(fTeoColor);
      fGrTeo->SetMarkerColor(fTeoColor);
      fGrTeo->SetMarkerStyle(21);
      fGrTeo->SetMarkerSize(0.8);
      if (drawnFirst) {
        fGrTeo->Draw("PL SAME");
      } else {
        fGrTeo->SetTitle(Form("%s - Year %s", graphName.Data(), year.Data()));
        fGrTeo->Draw("PL");
        drawnFirst = true;
      }
      teoDrawn = true;
    } else {
      delete fGrTeo;
      fGrTeo = nullptr;
      std::cout << "Warning: Empty O3 Teo graph for " << graphName.Data()
                << " in year " << year.Data() << std::endl;
    }
  } else if (teoObj && teoObj->InheritsFrom("TH1")) {
    TH1 *hTeo = (TH1 *)teoObj->Clone("h_teo");
    if (hTeo->GetEntries() > 0) {
      hTeo->SetLineColor(fTeoColor);
      hTeo->SetLineWidth(2);
      hTeo->SetLineStyle(2);
      if (drawnFirst) {
        hTeo->Draw("hist SAME");
      } else {
        hTeo->SetTitle(Form("%s - Year %s", graphName.Data(), year.Data()));
        hTeo->Draw("hist");
        drawnFirst = true;
      }
      teoDrawn = true;
    } else {
      delete hTeo;
      std::cout << "Warning: Empty O3 Teo histogram for " << graphName.Data()
                << " in year " << year.Data() << std::endl;
    }
  }

  // Add legend only for drawn components
  if (drawnFirst) {
    TLegend *legend = new TLegend(0.7, 0.75, 0.9, 0.9);
    legend->SetBorderSize(1);
    legend->SetFillColor(0);

    if (historyDrawn) {
      if (historyObj->InheritsFrom("TGraph")) {
        legend->AddEntry(fGrHistory, "History O3", "lp");
      } else if (historyObj->InheritsFrom("TH1")) {
        legend->AddEntry("h_history", "History O3", "l");
      }
    }

    if (teoDrawn) {
      if (teoObj->InheritsFrom("TGraph")) {
        legend->AddEntry(fGrTeo, "O3 Teo Study", "lp");
      } else if (teoObj->InheritsFrom("TH1")) {
        legend->AddEntry("h_teo", "O3 Teo Study", "l");
      }
    }

    legend->Draw();
  } else {
    // Nothing drawn - show a message on canvas
    TLatex *text = new TLatex(0.5, 0.5,
                              Form("No data available for %s (Year %s)",
                                   graphName.Data(), year.Data()));
    text->SetTextAlign(22);
    text->SetTextSize(0.05);
    text->SetTextColor(kRed);
    text->Draw();
  }

  canvas->Modified();
  canvas->Update();

  // Update status based on what was drawn
  if (historyDrawn && teoDrawn) {
    fStatusLabel->SetText(Form(
        "Superposition: History O3 + O3 Teo Study (Year %s)", year.Data()));
  } else if (historyDrawn) {
    fStatusLabel->SetText(
        Form("Superposition: Only History O3 (Year %s)", year.Data()));
  } else if (teoDrawn) {
    fStatusLabel->SetText(
        Form("Superposition: Only O3 Teo Study (Year %s)", year.Data()));
  } else {
    fStatusLabel->SetText(
        Form("No valid data for superposition (Year %s)", year.Data()));
  }
}

void O3ViewerGUI::LoadMultiYearPanel() {
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: File not loaded!");
    return;
  }

  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();

  Int_t catId = fCategoryCombo->GetSelected();
  TGTextLBEntry *graphEntry = (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

  if (!graphEntry) {
    fStatusLabel->SetText("Error: No graph selected!");
    canvas->Modified();
    canvas->Update();
    return;
  }

  // Collect all years
  std::vector<TString> years;
  TIter next(fRootFile->GetListOfKeys());
  TKey *key;

  while ((key = (TKey *)next())) {
    TString name = key->GetName();
    if (name.IsDigit()) {
      years.push_back(name);
    }
  }

  if (years.size() == 0) {
    fStatusLabel->SetText("Error: No years found!");
    canvas->Modified();
    canvas->Update();
    return;
  }

  // Calculate grid layout
  Int_t nYears = years.size();
  Int_t nCols = (Int_t)TMath::Ceil(TMath::Sqrt(nYears));
  Int_t nRows = (Int_t)TMath::Ceil((Double_t)nYears / nCols);

  canvas->Clear();
  canvas->Divide(nCols, nRows);

  TString graphName = graphEntry->GetText()->GetString();
  TString subdir;

  if (catId == 6) {
    // Superposition mode - load both history and comp
    for (Int_t i = 0; i < nYears; i++) {
      canvas->cd(i + 1);
      gPad->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
      gPad->SetGrid();

      TString historyPath =
          Form("%s/history/%s", years[i].Data(), graphName.Data());
      TString teoPath = Form("%s/comp/%s", years[i].Data(), graphName.Data());

      TObject *historyObj = fRootFile->Get(historyPath.Data());
      TObject *teoObj = fRootFile->Get(teoPath.Data());

      bool drawnFirst = false;
      bool anythingDrawn = false;

      // Draw history - only if it has points
      if (historyObj && historyObj->InheritsFrom("TGraph")) {
        TGraph *gr =
            (TGraph *)historyObj->Clone(Form("gr_hist_%s", years[i].Data()));
        TString drawOptHistory = fConnectHistory ? "APL" : "AP";
        if (gr->GetN() > 0) {
          gr->SetTitle(Form("%s", years[i].Data()));
          gr->SetLineColor(fHistoryColor);
          gr->SetMarkerColor(fHistoryColor);
          gr->SetMarkerStyle(20);
          gr->SetMarkerSize(0.6);
          gr->Draw(drawOptHistory.Data());
          drawnFirst = true;
          anythingDrawn = true;
        } else {
          delete gr;
        }
      }

      // Draw teo - only if it has points
      if (teoObj && teoObj->InheritsFrom("TGraph")) {
        TGraph *gr =
            (TGraph *)teoObj->Clone(Form("gr_teo_%s", years[i].Data()));
        TString drawOptTeo = fConnectTeo ? "PL" : "P";
        if (gr->GetN() > 0) {
          gr->SetLineColor(fTeoColor);
          gr->SetMarkerColor(fTeoColor);
          gr->SetMarkerStyle(21);
          gr->SetMarkerSize(0.6);
          if (drawnFirst) {
            gr->Draw(Form("%s SAME", drawOptTeo.Data()));
          } else {
            gr->SetTitle(Form("%s", years[i].Data()));
            gr->Draw(Form("A%s", drawOptTeo.Data()));
          }
          anythingDrawn = true;
        } else {
          delete gr;
        }
      }

      if (!anythingDrawn) {
        if (!historyObj && !teoObj) {
          TLatex *text =
              new TLatex(0.5, 0.5, Form("%s\nNot Found", years[i].Data()));
          text->SetTextAlign(22);
          text->SetTextSize(0.06);
          text->SetTextColor(kRed);
          text->Draw();
        } else {
          TLatex *text =
              new TLatex(0.5, 0.5, Form("%s\nEmpty Graph(s)", years[i].Data()));
          text->SetTextAlign(22);
          text->SetTextSize(0.06);
          text->SetTextColor(kOrange);
          text->Draw();
        }
      }
    }
  } else {
    // Normal mode
    switch (catId) {
    case 1:
      subdir = "fm";
      break;
    case 2:
      subdir = "history";
      break;
    case 3:
      subdir = "comp";
      break;
    case 4:
      subdir = "error";
      break;
    case 5:
      if (graphName.Contains("/")) {
        subdir = graphName(0, graphName.First('/'));
        graphName = graphName(graphName.First('/') + 1, graphName.Length());
      }
      break;
    }

    for (Int_t i = 0; i < nYears; i++) {
      canvas->cd(i + 1);
      gPad->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label
      gPad->SetGrid();

      TString path;
      if (catId >= 1 && catId <= 4) {
        path =
            Form("%s/%s/%s", years[i].Data(), subdir.Data(), graphName.Data());
      } else if (catId == 5) {
        path =
            Form("%s/%s/%s", years[i].Data(), subdir.Data(), graphName.Data());
      }

      TObject *obj = fRootFile->Get(path.Data());

      if (obj) {
        if (obj->InheritsFrom("TGraph")) {
          TGraph *gr = (TGraph *)obj->Clone(Form("gr_%s", years[i].Data()));
          gr->SetTitle(Form("%s - %s", years[i].Data(), graphName.Data()));
          gr->Draw("AP");
        } else if (obj->InheritsFrom("TH1")) {
          TH1 *h = (TH1 *)obj->Clone(Form("h_%s", years[i].Data()));
          h->SetTitle(Form("%s - %s", years[i].Data(), graphName.Data()));
          h->Draw("hist");
        }
      } else {
        TLatex *text =
            new TLatex(0.5, 0.5, Form("%s\nNot Found", years[i].Data()));
        text->SetTextAlign(22);
        text->SetTextSize(0.06);
        text->Draw();
      }
    }
  }

  canvas->Modified();
  canvas->Update();
  fStatusLabel->SetText(Form("Loaded multi-year panel: %d years", nYears));
}

void O3ViewerGUI::LoadGraph() {
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: File not loaded!");
    return;
  }

  Int_t catId = fCategoryCombo->GetSelected();

  // Check if superposition mode is selected
  if (catId == 6) {
    if (fMultiYearCheck->IsOn()) {
      LoadMultiYearPanel();
    } else {
      LoadSuperposition();
    }
    return;
  }

  // Check if multi-year mode is enabled
  if (fMultiYearCheck->IsOn() && catId != 0) {
    LoadMultiYearPanel();
    return;
  }

  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();
  canvas->cd();
  canvas->SetBottomMargin(0.20);  // Increase bottom margin to show x-axis label

  if (catId == 0) {
    TDirectory *anaDir = (TDirectory *)fRootFile->Get("ana");
    if (!anaDir) {
      fStatusLabel->SetText("Error: ana directory not found!");
      canvas->Modified();
      canvas->Update();
      return;
    }

    TGTextLBEntry *entry = (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();
    if (!entry) {
      fStatusLabel->SetText("Error: No graph selected!");
      canvas->Modified();
      canvas->Update();
      return;
    }

    TString sel = entry->GetText()->GetString();

    // 1) If the user selected a saved TCanvas in ana, draw it directly.
    if (sel == "Solar Calculus" || sel == "Form Factor f(m)" ||
        sel == "history o3" || sel == "o3teo study" || sel == "o3teo error") {
      TCanvas *storedCanvas = (TCanvas *)anaDir->Get(sel.Data());
      if (storedCanvas) {
        canvas->cd();
        storedCanvas->DrawClonePad();
        canvas->Modified();
        canvas->Update();
        fStatusLabel->SetText(Form("Loaded canvas: %s", sel.Data()));
      } else {
        fStatusLabel->SetText(
            Form("Error: Canvas '%s' not found in ana", sel.Data()));
        canvas->Modified();
        canvas->Update();
      }
    } else {
      // 2) Otherwise treat selection as one of the individual Graphs saved as
      // Graph;N Map known labels to the Graph cycle index (Graph;1..Graph;6)
      Int_t cycle = -1;
      if (sel == "delta") {
        cycle = 1;
      } else if (sel == "AST") {
        cycle = 2;
      } else if (sel == "omega_s") {
        cycle = 3;
      } else if (sel == "cos(z)") {
        cycle = 4;
      } else if (sel == "(R0/R)^2") {
        cycle = 5;
      } else if (sel == "G(Ise)") {
        cycle = 6;
      }

      if (cycle > 0) {
        // Graph;N are stored under ana. Use the full path so Get() finds it.
        TString graphPath = Form("ana/Graph;%d", cycle);
        TGraph *gr = (TGraph *)fRootFile->Get(graphPath.Data());
        if (gr) {
          TGraph *grClone = (TGraph *)gr->Clone();
          canvas->cd();
          grClone->Draw("A*");
          canvas->SetGrid();
          canvas->Modified();
          canvas->Update();
          fStatusLabel->SetText(
              Form("Loaded: %s (Graph;%d)", sel.Data(), cycle));
        } else {
          fStatusLabel->SetText(
              Form("Error: Graph not found at %s", graphPath.Data()));
          canvas->Modified();
          canvas->Update();
        }
      } else {
        fStatusLabel->SetText(
            Form("Error: Unknown selection '%s'", sel.Data()));
        canvas->Modified();
        canvas->Update();
      }
    }
  } else if (catId >= 1 && catId <= 4) {
    TDirectory *anaDir = (TDirectory *)fRootFile->Get("ana");
    if (!anaDir) {
      fStatusLabel->SetText("Error: ana directory not found!");
      canvas->Modified();
      canvas->Update();
      return;
    }

    TString canvasNames[] = {"", "Form Factor", "History O3", "O3 Teo Study",
                             "O3 Teo Error"};

    TCanvas *storedCanvas = (TCanvas *)anaDir->Get(canvasNames[catId].Data());
    if (storedCanvas) {
      canvas->cd();
      storedCanvas->DrawClonePad();
      canvas->Modified();
      canvas->Update();
      fStatusLabel->SetText(Form("Loaded: %s", canvasNames[catId].Data()));
      return;
    }

    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    TGTextLBEntry *graphEntry =
        (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

    if (!yearEntry || !graphEntry) {
      fStatusLabel->SetText("Error: Please select year and graph!");
      canvas->Modified();
      canvas->Update();
      return;
    }

    TString year = yearEntry->GetText()->GetString();
    TString graphName = graphEntry->GetText()->GetString();
    TString subdir;

    switch (catId) {
    case 1:
      subdir = "fm";
      break;
    case 2:
      subdir = "history";
      break;
    case 3:
      subdir = "comp";
      break;
    case 4:
      subdir = "error";
      break;
    }

    TString path =
        Form("%s/%s/%s", year.Data(), subdir.Data(), graphName.Data());
    TObject *obj = fRootFile->Get(path.Data());

    if (obj) {
      DrawObject(obj, path.Data());
      fStatusLabel->SetText(Form("Fallback loaded: %s", path.Data()));
    } else {
      fStatusLabel->SetText(Form("Error: Object not found at %s", path.Data()));
      canvas->Modified();
      canvas->Update();
    }
  } else if (catId == 5) {
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    TGTextLBEntry *graphEntry =
        (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

    if (!yearEntry || !graphEntry) {
      fStatusLabel->SetText("Error: Please select year and graph!");
      canvas->Modified();
      canvas->Update();
      return;
    }

    TString year = yearEntry->GetText()->GetString();
    TString graphPath = graphEntry->GetText()->GetString();
    TString fullPath = Form("%s/%s", year.Data(), graphPath.Data());

    TObject *obj = fRootFile->Get(fullPath.Data());
    if (obj) {
      DrawObject(obj, fullPath.Data());
      fStatusLabel->SetText(Form("Loaded: %s", fullPath.Data()));
    } else {
      fStatusLabel->SetText(
          Form("Error: Object not found at %s", fullPath.Data()));
      canvas->Modified();
      canvas->Update();
    }
  }
}

void O3ViewerGUI::ExportData() {
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: No file loaded!");
    return;
  }

  Int_t catId = fCategoryCombo->GetSelected();
  TGTextLBEntry *graphEntry = (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();
  TGTextLBEntry *locEntry = (TGTextLBEntry *)fLocationCombo->GetSelectedEntry();

  if (!locEntry) {
    fStatusLabel->SetText("Error: No location selected!");
    return;
  }

  TString locName = locEntry->GetText()->GetString();

  // Check if exporting all years
  bool exportAllYears = fExportAllYearsCheck->IsOn();

  std::vector<TString> yearsToExport;

  if (exportAllYears) {
    // Collect all years from the file
    TIter next(fRootFile->GetListOfKeys());
    TKey *key;
    while ((key = (TKey *)next())) {
      TString name = key->GetName();
      if (name.IsDigit()) {
        yearsToExport.push_back(name);
      }
    }

    if (yearsToExport.size() == 0) {
      fStatusLabel->SetText("Error: No years found in file!");
      return;
    }
  } else {
    // Export only selected year
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    if (!yearEntry) {
      fStatusLabel->SetText("Error: Please select a year!");
      return;
    }
    yearsToExport.push_back(yearEntry->GetText()->GetString());
  }

  if (!graphEntry) {
    fStatusLabel->SetText("Error: Please select a graph!");
    return;
  }

  TString graphName = graphEntry->GetText()->GetString();

  // Export for superposition mode (category 6) or individual History/Teo
  if (catId == 6) {
    // Superposition mode - export both History and Teo

    // Create output filename
    TString outputFile;
    if (exportAllYears) {
      outputFile =
          Form("%s_%s_AllYears_export.txt", locName.Data(), graphName.Data());
    } else {
      outputFile = Form("%s_%s_year%s_export.txt", locName.Data(),
                        graphName.Data(), yearsToExport[0].Data());
    }

    std::ofstream outFile(outputFile.Data());
    if (!outFile.is_open()) {
      fStatusLabel->SetText("Error: Cannot create export file!");
      return;
    }

    // Write header
    outFile << "# Location: " << locName.Data() << std::endl;
    outFile << "# Graph: " << graphName.Data() << std::endl;
    if (exportAllYears) {
      outFile << "# Years: ALL (" << yearsToExport.size() << " years)"
              << std::endl;
    } else {
      outFile << "# Year: " << yearsToExport[0].Data() << std::endl;
    }
    outFile << "# Export Date: " << TDatime().AsString() << std::endl;
    outFile << "#" << std::endl;

    int totalHistoryPoints = 0;
    int totalTeoPoints = 0;
    int yearsWithHistory = 0;
    int yearsWithTeo = 0;

    // Loop through all years
    for (size_t iYear = 0; iYear < yearsToExport.size(); iYear++) {
      TString year = yearsToExport[iYear];

      // Load History O3 data
      TString historyPath =
          Form("%s/history/%s", year.Data(), graphName.Data());
      TObject *historyObj = fRootFile->Get(historyPath.Data());

      // Load O3 Teo data
      TString teoPath = Form("%s/comp/%s", year.Data(), graphName.Data());
      TObject *teoObj = fRootFile->Get(teoPath.Data());

      // Export History O3 for this year
      if (historyObj && historyObj->InheritsFrom("TGraph")) {
        TGraph *grHistory = (TGraph *)historyObj;
        if (grHistory->GetN() > 0) {
          if (totalHistoryPoints == 0) {
            outFile << "# ===== HISTORY O3 DATA =====" << std::endl;
            outFile << "# Year\tPoint\tX\tY" << std::endl;
          }

          for (Int_t i = 0; i < grHistory->GetN(); i++) {
            Double_t x, y;
            grHistory->GetPoint(i, x, y);
            outFile << year.Data() << "\t" << i << "\t" << x << "\t" << y
                    << std::endl;
          }
          totalHistoryPoints += grHistory->GetN();
          yearsWithHistory++;
        }
      }

      // Export O3 Teo for this year
      if (teoObj && teoObj->InheritsFrom("TGraph")) {
        TGraph *grTeo = (TGraph *)teoObj;
        if (grTeo->GetN() > 0) {
          if (totalTeoPoints == 0) {
            outFile << std::endl;
            outFile << "# ===== O3 TEO STUDY DATA =====" << std::endl;
            outFile << "# Year\tPoint\tX\tY" << std::endl;
          }

          for (Int_t i = 0; i < grTeo->GetN(); i++) {
            Double_t x, y;
            grTeo->GetPoint(i, x, y);
            outFile << year.Data() << "\t" << i << "\t" << x << "\t" << y
                    << std::endl;
          }
          totalTeoPoints += grTeo->GetN();
          yearsWithTeo++;
        }
      }
    }

    // Write summary at the end
    outFile << std::endl;
    outFile << "# ===== SUMMARY =====" << std::endl;
    outFile << "# Years processed: " << yearsToExport.size() << std::endl;
    outFile << "# History O3: " << yearsWithHistory << " years, "
            << totalHistoryPoints << " total points" << std::endl;
    outFile << "# O3 Teo Study: " << yearsWithTeo << " years, "
            << totalTeoPoints << " total points" << std::endl;

    outFile.close();

    if (totalHistoryPoints > 0 || totalTeoPoints > 0) {
      fStatusLabel->SetText(Form("Exported %d years to: %s",
                                 (int)yearsToExport.size(), outputFile.Data()));
      std::cout << "Data exported to: " << outputFile.Data() << std::endl;
      std::cout << "  History points: " << totalHistoryPoints << std::endl;
      std::cout << "  Teo points: " << totalTeoPoints << std::endl;
    } else {
      fStatusLabel->SetText("Error: No valid data to export!");
    }

  } else if (catId == 2 || catId == 3) {
    // Export individual History O3 (cat 2) or O3 Teo Study (cat 3)

    TString subdir = (catId == 2) ? "history" : "comp";
    TString catName = (catId == 2) ? "HistoryO3" : "TeoO3";

    // Create output filename
    TString outputFile;
    if (exportAllYears) {
      outputFile = Form("%s_%s_%s_AllYears_export.txt", locName.Data(),
                        catName.Data(), graphName.Data());
    } else {
      outputFile =
          Form("%s_%s_%s_year%s_export.txt", locName.Data(), catName.Data(),
               graphName.Data(), yearsToExport[0].Data());
    }

    std::ofstream outFile(outputFile.Data());
    if (!outFile.is_open()) {
      fStatusLabel->SetText("Error: Cannot create export file!");
      return;
    }

    // Write header
    outFile << "# Location: " << locName.Data() << std::endl;
    outFile << "# Category: " << catName.Data() << std::endl;
    outFile << "# Graph: " << graphName.Data() << std::endl;
    if (exportAllYears) {
      outFile << "# Years: ALL (" << yearsToExport.size() << " years)"
              << std::endl;
    } else {
      outFile << "# Year: " << yearsToExport[0].Data() << std::endl;
    }
    outFile << "# Export Date: " << TDatime().AsString() << std::endl;
    outFile << "#" << std::endl;

    int totalPoints = 0;
    int yearsWithData = 0;
    bool isGraph = true;

    // Loop through all years
    for (size_t iYear = 0; iYear < yearsToExport.size(); iYear++) {
      TString year = yearsToExport[iYear];
      TString path =
          Form("%s/%s/%s", year.Data(), subdir.Data(), graphName.Data());
      TObject *obj = fRootFile->Get(path.Data());

      if (!obj) {
        if (yearsToExport.size() == 1) {
          outFile.close();
          fStatusLabel->SetText("Error: Object not found!");
          return;
        }
        continue;
      }

      if (obj->InheritsFrom("TGraph")) {
        TGraph *gr = (TGraph *)obj;
        if (gr->GetN() > 0) {
          if (totalPoints == 0) {
            outFile << "# Year\tPoint\tX\tY" << std::endl;
          }

          for (Int_t i = 0; i < gr->GetN(); i++) {
            Double_t x, y;
            gr->GetPoint(i, x, y);
            outFile << year.Data() << "\t" << i << "\t" << x << "\t" << y
                    << std::endl;
          }
          totalPoints += gr->GetN();
          yearsWithData++;
        }
      } else if (obj->InheritsFrom("TH1")) {
        isGraph = false;
        TH1 *h = (TH1 *)obj;
        if (totalPoints == 0) {
          outFile << "# Year\tBin\tBinCenter\tContent\tError" << std::endl;
        }

        for (Int_t i = 1; i <= h->GetNbinsX(); i++) {
          outFile << year.Data() << "\t" << i << "\t" << h->GetBinCenter(i)
                  << "\t" << h->GetBinContent(i) << "\t" << h->GetBinError(i)
                  << std::endl;
        }
        totalPoints += h->GetNbinsX();
        yearsWithData++;
      }
    }

    // Write summary
    outFile << std::endl;
    outFile << "# ===== SUMMARY =====" << std::endl;
    outFile << "# Years processed: " << yearsToExport.size() << std::endl;
    outFile << "# Years with data: " << yearsWithData << std::endl;
    outFile << "# Total " << (isGraph ? "points" : "bins") << ": "
            << totalPoints << std::endl;

    outFile.close();

    if (totalPoints > 0) {
      fStatusLabel->SetText(Form("Exported %d years to: %s",
                                 (int)yearsToExport.size(), outputFile.Data()));
      std::cout << "Data exported to: " << outputFile.Data() << std::endl;
      std::cout << "  Total " << (isGraph ? "points" : "bins") << ": "
                << totalPoints << std::endl;
    } else {
      fStatusLabel->SetText("Error: No valid data to export!");
    }

  } else {
    fStatusLabel->SetText("Export only available for History O3, O3 Teo Study, "
                          "or Superposition!");
  }
}

void O3ViewerGUI::RefreshData() {
  // Close current file if open
  if (fRootFile && fRootFile->IsOpen()) {
    fRootFile->Close();
    delete fRootFile;
    fRootFile = nullptr;
  }

  // Clear canvas
  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();
  canvas->Modified();
  canvas->Update();

  // Clear stored graphs
  if (fGrHistory) {
    delete fGrHistory;
    fGrHistory = nullptr;
  }
  if (fGrTeo) {
    delete fGrTeo;
    fGrTeo = nullptr;
  }

  // Rescan locations
  fStatusLabel->SetText("Refreshing locations...");
  ScanLocations();

  fStatusLabel->SetText("Locations refreshed!");
}

void O3ViewerGUI::CloseWindow() { gApplication->Terminate(0); }

// Main function
void viewO3Global(const char *basedir = ".") {
  if (!gApplication) {
    TApplication *theApp = new TApplication("App", 0, 0);
    new O3ViewerGUI(gClient->GetRoot(), 1200, 800, basedir);
    theApp->Run();
  } else {
    new O3ViewerGUI(gClient->GetRoot(), 1200, 800, basedir);
  }
}

#ifndef __CINT__

// ============================================================================
// Embeddable version for ozone_gui.cpp
// ============================================================================
TGMainFrame *CreateViewO3GlobalGUI(TGCompositeFrame *parent) {
  // Create instance of your existing O3ViewerGUI class
  TGMainFrame *mainFrame = new O3ViewerGUI(parent, 800, 600);

  mainFrame->SetCleanup(kDeepCleanup);
  mainFrame->MapSubwindows();
  return mainFrame;
}
#endif
