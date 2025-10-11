# Ozone GUI - Mermaid Diagrams

## Tab Structure Overview

```mermaid
graph TD
    A[Ozone GUI - Main Window] --> B[Welcome Tab]
    A --> C[Processor Tab]
    A --> D[Graph Viewer Tab]
    A --> E[Macro Runner Tab]
    A --> F[View O3 Global Tab]

    style A fill:#2c3e50,stroke:#34495e,stroke-width:3px,color:#fff
    style B fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style C fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style D fill:#2ecc71,stroke:#27ae60,stroke-width:2px,color:#fff
    style E fill:#f39c12,stroke:#d68910,stroke-width:2px,color:#fff
    style F fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff
```

## Processor Tab Component Hierarchy

```mermaid
graph TB
    PT[Processor Tab] --> MG[Mode Group]
    PT --> LG[Location Group]
    PT --> DP[Data Path Group]
    PT --> LR[Latitude Range]
    PT --> PG[Parameters Group]
    PT --> PO[Performance Options]
    PT --> BT[Buttons]
    PT --> PL[Process Log]

    MG --> MS[Mode Selector]
    LG --> LE[Location Entry]
    DP --> DPE[Path Entry]
    DP --> BB[Browse Button]
    LR --> LS[Lat Slider]
    LR --> LMN[Min Label]
    LR --> LMX[Max Label]
    PG --> PGR[Grid Param]
    PG --> PE[Events Param]
    PG --> PTH[Threads Param]
    PO --> SM[Silent Mode]
    PO --> NB[Notifications]
    BT --> RB[Run Button]
    BT --> CB[Cancel Button]
    PL --> LV[Log View]

    style PT fill:#e74c3c,stroke:#c0392b,stroke-width:3px,color:#fff
    style MG fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style LG fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style DP fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style LR fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style PG fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style PO fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style BT fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff
    style PL fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
```

## Graph Viewer Tab Component Hierarchy

```mermaid
graph TB
    GV[Graph Viewer Tab] --> GD[Graph Directory]
    GV --> FF[Folder Filter]
    GV --> FL[Folder List]
    GV --> FI[File Info]
    GV --> GC[Graph Canvas]

    GD --> GPE[Path Entry]
    GD --> GBB[Browse Button]
    GD --> RB[Refresh Button]
    FF --> FFE[Filter Entry]
    FF --> AB[Apply Button]
    FF --> CFB[Clear Button]
    FL --> FLB[Folder ListBox]
    FI --> FILB[File ListBox]
    GC --> EC[Embedded Canvas]

    style GV fill:#2ecc71,stroke:#27ae60,stroke-width:3px,color:#fff
    style GD fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style FF fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style FL fill:#d5f4e6,stroke:#27ae60,stroke-width:2px
    style FI fill:#d5f4e6,stroke:#27ae60,stroke-width:2px
    style GC fill:#aed6f1,stroke:#3498db,stroke-width:2px
```

## Macro Runner Tab Component Hierarchy

```mermaid
graph TB
    MR[Macro Runner Tab] --> MS[Macro Selection]
    MR --> MP[Macro Parameters]
    MR --> BT[Buttons]
    MR --> ML[Macro Log]

    MS --> MSC[Macro Selector]
    MP --> P1[Param 1-11]
    MP --> DYN[Dynamic Display]
    BT --> RMB[Run Macro Button]
    BT --> CMB[Cancel Button]
    ML --> MLV[Log View]

    style MR fill:#f39c12,stroke:#d68910,stroke-width:3px,color:#fff
    style MS fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
    style MP fill:#fdebd0,stroke:#f39c12,stroke-width:2px
    style BT fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff
    style ML fill:#ecf0f1,stroke:#95a5a6,stroke-width:2px
```

## View O3 Global Tab - Three Panel Layout

```mermaid
graph TB
    VO[View O3 Global Tab] --> DS[Data Selection Panel]
    VO --> DO[Display Options Panel]
    VO --> AC[Actions Panel]
    VO --> CV[Canvas]

    DS --> LC[Location Combo]
    DS --> CC[Category Combo]
    DS --> YC[Year Combo]
    DS --> GC[Graph Combo]

    DO --> MY[Multi-Year Check]
    DO --> SS[Superposition Settings]
    DO --> CH[Connect History]
    DO --> CT[Connect Teo]

    SS --> HCS[History Color]
    SS --> TCS[Teo Color]

    AC --> LB[Load Button]
    AC --> EXP[Export Section]
    AC --> EXIT[Exit Button]
    AC --> ST[Status Label]

    EXP --> EAY[Export All Years]
    EXP --> EB[Export Button]

    CV --> EMC[Embedded Canvas]

    style VO fill:#9b59b6,stroke:#8e44ad,stroke-width:3px,color:#fff
    style DS fill:#ebdef0,stroke:#9b59b6,stroke-width:2px
    style DO fill:#d7bde2,stroke:#9b59b6,stroke-width:2px
    style AC fill:#c39bd3,stroke:#9b59b6,stroke-width:2px
    style CV fill:#aed6f1,stroke:#3498db,stroke-width:2px
    style SS fill:#fdebd0,stroke:#f39c12,stroke-width:2px
    style EXP fill:#d5f4e6,stroke:#27ae60,stroke-width:2px
```

## User Workflow

```mermaid
flowchart LR
    A[Start] --> B[Welcome Tab]
    B --> C{Choose Task}
    C -->|Process Data| D[Processor Tab]
    C -->|View Results| E[Graph Viewer Tab]
    C -->|Run Analysis| F[Macro Runner Tab]
    C -->|Global View| G[View O3 Global Tab]

    D --> H[Configure Parameters]
    H --> I[Run Processing]
    I --> J[View Logs]
    J --> K{Success?}
    K -->|Yes| E
    K -->|No| H

    E --> L[Browse Folders]
    L --> M[Select Graphs]
    M --> N[View Results]

    F --> O[Select Macro]
    O --> P[Set Parameters]
    P --> Q[Execute]
    Q --> R{Success?}
    R -->|Yes| E
    R -->|No| P

    G --> S[Select Location]
    S --> T[Choose Category]
    T --> U[Load Graph]
    U --> V[Export Data]

    style A fill:#2c3e50,stroke:#34495e,stroke-width:2px,color:#fff
    style B fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style D fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style E fill:#2ecc71,stroke:#27ae60,stroke-width:2px,color:#fff
    style F fill:#f39c12,stroke:#d68910,stroke-width:2px,color:#fff
    style G fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff
```

## Data Flow Diagram

```mermaid
flowchart TD
    A[NASA Raw Data] --> B[Processor Tab]
    B --> C[optimized_ozone_processor]
    C --> D[Processed Data\nskim_* folders]
    D --> E[ROOT Files\n*_global.root]

    E --> F[Graph Viewer Tab]
    F --> G[Visualize Results]

    E --> H[Macro Runner Tab]
    H --> I[Advanced Analysis]
    I --> J[Analysis Results]

    E --> K[View O3 Global Tab]
    K --> L[Global Trends]
    L --> M[Exported Data\n.txt files]

    style A fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style B fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style C fill:#34495e,stroke:#2c3e50,stroke-width:2px,color:#fff
    style D fill:#f39c12,stroke:#d68910,stroke-width:2px,color:#fff
    style E fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff
    style F fill:#2ecc71,stroke:#27ae60,stroke-width:2px,color:#fff
    style G fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff
    style H fill:#f39c12,stroke:#d68910,stroke-width:2px,color:#fff
    style I fill:#e67e22,stroke:#d35400,stroke-width:2px,color:#fff
    style J fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff
    style K fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff
    style L fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style M fill:#2ecc71,stroke:#27ae60,stroke-width:2px,color:#fff
```

## Class Diagram

```mermaid
classDiagram
    class OzoneGUI {
        +TGTab fMainTabs
        +TGComboBox fModeSelector
        +TGTextEntry fDataPathEntry
        +TGDoubleHSlider fLatSlider
        +TRootEmbeddedCanvas fGraphCanvas
        +CreateWelcomeInterface()
        +CreateProcessorInterface()
        +CreateGraphInterface()
        +CreateMacroInterface()
        +RunProcessor()
        +LoadGraph()
        +RunMacro()
    }

    class O3ViewerGUI {
        +TGComboBox fLocationCombo
        +TGComboBox fCategoryCombo
        +TRootEmbeddedCanvas fEmbCanvas
        +LoadFile()
        +LoadGraph()
        +LoadSuperposition()
        +ExportData()
    }

    OzoneGUI "1" --> "1" O3ViewerGUI : embeds

    style OzoneGUI fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style O3ViewerGUI fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff
```

---

## How to View

1. **GitHub/GitLab**: Push this file to your repository - diagrams render automatically
2. **VS Code**: Install "Markdown Preview Mermaid Support" extension
3. **Online**: Copy and paste to https://mermaid.live/
4. **Documentation Tools**: Supports MkDocs, Sphinx, Docusaurus, etc.

## Color Legend

- 🔵 **Blue** (#3498db): Welcome/Information
- 🔴 **Red** (#e74c3c): Processor/Processing
- 🟢 **Green** (#2ecc71): Graph Viewer/Visualization
- 🟠 **Orange** (#f39c12): Macro Runner/Analysis
- 🟣 **Purple** (#9b59b6): View O3 Global/Data
- 🔘 **Gray** (#ecf0f1): UI Components/Groups
- 💚 **Teal** (#1abc9c): Action Buttons
