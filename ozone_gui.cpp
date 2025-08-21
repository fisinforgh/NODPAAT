
#include <TApplication.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGComboBox.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TSystem.h>
#include <iostream>

class OzoneGUI : public TGMainFrame {
private:
  TGComboBox *fModeSelector;
  TGTextEntry *fDataPathEntry;
  TGDoubleHSlider *fLatSlider;
  TGLabel *fLatMinLabel, *fLatMaxLabel;
  TGNumberEntry *fParam1Entry, *fParam2Entry, *fParam3Entry;
  TGTextView *fLogView;
  TString fExePath; // full path to executable

public:
  OzoneGUI(const TGWindow *p, UInt_t w, UInt_t h) : TGMainFrame(p, w, h) {
    // -------- Mode Section --------
    TGGroupFrame *modeFrame = new TGGroupFrame(this, "Execution Mode");

    TGHorizontalFrame *modeHFrame = new TGHorizontalFrame(modeFrame);
    modeHFrame->AddFrame(
        new TGLabel(modeHFrame, "Mode:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fModeSelector = new TGComboBox(modeHFrame);
    fModeSelector->AddEntry("pgrid", 1);
    fModeSelector->AddEntry("nmap", 2);
    fModeSelector->AddEntry("another_mode", 3);
    fModeSelector->Select(1);       // default pgrid
    fModeSelector->Resize(200, 28); // normalized height
    modeHFrame->AddFrame(
        fModeSelector,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    modeFrame->AddFrame(modeHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    AddFrame(modeFrame,
             new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 10, 10, 5));

    // -------- Data Path Section --------
    TGGroupFrame *pathFrame = new TGGroupFrame(this, "Data Path");
    TGHorizontalFrame *pathHFrame = new TGHorizontalFrame(pathFrame);

    fDataPathEntry = new TGTextEntry(pathHFrame, new TGTextBuffer(200));
    fDataPathEntry->SetText("../../wget_ozono_NASA");
    fDataPathEntry->Resize(300, 28); // normalized height
    pathHFrame->AddFrame(fDataPathEntry,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    TGTextButton *browseBtn = new TGTextButton(pathHFrame, "&Browse...");
    browseBtn->Resize(100, 28);
    browseBtn->Connect("Clicked()", "OzoneGUI", this, "BrowseForFolder()");
    pathHFrame->AddFrame(
        browseBtn,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 5, 5));

    pathFrame->AddFrame(pathHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    AddFrame(pathFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Latitude Section --------
    TGGroupFrame *latFrame = new TGGroupFrame(this, "Latitude Range");
    fLatSlider =
        new TGDoubleHSlider(latFrame, 200, kDoubleScaleBoth, -1,
                            kHorizontalFrame, GetDefaultFrameBackground());
    fLatSlider->SetRange(-90, 90);
    fLatSlider->SetPosition(-90, 90);
    fLatSlider->Connect("PositionChanged()", "OzoneGUI", this,
                        "UpdateLatLabels()");
    latFrame->AddFrame(fLatSlider,
                       new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    TGHorizontalFrame *latLabelFrame = new TGHorizontalFrame(latFrame);
    fLatMinLabel = new TGLabel(latLabelFrame, "Min: -90");
    fLatMaxLabel = new TGLabel(latLabelFrame, "Max: 90");
    latLabelFrame->AddFrame(fLatMinLabel, new TGLayoutHints(kLHintsLeft));
    latLabelFrame->AddFrame(fLatMaxLabel, new TGLayoutHints(kLHintsRight));
    latFrame->AddFrame(latLabelFrame,
                       new TGLayoutHints(kLHintsExpandX, 10, 10, 0, 10));
    AddFrame(latFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Parameters Section --------
    TGGroupFrame *paramFrame = new TGGroupFrame(this, "Parameters");

    TGHorizontalFrame *p1 = new TGHorizontalFrame(paramFrame);
    p1->AddFrame(new TGLabel(p1, "Param 1:"),
                 new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fParam1Entry = new TGNumberEntry(p1, 10, 6, -1, TGNumberFormat::kNESInteger,
                                     TGNumberFormat::kNEANonNegative,
                                     TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam1Entry->Resize(80, 28);
    p1->AddFrame(fParam1Entry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));
    paramFrame->AddFrame(p1, new TGLayoutHints(kLHintsExpandX));

    TGHorizontalFrame *p2 = new TGHorizontalFrame(paramFrame);
    p2->AddFrame(new TGLabel(p2, "Param 2:"),
                 new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fParam2Entry = new TGNumberEntry(p2, 7, 6, -1, TGNumberFormat::kNESInteger,
                                     TGNumberFormat::kNEANonNegative,
                                     TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam2Entry->Resize(80, 28);
    p2->AddFrame(fParam2Entry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));
    paramFrame->AddFrame(p2, new TGLayoutHints(kLHintsExpandX));

    TGHorizontalFrame *p3 = new TGHorizontalFrame(paramFrame);
    p3->AddFrame(new TGLabel(p3, "Param 3:"),
                 new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fParam3Entry = new TGNumberEntry(p3, 4, 6, -1, TGNumberFormat::kNESInteger,
                                     TGNumberFormat::kNEANonNegative,
                                     TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam3Entry->Resize(80, 28);
    p3->AddFrame(fParam3Entry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));
    paramFrame->AddFrame(p3, new TGLayoutHints(kLHintsExpandX));

    AddFrame(paramFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Run Button --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(this);
    TGTextButton *btnRun = new TGTextButton(btnFrame, "&Run Processor");
    btnRun->Resize(120, 28); // normalized height
    btnRun->Connect("Clicked()", "OzoneGUI", this, "RunProcessor()");
    btnFrame->AddFrame(btnRun, new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));
    AddFrame(btnFrame, new TGLayoutHints(kLHintsCenterX));

    // -------- Log Output --------
    TGGroupFrame *logFrame = new TGGroupFrame(this, "Process Log");
    fLogView = new TGTextView(logFrame, 400, 200); // log stays bigger
    logFrame->AddFrame(
        fLogView,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    AddFrame(logFrame,
             new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 5, 10));

    // -------- Setup exe path --------
    fExePath =
        gSystem->Which(".", "optimized_ozone_processor", kExecutePermission);
    if (fExePath.IsNull()) {
      AppendLog(
          "Warning: optimized_ozone_processor not found in current folder!");
    }

    // Final setup
    SetWindowName("Optimized Ozone Processor GUI");
    MapSubwindows();
    Resize(500, 600); // compact size
    MapWindow();

    UpdateLatLabels();
  }

  void UpdateLatLabels() {
    double latMin = fLatSlider->GetMinPosition();
    double latMax = fLatSlider->GetMaxPosition();
    fLatMinLabel->SetText(Form("Min: %.0f", latMin));
    fLatMaxLabel->SetText(Form("Max: %.0f", latMax));
    Layout();
  }

  void AppendLog(const char *msg) {
    fLogView->AddLine(msg);
    fLogView->ShowBottom();
  }

  void BrowseForFolder() {
    static TString dir(".");
    TGFileInfo fi;
    fi.fFileTypes = nullptr; // no file filter
    fi.fIniDir = StrDup(dir);

    new TGFileDialog(gClient->GetRoot(), this, kDOpen, &fi);

    if (fi.fFilename) {
      dir = fi.fIniDir; // remember last directory
      fDataPathEntry->SetText(fi.fFilename);
    }
  }

  void RunProcessor() {
    if (fExePath.IsNull()) {
      AppendLog("Error: processor executable not found.");
      return;
    }

    // Save current working directory
    TString oldDir = gSystem->WorkingDirectory();

    // Change to executable directory
    TString exeDir = gSystem->DirName(fExePath);
    gSystem->ChangeDirectory(exeDir);

    TGTextLBEntry *entry = (TGTextLBEntry *)fModeSelector->GetSelectedEntry();
    TString mode = entry ? entry->GetTitle() : "";

    double latMin = fLatSlider->GetMinPosition();
    double latMax = fLatSlider->GetMaxPosition();

    std::string cmd =
        std::string(fExePath.Data()) + " " + std::string(mode.Data()) + " " +
        std::string(fDataPathEntry->GetText()) + " " +
        std::to_string((int)latMin) + " " + std::to_string((int)latMax) + " " +
        std::to_string((int)fParam1Entry->GetNumber()) + " " +
        std::to_string((int)fParam2Entry->GetNumber()) + " " +
        std::to_string((int)fParam3Entry->GetNumber());

    AppendLog(Form("Running: %s", cmd.c_str()));

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      AppendLog("Error: failed to run process.");
      gSystem->ChangeDirectory(oldDir); // restore before returning
      return;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      AppendLog(buffer);
      gSystem->ProcessEvents();
    }

    int status = pclose(pipe);
    AppendLog(Form("Process finished with status %d", status));

    // Restore previous working directory
    gSystem->ChangeDirectory(oldDir);
  }

  ClassDef(OzoneGUI, 0);
};

void ozone_gui() { new OzoneGUI(gClient->GetRoot(), 500, 600); }
