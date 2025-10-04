#include "TApplication.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGButton.h"
#include "TGClient.h"
#include "TGComboBox.h"
#include "TGFrame.h"
#include "TGLabel.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TKey.h"
#include "TList.h"
#include "TRootEmbeddedCanvas.h"
#include <iostream>

class O3ViewerGUI : public TGMainFrame {
private:
  TGComboBox *fCategoryCombo;
  TGComboBox *fYearCombo;
  TGComboBox *fGraphCombo;
  TGTextButton *fLoadButton;
  TGTextButton *fExitButton;
  TGLabel *fStatusLabel;
  TRootEmbeddedCanvas *fEmbCanvas;
  TFile *fRootFile;
  TString fFileName;

public:
  O3ViewerGUI(const TGWindow *p, UInt_t w, UInt_t h, const char *filename);
  virtual ~O3ViewerGUI();

  void LoadFile();
  void PopulateYears();
  void PopulateGraphs();
  void LoadGraph();
  void OnCategorySelected();
  void OnYearSelected();
  void CloseWindow();

  ClassDef(O3ViewerGUI, 0)
};

O3ViewerGUI::O3ViewerGUI(const TGWindow *p, UInt_t w, UInt_t h,
                         const char *filename)
    : TGMainFrame(p, w, h), fRootFile(nullptr), fFileName(filename) {

  SetWindowName("O3 Global Analysis Viewer");
  SetCleanup(kDeepCleanup);

  // Main vertical frame
  TGVerticalFrame *vframe = new TGVerticalFrame(this, w, h);

  // Control panel frame
  TGHorizontalFrame *controlFrame = new TGHorizontalFrame(vframe, w, 150);

  // Left control panel
  TGVerticalFrame *leftControl = new TGVerticalFrame(controlFrame, w / 2, 150);

  // Category selection
  TGHorizontalFrame *catFrame = new TGHorizontalFrame(leftControl, w / 2, 30);
  TGLabel *catLabel = new TGLabel(catFrame, "Category:");
  catFrame->AddFrame(
      catLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
  fCategoryCombo = new TGComboBox(catFrame, 100);
  fCategoryCombo->AddEntry("Solar Calculus", 0);
  fCategoryCombo->AddEntry("Form Factor", 1);
  fCategoryCombo->AddEntry("History O3", 2);
  fCategoryCombo->AddEntry("O3 Teo Study", 3);
  fCategoryCombo->AddEntry("O3 Teo Error", 4);
  fCategoryCombo->AddEntry("Individual Graphs", 5);
  fCategoryCombo->Resize(200, 20);
  fCategoryCombo->Select(0);
  fCategoryCombo->Connect("Selected(Int_t)", "O3ViewerGUI", this,
                          "OnCategorySelected()");
  catFrame->AddFrame(fCategoryCombo,
                     new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
  leftControl->AddFrame(
      catFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

  // Year selection
  TGHorizontalFrame *yearFrame = new TGHorizontalFrame(leftControl, w / 2, 30);
  TGLabel *yearLabel = new TGLabel(yearFrame, "Year:");
  yearFrame->AddFrame(
      yearLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
  fYearCombo = new TGComboBox(yearFrame, 101);
  fYearCombo->Resize(200, 20);
  fYearCombo->Connect("Selected(Int_t)", "O3ViewerGUI", this,
                      "OnYearSelected()");
  yearFrame->AddFrame(fYearCombo, new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
  leftControl->AddFrame(
      yearFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

  // Graph selection
  TGHorizontalFrame *graphFrame = new TGHorizontalFrame(leftControl, w / 2, 30);
  TGLabel *graphLabel = new TGLabel(graphFrame, "Graph:");
  graphFrame->AddFrame(
      graphLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
  fGraphCombo = new TGComboBox(graphFrame, 102);
  fGraphCombo->Resize(200, 20);
  graphFrame->AddFrame(fGraphCombo, new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
  leftControl->AddFrame(
      graphFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

  controlFrame->AddFrame(
      leftControl, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));

  // Right control panel - Buttons
  TGVerticalFrame *rightControl = new TGVerticalFrame(controlFrame, w / 2, 150);

  fLoadButton = new TGTextButton(rightControl, "&Load Graph", 200);
  fLoadButton->Connect("Clicked()", "O3ViewerGUI", this, "LoadGraph()");
  rightControl->AddFrame(
      fLoadButton, new TGLayoutHints(kLHintsCenterX | kLHintsTop, 5, 5, 5, 5));

  fExitButton = new TGTextButton(rightControl, "&Exit", 201);
  fExitButton->Connect("Clicked()", "O3ViewerGUI", this, "CloseWindow()");
  rightControl->AddFrame(
      fExitButton, new TGLayoutHints(kLHintsCenterX | kLHintsTop, 5, 5, 5, 5));

  fStatusLabel = new TGLabel(rightControl, "Ready");
  rightControl->AddFrame(
      fStatusLabel,
      new TGLayoutHints(kLHintsCenterX | kLHintsTop, 5, 5, 10, 5));

  controlFrame->AddFrame(rightControl,
                         new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

  vframe->AddFrame(controlFrame,
                   new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

  // Embedded canvas
  fEmbCanvas = new TRootEmbeddedCanvas("EmbCanvas", vframe, w - 10, h - 160);
  vframe->AddFrame(
      fEmbCanvas,
      new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

  AddFrame(vframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();

  // Load the file
  LoadFile();
}

O3ViewerGUI::~O3ViewerGUI() {
  if (fRootFile && fRootFile->IsOpen()) {
    fRootFile->Close();
    delete fRootFile;
  }
  Cleanup();
}

void O3ViewerGUI::LoadFile() {
  fRootFile = TFile::Open(fFileName.Data());
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: Cannot open file!");
    std::cerr << "Error: Cannot open file " << fFileName.Data() << std::endl;
    return;
  }
  fStatusLabel->SetText("File loaded successfully");
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
  Int_t yearId = fYearCombo->GetSelected();

  if (catId == 0) {
    // Solar Calculus - individual graphs
    fGraphCombo->AddEntry("delta", 0);
    fGraphCombo->AddEntry("AST", 1);
    fGraphCombo->AddEntry("omega_s", 2);
    fGraphCombo->AddEntry("cos(z)", 3);
    fGraphCombo->AddEntry("(R0/R)^2", 4);
    fGraphCombo->AddEntry("G(Ise)", 5);
  } else if (catId >= 1 && catId <= 4) {
    // Year-based categories
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
    // All individual graphs
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

void O3ViewerGUI::OnCategorySelected() {
  Int_t catId = fCategoryCombo->GetSelected();

  if (catId == 0) {
    // Solar Calculus doesn't need year selection
    fYearCombo->SetEnabled(kFALSE);
    PopulateGraphs();
  } else {
    fYearCombo->SetEnabled(kTRUE);
    PopulateYears();
  }
}

void O3ViewerGUI::OnYearSelected() { PopulateGraphs(); }

void O3ViewerGUI::LoadGraph() {
  if (!fRootFile || fRootFile->IsZombie()) {
    fStatusLabel->SetText("Error: File not loaded!");
    return;
  }

  TCanvas *canvas = fEmbCanvas->GetCanvas();
  canvas->Clear();
  canvas->cd();

  Int_t catId = fCategoryCombo->GetSelected();

  if (catId == 0) {
    // Load from ana directory for Solar Calculus
    TDirectory *anaDir = (TDirectory *)fRootFile->Get("ana");
    if (!anaDir) {
      fStatusLabel->SetText("Error: ana directory not found!");
      return;
    }

    TCanvas *storedCanvas = (TCanvas *)anaDir->Get("Solar Calculus");
    if (storedCanvas) {
      storedCanvas->DrawClonePad();
      fStatusLabel->SetText("Loaded: Solar Calculus");
    } else {
      // Load individual graph
      TGTextLBEntry *entry = (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();
      if (!entry)
        return;

      TString graphNames[] = {"grdh",  "grAST",  "grws",
                              "grczh", "grRoR2", "grGIse"};
      Int_t graphId = fGraphCombo->GetSelected();

      if (graphId >= 0 && graphId < 6) {
        TGraph *gr = (TGraph *)anaDir->Get(graphNames[graphId].Data());
        if (gr) {
          gr->Draw("A*");
          canvas->SetGrid();
          fStatusLabel->SetText(Form("Loaded: %s", graphNames[graphId].Data()));
        }
      }
    }
  } else if (catId >= 1 && catId <= 4) {
    // Load canvas or individual graph
    TDirectory *anaDir = (TDirectory *)fRootFile->Get("ana");
    TString canvasNames[] = {"", "Form Factor f(m)", "history o3",
                             "o3teo study", "o3teo error"};

    TCanvas *storedCanvas = (TCanvas *)anaDir->Get(canvasNames[catId].Data());
    if (storedCanvas) {
      storedCanvas->DrawClonePad();
      fStatusLabel->SetText(Form("Loaded: %s", canvasNames[catId].Data()));
    } else {
      // Load individual graph from year directory
      TGTextLBEntry *yearEntry =
          (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
      TGTextLBEntry *graphEntry =
          (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

      if (!yearEntry || !graphEntry)
        return;

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

      if (obj && obj->InheritsFrom("TGraph")) {
        TGraph *gr = (TGraph *)obj;
        gr->Draw("AP");
        canvas->SetGrid();
        fStatusLabel->SetText(Form("Loaded: %s", path.Data()));
      }
    }
  } else if (catId == 5) {
    // Individual graphs
    TGTextLBEntry *yearEntry = (TGTextLBEntry *)fYearCombo->GetSelectedEntry();
    TGTextLBEntry *graphEntry =
        (TGTextLBEntry *)fGraphCombo->GetSelectedEntry();

    if (!yearEntry || !graphEntry)
      return;

    TString year = yearEntry->GetText()->GetString();
    TString graphPath = graphEntry->GetText()->GetString();
    TString fullPath = Form("%s/%s", year.Data(), graphPath.Data());

    TObject *obj = fRootFile->Get(fullPath.Data());
    if (obj && obj->InheritsFrom("TGraph")) {
      TGraph *gr = (TGraph *)obj;
      gr->Draw("AP");
      canvas->SetGrid();
      fStatusLabel->SetText(Form("Loaded: %s", fullPath.Data()));
    }
  }

  canvas->Modified();
  canvas->Update();
}

void O3ViewerGUI::CloseWindow() { gApplication->Terminate(0); }

// Main function
void viewO3Global(const char *filename = "skim_*/location_global.root") {
  TApplication theApp("App", 0, 0);
  new O3ViewerGUI(gClient->GetRoot(), 1200, 800, filename);
  theApp.Run();
}

// If compiling as standalone
#ifndef __CINT__
int main(int argc, char **argv) {
  TApplication theApp("App", &argc, argv);

  const char *filename = "global.root";
  if (argc > 1) {
    filename = argv[1];
  }

  new O3ViewerGUI(gClient->GetRoot(), 1200, 800, filename);
  theApp.Run();
  return 0;
}
#endif
