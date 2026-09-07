# Analysis of Dynamic Susceptibility Contrast (DSC) MRI

## Introduction
Dynamic Susceptibility Contrast (DSC) MRI imaging is an important functional imaging method that enables quantitative assessment of tissue hemodynamic patterns. Abnormality of blood flow, volume and permeability is frequently observed during tumor growth, and characterization of these perfusion attributes has become clinically important for both diagnosis and therapy planning. In the context of glial neoplasms, perfusion characteristics have been shown to correlate with tumor type and grade and hence influence treatment decisions.

See documentation at
[DSC MRI Analyais Wiki](http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Modules/DSC_MRI_Analysis)

## Functionality
* Estimation of quantitative parameters(rCBV,rCBF and MTT) from DSC MRI.
* Guide diagnosis, prognosis and therapy planning.
* Brain Tumor, Stroke, Adrenoleukodystrophy and other diseases. 

## Acknowledgments
Acknowledgments: This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, and by National Cancer Institute as part of the Quantitative Imaging Network initiative (U01CA154601) and QIICR (U24CA180918).

## References
1. [Introduction of DSC MRI from Siemens](http://www.healthcare.siemens.com/siemens_hwem-hwem_ssxa_websites-context-root/wcm/idc/groups/public/@global/@imaging/@mri/documents/download/mdaw/mtix/~edisp/brain_perfusion_how_why-00093544.pdf)
2. [Equations for conversion of signal to concentration for DSC](http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2657863/)
3. [Other method to investigate DSC MRI](http://www.ncbi.nlm.nih.gov/pmc/articles/PMC4208985/pdf/radiol.14132458.pdf)
4. [Leakage correction for DSC MRI](http://www.ncbi.nlm.nih.gov/pubmed/16611779)

# Building DSCMRIAnalysis Standalone on Apple Silicon (macOS, arm64)

This fork builds the DSC-MRI perfusion CLI natively, without a full Slicer install.

## Prerequisites

```bash
brew install itk dcm2niix
```

**Patch a stale SDK path baked into Homebrew's ITK.** Homebrew's ITK bottle hardcodes an
absolute Command Line Tools SDK path in three CMake module files, which can mismatch your
system's actual SDK and cause `<cstring>`/`<cmath>`/etc. header errors. Fix once:

```bash
for f in \
  $(brew --prefix itk)/lib/cmake/ITK-5.4/Modules/ITKPNG.cmake \
  $(brew --prefix itk)/lib/cmake/ITK-5.4/Modules/ITKZLIB.cmake \
  $(brew --prefix itk)/lib/cmake/ITK-5.4/Modules/ITKExpat.cmake
do
  sed -i '' \
    's#/Library/Developer/CommandLineTools/SDKs/MacOSX26.sdk/usr/include#'"$(xcrun --show-sdk-path)"'/usr/include#g' \
    "$f"
done
```
(Re-apply after any `brew upgrade`/`reinstall` of itk.)

## 1. Build SlicerExecutionModel (SEM)

```bash
git clone https://github.com/Slicer/SlicerExecutionModel.git
cd SlicerExecutionModel && mkdir build && cd build
cmake -DITK_DIR="$(brew --prefix itk)/lib/cmake/ITK-5.4" \
      -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" ..
make -j$(sysctl -n hw.ncpu)
```

## 2. Build this repo

```bash
git clone https://github.com/raphaelcalmon/DSC_Analysis.git
cd DSC_Analysis && mkdir build && cd build
cmake \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
  -DITK_DIR="$(brew --prefix itk)/lib/cmake/ITK-5.4" \
  -DSlicerExecutionModel_DIR=/path/to/SlicerExecutionModel/build \
  ..
make -j$(sysctl -n hw.ncpu)
```

Binary lands at `DSC_Analysis/build/CLI/bin/DSCMRIAnalysis`.

## 3. Prepare input data (DICOM → NRRD with required metadata)

The tool needs a 4D volume tagged with per-frame timing, TE, and flip angle
(`MultiVolume.*` fields) — a plain NIfTI/NRRD from a converter won't have these.

```bash
dcm2niix -e y -f perf -o /output/dir /path/to/dicom/series
./add_multivolume_fields.sh /output/dir/perf.nhdr /output/dir/perf_ready.nhdr
```

(`add_multivolume_fields.sh` is included in this repo — it reads TR/TE/FlipAngle and
frame count directly out of the dcm2niix header and appends the fields DSCMRIAnalysis
requires. No manual edits needed per scan.)

## 4. Run

```bash
DSC_Analysis/build/CLI/bin/DSCMRIAnalysis \
  --usePopAif \
  --outputCBF cbf.nii.gz --outputAUC cbv.nii.gz --outputMTT mtt.nii.gz \
  perf_ready.nhdr
```

Use `--aifMask <mask.nii.gz>` instead of `--usePopAif` for a patient-specific arterial
input function drawn from your own images (e.g. in ITK-SNAP).

## Notes on this fork's changes vs. upstream

- `CMakeLists.txt`: replaced Slicer-extension-only build with a standalone path
  (`find_package(SlicerExecutionModel)` + `find_package(ITK)` directly).
- `CLI/DSCMRIAnalysis.cxx`: removed dead `itkMultiThreader.h` include (ITK5 removed it);
  updated `ImageIOBase::IOPixelType`/`IOComponentType` and enum values to ITK5's
  `itk::IOPixelEnum`/`itk::IOComponentEnum`.
- `CLI/itkPluginUtilities.h`, `CLI/itkPluginFilterWatcher.h`, `CLI/Configuration.h`:
  copied from PkModeling (this module's original parent extension) and patched for
  ITK5 enum names — these generic helper headers were missing from the original
  DSC_Analysis checkout.
