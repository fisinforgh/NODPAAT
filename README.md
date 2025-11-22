# NODPAAT

**NASA Ozone Data Processing and Analysis Tool**

NODPAAT is an open-source computational tool designed for processing, visualizing, and analyzing NASA satellite ozone data from 1979 to 2024. The system integrates data from four satellite missions (Nimbus-7/TOMS, Meteor-3/TOMS, Earth Probe/TOMS, Aura/OMI) and provides parallel processing capabilities for efficient analysis of atmospheric ozone correlations with solar activity.

## Features

- **Multi-source data integration**: Unified processing of HDF5 files from four NASA satellite missions
- **Parallel processing**: ThreadPool implementation achieving up to 9x speedup with 12 threads
- **Statistical analysis**: Linear regression, chi-square tests, and correlation analysis for ozone-solar relationships
- **Interactive GUI**: ROOT-based graphical interface with tabbed architecture for data processing, visualization, and analysis
- **Export capabilities**: Results can be exported in PNG, PDF, CSV, and ROOT formats

## Requirements

### System Requirements

- Linux operating system (tested on Ubuntu, Kubuntu, Linux Mint)
- Minimum 12 GB RAM (31 GB recommended for large datasets)
- C++17 compatible compiler

### Dependencies

> **⚠️ Important**: ROOT must be installed from source or official packages. **Do not use the Snap version** as it lacks the necessary HDF5 integration required to process NASA data files.

- **ROOT CERN**: Follow the official installation guide at https://root.cern/install/
- **HDF5 Libraries**: `sudo apt install libhdf5-dev hdf5-tools`

## Compilation

```bash
g++ -O3 -march=native -std=c++17 optimized_ozone_processor.cpp -o optimized_ozone_processor
```

## Usage

### Graphical Interface

Launch the GUI:
```bash
root -l ozone_gui.cpp
```

The interface provides five tabs:
- **Welcome**: System information and quick start guide
- **Processor**: Data processing by geographic location
- **Graph Viewer**: Visualization of ozone-solar correlations
- **Macro Runner**: Execute scientific analysis macros
- **View O3 Global**: Global ozone data visualization

### Command Line

**Parallel processing with N threads:**
```bash
./optimized_ozone_processor pgrid /path/to/data/ -90 90 10 7 4
```

**Sequential processing:**
```bash
./optimized_ozone_processor grid /path/to/data/ -90 90 10 6
```

**Single location:**
```bash
./optimized_ozone_processor location BOG /path/to/data/ 4.36 -74.04 6
```

### Chi-Square Analysis

Compile:
```bash
g++ -g -o chi2LRSO3vsSnRunApp chi2LinearRelStudyO3vsSn.cxx `root-config --cflags --libs`
```

Run:
```bash
./bulkChi2LRSO3vsSnRunApp.sh 7 10 1.1
```

## Data

The system processes NASA ozone data in HDF5 format. Data can be obtained from:
- [NASA Goddard Space Flight Center](https://ozoneaq.gsfc.nasa.gov/)
- [NASA EarthData](https://earthdata.nasa.gov/)

### Downloading Data

The `files_download/` folder contains shell scripts as an alternative to NASA's slow sequential wget process. These scripts use `aria2c` to download multiple files simultaneously.

**Install aria2c:**
```bash
sudo apt install aria2
```

**Recommended approach:**
1. Run each `.sh` script one at a time (e.g., `aria2c_AURA.sh`, `aria2c_NIMBUS.sh`, etc.)
2. Repeat the process to verify no files are missing

## Documentation

- Technical Manual: `manual_tecnico/`
- User Manual: `manual_usuario_ozone_gui.pdf`

## Authors

- **Julián Andrés Salamanca Bernal** - Universidad Distrital Francisco José de Caldas
- **Jaiver Estiven Salazar Ortiz** - Universidad Distrital Francisco José de Caldas

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Universidad Distrital Francisco José de Caldas for institutional support
- NASA for providing publicly available satellite ozone data
