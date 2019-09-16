
Fluorescence Marker Detection
=============================

This example will help you to build the fluorescence marker detection app for Zion, Banff, Yz01 chips.

Data preparation for the fluorescence marker detection
======================================================

Fluorescence marker input
-------------------------

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

Fluorescence marker output
--------------------------

- Rotated image with gridded-line.

Fluorescence marker example
---------------------------

@snippet Example/image_rotation_and_gridding.cpp image_data_preparation

Parameters
----------

- Marker:
    Marker pattern loaded from tsv marker pattern file.
- Mask:
    Marker mask pattern loaded from tsv marker pattern file which set the ignore region for marker detection.
- Height (µm) of cell:
    The height (µm) of one cell (a probe).
- Width (µm) of cell:
    The width (µm) of one cell (a probe).
- Spacing µm of cell:
    µm between cell and cell (a probe and a probe). The white line between two cells in the figure 6.
- Row number of markers:
    Number of markers in one row.
- Column number of markers:
    Number of markers in one column.
- Spacing x of marker:
    The spacing (cell counts) between first top-left cell (probe) of each marker in X axis.
- Spacing y of marker:
    The spacing (cell counts) between first top-left cell (probe) of each marker in Y axis.
- µm to pixel:
    A magic number which is dependent with the reader (SUMMIT), this parameter is using to convert the µm to pixel.

Parameters for different chips
------------------------------

|  Parameters  | Zion | Banff | Yz01 |
|:------------:|:----:|:-----:|:----:|
| Row µm of cell | 9 | 4 | 3 |
| Column µm of cell | 9 | 4 | 3 |
| Spacing µm of cell | 2 | 1 | 1 |
| Row number of marker | 3 | 3 | 3 |
| Column number of marker | 3 | 3 | 3 |
| Spacing x of marker | 37 | 81 | 101 |
| Spacing y of marker | 37 | 81 | 101 |

Parameters for different reader
-------------------------------

| Parameters | DVT-1 | DVT-2 |  DVT-3 |
|:----------:|:-----:|:-----:|:------:|
| µm to pixel | 2.68 |  2.68 | 2.4145 |

Marker Detection
================

Marker detection input
----------------------

1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV.
2. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function.
3. Marker detection with pixel unit (PX) or cell unit (CELL). If we use `chipimgproc::MatUnit::PX` (`chipimgproc::MatUnit::CELL`), then the unit of the value we use to detect marker is PX (CELL), and the output unit of this function will be PX (CELL).
4. An integral specifying perfect marker pattern mode. Default mode is to use perfect marker pattern, that is, the marker pattern we set should perfectly match the chip image, set as 0.
5. A logger - Log output.

Marker detection output
-----------------------

1. A marker region object.

Marker detection example
------------------------

@snippet Example/image_rotation_and_gridding.cpp marker_detection
