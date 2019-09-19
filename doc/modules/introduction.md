
Introduction
===

Centrillion Technologies produces a series of chips, such as Banff, ZION, YZ01, etc. Chips are arranged on the 384 or 96 chip tray (Figure 1) and scanned by SUMMIT Reader. During the process of scanning, due to the view field size of the microscope, the chip will be separated into several parts to be scanned. The image of each part is called a field of view (FOV), and the amount of acquired FOVs depends on the types of chips. After SUMMIT Reader finishes the scanning process, the functions in this library can be applied to process the images of acquired FOVs, and stitch all FOVs of a chip to produce a high-resolution image of a chip (Figure 2).

@image html tray-to-chip.png width=800px
@image latex tray-to-chip.png
Figure 1 The chip layout on a chip tray.

@image html chip-to-aruco-marker.png width=550px
@image latex chip-to-aruco-marker.png
Figure 2 A stitched image of a chip. The marker region is enlarged and highlighted.

@image html aruco-marker-spec.png width=400px
@image latex aruco-marker-spec.png
Figure 3 Marker Composition

SUMMIT Reader usually uses three different channels, bright field (BF), red light (red), and green light (green), to scan a chip. Functions in this library are used to recognize the marker pattern in accordance with each channel. In most cases, the ArUco markers are recognized in the BF channel [[Bright-Field Marker Detection](@ref doc/modules/bright-field-marker-detection.md)]; the AM3 markers are recognized in the red channel; and the AM1 markers are recognized in the green channel [[Fluorescence Marker Detection](@ref doc/modules/fluorescence-marker-detection.md)] (Figure 3). The patterns of the ArUco markers differ from marker to marker in a chip, but the AM1 and AM3 markers are consistent in corresponding channel.

In addition, the scanning results often require a small-angle rotation calibration to correct the orientation of images. The estimation of the rotation angles is judged by the relative position of makers in each FOV [[Image Rotation Angle Estimation and Calibration](@ref doc/modules/image-rotation-angle-estimation-and-calibration.md)].

After finishing image correction, one might want to crop the image to discard the uninterested region. The image is gridded and each feature is assigned to an index (x, y) according to the grid [[Image Gridding](@ref doc/modules/image-gridding.md)].

Finally, the intensity of a feature probe can be determined by the most representative region defined by the minimum coefficient of variation (minCV) criterion in that region whose size is user-defined, in the corresponding cell [[Probe Intensity Extraction](@ref doc/modules/probe-intensity-extraction.md)]. The overall workflow of a typical use of this library is illustrated below (Figure 4).

@image html tutorial-flowchart.png width=600px
@image latex tutorial-flowchart.png

Figure 4 A demonstrative workflow of the ChipImgProc library
