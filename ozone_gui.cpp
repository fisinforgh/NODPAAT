#include <TApplication.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TGButton.h>
#include <TGComboBox.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGListBox.h>
#include <TGMsgBox.h>
#include <TGNumberEntry.h>
#include <TGTab.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH1.h>
#include <TH2.h>
#include <TKey.h>
#include <TList.h>
#include <TRegexp.h>
#include <TRootEmbeddedCanvas.h>
#include <TSystem.h>
#include <TTimer.h>
#include <chrono>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class OzoneGUI : public TGMainFrame {
private:
  // Original UI elements
  TGComboBox *fModeSelector;
  TGTextEntry *fDataPathEntry;
  TGTextEntry *fLocationEntry;
  TGDoubleHSlider *fLatSlider;
  TGLabel *fLatMinLabel, *fLatMaxLabel;
  TGNumberEntry *fParamGrid, *fParamEvents, *fParamThreads;
  TGTextView *fLogView;
  TGTextButton *fSilentModeButton;
  TGTextButton *fNotificationButton;
  TString fExePath;

  // Process control
  TGTextButton *fRunButton, *fCancelButton;
  pid_t fProcessPid;
  int fPipeFd[2];
  TTimer *fOutputTimer;
  Bool_t fProcessRunning;
  Bool_t fSilentMode;
  Bool_t fNotificationsEnabled;
  std::chrono::time_point<std::chrono::steady_clock> fProcessStartTime;
  std::vector<std::string> fOutputBuffer;
  static const int kMaxBufferSize = 50;

  // New graph viewer elements
  TGTab *fMainTabs;
  TGTextEntry *fGraphPathEntry;
  TGListBox *fFolderListBox;
  TGListBox *fFileListBox;
  TRootEmbeddedCanvas *fGraphCanvas;
  TGTextButton *fRefreshButton;
  TGLabel *fGraphInfoLabel;
  std::vector<TString> fCurrentFolders;
  std::vector<TString> fCurrentFiles;
  TString fCurrentGraphPath;
  TFile *fCurrentFile;                    // Keep file open
  std::vector<TObject *> fCurrentObjects; // Keep objects in memory
  TGTextEntry *fFolderFilterEntry;
  TGTextButton *fClearFilterButton;
  TString fCurrentFilter;

  // New macro runner elements
  TGNumberEntry *fMacroParam1, *fMacroParam3;
  TGTextEntry *fMacroParam2;
  TGTextButton *fRunMacroButton, *fCancelMacroButton;
  TGTextView *fMacroLogView;
  pid_t fMacroPid;
  TTimer *fMacroTimer;
  Bool_t fMacroRunning;
  std::chrono::time_point<std::chrono::steady_clock> fMacroStartTime;

  int fMacroPipeFd;

public:
  OzoneGUI(const TGWindow *p, UInt_t w, UInt_t h)
      : TGMainFrame(p, w, h), fProcessPid(-1), fOutputTimer(nullptr),
        fProcessRunning(kFALSE), fSilentMode(kFALSE),
        fNotificationsEnabled(kTRUE), fCurrentFile(nullptr), fMacroPid(-1),
        fMacroTimer(nullptr), fMacroRunning(kFALSE), fMacroPipeFd(-1) {

    // Initialize pipe
    fPipeFd[0] = fPipeFd[1] = -1;

    // Create main tab widget
    fMainTabs = new TGTab(this, 600, 800);

    // ======== PROCESSOR TAB ========
    TGCompositeFrame *processorTab = fMainTabs->AddTab("Processor");

    // Move all existing processor UI to this tab
    CreateProcessorInterface(processorTab);

    // ======== GRAPH VIEWER TAB ========
    TGCompositeFrame *graphTab = fMainTabs->AddTab("Graph Viewer");
    CreateGraphInterface(graphTab);

    // ======== MACRO RUNNER TAB ========
    TGCompositeFrame *macroTab = fMainTabs->AddTab("Macro Runner");
    CreateMacroInterface(macroTab);

    AddFrame(fMainTabs,
             new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

    // Setup exe path
    fExePath =
        gSystem->Which(".", "optimized_ozone_processor", kExecutePermission);
    if (fExePath.IsNull()) {
      AppendLog(
          "Warning: optimized_ozone_processor not found in current folder!");
    }

    // Final setup
    SetWindowName("Optimized Ozone Processor GUI with Graph Viewer");
    MapSubwindows();
    Resize(800, 900);
    MapWindow();

    UpdateLatLabels();

    // Initialize graph viewer with current directory
    fGraphPathEntry->SetText(gSystem->WorkingDirectory());
    RefreshFolderList();
  }

  void CreateProcessorInterface(TGCompositeFrame *parent) {
    // -------- Mode Section --------
    TGGroupFrame *modeFrame = new TGGroupFrame(parent, "Execution Mode");
    TGHorizontalFrame *modeHFrame = new TGHorizontalFrame(modeFrame);
    modeHFrame->AddFrame(
        new TGLabel(modeHFrame, "Mode:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fModeSelector = new TGComboBox(modeHFrame);
    fModeSelector->AddEntry("pgrid", 1);
    fModeSelector->AddEntry("location", 2);
    fModeSelector->Select(1);
    fModeSelector->Resize(200, 28);
    modeHFrame->AddFrame(
        fModeSelector,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    modeFrame->AddFrame(modeHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(modeFrame, new TGLayoutHints(kLHintsExpandX | kLHintsTop,
                                                  10, 10, 10, 5));

    TGGroupFrame *locFrame =
        new TGGroupFrame(parent, "Location (only for 'location' mode)");
    TGHorizontalFrame *locHFrame = new TGHorizontalFrame(locFrame);
    locHFrame->AddFrame(
        new TGLabel(locHFrame, "Code:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fLocationEntry = new TGTextEntry(locHFrame, new TGTextBuffer(50));
    fLocationEntry->SetText("BOG");
    fLocationEntry->Resize(100, 28);
    locHFrame->AddFrame(
        fLocationEntry,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    locFrame->AddFrame(locHFrame,
                       new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(locFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Data Path Section --------
    TGGroupFrame *pathFrame = new TGGroupFrame(parent, "Data Path");
    TGHorizontalFrame *pathHFrame = new TGHorizontalFrame(pathFrame);

    fDataPathEntry = new TGTextEntry(pathHFrame, new TGTextBuffer(200));
    fDataPathEntry->SetText("../../wget_ozono_NASA");
    fDataPathEntry->Resize(300, 28);
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
    parent->AddFrame(pathFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Latitude Section --------
    TGGroupFrame *latFrame = new TGGroupFrame(parent, "Latitude Range");
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
    parent->AddFrame(latFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Parameters Section --------
    TGGroupFrame *paramFrame = new TGGroupFrame(parent, "Parameters");
    TGCompositeFrame *paramMatrix =
        new TGCompositeFrame(paramFrame, 1, 1, kHorizontalFrame);
    paramMatrix->SetLayoutManager(new TGMatrixLayout(paramMatrix, 0, 2, 10, 5));

    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Grid:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParamGrid =
        new TGNumberEntry(paramMatrix, 10, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParamGrid->Resize(80, 28);
    paramMatrix->AddFrame(
        fParamGrid,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Events:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParamEvents =
        new TGNumberEntry(paramMatrix, 7, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParamEvents->Resize(80, 28);
    paramMatrix->AddFrame(
        fParamEvents,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Threads:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParamThreads =
        new TGNumberEntry(paramMatrix, 4, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParamThreads->Resize(80, 28);
    paramMatrix->AddFrame(
        fParamThreads,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    paramFrame->AddFrame(paramMatrix, new TGLayoutHints(kLHintsExpandX));
    parent->AddFrame(paramFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Performance Options --------
    TGGroupFrame *perfFrame = new TGGroupFrame(parent, "Performance Options");
    TGHorizontalFrame *perfHFrame = new TGHorizontalFrame(perfFrame);

    fSilentModeButton = new TGTextButton(perfHFrame, "Silent Mode: OFF");
    fSilentModeButton->Connect("Clicked()", "OzoneGUI", this,
                               "ToggleSilentMode()");
    fSilentModeButton->Resize(150, 28);
    perfHFrame->AddFrame(fSilentModeButton,
                         new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));

    fNotificationButton = new TGTextButton(perfHFrame, "Notifications: ON");
    fNotificationButton->Connect("Clicked()", "OzoneGUI", this,
                                 "ToggleNotifications()");
    fNotificationButton->Resize(150, 28);
    perfHFrame->AddFrame(fNotificationButton,
                         new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));

    perfFrame->AddFrame(perfHFrame, new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
    parent->AddFrame(perfFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Run and Cancel Buttons --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(parent);

    fRunButton = new TGTextButton(btnFrame, "&Run Processor");
    fRunButton->Resize(120, 28);
    fRunButton->Connect("Clicked()", "OzoneGUI", this, "RunProcessor()");
    btnFrame->AddFrame(fRunButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    fCancelButton = new TGTextButton(btnFrame, "&Cancel");
    fCancelButton->Resize(80, 28);
    fCancelButton->Connect("Clicked()", "OzoneGUI", this, "CancelProcessor()");
    fCancelButton->SetEnabled(kFALSE);
    btnFrame->AddFrame(fCancelButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    parent->AddFrame(btnFrame, new TGLayoutHints(kLHintsCenterX));

    // -------- Log Output --------
    TGGroupFrame *logFrame = new TGGroupFrame(parent, "Process Log");
    fLogView = new TGTextView(logFrame, 400, 150);
    logFrame->AddFrame(
        fLogView,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    parent->AddFrame(
        logFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 5, 10));
  }

  void CreateGraphInterface(TGCompositeFrame *parent) {
    // -------- Graph Path Selection --------
    TGGroupFrame *pathFrame = new TGGroupFrame(parent, "Graph Directory");
    TGHorizontalFrame *pathHFrame = new TGHorizontalFrame(pathFrame);

    fGraphPathEntry = new TGTextEntry(pathHFrame, new TGTextBuffer(200));
    fGraphPathEntry->SetText(gSystem->WorkingDirectory());
    fGraphPathEntry->Resize(400, 28);
    pathHFrame->AddFrame(fGraphPathEntry,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    TGTextButton *browseDirBtn = new TGTextButton(pathHFrame, "Browse...");
    browseDirBtn->Resize(80, 28);
    browseDirBtn->Connect("Clicked()", "OzoneGUI", this,
                          "BrowseForGraphFolder()");
    pathHFrame->AddFrame(browseDirBtn,
                         new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

    fRefreshButton = new TGTextButton(pathHFrame, "Refresh");
    fRefreshButton->Resize(80, 28);
    fRefreshButton->Connect("Clicked()", "OzoneGUI", this,
                            "RefreshFolderList()");
    pathHFrame->AddFrame(fRefreshButton,
                         new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

    pathFrame->AddFrame(pathHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(pathFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 5));

    // -------- NEW: Folder Filter Section --------
    TGGroupFrame *filterFrame = new TGGroupFrame(parent, "Folder Filter");
    TGHorizontalFrame *filterHFrame = new TGHorizontalFrame(filterFrame);

    filterHFrame->AddFrame(
        new TGLabel(filterHFrame, "Filter:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fFolderFilterEntry = new TGTextEntry(filterHFrame, new TGTextBuffer(100));
    fFolderFilterEntry->SetText("");
    fFolderFilterEntry->Resize(300, 28);
    fFolderFilterEntry->Connect("TextChanged(const char*)", "OzoneGUI", this,
                                "OnFilterChanged(const char*)");
    fFolderFilterEntry->Connect("ReturnPressed()", "OzoneGUI", this,
                                "ApplyFilter()");
    filterHFrame->AddFrame(fFolderFilterEntry,
                           new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    TGTextButton *applyFilterBtn = new TGTextButton(filterHFrame, "Apply");
    applyFilterBtn->Resize(60, 28);
    applyFilterBtn->Connect("Clicked()", "OzoneGUI", this, "ApplyFilter()");
    filterHFrame->AddFrame(applyFilterBtn,
                           new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

    fClearFilterButton = new TGTextButton(filterHFrame, "Clear");
    fClearFilterButton->Resize(60, 28);
    fClearFilterButton->Connect("Clicked()", "OzoneGUI", this, "ClearFilter()");
    filterHFrame->AddFrame(fClearFilterButton,
                           new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

    filterFrame->AddFrame(filterHFrame,
                          new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(filterFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Folder and File Selection --------
    TGHorizontalFrame *listFrame = new TGHorizontalFrame(parent);

    // Folder list (larger since it's the main selection)
    TGGroupFrame *folderFrame = new TGGroupFrame(listFrame, "Skim Folders");
    fFolderListBox = new TGListBox(folderFrame);
    fFolderListBox->Resize(300, 120); // Reduced height
    fFolderListBox->Connect("Selected(Int_t)", "OzoneGUI", this,
                            "OnFolderSelected(Int_t)");
    folderFrame->AddFrame(
        fFolderListBox,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    listFrame->AddFrame(
        folderFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

    // File info (just display, no selection needed)
    TGGroupFrame *fileFrame = new TGGroupFrame(listFrame, "File Info");
    fFileListBox = new TGListBox(fileFrame);
    fFileListBox->Resize(200, 120); // Smaller since it's just info
    fileFrame->AddFrame(
        fFileListBox,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    listFrame->AddFrame(
        fileFrame, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5));

    parent->AddFrame(listFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5,
                                                  5)); // Fixed height

    // -------- Graph Info --------
    fGraphInfoLabel =
        new TGLabel(parent, "Select a folder and file to view graphs");
    parent->AddFrame(fGraphInfoLabel,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Graph Canvas --------
    TGGroupFrame *canvasFrame = new TGGroupFrame(parent, "Graph Display");
    fGraphCanvas = new TRootEmbeddedCanvas("GraphCanvas", canvasFrame, 700,
                                           500); // Much larger
    canvasFrame->AddFrame(
        fGraphCanvas,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    parent->AddFrame(
        canvasFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 5, 10));
  }
  void CreateMacroInterface(TGCompositeFrame *parent) {
    // -------- Macro Parameters Section --------
    TGGroupFrame *paramFrame = new TGGroupFrame(parent, "Macro Parameters");

    // Parameter 1 (number)
    TGHorizontalFrame *param1Frame = new TGHorizontalFrame(paramFrame);
    param1Frame->AddFrame(
        new TGLabel(param1Frame, "Parameter 1 (number):"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fMacroParam1 =
        new TGNumberEntry(param1Frame, 7, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fMacroParam1->Resize(100, 28);
    param1Frame->AddFrame(
        fMacroParam1,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    paramFrame->AddFrame(param1Frame,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    // Parameter 2 (string)
    TGHorizontalFrame *param2Frame = new TGHorizontalFrame(paramFrame);
    param2Frame->AddFrame(
        new TGLabel(param2Frame, "Parameter 2 (location):"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fMacroParam2 = new TGTextEntry(param2Frame, new TGTextBuffer(50));
    fMacroParam2->SetText("BOG");
    fMacroParam2->Resize(100, 28);
    param2Frame->AddFrame(
        fMacroParam2,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    paramFrame->AddFrame(param2Frame,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    // Parameter 3 (double)
    TGHorizontalFrame *param3Frame = new TGHorizontalFrame(paramFrame);
    param3Frame->AddFrame(
        new TGLabel(param3Frame, "Parameter 3 (double):"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    fMacroParam3 =
        new TGNumberEntry(param3Frame, 1.1, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, 0.0, 10.0);
    fMacroParam3->Resize(100, 28);
    param3Frame->AddFrame(
        fMacroParam3,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    paramFrame->AddFrame(param3Frame,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    parent->AddFrame(paramFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 5));

    // -------- Run and Cancel Buttons --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(parent);

    fRunMacroButton = new TGTextButton(btnFrame, "&Run Macro");
    fRunMacroButton->Resize(120, 28);
    fRunMacroButton->Connect("Clicked()", "OzoneGUI", this, "RunMacro()");
    btnFrame->AddFrame(fRunMacroButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    fCancelMacroButton = new TGTextButton(btnFrame, "&Cancel");
    fCancelMacroButton->Resize(80, 28);
    fCancelMacroButton->Connect("Clicked()", "OzoneGUI", this, "CancelMacro()");
    fCancelMacroButton->SetEnabled(kFALSE);
    btnFrame->AddFrame(fCancelMacroButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    parent->AddFrame(btnFrame, new TGLayoutHints(kLHintsCenterX));

    // -------- Macro Log Output --------
    TGGroupFrame *logFrame = new TGGroupFrame(parent, "Macro Log");
    fMacroLogView = new TGTextView(logFrame, 400, 200);
    logFrame->AddFrame(
        fMacroLogView,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    parent->AddFrame(
        logFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 5, 10));
  }

  virtual ~OzoneGUI() {
    if (fOutputTimer) {
      fOutputTimer->TurnOff();
      delete fOutputTimer;
    }
    if (fMacroTimer) {
      fMacroTimer->TurnOff();
      delete fMacroTimer;
    }
    CleanupProcess();
    CleanupGraphs();
    // Cleanup macro process if running
    if (fMacroRunning && fMacroPid != -1) {
      kill(fMacroPid, SIGTERM);
      waitpid(fMacroPid, nullptr, 0);
    }
    if (fMacroPipeFd != -1) {
      close(fMacroPipeFd);
    }
  }

  void CleanupGraphs() {
    // Clean up current objects
    for (auto *obj : fCurrentObjects) {
      if (obj)
        delete obj;
    }
    fCurrentObjects.clear();

    // Close current file
    if (fCurrentFile) {
      fCurrentFile->Close();
      delete fCurrentFile;
      fCurrentFile = nullptr;
    }
  }

  // ======== GRAPH VIEWER METHODS ========

  void BrowseForGraphFolder() {
    static TString dir(".");
    TGFileInfo fi;
    fi.fFileTypes = nullptr;
    fi.fIniDir = StrDup(dir);

    new TGFileDialog(gClient->GetRoot(), this, kDOpen, &fi);

    if (fi.fFilename) {
      dir = fi.fIniDir;
      fGraphPathEntry->SetText(fi.fIniDir);
      RefreshFolderList();
    }
  }

  void ApplyFilter() {
    fCurrentFilter = fFolderFilterEntry->GetText();
    fCurrentFilter.ToLower(); // Make filter case-insensitive
    RefreshFolderList();
  }

  void ClearFilter() {
    fFolderFilterEntry->SetText("");
    fCurrentFilter = "";
    RefreshFolderList();
  }

  void RefreshFolderList() {
    fFolderListBox->RemoveAll();
    fCurrentFolders.clear();

    TString basePath = TString(fGraphPathEntry->GetText());
    DIR *dirp = opendir(basePath.Data());

    if (!dirp) {
      fGraphInfoLabel->SetText("Error: Cannot open directory");
      return;
    }

    struct dirent *dp;
    int entryId = 0;
    int totalSkimFolders = 0;
    int filteredFolders = 0;

    while ((dp = readdir(dirp)) != nullptr) {
      TString name = dp->d_name;

      // Skip . and ..
      if (name == "." || name == "..")
        continue;

      // Check if it's a directory and matches skim pattern
      TString fullPath = TString(basePath) + "/" + name;
      struct stat st;
      if (stat(fullPath.Data(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if (name.Contains("skim_")) {
          totalSkimFolders++;

          // Apply filter if one is set
          bool passesFilter = true;
          if (!fCurrentFilter.IsNull()) {
            TString nameLower = name;
            nameLower.ToLower();

            // Check if the filter text is contained in the folder name
            if (!nameLower.Contains(fCurrentFilter.Data())) {
              passesFilter = false;
            }
          }

          if (passesFilter) {
            fFolderListBox->AddEntry(name.Data(), entryId);
            fCurrentFolders.push_back(name);
            entryId++;
            filteredFolders++;
          }
        }
      }
    }
    closedir(dirp);

    fFolderListBox->Layout();

    // Update info label to show filter status
    if (!fCurrentFilter.IsNull()) {
      fGraphInfoLabel->SetText(Form("Filter '%s': %d of %d skim folders shown",
                                    fCurrentFilter.Data(), filteredFolders,
                                    totalSkimFolders));
    } else {
      fGraphInfoLabel->SetText(Form("Found %d skim folders", totalSkimFolders));
    }
  }

  void OnFolderSelected(Int_t id) {
    if (id < 0 || id >= (Int_t)fCurrentFolders.size())
      return;

    TString selectedFolder = fCurrentFolders[id];
    TString folderPath =
        TString(fGraphPathEntry->GetText()) + "/" + selectedFolder;

    // Find the ROOT file in this folder and load it automatically
    LoadGraphsFromFolder(folderPath);

    // Extract lat/lon info from folder name
    TString info = "Selected: " + selectedFolder;
    TRegexp latPattern("LAT-?[0-9]+");
    TRegexp lonPattern("LON-?[0-9]+");

    Ssiz_t latStart, latLen, lonStart, lonLen;
    if (selectedFolder.Index(latPattern, &latLen, latStart) != -1) {
      TString latPart = selectedFolder(latStart, latLen);
      latPart.ReplaceAll("LAT", "");
      info += " (Lat: " + latPart + "°";

      if (selectedFolder.Index(lonPattern, &lonLen, lonStart) != -1) {
        TString lonPart = selectedFolder(lonStart, lonLen);
        lonPart.ReplaceAll("LON", "");
        info += ", Lon: " + lonPart + "°)";
      }
    }

    fGraphInfoLabel->SetText(info.Data());
  }

  void LoadGraphsFromFolder(const TString &folderPath) {
    // Update file list for display (but auto-load the file)
    fFileListBox->RemoveAll();
    fCurrentFiles.clear();

    DIR *dirp = opendir(folderPath.Data());
    if (!dirp) {
      fGraphInfoLabel->SetText("Error: Cannot open selected folder");
      return;
    }

    struct dirent *dp;
    TString rootFile = "";

    while ((dp = readdir(dirp)) != nullptr) {
      TString name = dp->d_name;

      // Look for ROOT files
      if (name.EndsWith(".root")) {
        rootFile = folderPath + "/" + name;
        fFileListBox->AddEntry(name.Data(), 0);
        fCurrentFiles.push_back(rootFile);
        break; // Take the first (and likely only) ROOT file
      }
    }
    closedir(dirp);

    fFileListBox->Layout();

    // Auto-load the ROOT file if found
    if (!rootFile.IsNull()) {
      fFileListBox->Select(0); // Highlight the file
      LoadAndDisplayGraphs(rootFile);
    } else {
      fGraphInfoLabel->SetText("No ROOT files found in folder");
      TCanvas *canvas = fGraphCanvas->GetCanvas();
      canvas->Clear();
      canvas->Update();
    }
  }

  void LoadAndDisplayGraphs(const TString &filename) {
    // Clean up previous objects and file
    CleanupGraphs();

    fCurrentFile = TFile::Open(filename.Data(), "READ");
    if (!fCurrentFile || fCurrentFile->IsZombie()) {
      fGraphInfoLabel->SetText("Error: Cannot open ROOT file");
      return;
    }

    TCanvas *canvas = fGraphCanvas->GetCanvas();
    canvas->Clear();
    canvas->Divide(2, 2); // Create 2x2 subdivision for multiple plots

    // Get list of all objects in file
    TList *keyList = fCurrentFile->GetListOfKeys();
    TIter next(keyList);
    TKey *key;

    int padNum = 1;
    int objectsDrawn = 0;

    fGraphInfoLabel->SetText(
        Form("File: %s", gSystem->BaseName(filename.Data())));

    while ((key = (TKey *)next()) && padNum <= 4) {
      TObject *obj = key->ReadObj();
      if (!obj)
        continue;

      // Clone the object to keep it in memory
      TObject *clonedObj = obj->Clone();
      fCurrentObjects.push_back(clonedObj);

      canvas->cd(padNum);

      if (clonedObj->InheritsFrom("TH1")) {
        TH1 *hist = (TH1 *)clonedObj;
        hist->SetDirectory(0); // Detach from file
        hist->Draw();
        objectsDrawn++;
      } else if (clonedObj->InheritsFrom("TH2")) {
        TH2 *hist2d = (TH2 *)clonedObj;
        hist2d->SetDirectory(0); // Detach from file
        hist2d->Draw("COLZ");
        objectsDrawn++;
      } else if (clonedObj->InheritsFrom("TGraph")) {
        TGraph *graph = (TGraph *)clonedObj;
        graph->Draw("ALP");
        objectsDrawn++;
      } else if (clonedObj->InheritsFrom("TGraph2D")) {
        TGraph2D *graph2d = (TGraph2D *)clonedObj;
        graph2d->Draw("TRI2Z");
        objectsDrawn++;
      } else {
        // Remove from our list if not drawable
        fCurrentObjects.pop_back();
        delete clonedObj;
        continue;
      }

      padNum++;
    }

    if (objectsDrawn == 0) {
      canvas->cd();
      canvas->Clear();
      TText *text = new TText(0.5, 0.5, "No drawable objects found in file");
      text->SetTextAlign(22);
      text->SetTextSize(0.04);
      text->Draw();
      fCurrentObjects.push_back(text);
    }

    canvas->Update();
    canvas->Modified();

    fGraphInfoLabel->SetText(Form("File: %s - Displayed %d objects",
                                  gSystem->BaseName(filename.Data()),
                                  objectsDrawn));
  }

  // ======== ORIGINAL PROCESSOR METHODS ========

  void UpdateLatLabels() {
    double latMin = fLatSlider->GetMinPosition();
    double latMax = fLatSlider->GetMaxPosition();
    fLatMinLabel->SetText(Form("Min: %.0f", latMin));
    fLatMaxLabel->SetText(Form("Max: %.0f", latMax));
    Layout();
  }

  void AppendLog(const char *msg) {
    if (fSilentMode && fProcessRunning) {
      return;
    }
    fLogView->AddLine(msg);
    fLogView->ShowBottom();
  }

  void FlushOutputBuffer() {
    if (fOutputBuffer.empty())
      return;

    for (const auto &line : fOutputBuffer) {
      fLogView->AddLine(line.c_str());
    }
    fLogView->ShowBottom();
    fOutputBuffer.clear();
  }

  void ToggleSilentMode() {
    fSilentMode = !fSilentMode;
    if (fSilentMode) {
      fSilentModeButton->SetText("Silent Mode: ON");
      AppendLog("Silent mode enabled - output will be shown only when process "
                "completes.");
    } else {
      fSilentModeButton->SetText("Silent Mode: OFF");
      AppendLog("Silent mode disabled - real-time output enabled.");
    }
  }

  void ToggleNotifications() {
    fNotificationsEnabled = !fNotificationsEnabled;
    if (fNotificationsEnabled) {
      fNotificationButton->SetText("Notifications: ON");
      AppendLog("Process completion notifications enabled.");
    } else {
      fNotificationButton->SetText("Notifications: OFF");
      AppendLog("Process completion notifications disabled.");
    }
  }

  void ShowCompletionNotification(int exitCode, const std::string &duration) {
    if (!fNotificationsEnabled)
      return;

    gVirtualX->Bell(0);

    TString message;
    if (exitCode == 0) {
      message = Form("Process completed successfully!\n\nExecution time: "
                     "%s\n\nCheck the log for details.",
                     duration.c_str());
      new TGMsgBox(gClient->GetRoot(), this, "Process Complete", message.Data(),
                   kMBIconAsterisk, kMBOk);
    } else {
      message = Form("Process completed with errors (exit code: "
                     "%d)\n\nExecution time: %s\n\nCheck the log for details.",
                     exitCode, duration.c_str());
      new TGMsgBox(gClient->GetRoot(), this, "Process Complete", message.Data(),
                   kMBIconExclamation, kMBOk);
    }

    TString sysNotifyCmd;
    if (exitCode == 0) {
      sysNotifyCmd =
          Form("notify-send 'Ozone Processor' 'Process completed successfully "
               "in %s' --icon=dialog-information 2>/dev/null &",
               duration.c_str());
    } else {
      sysNotifyCmd =
          Form("notify-send 'Ozone Processor' 'Process failed (exit code: %d) "
               "after %s' --icon=dialog-error 2>/dev/null &",
               exitCode, duration.c_str());
    }
    gSystem->Exec(sysNotifyCmd.Data());
  }

  std::string FormatDuration(std::chrono::seconds duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(
        duration % std::chrono::hours(1));
    auto seconds = duration % std::chrono::minutes(1);

    if (hours.count() > 0) {
      return Form("%ldh %ldm %lds", hours.count(), minutes.count(),
                  seconds.count());
    } else if (minutes.count() > 0) {
      return Form("%ldm %lds", minutes.count(), seconds.count());
    } else {
      return Form("%lds", seconds.count());
    }
  }

  void BrowseForFolder() {
    static TString dir(".");
    TGFileInfo fi;
    fi.fFileTypes = nullptr;
    fi.fIniDir = StrDup(dir);

    new TGFileDialog(gClient->GetRoot(), this, kDOpen, &fi);

    if (fi.fFilename) {
      dir = fi.fIniDir;
      fDataPathEntry->SetText(fi.fFilename);
    }
  }

  void CleanupProcess() {
    if (fPipeFd[0] != -1) {
      close(fPipeFd[0]);
      fPipeFd[0] = -1;
    }
    if (fPipeFd[1] != -1) {
      close(fPipeFd[1]);
      fPipeFd[1] = -1;
    }
    fProcessPid = -1;
    fProcessRunning = kFALSE;
  }

  void RunProcessor() {
    if (fExePath.IsNull()) {
      AppendLog("Error: processor executable not found.");
      return;
    }

    if (fProcessRunning) {
      AppendLog("Process is already running. Cancel it first.");
      return;
    }

    fOutputBuffer.clear();

    if (!fSilentMode) {
      if (pipe(fPipeFd) == -1) {
        AppendLog("Error: failed to create pipe.");
        return;
      }
    }

    TString oldDir = gSystem->WorkingDirectory();
    TString exeDir = gSystem->DirName(fExePath);

    TGTextLBEntry *entry = (TGTextLBEntry *)fModeSelector->GetSelectedEntry();
    TString mode = entry ? entry->GetTitle() : "";

    double latMin = fLatSlider->GetMinPosition();
    double latMax = fLatSlider->GetMaxPosition();

    std::string cmd;
    if (mode == "location") {
      TString locCode = fLocationEntry->GetText();
      cmd = std::string(fExePath.Data()) + " location " +
            std::string(locCode.Data()) + " " +
            std::string(fDataPathEntry->GetText()) + " " +
            std::to_string(4.36) + " " + std::to_string(-74.04) + " " +
            std::to_string((int)fParamGrid->GetNumber());
    } else {
      cmd = std::string(fExePath.Data()) + " " + std::string(mode.Data()) +
            " " + std::string(fDataPathEntry->GetText()) + " " +
            std::to_string((int)latMin) + " " + std::to_string((int)latMax) +
            " " + std::to_string((int)fParamGrid->GetNumber()) + " " +
            std::to_string((int)fParamEvents->GetNumber()) + " " +
            std::to_string((int)fParamThreads->GetNumber());
    }

    AppendLog(Form("Running: %s", cmd.c_str()));
    if (fSilentMode) {
      AppendLog("Running in silent mode - please wait for completion...");
    }

    fProcessStartTime = std::chrono::steady_clock::now();

    fProcessPid = fork();

    if (fProcessPid == -1) {
      AppendLog("Error: failed to fork process.");
      CleanupProcess();
      return;
    }

    if (fProcessPid == 0) {
      if (!fSilentMode) {
        close(fPipeFd[0]);
        dup2(fPipeFd[1], STDOUT_FILENO);
        dup2(fPipeFd[1], STDERR_FILENO);
        close(fPipeFd[1]);
      }

      chdir(exeDir.Data());
      execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)nullptr);
      exit(1);
    }

    if (!fSilentMode) {
      close(fPipeFd[1]);
      fPipeFd[1] = -1;

      int flags = fcntl(fPipeFd[0], F_GETFL, 0);
      fcntl(fPipeFd[0], F_SETFL, flags | O_NONBLOCK);

      if (!fOutputTimer) {
        fOutputTimer = new TTimer();
        fOutputTimer->Connect("Timeout()", "OzoneGUI", this,
                              "CheckProcessOutput()");
      }
      fOutputTimer->Start(500);
    } else {
      if (!fOutputTimer) {
        fOutputTimer = new TTimer();
        fOutputTimer->Connect("Timeout()", "OzoneGUI", this,
                              "CheckProcessOutput()");
      }
      fOutputTimer->Start(1000);
    }

    fProcessRunning = kTRUE;
    fRunButton->SetEnabled(kFALSE);
    fCancelButton->SetEnabled(kTRUE);
  }

  void CheckProcessOutput() {
    if (!fProcessRunning)
      return;

    if (!fSilentMode && fPipeFd[0] != -1) {
      char buffer[4096];
      ssize_t bytesRead;

      while ((bytesRead = read(fPipeFd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';

        char *line = strtok(buffer, "\n");
        while (line != nullptr) {
          fOutputBuffer.push_back(std::string(line));
          line = strtok(nullptr, "\n");
        }

        if (fOutputBuffer.size() >= kMaxBufferSize) {
          FlushOutputBuffer();
          gSystem->ProcessEvents();
        }
      }
    }

    int status;
    pid_t result = waitpid(fProcessPid, &status, WNOHANG);

    if (result == fProcessPid) {
      auto endTime = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(
          endTime - fProcessStartTime);
      std::string durationStr = FormatDuration(duration);

      if (!fSilentMode) {
        FlushOutputBuffer();
      }

      int exitCode = 0;
      if (WIFEXITED(status)) {
        exitCode = WEXITSTATUS(status);
        AppendLog(Form("Process completed with exit code %d (Duration: %s)",
                       exitCode, durationStr.c_str()));
      } else if (WIFSIGNALED(status)) {
        exitCode = WTERMSIG(status);
        AppendLog(Form("Process terminated by signal %d (Duration: %s)",
                       exitCode, durationStr.c_str()));
      }

      ShowCompletionNotification(exitCode, durationStr);

      fOutputTimer->Stop();
      CleanupProcess();
      fRunButton->SetEnabled(kTRUE);
      fCancelButton->SetEnabled(kFALSE);
    } else if (result == -1) {
      AppendLog("Error: lost connection to process.");
      fOutputTimer->Stop();
      CleanupProcess();
      fRunButton->SetEnabled(kTRUE);
      fCancelButton->SetEnabled(kFALSE);
    }
  }

  void CancelProcessor() {
    if (!fProcessRunning || fProcessPid == -1) {
      AppendLog("No process to cancel.");
      return;
    }

    AppendLog(Form("Cancelling process (PID: %d)...", fProcessPid));

    if (kill(fProcessPid, SIGTERM) == 0) {
      AppendLog("Sent SIGTERM, waiting for process to terminate...");

      for (int i = 0; i < 30; i++) {
        int status;
        pid_t result = waitpid(fProcessPid, &status, WNOHANG);
        if (result == fProcessPid) {
          AppendLog("Process terminated gracefully.");
          fOutputTimer->Stop();
          CleanupProcess();
          fRunButton->SetEnabled(kTRUE);
          fCancelButton->SetEnabled(kFALSE);
          return;
        }
        gSystem->Sleep(100);
        gSystem->ProcessEvents();
      }

      AppendLog("Process didn't terminate gracefully, using SIGKILL...");
      if (kill(fProcessPid, SIGKILL) == 0) {
        waitpid(fProcessPid, nullptr, 0);
        AppendLog("Process killed successfully.");
      } else {
        AppendLog("Error: failed to kill process.");
      }
    } else {
      AppendLog("Error: failed to send termination signal.");
    }

    fOutputTimer->Stop();
    CleanupProcess();
    fRunButton->SetEnabled(kTRUE);
    fCancelButton->SetEnabled(kFALSE);
  }

  // ======== MACRO RUNNER METHODS ========

  void AppendMacroLog(const char *msg) {
    fMacroLogView->AddLine(msg);
    fMacroLogView->ShowBottom();
  }

  void RunMacro() {
    if (fMacroRunning) {
      AppendMacroLog("Macro is already running. Cancel it first.");
      return;
    }

    // Check if macro file exists
    if (gSystem->AccessPathName("linearRelStudyO3vsSn.C")) {
      AppendMacroLog(
          "Error: linearRelStudyO3vsSn.C not found in current directory.");
      return;
    }

    // Get parameters
    int param1 = (int)fMacroParam1->GetNumber();
    TString param2 = fMacroParam2->GetText();
    double param3 = fMacroParam3->GetNumber();

    // Build ROOT command
    TString rootCmd =
        Form("root -l -b -q 'linearRelStudyO3vsSn.C(%d,\"%s\",%g)'", param1,
             param2.Data(), param3);

    AppendMacroLog(Form("Executing: %s", rootCmd.Data()));
    AppendMacroLog("Starting macro execution...");

    fMacroStartTime = std::chrono::steady_clock::now();

    // Create pipes for output
    int pipeFd[2];
    if (pipe(pipeFd) == -1) {
      AppendMacroLog("Error: failed to create pipe for macro output.");
      return;
    }

    fMacroPid = fork();

    if (fMacroPid == -1) {
      AppendMacroLog("Error: failed to fork process for macro.");
      close(pipeFd[0]);
      close(pipeFd[1]);
      return;
    }

    if (fMacroPid == 0) {
      // Child process
      close(pipeFd[0]);
      dup2(pipeFd[1], STDOUT_FILENO);
      dup2(pipeFd[1], STDERR_FILENO);
      close(pipeFd[1]);

      execl("/bin/sh", "sh", "-c", rootCmd.Data(), (char *)nullptr);
      exit(1);
    }

    // Parent process
    close(pipeFd[1]);

    // Set pipe to non-blocking
    int flags = fcntl(pipeFd[0], F_GETFL, 0);
    fcntl(pipeFd[0], F_SETFL, flags | O_NONBLOCK);

    // Store pipe fd (you'll need to add this as a member variable)
    fMacroPipeFd = pipeFd[0];

    // Start timer to check output
    if (!fMacroTimer) {
      fMacroTimer = new TTimer();
      fMacroTimer->Connect("Timeout()", "OzoneGUI", this, "CheckMacroOutput()");
    }
    fMacroTimer->Start(500);

    fMacroRunning = kTRUE;
    fRunMacroButton->SetEnabled(kFALSE);
    fCancelMacroButton->SetEnabled(kTRUE);
  }

  void CheckMacroOutput() {
    if (!fMacroRunning)
      return;

    // Read output from pipe
    if (fMacroPipeFd != -1) {
      char buffer[4096];
      ssize_t bytesRead;

      while ((bytesRead = read(fMacroPipeFd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';

        char *line = strtok(buffer, "\n");
        while (line != nullptr) {
          AppendMacroLog(line);
          line = strtok(nullptr, "\n");
        }
        gSystem->ProcessEvents();
      }
    }

    // Check if process completed
    int status;
    pid_t result = waitpid(fMacroPid, &status, WNOHANG);

    if (result == fMacroPid) {
      auto endTime = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(
          endTime - fMacroStartTime);
      std::string durationStr = FormatDuration(duration);

      int exitCode = 0;
      if (WIFEXITED(status)) {
        exitCode = WEXITSTATUS(status);
        AppendMacroLog(Form("Macro completed with exit code %d (Duration: %s)",
                            exitCode, durationStr.c_str()));

        if (exitCode == 0) {
          AppendMacroLog(
              "You can use the Graph Viewer tab to browse the results.");
        }
      } else if (WIFSIGNALED(status)) {
        exitCode = WTERMSIG(status);
        AppendMacroLog(Form("Macro terminated by signal %d (Duration: %s)",
                            exitCode, durationStr.c_str()));
      }

      // Cleanup
      fMacroTimer->Stop();
      if (fMacroPipeFd != -1) {
        close(fMacroPipeFd);
        fMacroPipeFd = -1;
      }
      fMacroPid = -1;
      fMacroRunning = kFALSE;
      fRunMacroButton->SetEnabled(kTRUE);
      fCancelMacroButton->SetEnabled(kFALSE);

      // Show notification
      ShowCompletionNotification(exitCode, durationStr);

    } else if (result == -1) {
      AppendMacroLog("Error: lost connection to macro process.");
      fMacroTimer->Stop();
      if (fMacroPipeFd != -1) {
        close(fMacroPipeFd);
        fMacroPipeFd = -1;
      }
      fMacroPid = -1;
      fMacroRunning = kFALSE;
      fRunMacroButton->SetEnabled(kTRUE);
      fCancelMacroButton->SetEnabled(kFALSE);
    }
  }

  void CancelMacro() {
    if (!fMacroRunning || fMacroPid == -1) {
      AppendMacroLog("No macro process to cancel.");
      return;
    }

    AppendMacroLog(Form("Cancelling macro process (PID: %d)...", fMacroPid));

    if (kill(fMacroPid, SIGTERM) == 0) {
      AppendMacroLog("Sent SIGTERM, waiting for process to terminate...");

      for (int i = 0; i < 30; i++) {
        int status;
        pid_t result = waitpid(fMacroPid, &status, WNOHANG);
        if (result == fMacroPid) {
          AppendMacroLog("Macro process terminated gracefully.");
          fMacroTimer->Stop();
          if (fMacroPipeFd != -1) {
            close(fMacroPipeFd);
            fMacroPipeFd = -1;
          }
          fMacroPid = -1;
          fMacroRunning = kFALSE;
          fRunMacroButton->SetEnabled(kTRUE);
          fCancelMacroButton->SetEnabled(kFALSE);
          return;
        }
        gSystem->Sleep(100);
        gSystem->ProcessEvents();
      }

      AppendMacroLog("Process didn't terminate gracefully, using SIGKILL...");
      if (kill(fMacroPid, SIGKILL) == 0) {
        waitpid(fMacroPid, nullptr, 0);
        AppendMacroLog("Macro process killed successfully.");
      } else {
        AppendMacroLog("Error: failed to kill macro process.");
      }
    } else {
      AppendMacroLog("Error: failed to send termination signal to macro.");
    }

    fMacroTimer->Stop();
    if (fMacroPipeFd != -1) {
      close(fMacroPipeFd);
      fMacroPipeFd = -1;
    }
    fMacroPid = -1;
    fMacroRunning = kFALSE;
    fRunMacroButton->SetEnabled(kTRUE);
    fCancelMacroButton->SetEnabled(kFALSE);
  }
  ClassDef(OzoneGUI, 0);
};

void ozone_gui() { new OzoneGUI(gClient->GetRoot(), 800, 900); }
