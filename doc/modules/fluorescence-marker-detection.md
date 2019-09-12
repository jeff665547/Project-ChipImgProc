
Fluorescence Marker Detection
=============================

This example will help you to build the fluorescence marker detection app for Zion, Banff, Yz01 chips.

Data preparation for the fluorescence marker detection
======================================================

Input
-----

- A TIFF file of the raw image from SUMMIT. Its path is specified in variable `image_path` in the example code.
- A TSV file of marker pattern
  - ZION  (AM1)
    @code
    . . . . . . . . . .
    . . . . . . . . . .
    . . X X . . X X . .
    . . X X . . X X . .
    . . . . X X . . . .
    . . . . X X . . . .
    . . X X . . X X . .
    . . X X . . X X . .
    . . . . . . . . . .
    . . . . . . . . . .
    @endcode
  - ZION  (AM3)
    @code
    . . . . . . . . . .
    . . . . . . . . . .
    . . . . X X . . . .
    . . . . X X . . . .
    . . X X . . X X . .
    . . X X . . X X . .
    . . . . X X . . . .
    . . . . X X . . . .
    . . . . . . . . . .
    . . . . . . . . . .
    @endcode
  - Banff (AM1)
    @code
    . . . . . . . . . .
    . X X X . . X X X .
    . X . . . . . . X .
    . X . . . . . . X .
    . . . . . . . . . .
    . . . . . . . . . .
    . X . . . . . . X .
    . X . . . . . . X .
    . X X X . . X X X .
    . . . . . . . . . .
    @endcode
  - Banff (AM3)
    @code
    . . . . . . . . . .
    . . . . X X . . . .
    . . . . . . . . . .
    . . . . . . . . . .
    . X . . . . . . X .
    . X . . . . . . X .
    . . . . . . . . . .
    . . . . . . . . . .
    . . . . X X . . . .
    . . . . . . . . . .
    @endcode
  - YZ01  (AM1)
    @code
    . . . . . . . . . .
    . X X X . . X X X .
    . X . . . . . . X .
    . X . . . . . . X .
    . . . . . . . . . .
    . . . . . . . . . .
    . X . . . . . . X .
    . X . . . . . . X .
    . X X X . . X X X .
    . . . . . . . . . .
    @endcode
  - YZ01  (AM3)
    @code
    . . . . . . . . . .
    . . . . X X . . . .
    . . . . . . . . . .
    . . . . . . . . . .
    . X . . . . . . X .
    . X . . . . . . X .
    . . . . . . . . . .
    . . . . . . . . . .
    . . . . X X . . . .
    . . . . . . . . . .
    @endcode

Output
------

- Rotated image with gridded-line.

Example
-------

