
Introduction
===

This C++ library is a chip image-processing library that helps user correct the chip images, and applies gridding process to extract the biological messages of each probe on microarray.

Centrillion Technologies produce a series of chips, such as Banff, ZION, YZ01, etc. Each sample-hybridized chip will be arranged on the 384 or 96 chip tray and scanned by SUMMIT Reader. During the process of scanning, due to the view field size of the microscope, the chip will be separated into several parts to be scanned. The image of each part is then called the field of view (FOV), and the amount of acquried FOVs is distinct from the types of chips. After SUMMIT Reader finishes the scanning process, the functions in this library will preliminarily correct the acquired FOVs, and all FOVs of a chip will then be stitched to produce a high-resolution image of a chip.

@image html tray-to-chip.png width=800px
@image latex tray-to-chip.png
Figure 1 From Chip Tray to Chip

@image html chip-to-aruco-marker.png width=550px
@image latex chip-to-aruco-marker.png
Figure 2 From Chip to Marker

@image html aruco-marker-spec.png width=400px
@image latex aruco-marker-spec.png
Figure 3 Marker Composition

SUMMIT Reader applies three different channels, bright field (BF), red light (red), and green light (green), to scan chips. Each channel will make functions in this library recognize its own marker pattern, which helps us locate the chip position in the FOV. In most cases, the ArUco markers are recognized in the BF channel [[Bright-Field Marker Detection](#bright-field-marker-detection)]; the AM3 markers are recognized in the red channel; and the AM1 markers are recognized in the green channel [[Fluorescence Marker Detection](#fluorescence-marker-detection)]. The patterns of the ArUco markers are distinct from marker to marker in a chip, but the AM1 and AM3 markers are consistent to each other with respect to its imaging channel.

On the other hand, because of the placement uncertainty of the chip, the scanning results often require a small-angle rotation calibration to correct the inclination of images. We perform this process with the estimation of the rotation angles judged by makers in each FOV [[Image Rotation Angle Estimation and Calibration](#image-rotation-angle-estimation-and-calibration)].

After finishing image correction, we will crop the image to discard the region of uninterest of the chip. We will also grid the image to build an index for each feature probe location [[Image Gridding](#image-gridding)].

Finally, the intensity of a feature probe is determined by the most representative region defined by the minimum coefficient of variation (minCV) criterion in that region whose size is user-defined, in the corresponding cell [[Image Feature Extraction](#image-feature-extraction)]. The overall workflow of this library is illustrated below.

@image html tutorial-flowchart.png width=600px
@image latex tutorial-flowchart.png

Figure 4 The workflow of the ChipImgProc library
