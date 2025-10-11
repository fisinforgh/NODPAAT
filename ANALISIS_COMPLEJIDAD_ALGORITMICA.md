# Análisis de Complejidad Algorítmica - NASA Ozone Analysis Codes

**Fecha de Análisis:** 2025-10-11
**Autor del Análisis:** Análisis Automatizado
**Enfoque:** Complejidad computacional de algoritmos (O notation), no ecuaciones físicas

---

## Resumen Ejecutivo

Este repositorio contiene códigos para análisis de datos de ozono de la NASA. El análisis se centra en la **complejidad algorítmica** de las operaciones, no en las ecuaciones físicas o matemáticas subyacentes.

### Índice de Complejidad por Archivo
| Archivo | Complejidad Dominante | Bottleneck Principal |
|---------|----------------------|---------------------|
| `analysis_runner.cpp` | **O(N × T)** | Procesamiento paralelo de coordenadas |
| `optimized_skim.cpp` | **O(F × D)** | Lectura y procesamiento de archivos |
| `optimized_ozone_processor.cpp` | **O(L × P)** | Compilación + procesamiento por ubicación |
| `optimized_aprobe.cpp` | **O(Y × F)** | Extracción de datos HDF5 |
| `nmeprobeData.cpp` | **O(Y × F × log N)** | Búsqueda con `sed` y procesamiento de texto |
| `make_1995.cpp` | **O(1)** constante | Generación de días del año 1995 |
| `viewO3Global.cpp` | **O(K + G)** | Escaneo de directorios y graficación |
| `ozone_gui.cpp` | **O(1)** por operación GUI | Respuesta a eventos de interfaz |

---

## 1. analysis_runner.cpp

### Descripción
Ejecutor paralelo de análisis usando ThreadPool para procesar múltiples ubicaciones geográficas.

### Estructuras de Datos Principales
- **`std::queue<std::function<void()>>`** - Cola FIFO para tareas (ThreadPool)
- **`std::vector<std::thread>`** - Vector de threads workers

### Análisis de Complejidad

#### ThreadPool Constructor (líneas 19-36)
```
Complejidad Temporal: O(T)
donde T = número de threads
```
- **Línea 20:** `for (size_t i = 0; i < numThreads; ++i)` → **O(T)**
- Cada thread entra en un bucle infinito (`while (true)`)
- Operaciones de cola: `tasks.pop()` → **O(1)**

#### Función `enqueue` (líneas 39-47)
```
Complejidad Temporal: O(1)
```
- `tasks.emplace()` → **O(1)** (push a queue)
- `condition.notify_one()` → **O(1)**

#### Destructor (líneas 49-57)
```
Complejidad Temporal: O(T)
```
- **Línea 55-56:** `for (std::thread &worker : workers)` → **O(T)**
- `worker.join()` espera la terminación de cada thread

#### Función `run_analysis` (líneas 69-94)
```
Complejidad Temporal: O(1)
```
- Construcción de comando con `ostringstream` → **O(k)** donde k es longitud de strings
- `system(cmd.str().c_str())` → depende del programa externo (no contabilizable)

#### Función `main` (líneas 96-122)
```
Complejidad Temporal: O(N)
donde N = número total de coordenadas a procesar
```
- **Líneas 114-117:** Bucle doble anidado:
  - Longitudes: `(lonMax - lonMin) / gridPrecision` → L iteraciones
  - Latitudes: `(latMax - latMin) / gridPrecision` → M iteraciones
  - **Total:** **O(L × M) = O(N)** donde N = L × M

**Análisis:**
```
N = [(180°-(-180°))/precision] × [(90°-(-90°))/precision]
Si precision = 10: N = 36 × 18 = 648 ubicaciones
Si precision = 5: N = 72 × 36 = 2,592 ubicaciones
```

### Complejidad Espacial
```
O(T + N)
```
- **T threads:** cada uno con su stack
- **N tareas:** en cola (peor caso: todas encoladas antes de procesarse)

### Optimizaciones Identificadas
1. **ThreadPool pattern:** evita overhead de crear/destruir threads
2. **Hardware concurrency detection:** ajuste automático al sistema
3. **Lock-free entre workers:** cada tarea es independiente

---

## 2. optimized_skim.cpp

### Descripción
Procesa archivos de datos de ozono, completa días faltantes y reorganiza datos por año.

### Estructuras de Datos Principales
- **`std::vector<DateData>`** - Datos de fechas con valores
- **`std::unordered_map<int, size_t>`** - Hash map para lookup O(1) de días
- **Arrays lookup estáticos:** `days_in_month_normal`, `cumulative_days_leap`

### Análisis de Complejidad

#### Función `isLeapYear` (líneas 39-41)
```
Complejidad Temporal: O(1)
```
- Operaciones bitwise y aritméticas: constante

#### Función `dayOfYear` (líneas 43-57)
```
Complejidad Temporal: O(1)
```
- Acceso directo a arrays: **O(1)**
- Lookup en `cumulative[month - 1]`: **O(1)**

#### Función `dayOfYearToDate` (líneas 60-73)
```
Complejidad Temporal: O(log M)
donde M = 12 meses
```
- **Línea 68-69:** `lower_bound()` en array de 13 elementos → **O(log 13) ≈ O(1)** en la práctica
- Binary search en conjunto fijo pequeño

#### Función `getDataFiles` (líneas 76-93)
```
Complejidad Temporal: O(F)
donde F = número de archivos en directorio
```
- **Línea 82:** Iterador de directorio → **O(F)**
- **Línea 91:** `sort()` → **O(F log F)**
- **Total:** **O(F log F)**

#### Función `readFileData` (líneas 96-117)
```
Complejidad Temporal: O(D)
donde D = número de líneas en archivo (días con datos)
```
- **Línea 112-114:** `while (file >> ...)` → **O(D)**
- Cada `emplace_back()` → **O(1)** amortizado

#### Función `process` (líneas 122-188) - **CRÍTICA**
```
Complejidad Temporal: O(F × D + F × 365)
Simplificado: O(F × D)
donde:
  F = número de archivos (años)
  D = días con datos por archivo
```

**Análisis detallado:**
- **Línea 145:** `for (const string &filename : files)` → **O(F)**
  - **Línea 148:** `readFileData()` → **O(D)**
  - **Líneas 161-167:** Construir hash map
    - `for (size_t i = 0; i < fileData.size())` → **O(D)**
    - `dayOfYear()` → **O(1)**
    - `dayToIndex[doy] = i` (inserción en unordered_map) → **O(1)** amortizado
  - **Líneas 171-183:** Procesar todos los días del año
    - `for (int doy = 1; doy <= numDays)` → **O(365)**
    - `dayToIndex.find(doy)` → **O(1)** promedio
    - `dayOfYearToDate()` → **O(1)**

**Cálculo por archivo:**
```
Tiempo por archivo = O(D) + O(D) + O(365)
                  = O(D + 365)
                  ≈ O(D) si D >> 365
```

**Total para F archivos:**
```
T_total = F × O(D) = O(F × D)
```

### Complejidad Espacial
```
O(D + F)
```
- **Vector `fileData`:** hasta 400 elementos reservados → **O(D)**
- **Hash map `dayToIndex`:** hasta D entradas → **O(D)**
- **Lista de archivos:** F strings → **O(F)**

### Optimizaciones Identificadas
1. **Uso de `unordered_map`:** Lookup O(1) vs. O(n) con búsqueda lineal
2. **Lookup tables precompiladas:** arrays constexpr para cálculos de fechas
3. **Buffer reservado:** `reserve()` evita reallocations
4. **Fast leap year check:** bitwise operations `(year & 3) == 0`

---

## 3. optimized_ozone_processor.cpp

### Descripción
Orquestador principal que compila subprogramas y los ejecuta en paralelo o secuencialmente para procesar datos de ozono por ubicación geográfica.

### Estructuras de Datos Principales
- **`std::unordered_set<std::string>`** - Set de programas compilados (para caché)
- **`std::vector<std::pair<int, int>>`** - Coordenadas geográficas a procesar
- **`std::vector<std::future<bool>>`** - Futures para procesamiento paralelo

### Análisis de Complejidad

#### Función `compilePrograms` (líneas 24-54)
```
Complejidad Temporal: O(P) o O(1) con caché
donde P = 4 programas
```
- **Línea 33:** `for (const auto &[source, executable] : programs)` → **O(4) = O(1)**
- **Línea 35:** Verificación de caché → **O(1)** (hash set lookup)
- **Línea 46:** `std::system()` para compilación → **Externo (no contable)**

**Con caché activo:** **O(1)** (solo verificaciones hash)

#### Función `executeCommandThreadSafe` (líneas 57-102)
```
Complejidad Temporal: O(F)
donde F = número de archivos temporales a limpiar (≈4)
```
- **Líneas 74-83:** Limpieza de archivos temporales → **O(F) = O(1)**
- **Línea 85:** `std::system(command)` → **Externo**

#### Función `moveDataFiles` (líneas 104-141)
```
Complejidad Temporal: O(N)
donde N = número de archivos a mover
```
- **Líneas 112-120:** Recolección de archivos → **O(N)**
- **Líneas 123-134:** Mover archivos → **O(N)**

#### Función `processLocation` (líneas 153-232) - **FUNCIÓN CLAVE**
```
Complejidad Temporal: O(1) compilación + O(P) procesamiento
donde P = número de pasos de procesamiento por ubicación
```
- **Línea 159:** `compilePrograms()` → **O(1)** con caché
- **Línea 173:** `executeCommandThreadSafe("./aprobe.exe")` → **O(Y × F)** donde Y=años, F=archivos HE5
- **Líneas 199-214:** Bucle `for (int s = 1; s <= 3; s++)` → **O(3) = O(1)**
- **Línea 217:** `executeCommandThreadSafe("./make_1995.exe")` → **O(365) = O(1)**
- **Línea 222:** `moveDataFiles()` → **O(N_archivos)**
- **Línea 227:** `executeCommandThreadSafe("./skim.exe")` → **O(F × D)**

**Total por ubicación:**
```
T_location ≈ O(ejecución aprobe) + O(ejecución nmprobe) + O(ejecución skim)
```

#### Función `processGridParallel` (líneas 235-312) - **ORQUESTADOR PRINCIPAL**
```
Complejidad Temporal: O(L) preparación + O((L/T) × T_location) procesamiento
Simplificado: O(L × T_location / T)
donde:
  L = número total de ubicaciones
  T = número de threads
  T_location = tiempo por ubicación
```

**Análisis detallado:**
- **Líneas 250-254:** Generación de coordenadas
  ```
  L = [(lonMax - lonMin) / gridPrecision] × [(latMax - latMin) / gridPrecision]
  Ejemplo: precision=10 → L = 36 × 18 = 648
  ```
- **Línea 291:** Cálculo de chunk size → **O(1)**
- **Líneas 293-301:** Creación de futures → **O(T)** threads
- **Lambda `processChunk`** (líneas 265-288):
  - **Línea 270:** `for (const auto &[lat, lon] : chunk)` → **O(L/T)** iteraciones por thread
  - **Línea 280:** `processLocation()` → **O(T_location)**

**Speedup teórico:**
```
Tiempo secuencial: L × T_location
Tiempo paralelo: (L / T) × T_location + overhead
Speedup: ≈ T (en condiciones ideales)
```

#### Función `processGrid` (líneas 315-342) - **VERSIÓN SECUENCIAL**
```
Complejidad Temporal: O(L × T_location)
```
- **Líneas 326-338:** Bucle doble anidado → **O(L)**
- Cada iteración llama `processLocation()` → **O(T_location)**

### Complejidad Espacial
```
O(L + T)
```
- **Vector de coordenadas:** L pares → **O(L)**
- **Threads activos:** T threads → **O(T)**
- **Futures:** T futures → **O(T)**

### Optimizaciones Identificadas
1. **Caché de compilación:** `compiled_programs` set previene recompilaciones
2. **Procesamiento paralelo:** divide trabajo en chunks entre T threads
3. **Mutex granular:** `compilation_mutex` solo protege compilación, no procesamiento
4. **Archivos temporales únicos:** evita colisiones entre threads paralelos
5. **Hardware detection:** `std::thread::hardware_concurrency()`

---

## 4. optimized_aprobe.cpp

### Descripción
Extrae datos de archivos HDF5 (.he5) de Aura/OMI para coordenadas geográficas específicas.

### Estructuras de Datos Principales
- **`std::vector<string>`** - Lista de archivos HE5
- **Constexpr lookup tables:** STEP_A, XMIN_LAT_A, XMIN_LON_A

### Análisis de Complejidad

#### Función `calculateLatBin` / `calculateLonBin` (líneas 29-37)
```
Complejidad Temporal: O(1)
```
- Cálculo aritmético: división, suma, redondeo → **O(1)**

#### Función `directoryExists` (líneas 40-42)
```
Complejidad Temporal: O(1)
```
- Llamada al sistema de archivos (típicamente O(1) en caché)

#### Función `getHE5Files` (líneas 44-67)
```
Complejidad Temporal: O(F)
donde F = número de archivos en directorio
```
- **Línea 56:** `fs::directory_iterator` → **O(F)**
- **Línea 65:** `sort()` → **O(F log F)**
- **Total:** **O(F log F)**

#### Función `extractDate` (líneas 75-92)
```
Complejidad Temporal: O(1)
```
- Operación `find()` en string → **O(k)** donde k = longitud filename (constante)
- Operaciones `substr()` → **O(1)**

#### Función `executeH5Dump` (líneas 95-118) - **BOTTLENECK**
```
Complejidad Temporal: O(T_extract)
donde T_extract = tiempo de extracción HDF5 (I/O bound)
```
- Construcción de comando → **O(1)**
- **Línea 104:** `popen()` ejecuta h5dump + awk → **Externo (I/O bound)**
- Parsing de valor → **O(1)**

**Nota:** Esta es la operación más costosa del programa debido a I/O de disco.

#### Función `process` (líneas 131-192) - **FUNCIÓN PRINCIPAL**
```
Complejidad Temporal: O(Y × F × T_extract)
donde:
  Y = número de años (YMAX - YMIN + 1) ≈ 20
  F = archivos HE5 por año ≈ 365
  T_extract = tiempo de extracción por archivo
```

**Análisis detallado:**
- **Línea 146:** `for (int year = YMIN; year <= YMAX)` → **O(Y)**
  - **Línea 160:** `getHE5Files(year)` → **O(F log F)**
  - **Línea 168:** `for (const string &filename : he5Files)` → **O(F)**
    - **Línea 172:** `extractDate()` → **O(1)**
    - **Línea 178:** `executeH5Dump()` → **O(T_extract)**

**Cálculo:**
```
T_total = Y × (F log F + F × T_extract)
        ≈ Y × F × T_extract (dominante)
        ≈ 20 × 365 × T_extract
        = 7,300 × T_extract
```

### Complejidad Espacial
```
O(F)
```
- **Vector de archivos HE5:** hasta 400 reservados → **O(F)**
- Buffers temporales → **O(1)**

### Optimizaciones Identificadas
1. **Constexpr para cálculos:** pre-computación en compile-time
2. **Inline functions:** `noexcept` y pequeñas para inlining
3. **Buffer reservado:** `files.reserve(400)` evita reallocations
4. **Pipe directo:** `popen()` evita archivos temporales intermedios

### Bottleneck Principal
**`executeH5Dump` → I/O bound** (lectura de disco + extracción HDF5)
- Dominado por latencia de disco y parsing HDF5
- No hay mucho margen de optimización sin modificar h5dump

---

## 5. nmeprobeData.cpp

### Descripción
Procesa archivos de texto con datos de ozono históricos (Nimbus, Meteor, Earth Probe) para extraer valores por coordenada.

### Estructuras de Datos Principales
- **Arrays de caracteres C-style:** `char cmdName[1000]`, `char fileName[1000]`
- **Archivos temporales:** generados con process ID para evitar colisiones

### Análisis de Complejidad

#### Función `main` - Bucle Principal (líneas 211-357)
```
Complejidad Temporal: O(Y × F × (L_sed + L_awk))
donde:
  Y = número de años procesados
  F = archivos por año
  L_sed = búsqueda con sed en archivo (~O(N) líneas)
  L_awk = extracción con awk en archivo (~O(N))
```

**Análisis detallado:**

1. **Línea 211:** `for (int i = YMIN; i <= YMAX; i++)` → **O(Y)**
   - Nimbus: 1979-1993 (15 años)
   - Meteor: 1994 (1 año)
   - Earth Probe: 1996-2004 (9 años)

2. **Línea 222:** `system(cmdName)` - Lista archivos con `ls` → **O(F log F)**

3. **Líneas 238-350:** `while (!inList.eof())` → **O(F)** iteraciones por año
   - **Línea 267:** `system(cmdSed)` - Búsqueda con sed:
     ```bash
     sed -n '/lat = <value>/=' filename
     ```
     - Complejidad: **O(N)** donde N = líneas en archivo
     - Cada archivo típicamente tiene ~12,000 líneas (25 bins × 180 lat × ~3)

   - **Línea 292:** `system(cmdAwk)` - Extracción con awk:
     ```bash
     awk 'FNR==<nLine>' filename
     ```
     - Complejidad: **O(nLine)** (lectura secuencial hasta línea objetivo)

   - **Líneas 305-313:** Extracción de substring → **O(1)**

**Cálculo total:**
```
T_total = Y × F × (N_sed + nLine_awk)
        ≈ Y × F × N (donde N ≈ 12,000 líneas)

Ejemplo Nimbus:
  15 años × 365 archivos × 12,000 líneas = 65,700,000 operaciones de búsqueda
```

### Complejidad Espacial
```
O(1)
```
- Uso de buffers C-style de tamaño fijo
- Archivos temporales escritos a disco (no en memoria)

### Optimizaciones Identificadas
1. **Process ID único:** evita colisiones en archivos temporales en ejecuciones paralelas
2. **Limpieza de temporales:** `rm` commands después de cada uso

### Bottlenecks Identificados
1. **Búsqueda con `sed`:** O(N) por archivo, no aprovecha índices
2. **Múltiples llamadas a `system()`:** overhead de fork/exec por archivo
3. **I/O de archivos temporales:** escritura y lectura de disco repetidas

### Posibles Optimizaciones (no implementadas)
- **Parseo en C++:** eliminar dependencia de sed/awk → **O(N)** pero más eficiente
- **Caché de líneas:** guardar posiciones de latitudes en memoria
- **Binary search:** si archivos están ordenados por latitud

---

## 6. make_1995.cpp

### Descripción
Genera archivo .dat para el año 1995 con todos los días marcados como datos faltantes (-2).

### Análisis de Complejidad

#### Función `funDY` (líneas 17-78)
```
Complejidad Temporal: O(1)
```
- Serie de condiciones `if-else` con límites constantes → **O(1)**
- Máximo 12 comparaciones (una por mes)

#### Función `main` (líneas 83-138)
```
Complejidad Temporal: O(365)
```
- **Línea 127:** `for( doy = 1; doy <= 365; doy++)` → **O(365) = O(1)** constante
  - **Línea 129:** `funDY(doy, cYY)` → **O(1)**
  - **Línea 135:** Escritura a archivo → **O(1)**

### Complejidad Espacial
```
O(1)
```
- Variables locales de tamaño fijo
- Sin estructuras de datos dinámicas

### Análisis General
**Programa extremadamente simple:** Complejidad constante respecto al tamaño de entrada (no hay entrada variable).

---

## 7. viewO3Global.cpp

### Descripción
Interfaz gráfica ROOT para visualizar datos de ozono desde archivos .root con múltiples gráficos organizados por año.

### Estructuras de Datos Principales
- **`std::set<TString>`** - Conjunto ordenado de nombres de gráficos (para unicidad)
- **`std::vector<TString>`** - Años disponibles, archivos actuales
- **ROOT Objects:** TFile, TGraph, TH1, TCanvas, TDirectory

### Análisis de Complejidad

#### Función `ScanLocations` (líneas 290-331)
```
Complejidad Temporal: O(D + L × R)
donde:
  D = número de directorios en base path
  L = número de ubicaciones válidas
  R = verificaciones de archivos ROOT
```
- **Líneas 302-320:** `while ((entry = gSystem->GetDirEntry(dirp)))` → **O(D)**
- **Línea 308:** `gSystem->OpenDirectory()` → **O(1)** por directorio
- **Línea 315:** `gSystem->AccessPathName()` (verificar archivo ROOT) → **O(1)**
- **Línea 316:** `AddEntry()` a combobox → **O(1)**

**Total:** **O(D)** lineal en número de directorios

#### Función `LoadFile` (líneas 348-387)
```
Complejidad Temporal: O(1)
```
- Bucle fijo de 3 intentos de paths → **O(1)**
- `TFile::Open()` → I/O bound, pero contado como **O(1)** operación

#### Función `PopulateYears` (líneas 389-410)
```
Complejidad Temporal: O(K)
donde K = número de keys (directorios) en ROOT file
```
- **Línea 399:** `while ((key = (TKey *)next()))` → **O(K)**
- **Línea 402:** `AddEntry()` → **O(1)**

#### Función `PopulateGraphs` (líneas 412-535)
```
Complejidad Temporal: O(K) o O(G log G) en modo superposición
donde:
  K = número de keys
  G = número de gráficos
```

**Análisis por categoría:**

1. **Category 0 (Analysis graphs):** **O(1)** - Lista hardcodeada (12 entries)

2. **Category 6 (Superposition):**
   - **Líneas 443-462:** Escaneo de history y comp directories → **O(K)**
   - **Línea 441:** `std::set<TString> graphNames` → **O(log G)** por inserción
   - **Líneas 466-468:** Iteración sobre set → **O(G)**
   - **Total:** **O(K + G log G)**

3. **Categories 1-4:** **O(K)** - Iteración sobre keys de subdirectorio

4. **Category 5 (Individual):** **O(K²)** - Bucle anidado sobre subdirectorios y gráficos

#### Función `LoadSuperposition` (líneas 700-876) - **FUNCIÓN CLAVE DE GRAFICACIÓN**
```
Complejidad Temporal: O(N_hist + N_teo)
donde:
  N_hist = número de puntos en History O3 graph
  N_teo = número de puntos en O3 Teo graph
```
- **Líneas 725-730:** Carga de objetos desde ROOT file → **O(1)** (lookup hash interno)
- **Líneas 747-780:** Draw History O3:
  - `Clone()` → **O(N_hist)** (copia de puntos)
  - `Draw()` → **O(N_hist)** (rendering)
- **Líneas 783-823:** Draw O3 Teo:
  - Similar: **O(N_teo)**

**Total:** **O(N_hist + N_teo)**

#### Función `LoadMultiYearPanel` (líneas 878-1063) - **GRAFICACIÓN MÚLTIPLE**
```
Complejidad Temporal: O(Y × (N_hist + N_teo))
donde:
  Y = número de años
  N_hist, N_teo = puntos por gráfico
```
- **Líneas 899-907:** Recolección de años → **O(K)**
- **Línea 918:** Cálculo de grid → **O(1)**
- **Líneas 929-1001:** Bucle sobre años → **O(Y)**
  - Cargar y dibujar 2 gráficos por año → **O(N_hist + N_teo)**

**Total:** **O(Y × N_points)** donde N_points = N_hist + N_teo

#### Función `LoadGraph` (líneas 1065-1263)
```
Complejidad Temporal: O(1) o O(Y × N) en modo multi-year
```
- Redirige a `LoadMultiYearPanel` o `LoadSuperposition`
- En modo simple: **O(N)** donde N = puntos en gráfico

#### Función `ExportData` (líneas 1265-1545)
```
Complejidad Temporal: O(Y × N)
donde:
  Y = años a exportar
  N = puntos promedio por gráfico
```
- **Líneas 1287-1310:** Recolección de años → **O(K)** o **O(1)**
- **Líneas 1357-1408:** Bucle de exportación:
  - **Línea 1357:** `for (size_t iYear = 0; iYear < yearsToExport.size())` → **O(Y)**
  - **Líneas 1378-1383:** Export History O3 → **O(N_hist)**
  - **Líneas 1399-1405:** Export O3 Teo → **O(N_teo)**

**Total:** **O(Y × N)** escritura secuencial a archivo

### Complejidad Espacial
```
O(Y + G + N)
```
- **Vector de años:** Y strings → **O(Y)**
- **Set de gráficos:** G nombres → **O(G)**
- **Objetos clonados:** hasta 2N puntos en memoria (history + teo) → **O(N)**

### Optimizaciones Identificadas
1. **Uso de `std::set`:** Evita duplicados automáticamente en O(log G)
2. **Clone de gráficos:** Evita sobrescribir datos originales
3. **Layout managers:** Optimización de redibujado de GUI
4. **Caché de archivo ROOT:** Mantiene TFile abierto durante sesión

### Bottlenecks Identificados
1. **Rendering de gráficos:** O(N) por gráfico, CPU-bound para N grande
2. **Escritura de archivos:** Export con I/O secuencial
3. **Escaneo de directorios:** O(D) al inicio de sesión

---

## 8. ozone_gui.cpp

### Descripción
Interfaz gráfica principal con múltiples tabs: Processor, Graph Viewer, Macro Runner, y View O3 Global embebido.

### Estructuras de Datos Principales
- **`std::vector<std::string>`** - Buffer de salida de procesos
- **`std::vector<TString>`** - Listas de folders y archivos actuales
- **`std::vector<TObject *>`** - Objetos gráficos en memoria
- **TFile*** - Archivo ROOT abierto actual

### Análisis de Complejidad

#### Constructor `OzoneGUI` (líneas 104-177)
```
Complejidad Temporal: O(1)
```
- Creación de widgets ROOT: constante (número fijo de controles)
- `UpdateLatLabels()` → **O(1)**
- `RefreshFolderList()` → **O(D)** (analizado más abajo)

#### Función `RefreshFolderList` (líneas 915-979)
```
Complejidad Temporal: O(D)
donde D = número de entradas en directorio
```
- **Línea 932:** `while ((dp = readdir(dirp)) != nullptr)` → **O(D)**
- **Línea 942:** `stat()` system call → **O(1)** por entrada
- **Líneas 948-963:** Filtrado con string matching → **O(k)** donde k = longitud nombre
  - `nameLower.Contains(fCurrentFilter)` → **O(k × f)** donde f = longitud filtro
- **Línea 959:** `AddEntry()` a listbox → **O(1)**

**Total:** **O(D × k)** ≈ **O(D)** si nombres son longitud constante

#### Función `OnFolderSelected` (líneas 981-1011)
```
Complejidad Temporal: O(D) + O(G)
donde:
  D = archivos en folder
  G = gráficos en ROOT file
```
- **Línea 990:** `LoadGraphsFromFolder()` → **O(D)** (escaneo de folder)
- Carga y renderizado de gráficos → **O(G × N)** donde N = puntos por gráfico

#### Función `LoadGraphsFromFolder` (líneas 1013-1052)
```
Complejidad Temporal: O(D)
```
- **Líneas 1027-1037:** `while ((dp = readdir(dirp)) != nullptr)` → **O(D)**
- **Línea 1045:** `LoadAndDisplayGraphs()` → **O(G × N)**

#### Función `LoadAndDisplayGraphs` (líneas 1054-1170) - **RENDERING PRINCIPAL**
```
Complejidad Temporal: O(G × N)
donde:
  G = número de gráficos/canvases
  N = puntos por gráfico (promedio)
```

**Análisis detallado:**

1. **Líneas 1076-1090:** Iteración sobre keys en ROOT file → **O(K)**
2. **Modo Canvas (líneas 1092-1118):**
   - **Línea 1102:** `for (size_t i = 0; i < canvasKeys.size())` → **O(C)** canvases
   - **Línea 1111:** `DrawClonePad()` → **O(N)** por canvas (rendering)
   - **Total:** **O(C × N)**

3. **Modo Individual (líneas 1119-1166):**
   - **Línea 1134:** `for (size_t i = 0; i < drawableKeys.size())` → **O(G)**
   - **Líneas 1146-1160:** Draw por tipo:
     - TH1: `hist->Draw()` → **O(bins)** ≈ **O(N)**
     - TGraph: `graph->Draw()` → **O(N)**
   - **Total:** **O(G × N)**

#### Función `RunProcessor` (líneas 1303-1402)
```
Complejidad Temporal: O(1) para inicio de proceso
```
- Construcción de comando → **O(k)** donde k = longitud strings
- `fork()` → **O(1)**
- `execl()` → lanza proceso externo (no contable)

#### Función `CheckProcessOutput` (líneas 1404-1465) - **MONITOREO CONTINUO**
```
Complejidad Temporal: O(B) por llamada del timer
donde B = bytes leídos en buffer (hasta 4096)
```
- **Línea 1412:** `while ((bytesRead = read(...)) > 0)` → **O(B/4096)** iteraciones
- **Líneas 1415-1419:** Tokenización con `strtok()` → **O(B)** (escaneo de buffer)
- **Líneas 1421-1424:** Flush si buffer lleno → **O(kMaxBufferSize) = O(50) = O(1)**

**Frecuencia:** Timer ejecuta cada 500ms, por lo que:
```
Carga_total = (O(B) cada 500ms) durante T segundos de ejecución
```

#### Función `RunMacro` (líneas 1517-1621)
```
Complejidad Temporal: O(1) para inicio
```
- Similar a `RunProcessor`: fork + exec
- Construcción de comando ROOT → **O(k)**

### Complejidad Espacial

```
O(B + G + N)
```
- **Buffer de salida:** hasta 50 líneas → **O(B)**
- **Lista de folders:** D strings → **O(D)**
- **Objetos gráficos:** G objetos × N puntos → **O(G × N)**
- **TFile abierto:** mantiene índice interno → **O(K)** keys

### Optimizaciones Identificadas

1. **Buffering de salida:** `kMaxBufferSize = 50` evita saturar GUI
2. **Non-blocking I/O:** `fcntl(..., O_NONBLOCK)` permite lectura asíncrona
3. **Timer-based polling:** 500ms reduce overhead vs. busy-wait
4. **Object caching:** `fCurrentObjects` mantiene gráficos en memoria para redibujar
5. **Filter con early-exit:** `!nameLower.Contains(fCurrentFilter)` → continua si no match
6. **Reserved vectors:** `files.reserve(50)` previene reallocations

### Bottlenecks Identificados

1. **Rendering de canvas:** O(G × N) para múltiples gráficos grandes
2. **Lectura de directorios:** O(D) con stat() por entrada (I/O bound)
3. **Polling de procesos:** overhead de timer cada 500ms
4. **Lectura de ROOT files:** I/O desde disco para cargar gráficos

### Análisis de Uso de Memoria

**Peor caso (Graph Viewer con múltiples gráficos):**
```
Si G = 10 gráficos, cada uno con N = 1000 puntos:
Memoria_graficos = G × N × sizeof(TObject + double data)
                 ≈ 10 × 1000 × (200 bytes overhead + 16 bytes/punto)
                 ≈ 10 × 1000 × 216 bytes
                 ≈ 2.16 MB
```
**Aceptable** para aplicaciones GUI modernas.

---

## Conclusiones Generales

### 1. Complejidades Dominantes por Tipo de Operación

| Operación | Complejidad | Dominado por |
|-----------|------------|--------------|
| Procesamiento paralelo de grid | **O((L × M) / T)** | División de trabajo entre T threads |
| Extracción HDF5 (aprobe) | **O(Y × F)** | I/O de disco + parsing HDF5 |
| Procesamiento de archivos (skim) | **O(F × D)** | Lectura + escritura secuencial |
| Búsqueda en texto (nmprobe) | **O(Y × F × N)** | Múltiples llamadas a sed/awk |
| Renderizado de gráficos | **O(G × N)** | CPU para dibujar N puntos |

### 2. Bottlenecks Principales del Sistema

#### A. I/O Bound Operations (Más Críticos)
1. **`executeH5Dump` en aprobe:** Lectura de archivos HDF5 grandes
   - **Latencia dominante:** Tiempo de acceso a disco
   - **Mitigación:** Paralelización (ya implementada en processor)

2. **`sed` y `awk` en nmprobe:** Búsqueda de texto repetitiva
   - **Ineficiencia:** Múltiples forks de procesos
   - **Mejora sugerida:** Reemplazar con parsing C++ in-memory

#### B. CPU Bound Operations
1. **Rendering de gráficos ROOT:** O(G × N) para múltiples gráficos
   - **Mitigación actual:** Lazy loading (solo cuando usuario selecciona)

2. **Compilación de subprogramas:** O(P) pero con caché eficiente
   - **Optimización existente:** Set de programas compilados

### 3. Escalabilidad del Sistema

#### Escenario: Grid Global (precision = 5°)
```
Ubicaciones: 72 × 36 = 2,592
Threads: 8
Años: 20
Archivos/año: 365

Tiempo estimado por ubicación:
  - aprobe: 20 × 365 × T_extract ≈ 7,300 × T_extract
  - nmprobe: 25 × 365 × 12,000 líneas ≈ 109M operaciones
  - skim: 20 × 365 días = 7,300 procesados

Speedup con 8 threads:
  T_parallel ≈ T_sequential / 8 ≈ 12.5% del tiempo original
```

### 4. Recomendaciones de Optimización

#### Alta Prioridad
1. **nmprobe:** Reemplazar sed/awk con parseo C++ nativo
   - **Ganancia esperada:** 10-50x más rápido
   - **Complejidad de implementación:** Media

2. **aprobe:** Usar librería HDF5 directa en lugar de h5dump
   - **Ganancia esperada:** 5-10x más rápido (menos overhead de pipes)
   - **Complejidad de implementación:** Alta

#### Media Prioridad
3. **Caché de coordenadas:** Pre-calcular bins para grid común
   - **Ganancia esperada:** Marginal (ya es O(1))
   - **Beneficio:** Reduce cálculos repetitivos

4. **Batch processing:** Agrupar múltiples extracciones HDF5 por archivo
   - **Ganancia esperada:** Reduce I/O de apertura/cierre de archivos
   - **Complejidad de implementación:** Media

#### Baja Prioridad
5. **GUI rendering:** Double-buffering para gráficos grandes
   - **Beneficio:** Mejor experiencia de usuario, no velocidad

### 5. Análisis de Correctitud Algorítmica

Todos los algoritmos identificados son **correctos** para sus propósitos:
- **No hay algoritmos exponenciales** que puedan causar timeouts
- **Uso apropiado de estructuras de datos:**
  - Hash maps para lookups O(1)
  - Vectors para almacenamiento secuencial
  - Sets para unicidad
- **Buenas prácticas de concurrencia:**
  - Mutex granular
  - Lock-free donde sea posible
  - Archivos temporales únicos por proceso

### 6. Comparación con Complejidad Teórica Óptima

| Operación | Implementado | Teórico Óptimo | Gap |
|-----------|-------------|----------------|-----|
| Buscar día en año | **O(1)** hash map | O(1) | ✓ Óptimo |
| Ordenar archivos | **O(F log F)** | O(F log F) | ✓ Óptimo |
| Paralelización | **O(L/T)** | O(L/T) | ✓ Óptimo |
| Búsqueda lat en texto | **O(N)** sed | O(log N) si ordenado | ⚠ Subóptimo |
| Extracción HDF5 | **O(T_extract)** h5dump | O(T_read) librería directa | ⚠ Overhead evitable |

---

## Apéndice: Tabla Resumen de Notación O

| Notación | Nombre | Ejemplo en Código | Escalabilidad |
|----------|--------|-------------------|--------------|
| **O(1)** | Constante | Cálculo de bins, lookup hash | Excelente |
| **O(log N)** | Logarítmica | Binary search en lookup tables | Muy buena |
| **O(N)** | Lineal | Iteración sobre archivos, días | Buena |
| **O(N log N)** | Lineal-logarítmica | Sort de archivos HE5 | Aceptable |
| **O(N²)** | Cuadrática | Bucles anidados (lat × lon) | Limitada |
| **O(Y × F)** | Lineal múltiple | Procesar años × archivos | Depende de Y, F |
| **O(L/T)** | Paralelo | Grid dividido entre T threads | Escalable con HW |

---

**Fin del Análisis de Complejidad Algorítmica**
