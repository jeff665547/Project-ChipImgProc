
Image rotation angle estimation and calibration
===============================================

[TOC]

The estimation method of the rotation angle for Zion, Banff and Yz01 chip is illustrated as follows.

@image html rotation-estimation.png width=540px
@image latex rotation-estimation.png width=13cm

- @f$N_h@f$ is the number of the blue line. (In this example @f$N_h = 3@f$ )
- @f$N_v@f$ is the number of the orange line. (In this example @f$N_v = 3@f$ )
- @f$\mathbb E[\theta_h]@f$ is the average of all @f$\theta_h@f$s that are computed from the blue lines in an FOV. (In this example, there are three @f$\theta_h@f$s.)
- @f$\mathbb E[\theta_v]@f$ is the average of all @f$\theta_v@f$s that are computed from the orange lines in an FOV. (In this example, there are three @f$\theta_v@f$s.)

The following is a brief introduction on the code of the image rotation.

Estimate the rotation angle
===========================

Estimation input
----------------

1. A marker region object that is generated from `chipimgproc::marker::detection::RegMat marker_detector` in the AM1, AM3 marker detection section.
2. A logger - Log output.

Estimation output
-----------------

1. The rotation angle (degree).

Estimation example
------------------

@snippet Example/image_rotation_and_gridding.cpp angle_detection

Rotate the Image
================

Rotation input
--------------

1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
2. The rotation angle (degree).

Rotation output
---------------

It doesn't return anything; rather it rotates and modfies the original image.

Rotation Example
----------------

@snippet Example/image_rotation_and_gridding.cpp image_rotate

Refine marker position
======================

Re-detect the position of the marker to get the corrected position of the marker after calibrating the image.

Refinement input
----------------

1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
2. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.
3. Marker detection with pixel unit (PX) or cell unit (CELL). If we use `chipimgproc::MatUnit::PX` (`chipimgproc::MatUnit::CELL`), then the unit of the value we use to detect marker is PX (CELL), and the output unit of this function will be PX (CELL).
4. An integral specifying perfect marker pattern mode. Default mode is to use perfect marker pattern, that is, the marker pattern we set should perfectly match the chip image, set as 0.
5. A logger - Log output.

Refinement output
-----------------

1. A marker region object.

Refinement example
------------------

@snippet Example/image_rotation_and_gridding.cpp image_marker_refine

Vacant markers inference
========================

Infer the position of the vacant markers that cannot be recognized by our previous functions.

Inference input
---------------

1. A marker region object that is generated from `chipimgproc::marker::detection::RegMat marker_detector`.
2. Number of markers in one marker row in an FOV.
3. Number of markers in one marker column in an FOV.
4. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.

Inference output
---------------

1. A modified marker region object. (fill the vacant marker positions)

Inference example
-----------------

@snippet Example/image_rotation_and_gridding.cpp vacant_markers_inference
