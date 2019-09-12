
Bright-Field Marker Detection
=============================

ArUco Marker Detection
======================

- This example code will help you build the Image-ArUco detection app for Banff and Yz01 chip.
- Input:
  1. A TIFF file of chip image (The raw image from SUMMIT). Its path is specified in variable `image_path` in the example code.
  2. A JSON file of ArUco database (A json database which stores 250 of ArUco code with 6x6 bits size each). Its path is specified in variable `aruco_database_path` in the example code.
  3. A collection of ArUco IDs arranged in an std::vector. The arrangement order of markers is from the top-left to the bottom-right of a chip. It is specified in variable `aruco_ids` in the example code.
  4. Two TIFF files for marker frame template and marker frame mask. Their paths are specified in variables `marker_frame_template_path` and `marker_frame_mask_path` respectively.
    @image html aruco-single-mk-spec.png width=600px
    @image latex aruco-single-mk-spec.png
    Figure 5 Marker Recognition Principle
      - The marker frame template and the marker frame mask are used to identify the marker locations within an FOV roughly.
- Output:
  - A collection of detected ArUco IDs and their xy-positions in pixel scale.
- Example:
  - Data preparation for ArUco marker detection
    @snippet Example/aruco_detection.cpp data_preparation
  - ArUco Detection
    - Detect and decode the ArUco marker in the chip image one by one.
    - This process will be repeated until all ArUco markers in a FOV are detected, and the detection details are discribed as follows:
        1. Identify the most similar region to the marker frame in an FOV by using marker frame template (`marker_frame_template`) and marker frame mask (`marker_frame_mask`), and take that region as the true marker position.
        2. Decode the ArUco marker in the marker frame found in the previous step, and store the decoding result into a collection.
        3. To find the position of the next marker frame more accurately, we take the current marker as the center, and use the non-maximum suppression (NMS) algorithm to exclude an user-defined circular region indicating the non-optimal matching locations.
        4. Detect the next marker frame in the chip image without the use of the excluded regions.
        @snippet Example/aruco_detection.cpp aruco_detection
    - Parameters:
      - ArUco database:

        Database of 250 ArUco code with size 6x6.
      - Pyramid level:

        A level for down-sampling which to speed up the ArUco marker detection.
      - Border bits:

        A bit distance between coding region of ArUco and  marker frame template. (Illustrated as "b" in Figure 6)
      - Fringe bits:

        A bit width of marker frame template. (Illustrated as "f" in Figure 6)
      - Bits width:

        A bit width in pixels. (Illustrated as "p" in Figure 6) (unit: pixel)
      - Margin size:

        A bit width of marker frame mask. (Illustrated as "m" in Figure 6)
      - Marker frame template:

        Marker frame inside pattern. It is used in the NMS algorithm.
      - Marker frame mask:

        Marker frame outside border. It is used in the NMS algorithm.
      - Number of marker counts:

        Maximum count of ArUco markers in an FOV.
      - Number of radius:

        Minimum distance between each ArUco markers. It is used in the NMS algorithm.
      - Cell size:

        A bit with of binary determination region. (Illustrated as "s" in Figure 6)
      - ArUco IDs:

        Vector of ArUco IDs.
      - Logger:

        Log output.

    @image html aruco-single-mk-spec.png width=540px
    @image latex aruco-single-mk-spec.png width=13cm
    Figure 6 Marker Pattern Description and Corresponding Auxiliary Parameters for Recognition. (A cell is equivalent to a bit here.)
    @image html aruco-radius-define.png width=650px
    @image latex aruco-radius-define.png width=13cm
    Figure 7 The radius used in NMS algorithm with markers in an FOV (the blue rectangle).
  
  - Output Files
    @snippet Example/aruco_detection.cpp output
