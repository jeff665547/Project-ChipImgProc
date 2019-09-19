
Bright-Field Marker Detection
=============================

ArUco Marker Detection
======================

- This example code will help you build the Image-ArUco detection app for Banff and Yz01 chip.
- Input:
  1. A TIFF file of chip image (The raw image from SUMMIT). Its path is specified in `image_path` in the example code.
  2. A JSON file of ArUco database. It stores 250 of ArUco code with 6x6 bits size each. Its path is specified in `aruco_database_path` in the example code.
  3. An std::vector of arranged ArUco IDs. The order of ArUco IDs is from the top-left to the bottom-right of a chip ([row major](https://en.wikipedia.org/wiki/Row-_and_column-major_order)). It is specified in `aruco_ids` in the example code.
  4. Two TIFF files of the marker frame template and marker frame mask (Figure 5). Their paths are specified in  `marker_frame_template_path` and `marker_frame_mask_path` respectively.

    @image html aruco-mk-intro.png width=600px
    @image latex aruco-mk-intro.png

    @image html marker-frame-tiff.png width=600px
    @image latex marker-frame-tiff.png
    Figure 5 Marker Recognition Principle. The marker frame template and the marker frame mask are used to identify the marker locations within an FOV roughly.
- Output:
  - A collection of detected ArUco IDs and their xy-positions in pixels.
- Example:
  - Data preparation for ArUco marker detection
    @snippet Example/aruco_detection.cpp data_preparation
  - ArUco Detection
    - Detect and decode the ArUco marker in the chip image one by one.
    - This process will be repeated until all ArUco markers in a FOV are detected, and the detection details are described as follows:
        1. Identify the most similar region to the marker frame in an FOV by using marker frame template (`marker_frame_template`) and marker frame mask (`marker_frame_mask`), and take that region as the true marker position.
        2. Decode the ArUco marker in the marker frame found in the previous step, and store the decoding result into a collection.
        3. To find the position of the next marker frame more accurately, we take the current marker as the center, and exclude an user-defined circular region indicating the non-optimal matching locations.
        4. Repeat step 1 - step 3 to detect the next marker frame in the chip image with the remaining regions until all ArUco markers in an FOV are detected.

        @snippet Example/aruco_detection.cpp aruco_detection
    - Parameters:
      - ArUco database:

        Database of 250 ArUco code with size 6x6. Illustrated in the figure below.

        @image html aruco-database.png width=540px
        @image latex aruco-database.png width=13cm
        Figure: The relationship between the ArUco marker and the database. (A cell in the ArUco marker is equivalent to a bit here.)

      - Pyramid level:

        The "Level" parameter (unit: counting numbers) means the number of iterations of the pyramid down-smapling process. This parameter is used to speed up the ArUco marker localization. For more information on this algorithm, see [here](https://en.wikipedia.org/wiki/Pyramid_(image_processing)).
      - Border width:

        The distance (unit: a side length of a cell) between coding region of ArUco and marker frame template. (Illustrated as "b" in figure 6)
      - Fringe width:

        The width (unit: a side length of a cell) of marker frame template. (Illustrated as "f" in figure 6, dark blue area.)
      - Bits width:

        The width (unit: pixel) of a cell. (Illustrated as "p" in figure 6)
      - Margin size:

        The width (unit: pixel) of the marker frame mask. (Illustrated as "m" in figure 6, light blue area.)
      - Marker frame template:

        The dark blue area in the figure 6. It is loaded from a raw chip image with the use of OpenCV.
      - Marker frame mask:

        The light blue area in the figure 6. It is loaded from a raw chip image with the use of OpenCV.
      - Number of marker counts:

        The maximum number of counts (unit: counting numbers) of ArUco markers in an FOV. For example, the number of marker counts is nine for the Yz01 and Banff chip.
      - Length of radius:

        The minimum distance (unit: pixel) between each ArUco markers. That means there are no other ArUco markers of interest presenting in the circular region illustrated in the figure 7. The radius of the circular region is user-specified.
      - Cell size:

        The width (unit: pixel) of the binary determination region. (Illustrated as "s" in figure 6)
      - ArUco IDs:

        The vector of ArUco IDs.
      - Logger:

        Log output.

    @image html aruco-single-mk-spec.png width=540px
    @image latex aruco-single-mk-spec.png width=13cm
    Figure 6 Marker Pattern Description and Corresponding Auxiliary Parameters for Recognition. (A cell is equivalent to a bit here.)
    @image html aruco-radius-define.png width=650px
    @image latex aruco-radius-define.png width=13cm
    Figure 7 The radius of the circular region between markers in an FOV (the blue rectangle).
  
  - Output Files
    @snippet Example/aruco_detection.cpp output
