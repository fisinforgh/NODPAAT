
#include <TApplication.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGComboBox.h>
#include <TGDoubleSlider.h>
#include <TGFrame.h>
#include <TGLabel.h>
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
  TGTextEntry *fParam1Entry, *fParam2Entry, *fParam3Entry;
  TGTextView *fLogView;

public:
  OzoneGUI(const TGWindow *p, UInt_t w, UInt_t h) : TGMainFrame(p, w, h) {
    // -------- Mode Section --------
    TGGroupFrame *modeFrame = new TGGroupFrame(this, "Execution Mode");
    fModeSelector = new TGComboBox(modeFrame);
    fModeSelector->AddEntry("pgrid", 1);
    fModeSelector->AddEntry("nmap", 2);
    fModeSelector->AddEntry("another_mode", 3);
    fModeSelector->Select(1); // default pgrid
    modeFrame->AddFrame(fModeSelector,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    AddFrame(modeFrame,
             new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 10, 10, 5));

    // -------- Data Path Section --------
    TGGroupFrame *pathFrame = new TGGroupFrame(this, "Data Path");
    fDataPathEntry = new TGTextEntry(pathFrame, new TGTextBuffer(200));
    fDataPathEntry->SetText("../../wget_ozono_NASA");
    pathFrame->AddFrame(fDataPathEntry,
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
    latLabelFrame->AddFrame(fLatMinLabel,
                            new TGLayoutHints(kLHintsLeft | kLHintsExpandX));
    latLabelFrame->AddFrame(fLatMaxLabel,
                            new TGLayoutHints(kLHintsRight | kLHintsExpandX));
    latFrame->AddFrame(latLabelFrame,
                       new TGLayoutHints(kLHintsExpandX, 10, 10, 0, 10));
    AddFrame(latFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Parameters Section --------
    TGGroupFrame *paramFrame = new TGGroupFrame(this, "Parameters");

    TGHorizontalFrame *p1 = new TGHorizontalFrame(paramFrame);
    p1->AddFrame(new TGLabel(p1, "Param 1:"),
                 new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
    fParam1Entry = new TGTextEntry(p1, new TGTextBuffer(20));
    fParam1Entry->SetText("10");
    p1->AddFrame(fParam1Entry, new TGLayoutHints(kLHintsExpandX));
    paramFrame->AddFrame(p1, new TGLayoutHints(kLHintsExpandX));

    TGHorizontalFrame *p2 = new TGHorizontalFrame(paramFrame);
    p2->AddFrame(new TGLabel(p2, "Param 2:"),
                 new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
    fParam2Entry = new TGTextEntry(p2, new TGTextBuffer(20));
    fParam2Entry->SetText("7");
    p2->AddFrame(fParam2Entry, new TGLayoutHints(kLHintsExpandX));
    paramFrame->AddFrame(p2, new TGLayoutHints(kLHintsExpandX));

    TGHorizontalFrame *p3 = new TGHorizontalFrame(paramFrame);
    p3->AddFrame(new TGLabel(p3, "Param 3:"),
                 new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
    fParam3Entry = new TGTextEntry(p3, new TGTextBuffer(20));
    fParam3Entry->SetText("4");
    p3->AddFrame(fParam3Entry, new TGLayoutHints(kLHintsExpandX));
    paramFrame->AddFrame(p3, new TGLayoutHints(kLHintsExpandX));

    AddFrame(paramFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Run Button --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(this);
    TGTextButton *btnRun = new TGTextButton(btnFrame, "&Run Processor");
    btnRun->Connect("Clicked()", "OzoneGUI", this, "RunProcessor()");
    btnFrame->AddFrame(btnRun, new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));
    AddFrame(btnFrame, new TGLayoutHints(kLHintsCenterX));

    // -------- Log Output --------
    TGGroupFrame *logFrame = new TGGroupFrame(this, "Process Log");
    fLogView = new TGTextView(logFrame, 600, 200);
    logFrame->AddFrame(
        fLogView,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    AddFrame(logFrame,
             new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 5, 10));

    SetWindowName("Optimized Ozone Processor GUI");
    MapSubwindows();
    Resize(GetDefaultSize());
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

  void RunProcessor() {
    TGTextLBEntry *entry = (TGTextLBEntry *)fModeSelector->GetSelectedEntry();
    TString mode = entry ? entry->GetTitle() : "";

    double latMin = fLatSlider->GetMinPosition();
    double latMax = fLatSlider->GetMaxPosition();

    std::string cmd =
        "./optimized_ozone_processor " + std::string(mode.Data()) + " " +
        std::string(fDataPathEntry->GetText()) + " " +
        std::to_string((int)latMin) + " " + std::to_string((int)latMax) + " " +
        std::string(fParam1Entry->GetText()) + " " +
        std::string(fParam2Entry->GetText()) + " " +
        std::string(fParam3Entry->GetText());

    AppendLog(Form("Running: %s", cmd.c_str()));

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      AppendLog("Error: failed to run process.");
      return;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      AppendLog(buffer);
      gSystem->ProcessEvents();
    }

    int status = pclose(pipe);
    AppendLog(Form("Process finished with status %d", status));
  }

  ClassDef(OzoneGUI, 0);
};

void ozone_gui() { new OzoneGUI(gClient->GetRoot(), 800, 600); }
