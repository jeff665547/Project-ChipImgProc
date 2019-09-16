
Image gridding
==============

Gridding
========

Turning now to the brief introduction on image rotation

Input
-----

1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
2. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.
3. A modified marker region object. (The vacant markers are modified by `chipimgproc::marker::detection::reg_mat_infer()`)
4. A logger - Log output.
5. (Optional) Output file settings.

Output
------

1. A grid line object.

Example
-------

@snippet Example/image_rotation_and_gridding.cpp image_gridding
