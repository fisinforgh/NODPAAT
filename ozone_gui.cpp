#include <TApplication.h>
#include <TGButton.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TSystem.h>
#include <TTimer.h>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

class OzoneGUI : public TGMainFrame {
private:
  TGComboBox *fModeSelector;
  TGTextEntry *fDataPathEntry;
  TGTextEntry *fLocationEntry;
  TGDoubleHSlider *fLatSlider;
  TGLabel *fLatMinLabel, *fLatMaxLabel;
  TGNumberEntry *fParam1Entry, *fParam2Entry, *fParam3Entry;
  TGTextView *fLogView;
  TString fExePath; // full path to executable

  // Process control
  TGTextButton *fRunButton, *fCancelButton;
  pid_t fProcessPid;
  int fPipeFd[2];       // pipe for reading process output
  TTimer *fOutputTimer; // timer to check for output
  Bool_t fProcessRunning;

public:
  OzoneGUI(const TGWindow *p, UInt_t w, UInt_t h)
      : TGMainFrame(p, w, h), fProcessPid(-1), fOutputTimer(nullptr),
        fProcessRunning(kFALSE) {

    // Initialize pipe
    fPipeFd[0] = fPipeFd[1] = -1;

    // -------- Mode Section --------
    TGGroupFrame *modeFrame = new TGGroupFrame(this, "Execution Mode");
    TGHorizontalFrame *modeHFrame = new TGHorizontalFrame(modeFrame);
    modeHFrame->AddFrame(
        new TGLabel(modeHFrame, "Mode:"),
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

    fModeSelector = new TGComboBox(modeHFrame);
    fModeSelector->AddEntry("pgrid", 1);
    fModeSelector->AddEntry("location", 2);
    fModeSelector->Select(1);       // default pgrid
    fModeSelector->Resize(200, 28); // normalized height
    modeHFrame->AddFrame(
        fModeSelector,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    modeFrame->AddFrame(modeHFrame,
                        new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    AddFrame(modeFrame,
             new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 10, 10, 5));

    TGGroupFrame *locFrame =
        new TGGroupFrame(this, "Location (only for 'location' mode)");
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
    AddFrame(locFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));
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

    // Use a matrix layout: 3 rows x 2 columns
    TGCompositeFrame *paramMatrix =
        new TGCompositeFrame(paramFrame, 1, 1, kHorizontalFrame);
    paramMatrix->SetLayoutManager(new TGMatrixLayout(paramMatrix, 0, 2, 10, 5));

    // Grid row
    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Grid:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParam1Entry =
        new TGNumberEntry(paramMatrix, 10, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam1Entry->Resize(80, 28);
    paramMatrix->AddFrame(
        fParam1Entry,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    // Events row
    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Events:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParam2Entry =
        new TGNumberEntry(paramMatrix, 7, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam2Entry->Resize(80, 28);
    paramMatrix->AddFrame(
        fParam2Entry,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    // Threads row
    paramMatrix->AddFrame(
        new TGLabel(paramMatrix, "Threads:"),
        new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));
    fParam3Entry =
        new TGNumberEntry(paramMatrix, 4, 6, -1, TGNumberFormat::kNESInteger,
                          TGNumberFormat::kNEANonNegative,
                          TGNumberFormat::kNELLimitMinMax, 0, 100);
    fParam3Entry->Resize(80, 28);
    paramMatrix->AddFrame(
        fParam3Entry,
        new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    paramFrame->AddFrame(paramMatrix, new TGLayoutHints(kLHintsExpandX));
    AddFrame(paramFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 5, 5));

    // -------- Run and Cancel Buttons --------
    TGHorizontalFrame *btnFrame = new TGHorizontalFrame(this);

    fRunButton = new TGTextButton(btnFrame, "&Run Processor");
    fRunButton->Resize(120, 28);
    fRunButton->Connect("Clicked()", "OzoneGUI", this, "RunProcessor()");
    btnFrame->AddFrame(fRunButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

    fCancelButton = new TGTextButton(btnFrame, "&Cancel");
    fCancelButton->Resize(80, 28);
    fCancelButton->Connect("Clicked()", "OzoneGUI", this, "CancelProcessor()");
    fCancelButton->SetEnabled(kFALSE); // disabled initially
    btnFrame->AddFrame(fCancelButton,
                       new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 10));

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

  virtual ~OzoneGUI() {
    if (fOutputTimer) {
      fOutputTimer->TurnOff();
      delete fOutputTimer;
    }
    CleanupProcess();
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

    // Create pipe for reading output
    if (pipe(fPipeFd) == -1) {
      AppendLog("Error: failed to create pipe.");
      return;
    }

    // Save current working directory
    TString oldDir = gSystem->WorkingDirectory();
    TString exeDir = gSystem->DirName(fExePath);

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
    if (mode == "location") {
      TString locCode = fLocationEntry->GetText();
      // Example: ./optimized_ozone_processor location BOG /path 4.36 -74.04 6
      cmd = std::string(fExePath.Data()) + " location " +
            std::string(locCode.Data()) + " " +
            std::string(fDataPathEntry->GetText()) + " " +
            std::to_string(4.36) + " " +                    // latitude
            std::to_string(-74.04) + " " +                  // longitude
            std::to_string((int)fParam1Entry->GetNumber()); // last param
    } else {
      // Default mode (pgrid, location)
      cmd = std::string(fExePath.Data()) + " " + std::string(mode.Data()) +
            " " + std::string(fDataPathEntry->GetText()) + " " +
            std::to_string((int)latMin) + " " + std::to_string((int)latMax) +
            " " + std::to_string((int)fParam1Entry->GetNumber()) + " " +
            std::to_string((int)fParam2Entry->GetNumber()) + " " +
            std::to_string((int)fParam3Entry->GetNumber());
    }
    AppendLog(Form("Running: %s", cmd.c_str()));

    // Fork process
    fProcessPid = fork();

    if (fProcessPid == -1) {
      AppendLog("Error: failed to fork process.");
      CleanupProcess();
      return;
    }

    if (fProcessPid == 0) {
      // Child process
      close(fPipeFd[0]);               // close read end
      dup2(fPipeFd[1], STDOUT_FILENO); // redirect stdout to pipe
      dup2(fPipeFd[1], STDERR_FILENO); // redirect stderr to pipe
      close(fPipeFd[1]);

      // Change to executable directory
      chdir(exeDir.Data());

      // Execute the command
      execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)nullptr);
      exit(1); // if execl fails
    }

    // Parent process
    close(fPipeFd[1]); // close write end
    fPipeFd[1] = -1;

    // Make read end non-blocking
    int flags = fcntl(fPipeFd[0], F_GETFL, 0);
    fcntl(fPipeFd[0], F_SETFL, flags | O_NONBLOCK);

    fProcessRunning = kTRUE;
    fRunButton->SetEnabled(kFALSE);
    fCancelButton->SetEnabled(kTRUE);

    // Start timer to check for output
    if (!fOutputTimer) {
      fOutputTimer = new TTimer();
      fOutputTimer->Connect("Timeout()", "OzoneGUI", this,
                            "CheckProcessOutput()");
    }
    fOutputTimer->Start(100); // check every 100ms
  }

  void CheckProcessOutput() {
    if (!fProcessRunning)
      return;

    char buffer[1024];
    ssize_t bytesRead;

    // Read available output
    while ((bytesRead = read(fPipeFd[0], buffer, sizeof(buffer) - 1)) > 0) {
      buffer[bytesRead] = '\0';

      // Split by lines and add each line separately
      char *line = strtok(buffer, "\n");
      while (line != nullptr) {
        AppendLog(line);
        line = strtok(nullptr, "\n");
      }
      gSystem->ProcessEvents();
    }

    // Check if process is still running
    int status;
    pid_t result = waitpid(fProcessPid, &status, WNOHANG);

    if (result == fProcessPid) {
      // Process finished
      AppendLog(Form("Process finished with status %d", WEXITSTATUS(status)));
      fOutputTimer->Stop();
      CleanupProcess();
      fRunButton->SetEnabled(kTRUE);
      fCancelButton->SetEnabled(kFALSE);
    } else if (result == -1) {
      // Error occurred
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

    // First try SIGTERM
    if (kill(fProcessPid, SIGTERM) == 0) {
      AppendLog("Sent SIGTERM, waiting for process to terminate...");

      // Wait up to 3 seconds for graceful termination
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
        gSystem->Sleep(100); // sleep 100ms
        gSystem->ProcessEvents();
      }

      // If still running, use SIGKILL
      AppendLog("Process didn't terminate gracefully, using SIGKILL...");
      if (kill(fProcessPid, SIGKILL) == 0) {
        waitpid(fProcessPid, nullptr, 0); // wait for process to die
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

  ClassDef(OzoneGUI, 0);
};

void ozone_gui() { new OzoneGUI(gClient->GetRoot(), 500, 600); }
