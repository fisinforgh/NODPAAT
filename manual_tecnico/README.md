# Manual Técnico del Sistema de Análisis de Datos de Ozono Atmosférico de NASA

Este directorio contiene el manual técnico completo del sistema de análisis de ozono desarrollado para procesar datos satelitales de NASA (1979-2024).

## Estructura del Manual

El manual sigue la misma estructura del manual de referencia SEEDPM y contiene:

### Documento Principal
- `manual_OzoneAnalysis.tex` - Archivo fuente LaTeX del manual completo

### Bibliografía
- `bib_ozone.bib` - Referencias bibliográficas en formato BibTeX

### Directorios de Imágenes
- `img/` - Directorio principal de imágenes
  - `img/install_guide/` - Capturas de pantalla de instalación
  - `img/uninstall_guide/` - Capturas de pantalla de desinstalación
  - `img/screenshots/` - Capturas de interfaz del software
  - `img/diagrams/` - Diagramas técnicos y de arquitectura
  - `img/logos/` - Logos institucionales

## Contenido del Manual

El manual incluye las siguientes secciones principales:

1. **Introducción** - Contexto del sistema de análisis de ozono
2. **Validación del Sistema** - Metodología de desarrollo y validación
   - Matriz de trazabilidad
   - Contexto y análisis del problema de investigación
   - Reconocimiento y especificación de requerimientos
   - Diseño de software (arquitectura de 5 capas)
   - Implementación y codificación
   - Fase de iteraciones y pruebas
   - Mantenimiento via Git
3. **Evaluación del Sistema** - Criterios de calidad científica
4. **Conclusiones**
5. **Anexo A: Guía de instalación**
6. **Anexo B: Guía de uso**

## Características del Sistema Documentadas

- **Extracción de datos HDF5** de archivos satelitales NASA
- **Procesamiento paralelo** con ThreadPool
- **Análisis estadístico** (regresión lineal, chi-cuadrado)
- **Interfaz gráfica** basada en ROOT-C++ con 5 tabs
- **Correlación ozono-manchas solares**
- **Visualización científica** en formatos ROOT, PDF, PNG

## Compilación del Manual

### Requisitos
- LaTeX (TeX Live o MiKTeX)
- Biber (para bibliografía)
- Paquetes LaTeX: babel, biblatex, graphicx, hyperref, etc.

### Comandos de compilación

```bash
cd OzoneAnalysis_manual

# Compilación completa
pdflatex manual_OzoneAnalysis.tex
biber manual_OzoneAnalysis
pdflatex manual_OzoneAnalysis.tex
pdflatex manual_OzoneAnalysis.tex
```

O usando latexmk (recomendado):

```bash
latexmk -pdf -bibtex manual_OzoneAnalysis.tex
```

## Personalización

Para personalizar el manual para tu institución, edita las siguientes secciones en `manual_OzoneAnalysis.tex`:

1. **Línea 69**: Título del manual
2. **Líneas 71-72**: Autor(es) y afiliación(es)
3. **Líneas 87-91**: Logos institucionales en el encabezado
4. **Línea 99**: Nombres de autores en encabezado
5. **Líneas 119-121**: Información de contacto de autores

## Imágenes Necesarias

El manual requiere las siguientes imágenes (a agregar en el directorio `img/`):

### Logos Institucionales
- Logo de la institución principal
- Logos de grupos de investigación

### Diagramas Técnicos
- Diagrama de arquitectura del sistema (5 capas)
- Diagrama de flujo de datos
- Matriz de trazabilidad
- Árbol de archivos del código fuente

### Capturas de Pantalla
- GUI principal del sistema (viewO3teoData.C)
- Cada uno de los 5 tabs de análisis
- Ejemplos de gráficos generados
- Visualizaciones de correlaciones

### Guías de Instalación
- Pasos de instalación de dependencias
- Verificación de instalación
- Capturas de terminal con comandos

## Notas Técnicas

### Arquitectura del Sistema Documentada

El manual documenta las 5 capas principales:

1. **Capa de Extracción** - 5 programas de extracción HDF5
2. **Capa de Procesamiento** - Consolidación de series temporales
3. **Capa de Análisis** - Análisis estadístico y correlaciones
4. **Capa de Visualización** - GUI y generación de gráficos
5. **Capa de Soporte** - Utilidades (ThreadPool, SunPhysics, etc.)

### Tecnologías Documentadas

- C++17
- ROOT-C++ (CERN framework)
- HDF5
- ThreadPool para procesamiento paralelo
- GNU Make para compilación
- Git para control de versiones

## Licencia

Este manual documenta un sistema de código abierto para investigación científica en ciencias atmosféricas.

## Contacto

[Agregar información de contacto del autor/mantenedor]

---

**Fecha de creación**: 2025-11-08
**Versión del manual**: 1.0
**Sistema documentado**: Sistema de Análisis de Datos de Ozono Atmosférico de NASA (1979-2024)
