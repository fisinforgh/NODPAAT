# Instrucciones de Compilación del Manual

## Requisitos Previos

### 1. Instalación de LaTeX

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install texlive-full
sudo apt-get install texlive-lang-spanish
sudo apt-get install biber
```

#### Fedora/RHEL:
```bash
sudo dnf install texlive-scheme-full
sudo dnf install biber
```

### 2. Verificar Instalación

```bash
pdflatex --version
biber --version
```

## Compilación del Manual

### Opción 1: Compilación Manual (Paso por Paso)

```bash
cd /home/jaiver/Documents/NasaCodes/OzoneAnalysis_manual

# Primera compilación - genera archivos auxiliares
pdflatex manual_OzoneAnalysis.tex

# Procesar bibliografía
biber manual_OzoneAnalysis

# Segunda compilación - incorpora referencias
pdflatex manual_OzoneAnalysis.tex

# Tercera compilación - actualiza referencias cruzadas
pdflatex manual_OzoneAnalysis.tex
```

### Opción 2: Compilación Automática con latexmk (Recomendado)

```bash
cd /home/jaiver/Documents/NasaCodes/OzoneAnalysis_manual

# Compilación completa automática
latexmk -pdf -bibtex manual_OzoneAnalysis.tex
```

Para limpiar archivos auxiliares después:
```bash
latexmk -c
```

### Opción 3: Script de Compilación

Guarda esto como `compile_manual.sh`:

```bash
#!/bin/bash

echo "=== Compilando Manual de OzoneAnalysis ==="

cd /home/jaiver/Documents/NasaCodes/OzoneAnalysis_manual

echo "Paso 1/4: Primera compilación pdflatex..."
pdflatex -interaction=nonstopmode manual_OzoneAnalysis.tex

echo "Paso 2/4: Procesando bibliografía con biber..."
biber manual_OzoneAnalysis

echo "Paso 3/4: Segunda compilación pdflatex..."
pdflatex -interaction=nonstopmode manual_OzoneAnalysis.tex

echo "Paso 4/4: Tercera compilación pdflatex..."
pdflatex -interaction=nonstopmode manual_OzoneAnalysis.tex

echo ""
echo "=== Compilación completada ==="
echo "Archivo generado: manual_OzoneAnalysis.pdf"
echo ""

# Verificar si el PDF fue creado
if [ -f "manual_OzoneAnalysis.pdf" ]; then
    echo "✓ PDF generado exitosamente"
    ls -lh manual_OzoneAnalysis.pdf
else
    echo "✗ Error: PDF no fue generado"
    exit 1
fi
```

Darle permisos de ejecución y ejecutar:
```bash
chmod +x compile_manual.sh
./compile_manual.sh
```

## Solución de Problemas

### Error: "Package babel Error: Unknown option 'spanish'"

**Solución**: Instalar paquetes de idioma español
```bash
sudo apt-get install texlive-lang-spanish
```

### Error: "biber: command not found"

**Solución**: Instalar biber
```bash
sudo apt-get install biber
```

### Error: "File `biblatex.sty' not found"

**Solución**: Instalar biblatex
```bash
sudo apt-get install texlive-bibtex-extra
```

### Error: "File 'graphicx.sty' not found"

**Solución**: Instalar paquetes de gráficos
```bash
sudo apt-get install texlive-latex-extra
```

### Advertencia: "Package hyperref Warning: Draft mode on"

**Solución**: Esta es solo una advertencia. El PDF se genera correctamente.

### Error con imágenes faltantes

Si ves errores como "File 'img/logo.png' not found":
1. Las imágenes aún no se han creado
2. Puedes compilar el manual de todas formas (aparecerán placeholders)
3. Agrega las imágenes después en el directorio `img/`

## Estructura de Archivos Generados

Después de la compilación exitosa, se generarán estos archivos:

```
OzoneAnalysis_manual/
├── manual_OzoneAnalysis.pdf          # ← ARCHIVO FINAL
├── manual_OzoneAnalysis.tex          # Fuente LaTeX
├── bib_ozone.bib                     # Bibliografía
├── manual_OzoneAnalysis.aux          # Archivo auxiliar
├── manual_OzoneAnalysis.bbl          # Bibliografía procesada
├── manual_OzoneAnalysis.bcf          # Configuración biber
├── manual_OzoneAnalysis.blg          # Log de biber
├── manual_OzoneAnalysis.log          # Log de compilación
├── manual_OzoneAnalysis.out          # Marcadores PDF
├── manual_OzoneAnalysis.run.xml      # Metadata de ejecución
├── manual_OzoneAnalysis.toc          # Tabla de contenidos
├── manual_OzoneAnalysis.lof          # Lista de figuras
├── manual_OzoneAnalysis.lot          # Lista de tablas
└── img/                              # Directorio de imágenes
```

## Limpiar Archivos Auxiliares

Para eliminar archivos temporales pero mantener el PDF:

```bash
rm -f *.aux *.bbl *.bcf *.blg *.log *.out *.run.xml *.toc *.lof *.lot
```

O con latexmk:
```bash
latexmk -c
```

Para eliminar TODO incluyendo el PDF:
```bash
latexmk -C
```

## Visualizar el PDF Generado

```bash
# Con visor predeterminado
xdg-open manual_OzoneAnalysis.pdf

# Con Evince
evince manual_OzoneAnalysis.pdf

# Con Okular
okular manual_OzoneAnalysis.pdf
```

## Personalización Antes de Compilar

Antes de compilar, personaliza estas secciones en `manual_OzoneAnalysis.tex`:

1. **Línea 69**: `\newcommand{\seh}{{\bf OzoneAnalysisSys}}`
   - Cambia el nombre corto del sistema si lo deseas

2. **Líneas 71-72**: Autor y afiliación
   ```latex
   \author[1]{Tu Nombre Completo}
   \affil[1]{Tu Institución}
   ```

3. **Línea 99**: Encabezado
   ```latex
   \lhead{
     Tu-Apellido
   }
   ```

4. **Líneas 119-121**: Información de contacto
   ```latex
   {\bf Información del autor.}
   \\Tu Nombre$^{1}$, PhD. Cargo Institución.\\
   \href{mailto:tu@email.com}{tu@email.com}
   ```

## Verificación de Compilación Exitosa

El manual se compiló correctamente si:

1. ✓ Se genera el archivo `manual_OzoneAnalysis.pdf`
2. ✓ El PDF tiene aproximadamente 15-20 páginas
3. ✓ La tabla de contenidos muestra todas las secciones
4. ✓ La bibliografía aparece al final con todas las referencias
5. ✓ No hay errores críticos en el archivo `.log`

## Próximos Pasos

1. **Agregar imágenes**: Crea o captura las imágenes necesarias y guárdalas en `img/`
2. **Personalizar contenido**: Edita el `.tex` con información específica de tu proyecto
3. **Agregar capturas de pantalla**: Documenta la instalación y uso del sistema
4. **Revisar y corregir**: Lee el PDF generado y ajusta según sea necesario

---

**Nota**: La primera compilación puede tardar más tiempo mientras LaTeX descarga/procesa paquetes por primera vez.
