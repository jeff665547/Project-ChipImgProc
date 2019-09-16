
Image Feature Extraction
========================

[TOC]

In this section, we determine the intensity of a feature probe by the most representative region defined by the minimum coefficient of variation (minCV) criterion in that cell. The size of the representative region is user-defined.

Extraction method
==================

Extraction method for Zion, Banff, and Yz01 chips

1. Let @f$X_{ij}@f$ be the intensity pixel value of the @f$i_{th}@f$ row and the @f$j_{th}@f$ column in a cell, @f$\mathbb \mu = \frac{\sum_{i,j} X_{ij}}{mn}@f$,  @f$\mathbb \sigma^2 = \frac{\sum_{i,j} (X_{ij}-\mu)^2}{mn-1}@f$,  coefficient of variation @f$CV = \frac{\sigma}{\mu}@f$, where m is the number of rows (pixel counts) in a given size region, n is the number of columns (pixel counts) in a given size region.
2. For each cell, minimize CV subject to all the regions in that cell.
  @image html image-margin.jpg width=670px
  @image latex image-margin.jpg width=13cm
  Figure 8 The tile matrix of an FOV (Banff (AM1), Yz01(AM1)).

The following is a brief introduction on the code of the intensity extraction.

Data preparation for image feature extraction
=============================================

Data preparation input
----------------------

1. A grid line object generated from image_gridder in the image gridding section.
2. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
3. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.

Data preparation output
-----------------------

1. A tile matrix object (Figure 8).

Data preparation example
------------------------

@snippet Example/image_stitch_and_extraction.cpp image-feature-preparation

Feature extraction
==================

Feature extraction input
------------------------

1. A string that set the feature extracting mode. There are two modes can be chosen. If we use "auto_min_cv", that means we ignore the segmentation rate and find the minimum CV of this tile automatically. The default mode for feature extraction is "mid_seg" using only segmentation rate and reducing the tile from outside to inside.
2. A float that stand for the segmentation rate, that is, the percent of margin segmentation from outside to inside, only use in "mid_seg" mode. Default rate is 0.6.
3. A reference to the tile matrix.
4. A boolean value indicating whether segmented tile results replace the tile-matrix or not.
5. A null pointer.

Feature extraction output
-------------------------

1. A feature extracting result object.

Feature extraction example
--------------------------

@snippet Example/image_stitch_and_extraction.cpp image-feature-extraction
