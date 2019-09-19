
Image Stitching
===============

After finishing extracting features, the final section of this tutorial addresses ways of stitching all FOVs into a complete chip image for Zion, Banff, and Yz01 chips.

Data Preparation for Image Stitching
====================================

Data preparation input
----------------------

1. A tile matrix object by finishing feature extracting in the [image feature extraction](@ref doc/modules/image-feature-extraction.md) section.
2. A segmentation matrix object storing the feature extraction result object generated  in the [image feature extraction](@ref doc/modules/image-feature-extraction.md)
3. A vector of stitching points {x, y} identifying all the stitching positions in the whole chip. (Demonstrated in the exmple code.)
4. A vector of xy-index for each FOV in the whole chip, the index order is row major, and it started from the top left.

Data preparation output
-----------------------

None

Data preparation example
------------------------

@snippet Example/image_stitch_and_extraction.cpp image-multi_tiled-stitch

Data FOV Images Stitching
=========================

Data FOV images input
---------------------

1. An data structure containing the four input arguments specified in data preparation for image stitching.

Data FOV images output
----------------------

1. A stitched image object.

Data FOV images example
-----------------------

@snippet Example/image_stitch_and_extraction.cpp image-stitching
