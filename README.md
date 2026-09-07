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
 - itk
 - dcm2niix
 - itk-snap

```zsh
brew install itk dcm2niix itk-snap
```

**Patch a stale SDK path baked into Homebrew's ITK.** Homebrew's ITK bottle hardcodes an
absolute Command Line Tools SDK path in three CMake module files, which can mismatch your
system's actual SDK and cause `<cstring>`/`<cmath>`/etc. header errors. Fix once:

```zsh
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

```zsh
git clone https://github.com/Slicer/SlicerExecutionModel.git
cd SlicerExecutionModel
mkdir build && cd build
cmake -DITK_DIR="$(brew --prefix itk)/lib/cmake/ITK-5.4" \
      -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" ..
make -j$(sysctl -n hw.ncpu)
```

## 2. Build this repo

```zsh
git clone https://github.com/raphaelcalmon/DSC_Analysis.git
cd DSC_Analysis
mkdir build && cd build
```

```zsh
cmake \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
  -DITK_DIR="$(brew --prefix itk)/lib/cmake/ITK-5.4" \
  -DSlicerExecutionModel_DIR=/../../SlicerExecutionModel/build \
  ..
make -j$(sysctl -n hw.ncpu)
```

install 

```zsh
sudo make install
```


## 3. Prepare input data (DICOM → NRRD with required metadata)

The tool needs a 4D volume tagged with per-frame timing, TE, and flip angle
(`MultiVolume.*` fields) — a plain NIfTI/NRRD from a converter won't have these.

```zsh
dcm2niix -e y -o . . 
```


DSCMRIAnalysis requires  TR, TE, FlipAngle, and frame count information in a specific format,
the following code reads them directly out of the dcm2niix header and adjusts it. (No manual edits).

```zsh
#!/usr/bin/env zsh

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <.nhdr file>"
  exit 1
fi

in_file=$1
seriestime=${${1:r}##*.}

if grep "MultiVolume.FrameIdentifyingDICOMTagName:=AcquisitionTime" $in_file >/dev/null ; then 
  echo "$in_file file already adjusted for DSCMRIAnalysis"
  # exit 0
else
  # Identify Number of frames by the kind of the dimension / size
  dim_kinds=($(grep kinds $in_file))
  dim_sizes=($(grep sizes $in_file))
  index=$(echo ${dim_kinds[(i)list]})
  frames=$dim_sizes[$index]
  # frames=$(awk '/sizes/{print $5}' $in_file)
  TR=$(grep "DICOM_0018_0080" $in_file | cut -d "=" -f 2)
  TE=$(grep "DICOM_0018_0081" $in_file | cut -d "=" -f 2)
  FA=$(grep "DICOM_0018_1314" $in_file | cut -d "=" -f 2)

  frame_labels=$(awk -v n="$frames" -v tr="$TR" 'BEGIN{ for (i=0; i<n; i++) { printf "%s%.1f", (i==0 ? "" : ","), i*tr } }')

  sed -i '' '/DWMRI/,$d' $in_file

  cat << EOF >> $in_file
MultiVolume.FrameIdentifyingDICOMTagName:=AcquisitionTime
MultiVolume.FrameLabels:=${frame_labels}
MultiVolume.NumberOfFrames:=${frames}
MultiVolume.DICOM.EchoTime:=${TE}
MultiVolume.DICOM.FlipAngle:=${FA}
EOF
fi

echo "\nRun:\n"
echo "DSCMRIAnalysis --usePopAif --outputCBF cbf.nii --outputAUC cbv.nii $in_file\n"

echo "DSCMRIAnalysis --aifMask aif.nrrd --outputAUC aif.cbv.$seriestime.nii --outputCBF aif.cbf.$seriestime.nii $in_file\n"
```

## 4. Run

```zsh
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
