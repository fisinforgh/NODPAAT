#include "viewO3Global.cpp" // <--- Add this line
#include <TApplication.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TGButton.h>
#include <TGComboBox.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGFont.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGListBox.h>
#include <TGMsgBox.h>
#include <TGNumberEntry.h>
#include <TGPicture.h>
#include <TGTab.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH1.h>
#include <TH2.h>
#include <TImage.h>
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
  Bool_t fCompletionHandled;
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
  TGTextButton *fSaveGraphButton;
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
  TGComboBox *fMacroSelector; // Selector for choosing macro
  TGCompositeFrame *fParam1Frame, *fParam2Frame, *fParam3Frame, *fParam4Frame,
      *fParam5Frame, *fParam6Frame, *fParam7Frame, *fParam8Frame, *fParam9Frame,
      *fParam10Frame, *fParam11Frame;
  TGLabel *fParam1Label, *fParam2Label, *fParam3Label, *fParam4Label,
      *fParam5Label, *fParam6Label, *fParam7Label, *fParam8Label, *fParam9Label,
      *fParam10Label, *fParam11Label; // Labels for parameters
  TGNumberEntry *fMacroParam1, *fMacroParam3, *fMacroParam4, *fMacroParam5,
      *fMacroParam6, *fMacroParam7, *fMacroParam8, *fMacroParam9,
      *fMacroParam10, *fMacroParam11;
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
        fProcessRunning(kFALSE), fCompletionHandled(kFALSE), fSilentMode(kFALSE),
        fNotificationsEnabled(kTRUE), fCurrentFile(nullptr), fMacroPid(-1),
        fMacroTimer(nullptr), fMacroRunning(kFALSE), fMacroPipeFd(-1),
        fParam1Frame(nullptr), fParam2Frame(nullptr), fParam3Frame(nullptr),
        fParam4Frame(nullptr), fParam5Frame(nullptr), fParam6Frame(nullptr),
        fParam7Frame(nullptr), fParam8Frame(nullptr), fParam9Frame(nullptr),
        fParam10Frame(nullptr), fParam11Frame(nullptr) {

    // Initialize pipe
    fPipeFd[0] = fPipeFd[1] = -1;

    // Create main tab widget
    fMainTabs = new TGTab(this, 600, 800);

    // ======== WELCOME TAB ========
    TGCompositeFrame *welcomeTab = fMainTabs->AddTab("Welcome");
    CreateWelcomeInterface(welcomeTab);

    // ======== PROCESSOR TAB ========
    TGCompositeFrame *processorTab = fMainTabs->AddTab("Processor");

    // Move all existing processor UI to this tab
    CreateProcessorInterface(processorTab);

    // ======== MACRO RUNNER TAB ========
    TGCompositeFrame *macroTab = fMainTabs->AddTab("Macro Runner");
    CreateMacroInterface(macroTab);

    // ======== GRAPH VIEWER TAB ========
    TGCompositeFrame *graphTab = fMainTabs->AddTab("Graph Viewer");
    CreateGraphInterface(graphTab);

    // ============================================================================
    // View O3 Global Tab Integration
    // ============================================================================
    {
      TGCompositeFrame *tabViewO3 = fMainTabs->AddTab("View O3 Global");

      TGHorizontalFrame *viewO3Frame =
          new TGHorizontalFrame(tabViewO3, 800, 600);
      tabViewO3->AddFrame(viewO3Frame,
                          new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

      TGMainFrame *viewO3GlobalWin = CreateViewO3GlobalGUI(viewO3Frame);
      viewO3Frame->AddFrame(viewO3GlobalWin,
                            new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

      viewO3GlobalWin->MapSubwindows();
      viewO3GlobalWin->MapWindow();
    }
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

    // Get screen dimensions and resize to fullscreen
    UInt_t screenWidth = gClient->GetDisplayWidth();
    UInt_t screenHeight = gClient->GetDisplayHeight();
    Resize(screenWidth, screenHeight);
    Move(0, 0);

    MapWindow();

    UpdateLatLabels();

    // Initialize graph viewer with current directory
    fGraphPathEntry->SetText(gSystem->WorkingDirectory());
    RefreshFolderList();

    // Initialize macro parameter visibility
    UpdateMacroParameters();
  }

  void CreateWelcomeInterface(TGCompositeFrame *parent) {
    // Main container with vertical layout - everything centered
    TGVerticalFrame *mainFrame = new TGVerticalFrame(parent, 600, 700);

    // -------- Title Section (First) --------
    TGVerticalFrame *titleFrame = new TGVerticalFrame(mainFrame);

    TGLabel *programTitle =
        new TGLabel(titleFrame, "Ozone Processor - Graphical Interface");
    TGFont *progFont =
        gClient->GetFont("-*-helvetica-bold-r-*-*-20-*-*-*-*-*-*-*");
    if (progFont) {
      programTitle->SetTextFont(progFont);
    }
    titleFrame->AddFrame(programTitle,
                         new TGLayoutHints(kLHintsCenterX, 10, 10, 20, 5));

    TGLabel *subtitle =
        new TGLabel(titleFrame, "NASA Ozone Data Processing and Analysis Tool");
    TGFont *subFont =
        gClient->GetFont("-*-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*");
    if (subFont) {
      subtitle->SetTextFont(subFont);
    }
    titleFrame->AddFrame(subtitle,
                         new TGLayoutHints(kLHintsCenterX, 10, 10, 5, 15));

    mainFrame->AddFrame(
        titleFrame,
        new TGLayoutHints(kLHintsCenterX | kLHintsTop, 10, 10, 10, 5));

    // -------- Logo Section (Below Title) --------
    TGVerticalFrame *logoFrame = new TGVerticalFrame(mainFrame, 150, 150);

    // Try to load the university logo
    TString logoPath = "logo_ud.png";
    const TGPicture *logoPic = nullptr;

    if (!gSystem->AccessPathName(logoPath)) {
      logoPic = gClient->GetPicture(logoPath);
    }

    if (logoPic) {
      // Scale logo to max 150x150 pixels
      UInt_t maxSize = 150;
      UInt_t logoWidth = logoPic->GetWidth();
      UInt_t logoHeight = logoPic->GetHeight();

      // Calculate scaled dimensions maintaining aspect ratio
      if (logoWidth > maxSize || logoHeight > maxSize) {
        Float_t scale = TMath::Min((Float_t)maxSize / logoWidth,
                                   (Float_t)maxSize / logoHeight);
        logoWidth = (UInt_t)(logoWidth * scale);
        logoHeight = (UInt_t)(logoHeight * scale);
      }

      TGIcon *logoIcon = new TGIcon(logoFrame, logoPic, logoWidth, logoHeight);
      logoFrame->AddFrame(
          logoIcon,
          new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 5, 5));
    } else {
      // If logo not found, display university name
      TGLabel *uniLabel = new TGLabel(
          logoFrame, "Universidad Distrital\nFrancisco Jose de Caldas");
      uniLabel->SetTextJustify(kTextCenterX);
      TGFont *uniFont =
          gClient->GetFont("-*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*");
      if (uniFont) {
        uniLabel->SetTextFont(uniFont);
      }
      logoFrame->AddFrame(
          uniLabel,
          new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 20, 20));
    }

    mainFrame->AddFrame(
        logoFrame,
        new TGLayoutHints(kLHintsCenterX | kLHintsTop, 10, 10, 5, 10));

    // -------- Program Description --------
    TGGroupFrame *descFrame = new TGGroupFrame(mainFrame, "Program Overview");
    TGTextView *descText = new TGTextView(descFrame, 700, 500);

    descText->AddLine("OVERVIEW");
    descText->AddLine("--------");
    descText->AddLine("This application provides a comprehensive interface for "
                      "processing and analyzing");
    descText->AddLine("NASA ozone data from satellite measurements.");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("MAIN FEATURES");
    descText->AddLine("=============");
    descText->AddLine("");
    descText->AddLine("[ 1 ] PROCESSOR TAB");
    descText->AddLine("      Process ozone data using optimized algorithms");
    descText->AddLine("");
    descText->AddLine("      Execution Modes:");
    descText->AddLine(
        "      • pgrid     - Process data for a latitude range grid");
    descText->AddLine(
        "      • location  - Process data for a specific location");
    descText->AddLine("");
    descText->AddLine("      Features:");
    descText->AddLine(
        "      • Configurable parameters (grid size, events, threads)");
    descText->AddLine("      • Real-time process monitoring with log output");
    descText->AddLine("      • Silent mode for improved performance");
    descText->AddLine("      • Desktop notifications on completion");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("[ 2 ] GRAPH VIEWER TAB");
    descText->AddLine("      Browse and visualize processed ozone data");
    descText->AddLine("");
    descText->AddLine("      Features:");
    descText->AddLine(
        "      • Navigate through skim folders organized by coordinates");
    descText->AddLine("      • Automatic graph loading and display");
    descText->AddLine("      • Filter folders by name for quick access");
    descText->AddLine(
        "      • Display multiple graphs and histograms simultaneously");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("[ 3 ] MACRO RUNNER TAB");
    descText->AddLine("      Execute ROOT macros for advanced analysis");
    descText->AddLine("");
    descText->AddLine("      Available Macros:");
    descText->AddLine("      • linearRelStudyO3vsSn.C    - Linear relation "
                      "studies between O3 and Sn");
    descText->AddLine(
        "      • macroO3teoGlobalHttp.C    - Global O3 theoretical analysis");
    descText->AddLine("");
    descText->AddLine("      Features:");
    descText->AddLine("      • Dynamic parameter configuration");
    descText->AddLine("      • Real-time macro execution monitoring");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("[ 4 ] VIEW O3 GLOBAL TAB");
    descText->AddLine("      Visualize global ozone data from the ana schema");
    descText->AddLine("");
    descText->AddLine("      Features:");
    descText->AddLine(
        "      • Download historical data (O3 history and O3 theoretical)");
    descText->AddLine(
        "      • Interactive graphs with customizable display options");
    descText->AddLine("      • Global ozone trend analysis");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("RECOMMENDED WORKFLOW");
    descText->AddLine("====================");
    descText->AddLine("");
    descText->AddLine("  1. Process raw NASA data using the Processor tab");
    descText->AddLine("  2. View results in the Graph Viewer tab");
    descText->AddLine("  3. Run advanced analysis using the Macro Runner");
    descText->AddLine("  4. Explore global trends in View O3 Global tab");
    descText->AddLine("");
    descText->AddLine("");
    descText->AddLine("--------------------------------------------------------"
                      "---------------------");
    descText->AddLine(
        "Developed at Universidad Distrital Francisco Jose de Caldas");
    descText->AddLine(
        "For research on atmospheric ozone measurements and analysis");
    descText->AddLine("--------------------------------------------------------"
                      "---------------------");

    descFrame->AddFrame(
        descText,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10));
    mainFrame->AddFrame(
        descFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 20, 20, 10, 20));

    parent->AddFrame(
        mainFrame,
        new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
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
    fModeSelector->Select(2);
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
    fDataPathEntry->SetText("Please select your data folder");
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

    fSaveGraphButton = new TGTextButton(pathHFrame, "Save Graph");
    fSaveGraphButton->Resize(100, 28);
    fSaveGraphButton->Connect("Clicked()", "OzoneGUI", this,
                              "SaveGraphFromViewer()");
    pathHFrame->AddFrame(fSaveGraphButton,
                         new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

    pathFrame->AddFrame(pathHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(pathFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 5));

    // -------- Folder Filter Section --------
    TGGroupFrame *filterFrame = new TGGroupFrame(parent, "Folder Filter");
    TGHorizontalFrame *filterHFrame = new TGHorizontalFrame(filterFrame);

    filterHFrame->AddFrame(
        new TGLabel(filterHFrame, "Filter:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fFolderFilterEntry = new TGTextEntry(filterHFrame, new TGTextBuffer(100));
    fFolderFilterEntry->SetText("");
    fFolderFilterEntry->Resize(300, 28);
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
    const int kWidgetHeight = 28;

    // -------- Macro Selection --------
    TGGroupFrame *macroFrame = new TGGroupFrame(parent, "Macro Selection");
    TGHorizontalFrame *macroHFrame = new TGHorizontalFrame(macroFrame);

    // Label frame for "Macro:"
    TGHorizontalFrame *macroLabelFrame = new TGHorizontalFrame(macroHFrame);
    macroLabelFrame->AddFrame(
        new TGLabel(macroLabelFrame, "Macro:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 10,
                          5, 5));
    macroLabelFrame->Resize(250, kWidgetHeight); // Wider label space
    macroHFrame->AddFrame(
        macroLabelFrame,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fMacroSelector = new TGComboBox(macroHFrame);
    fMacroSelector->AddEntry("linearRelStudyO3vsSn.C", 1);
    fMacroSelector->AddEntry("macroO3teoGlobalHttp.C", 2);
    fMacroSelector->Select(1);
    fMacroSelector->Resize(200, kWidgetHeight);
    fMacroSelector->Connect("Selected(Int_t)", "OzoneGUI", this,
                            "UpdateMacroParameters()");
    macroHFrame->AddFrame(
        fMacroSelector,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 10, 5, 5, 5));

    macroFrame->AddFrame(macroHFrame,
                         new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    parent->AddFrame(macroFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 5));

    // -------- Macro Parameters Section --------
    TGGroupFrame *paramFrame = new TGGroupFrame(parent, "Macro Parameters");
    TGCompositeFrame *paramMatrix =
        new TGCompositeFrame(paramFrame, 1, 1, kHorizontalFrame);
    paramMatrix->SetLayoutManager(new TGMatrixLayout(
        paramMatrix, 0, 2, 15, 10)); // Increased horizontal spacing

    // Parameter 1
    fParam1Label = new TGLabel(paramMatrix, "Minimum number of events:");
    fParam1Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam1Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam1 =
        new TGNumberEntry(paramMatrix, 4.60, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -90.0, 90.0);
    fMacroParam1->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam1,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 2
    fParam2Label = new TGLabel(paramMatrix, "Location prefix:");
    fParam2Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam2Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam2 = new TGTextEntry(paramMatrix, new TGTextBuffer(50));
    fMacroParam2->SetText("BOG");
    fMacroParam2->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam2,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 3
    fParam3Label = new TGLabel(paramMatrix, "Alpha(scaling factor)");
    fParam3Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam3Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam3 =
        new TGNumberEntry(paramMatrix, 1, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -180.0, 180.0);
    fMacroParam3->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam3,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 4
    fParam4Label = new TGLabel(paramMatrix, "Solar cycle period");
    fParam4Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam4Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam4 =
        new TGNumberEntry(paramMatrix, 11, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fMacroParam4->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam4,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 5
    fParam5Label = new TGLabel(paramMatrix, "Linear fit option");
    fParam5Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam5Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam5 =
        new TGNumberEntry(paramMatrix, 1, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fMacroParam5->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam5,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 6
    fParam6Label = new TGLabel(paramMatrix, "Minimum ozone plot range");
    fParam6Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam6Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam6 =
        new TGNumberEntry(paramMatrix, 195, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 1000);
    fMacroParam6->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam6,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 7
    fParam7Label = new TGLabel(paramMatrix, "Maximum ozone plot range");
    fParam7Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam7Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam7 =
        new TGNumberEntry(paramMatrix, 345, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 1000);
    fMacroParam7->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam7,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 8
    fParam8Label = new TGLabel(paramMatrix, "Minimum form factor range");
    fParam8Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam8Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam8 =
        new TGNumberEntry(paramMatrix, 0.91, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fMacroParam8->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam8,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 9
    fParam9Label = new TGLabel(paramMatrix, "Maximum form factor range");
    fParam9Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam9Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam9 =
        new TGNumberEntry(paramMatrix, 1.27, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fMacroParam9->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam9,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 10
    fParam10Label = new TGLabel(paramMatrix, "Minimum error plot range");
    fParam10Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam10Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam10 =
        new TGNumberEntry(paramMatrix, -0.03, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fMacroParam10->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam10,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    // Parameter 11
    fParam11Label = new TGLabel(paramMatrix, "Maximum error plot range");
    fParam11Label->SetTextJustify(kTextLeft);
    paramMatrix->AddFrame(
        fParam11Label,
        new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX, 5, 15,
                          5, 5));
    fMacroParam11 =
        new TGNumberEntry(paramMatrix, -0.03, 8, -1, TGNumberFormat::kNESReal,
                          TGNumberFormat::kNEAAnyNumber,
                          TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fMacroParam11->Resize(80, kWidgetHeight); // Smaller input
    paramMatrix->AddFrame(
        fMacroParam11,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 5, 5));

    paramFrame->AddFrame(paramMatrix, new TGLayoutHints(kLHintsExpandX));
    parent->AddFrame(paramFrame,
                     new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 5));

    // -------- Run and Cancel Buttons --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(parent);

    fRunMacroButton = new TGTextButton(btnFrame, "&Run Macro");
    fRunMacroButton->Resize(120, kWidgetHeight);
    fRunMacroButton->Connect("Clicked()", "OzoneGUI", this, "RunMacro()");
    btnFrame->AddFrame(fRunMacroButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    fCancelMacroButton = new TGTextButton(btnFrame, "&Cancel");
    fCancelMacroButton->Resize(80, kWidgetHeight);
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

    // Initialize parameter visibility and labels
    UpdateMacroParameters();
  }

  void UpdateMacroParameters() {
    TGTextLBEntry *entry = (TGTextLBEntry *)fMacroSelector->GetSelectedEntry();
    TString macroName = entry ? entry->GetTitle() : "";

    // Helper function to show/hide parameter
    auto setParamVisibility = [&](TGLabel *label, TGFrame *input, Bool_t show,
                                  const char *labelText) {
      if (show) {
        if (label) {
          label->SetText(labelText);
          label->MapWindow();
        }
        if (input)
          input->MapWindow();
      } else {
        if (label)
          label->UnmapWindow();
        if (input)
          input->UnmapWindow();
      }
    };

    if (macroName == "linearRelStudyO3vsSn.C") {
      fMacroParam1->SetFormat(TGNumberFormat::kNESInteger,
                              TGNumberFormat::kNEANonNegative);
      fMacroParam1->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 100000);
      fMacroParam1->SetIntNumber(7); // default integer
      setParamVisibility(fParam1Label, fMacroParam1, kTRUE,
                         "Minimum number of events");
      setParamVisibility(fParam2Label, fMacroParam2, kTRUE, "Location prefix");
      setParamVisibility(fParam3Label, fMacroParam3, kTRUE,
                         "Alpha(scaling factor)");
      setParamVisibility(fParam4Label, fMacroParam4, kFALSE, "");
      setParamVisibility(fParam5Label, fMacroParam5, kFALSE, "");
      setParamVisibility(fParam6Label, fMacroParam6, kFALSE, "");
      setParamVisibility(fParam7Label, fMacroParam7, kFALSE, "");
      setParamVisibility(fParam8Label, fMacroParam8, kFALSE, "");
      setParamVisibility(fParam9Label, fMacroParam9, kFALSE, "");
      setParamVisibility(fParam10Label, fMacroParam10, kFALSE, "");
      setParamVisibility(fParam11Label, fMacroParam11, kFALSE, "");
    } else if (macroName == "macroO3teoGlobalHttp.C") {
      fMacroParam1->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam1->SetLimits(TGNumberFormat::kNELLimitMinMax, -90.0, 90.0);
      fMacroParam1->SetNumber(4.60);
      // Location prefix (string)
      fMacroParam2->SetText("BOG");
      // Longitude
      fMacroParam3->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam3->SetLimits(TGNumberFormat::kNELLimitMinMax, -180.0, 180.0);
      fMacroParam3->SetNumber(-74.08);
      // Solar cycle period (int)
      fMacroParam4->SetFormat(TGNumberFormat::kNESInteger,
                              TGNumberFormat::kNEAPositive);
      fMacroParam4->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 20);
      fMacroParam4->SetNumber(11);
      // Linear fit option (int)
      fMacroParam5->SetFormat(TGNumberFormat::kNESInteger,
                              TGNumberFormat::kNEAPositive);
      fMacroParam5->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 1);
      fMacroParam5->SetNumber(1);
      // Minimum ozone plot range (float)
      fMacroParam6->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam6->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 600.0);
      fMacroParam6->SetNumber(195);
      // Maximum ozone plot range (float)
      fMacroParam7->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam7->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 600.0);
      fMacroParam7->SetNumber(345);
      // Minimum form factor range (float)
      fMacroParam8->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam8->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 10.0);
      fMacroParam8->SetNumber(0.91);
      // Maximum form factor range (float)
      fMacroParam9->SetFormat(TGNumberFormat::kNESReal,
                              TGNumberFormat::kNEAAnyNumber);
      fMacroParam9->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 10.0);
      fMacroParam9->SetNumber(1.27);
      // Minimum error plot range (float)
      fMacroParam10->SetFormat(TGNumberFormat::kNESReal,
                               TGNumberFormat::kNEAAnyNumber);
      fMacroParam10->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 100.0);
      fMacroParam10->SetNumber(-0.03);
      // Maximum error plot range (float)
      fMacroParam11->SetFormat(TGNumberFormat::kNESReal,
                               TGNumberFormat::kNEAAnyNumber);
      fMacroParam11->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.0, 100.0);
      fMacroParam11->SetNumber(0.32);
      setParamVisibility(fParam1Label, fMacroParam1, kTRUE, "Latitude");
      setParamVisibility(fParam2Label, fMacroParam2, kTRUE, "Location prefix");
      setParamVisibility(fParam3Label, fMacroParam3, kTRUE, "Longitude");
      setParamVisibility(fParam4Label, fMacroParam4, kTRUE,
                         "Solar cycle period");
      setParamVisibility(fParam5Label, fMacroParam5, kTRUE,
                         "Linear fit option");
      setParamVisibility(fParam6Label, fMacroParam6, kTRUE,
                         "Minimum ozone plot range");
      setParamVisibility(fParam7Label, fMacroParam7, kTRUE,
                         "Maximum ozone plot range");
      setParamVisibility(fParam8Label, fMacroParam8, kTRUE,
                         "Minimum form factor range");
      setParamVisibility(fParam9Label, fMacroParam9, kTRUE,
                         "Maximum form factor range");
      setParamVisibility(fParam10Label, fMacroParam10, kTRUE,
                         "Minimum error plot range");
      setParamVisibility(fParam11Label, fMacroParam11, kTRUE,
                         "Maximum error plot range");
    } else {
      // Hide all parameters
      setParamVisibility(fParam1Label, fMacroParam1, kFALSE, "");
      setParamVisibility(fParam2Label, fMacroParam2, kFALSE, "");
      setParamVisibility(fParam3Label, fMacroParam3, kFALSE, "");
      setParamVisibility(fParam4Label, fMacroParam4, kFALSE, "");
      setParamVisibility(fParam5Label, fMacroParam5, kFALSE, "");
      setParamVisibility(fParam6Label, fMacroParam6, kFALSE, "");
      setParamVisibility(fParam7Label, fMacroParam7, kFALSE, "");
      setParamVisibility(fParam8Label, fMacroParam8, kFALSE, "");
      setParamVisibility(fParam9Label, fMacroParam9, kFALSE, "");
      setParamVisibility(fParam10Label, fMacroParam10, kFALSE, "");
      setParamVisibility(fParam11Label, fMacroParam11, kFALSE, "");
    }

    // Force layout update
    Layout();
    gClient->NeedRedraw(this);
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

    // Get list of all objects in file
    TList *keyList = fCurrentFile->GetListOfKeys();
    TIter next(keyList);
    TKey *key;

    std::vector<TKey *> drawableKeys;
    std::vector<TKey *> canvasKeys;

    // Collect all drawable objects and canvases
    while ((key = (TKey *)next())) {
      TString className = key->GetClassName();
      TString keyName = key->GetName();

      std::cout << "Found: " << keyName << " (type: " << className << ")"
                << std::endl;

      if (className == "TCanvas") {
        canvasKeys.push_back(key);
      } else if (className.Contains("TH1") || className.Contains("TH2") ||
                 className.Contains("TGraph") ||
                 className.Contains("TProfile")) {
        drawableKeys.push_back(key);
      }
    }

    // If we have canvases, display them instead of individual histograms
    if (!canvasKeys.empty()) {
      std::cout << "Found " << canvasKeys.size()
                << " canvas(es), displaying canvases" << std::endl;

      int nCanvases = canvasKeys.size();
      int nCols = (nCanvases == 1) ? 1 : 2;
      int nRows = (nCanvases + nCols - 1) / nCols;
      canvas->Divide(nCols, nRows);

      for (size_t i = 0; i < canvasKeys.size(); i++) {
        TKey *canvasKey = canvasKeys[i];
        TCanvas *storedCanvas = (TCanvas *)canvasKey->ReadObj();
        if (!storedCanvas)
          continue;

        std::cout << "Drawing canvas: " << canvasKey->GetName() << std::endl;

        canvas->cd(i + 1);
        storedCanvas->DrawClonePad();

        fCurrentObjects.push_back(storedCanvas);
      }

      fGraphInfoLabel->SetText(Form("File: %s - Displayed %d canvas(es)",
                                    gSystem->BaseName(filename.Data()),
                                    nCanvases));
    } else {
      // Fall back to individual histograms/graphs
      std::cout << "No canvases found, displaying individual objects"
                << std::endl;

      int nObjects = drawableKeys.size();
      int nCols = 2, nRows = 2;
      if (nObjects > 4) {
        nRows = (nObjects + 1) / 2;
        if (nRows > 3)
          nRows = 3;
      }
      canvas->Divide(nCols, nRows);

      int objectsDrawn = 0;
      for (size_t i = 0; i < drawableKeys.size() && i < (size_t)(nCols * nRows);
           i++) {
        TKey *drawKey = drawableKeys[i];
        TObject *obj = drawKey->ReadObj();
        if (!obj)
          continue;

        TObject *clonedObj = obj->Clone();
        fCurrentObjects.push_back(clonedObj);

        canvas->cd(i + 1);

        if (clonedObj->InheritsFrom("TH1")) {
          TH1 *hist = (TH1 *)clonedObj;
          hist->SetDirectory(0);
          hist->Draw();
          objectsDrawn++;
        } else if (clonedObj->InheritsFrom("TH2")) {
          TH2 *hist2d = (TH2 *)clonedObj;
          hist2d->SetDirectory(0);
          hist2d->Draw("COLZ");
          objectsDrawn++;
        } else if (clonedObj->InheritsFrom("TGraph")) {
          TGraph *graph = (TGraph *)clonedObj;
          graph->Draw("ALP");
          objectsDrawn++;
        }
      }

      fGraphInfoLabel->SetText(Form("File: %s - Displayed %d objects",
                                    gSystem->BaseName(filename.Data()),
                                    objectsDrawn));
    }

    canvas->Update();
    canvas->Modified();
  }

  void SaveGraphFromViewer() {
    TCanvas *canvas = fGraphCanvas->GetCanvas();

    if (!canvas) {
      new TGMsgBox(gClient->GetRoot(), this, "Error",
                   "No canvas to save!", kMBIconStop, kMBOk);
      return;
    }

    // Check if there's anything drawn on the canvas
    if (!canvas->GetListOfPrimitives() ||
        canvas->GetListOfPrimitives()->GetSize() == 0) {
      new TGMsgBox(gClient->GetRoot(), this, "Error",
                   "No graph loaded to save!", kMBIconStop, kMBOk);
      return;
    }

    // Get the current folder name from fGraphInfoLabel
    TString infoText = fGraphInfoLabel->GetText()->GetString();

    // Build filename from current selection
    TString baseFilename;

    // Try to extract folder name from the info label
    if (infoText.Contains("Selected:")) {
      // Extract the folder name between "Selected: " and the next space or parenthesis
      TString folderName = infoText;
      folderName.ReplaceAll("Selected: ", "");

      // Remove lat/lon info if present
      Ssiz_t parenPos = folderName.First('(');
      if (parenPos != kNPOS) {
        folderName = folderName(0, parenPos);
      }
      folderName = folderName.Strip(TString::kBoth);

      baseFilename = folderName;
    } else if (infoText.Contains("File:")) {
      // Extract filename
      TString fileName = infoText;
      fileName.ReplaceAll("File: ", "");

      // Get just the part before " - "
      Ssiz_t dashPos = fileName.First('-');
      if (dashPos != kNPOS) {
        fileName = fileName(0, dashPos);
      }
      fileName = fileName.Strip(TString::kBoth);

      // Remove .root extension
      fileName.ReplaceAll(".root", "");

      baseFilename = fileName;
    } else {
      // Default name
      baseFilename = "graph_viewer_export";
    }

    // Add timestamp to make filename unique
    TDatime now;
    baseFilename += Form("_%d%02d%02d_%02d%02d%02d",
                        now.GetYear(), now.GetMonth(), now.GetDay(),
                        now.GetHour(), now.GetMinute(), now.GetSecond());

    // Replace spaces and special characters with underscores
    baseFilename.ReplaceAll(" ", "_");
    baseFilename.ReplaceAll("/", "_");
    baseFilename.ReplaceAll("(", "");
    baseFilename.ReplaceAll(")", "");
    baseFilename.ReplaceAll("°", "deg");

    // Save in multiple formats
    TString pngFile = baseFilename + ".png";
    TString pdfFile = baseFilename + ".pdf";
    TString rootFile = baseFilename + ".root";

    canvas->SaveAs(pngFile.Data());
    canvas->SaveAs(pdfFile.Data());
    canvas->SaveAs(rootFile.Data());

    // Show success message
    TString message = Form("Graph saved successfully:\n\n%s\n%s\n%s",
                          pngFile.Data(), pdfFile.Data(), rootFile.Data());
    new TGMsgBox(gClient->GetRoot(), this, "Success",
                 message.Data(), kMBIconAsterisk, kMBOk);

    std::cout << "Graph saved to:" << std::endl;
    std::cout << "  " << pngFile.Data() << std::endl;
    std::cout << "  " << pdfFile.Data() << std::endl;
    std::cout << "  " << rootFile.Data() << std::endl;
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
      // Create a new process group so we can kill all child processes
      setpgid(0, 0);

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
      // Ensure timer is on and start it
      fOutputTimer->TurnOn();
      fOutputTimer->Start(500);
    } else {
      if (!fOutputTimer) {
        fOutputTimer = new TTimer();
        fOutputTimer->Connect("Timeout()", "OzoneGUI", this,
                              "CheckProcessOutput()");
      }
      // Ensure timer is on and start it
      fOutputTimer->TurnOn();
      fOutputTimer->Start(1000);
    }

    fProcessRunning = kTRUE;
    fCompletionHandled = kFALSE;
    fRunButton->SetEnabled(kFALSE);
    fCancelButton->SetEnabled(kTRUE);
  }

  void CheckProcessOutput() {
    if (!fProcessRunning || fCompletionHandled)
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
      // Guard against re-entry - this MUST be the first check
      if (fCompletionHandled)
        return;

      // Set flags immediately to prevent re-entry from queued timer events
      fCompletionHandled = kTRUE;
      fProcessRunning = kFALSE;

      // Stop and turn off timer completely to prevent any queued events
      if (fOutputTimer) {
        fOutputTimer->TurnOff();
        fOutputTimer->Stop();
      }

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

      // Cleanup pipes and process state
      if (fPipeFd[0] != -1) {
        close(fPipeFd[0]);
        fPipeFd[0] = -1;
      }
      if (fPipeFd[1] != -1) {
        close(fPipeFd[1]);
        fPipeFd[1] = -1;
      }
      fProcessPid = -1;

      fRunButton->SetEnabled(kTRUE);
      fCancelButton->SetEnabled(kFALSE);

      ShowCompletionNotification(exitCode, durationStr);
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

    // Stop the timer first to prevent CheckProcessOutput from being called during cancellation
    if (fOutputTimer) {
      fOutputTimer->Stop();
    }

    AppendLog(Form("Cancelling process group (PID: %d)...", fProcessPid));

    // Kill the entire process group (negative PID) to ensure child processes are also terminated
    if (kill(-fProcessPid, SIGTERM) == 0) {
      AppendLog("Sent SIGTERM to process group, waiting for processes to terminate...");

      for (int i = 0; i < 30; i++) {
        int status;
        pid_t result = waitpid(fProcessPid, &status, WNOHANG);
        if (result == fProcessPid) {
          AppendLog("Process group terminated gracefully.");
          CleanupProcess();
          fRunButton->SetEnabled(kTRUE);
          fCancelButton->SetEnabled(kFALSE);
          return;
        }
        gSystem->Sleep(100);
        gSystem->ProcessEvents();
      }

      AppendLog("Process group didn't terminate gracefully, using SIGKILL...");
      if (kill(-fProcessPid, SIGKILL) == 0) {
        waitpid(fProcessPid, nullptr, 0);
        AppendLog("Process group killed successfully.");
      } else {
        AppendLog("Error: failed to kill process group.");
      }
    } else {
      AppendLog("Error: failed to send termination signal to process group.");
    }

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

    // Get selected macro
    TGTextLBEntry *entry = (TGTextLBEntry *)fMacroSelector->GetSelectedEntry();
    TString macroName = entry ? entry->GetTitle() : "";
    if (macroName.IsNull()) {
      AppendMacroLog("Error: No macro selected.");
      return;
    }

    // Get the directory where the executable (and macros) are located
    TString exeDir = gSystem->DirName(fExePath);
    if (exeDir.IsNull() || fExePath.IsNull()) {
      exeDir = gSystem->WorkingDirectory();
    }

    // Check if macro file exists in the executable directory
    TString macroFullPath = Form("%s/%s", exeDir.Data(), macroName.Data());
    if (gSystem->AccessPathName(macroFullPath)) {
      AppendMacroLog(
          Form("Error: %s not found in directory: %s", macroName.Data(), exeDir.Data()));
      return;
    }

    // Build ROOT command based on selected macro with full path
    TString rootCmd;
    if (macroName == "linearRelStudyO3vsSn.C") {
      // Get parameters for linearRelStudyO3vsSn
      int param1 = (int)fMacroParam1->GetNumber();
      TString param2 = fMacroParam2->GetText();
      double param3 = fMacroParam3->GetNumber();
      TString macroPath = Form("%s/%s", exeDir.Data(), macroName.Data());
      rootCmd = Form("root -l -b -q '%s(%d,\"%s\",%g)'",
                     macroPath.Data(), param1, param2.Data(), param3);
    } else if (macroName == "macroO3teoGlobalHttp.C") {
      // Get parameters for macroO3teoGlobalHttp
      double lat = fMacroParam1->GetNumber();
      double lon = fMacroParam3->GetNumber();
      TString loc = fMacroParam2->GetText();
      int param4 = (int)fMacroParam4->GetNumber();
      int param5 = (int)fMacroParam5->GetNumber();
      int param6 = (int)fMacroParam6->GetNumber();
      int param7 = (int)fMacroParam7->GetNumber();
      double param8 = fMacroParam8->GetNumber();
      double param9 = fMacroParam9->GetNumber();
      double param10 = fMacroParam10->GetNumber();
      double param11 = fMacroParam11->GetNumber();
      TString macroPath = Form("%s/%s", exeDir.Data(), macroName.Data());
      rootCmd =
          Form("root -l -b -q "
               "'%s(%g,%g,\"%s\",%d,%d,%d,%d,%g,%g,%g,%g)'",
               macroPath.Data(), lat, lon, loc.Data(), param4, param5, param6, param7, param8,
               param9, param10, param11);
    } else {
      AppendMacroLog("Error: Unknown macro selected.");
      return;
    }

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

      // Change to executable directory so macros can find their data files
      if (chdir(exeDir.Data()) != 0) {
        std::cerr << "Warning: Could not change to directory: " << exeDir.Data() << std::endl;
      }

      execl("/bin/sh", "sh", "-c", rootCmd.Data(), (char *)nullptr);
      exit(1);
    }

    // Parent process
    close(pipeFd[1]);

    // Set pipe to non-blocking
    int flags = fcntl(pipeFd[0], F_GETFL, 0);
    fcntl(pipeFd[0], F_SETFL, flags | O_NONBLOCK);

    // Store pipe fd
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
