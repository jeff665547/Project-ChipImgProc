# ChipImgProc
## Outline
[TOC]

## Introduction
This C++ library is a chip image-processing library that helps user correct the chip images, and applies gridding process to extract the biological message of each probe.

Centrillion Technologies produce a series of chips, such as Banff, ZION, YZ01 etc. Each hybridized chip will be arranged on the chip tray and scanned by SUMMIT. During the process of scanning, due to the size of camera lens, the chip will be separated into several parts to be scanned. The image of each part is called the field of view (FOV), and the number of FOV is different from the types of chips. After SUMMIT finishes the scanning process, the functions in this library will preliminarily correct the FOVs, and all FOVs of a chip will then be stitched to produce a high-resolution image of a chip in the next library (Image Stitching).

![](https://i.imgur.com/wmcPRZy.png)
Figure 1 From Chip Tray to Chip

![](https://i.imgur.com/ZCvSzra.png =600x520)
Figure 2 From Chip to Marker

![](https://i.imgur.com/2B6f98E.png =400x480)
Figure 3 Marker Composition


Figure 4 Grid Lines, Cells and Probes

SUMMIT uses three different channels, bright field (BF), red light (red), and green light (green), to scan chips. Each channel will make functions in this library recognize its own marker pattern, which helps us locate the chip position in the FOV. In most cases, the ArUco marker is recognized on the BF channel; the AM3 marker is recognized on the red channel; and the AM1 marker is recognized on the green channel [[Marker Recognition](#pattern-recognition)]. The patterns of the ArUco markers are distinct from marker to marker, but the AM1 and AM3 markers are not. On the other hand, because of the systematic error caused by the imperfect system, the scanning results often require small-angle rotation calibration to correct the images. We perform this process with the estimation of the rotation angles judged by makers in each FOV [[Image Rotation](#image-rotation)].

After finishing image correction, we will crop the image to remove the noise surrounding the chips. We also grid the image to index each probe location [[Image Gridding](#image-gridding)]. Finally, the intensity of a probe is determined by the most stable region, defined as the minimum coefficient of variation (minCV) in that region whose size is user-defined, in the corresponding cell [[Intensity Extraction](#intensity-extraction)]. The overall workflow of this library is illustrated below.


```flow
st=>start: Image, Setting files
e=>end: Image Stitching
op=>operation: Marker Recognition (ARUCO, AM1, AM3)
op2=>operation: Rotation Angle Estimation
op3=>operation: Image Rotation
op4=>operation: Image Cropping
op5=>operation: Image Gridding
op6=>operation: Intensity Extraction (minCV)

st->op->op2->op3->op4->op5->op6->e
```
Figure 5 The workflow of ChipImgProc


## Quick Start
### Installation

### A Toy Example


### Where to next?

[Marker Recognition](#marker-recognition)
[ArUco Detection](#aruco-detection)
[AM1, AM3 Detection](#am1-am3-detection)
[Image Rotation](#image-rotation)
[Image Gridding](#image-gridding)
[Intensity Extraction](#intensity-extraction)






## Marker Recognition
- 
### ArUco Detection

- This example code will help you build the Image-ArUco detection app.
- Input:
    1. A TIFF file of chip image (The raw image from SUMMIT). Its path is specified in variable `raw_image_path` in the example code.
    2. A JSON file of ArUco database (A json database which stores 250 of ArUco code with 6x6 bits size each). Its path is specified in variable `aruco_database_path` in the example code.
    3. A collection of ArUco IDs arranged in an std::vector. The arrangement order of markers is from the top-left to the bottom-right of the chip. It is specified in variable `aruco_ids` in the example code.
    4. Two TIFF files for marker frame template and marker frame mask. Their paths are specified in variables `marker_frame_template_path` and `marker_frame_mask_path` respectively.
    
        ![](https://i.imgur.com/ZoQEm5M.png)
        
        Figure 6 Marker Recognition Principle 
        
        - The marker frame template and the marker frame mask are used to identify the marker locations within an FOV roughly.

- Output:
    - A collection of detected ArUco IDs and their xy-positions in pixel scale.
- Example:
    - Data Preparation
	    ```cpp=
    	  int main( int argc, char** argv )
	    {
	    /*
	     *  +========================+
	     *  | Image data preparation |
	     *  +========================+
	     */
	
	    //  Load raw chip image from path via OpenCV
	    auto raw_image = cv::imread( raw_image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );
	
	    /*
	     *  By default, OpenCV read the image with 8 bits RGB color format via cv::imread
	     *  We don't want OpenCV reduce our raw chip image to the depth with 8 bits only
	     *  So we give the parameter and set to:
	     * 
	     *      "cv::IMREAD_ANYCOLOR" for RGB color
	     *      "cv::IMREAD_ANYDEPTH" for depth with our raw image (16 bits)
	    */
	
	    /*
	     *  +========================+
	     *  | ArUco code preparation |
	     *  +========================+
	     */
	
	    //  Declare the ArUco ID and their arrangement order with raw chip image. 
	    //  The arrangement order of markers is from the top-left to the bottom-right of the chip.
	    
	    std::vector<std::int32_t> aruco_ids {
	        47, 48, 49, 05, 50, 51, 52,
	        40, 41, 42, 43, 44, 45, 46,
	        34, 35, 36, 37, 38, 39, 04,
	        28, 29, 03, 30, 31, 32, 33,
	        21, 22, 23, 24, 25, 26, 27,
	        15, 16, 17, 18, 19, 02, 20,
	        00, 01, 10, 11, 12, 13, 14
	    };
	
	    /*
	     *  The coordinate is mapping start at top-left between the raw chip image and the vector of ArUco ID
	     *  And each ArUco ID represente to an ArUco marker on the raw chip image
	     * 
	     *      Raw Chip Image      Vector of ArUco ID
	     *      +------------>      +------------>
	     *      | []   []   []      | 47   48   49
	     *      |                   |
	     *      V []   []   []      V 40   41   42
	     */
	
	    //  Load ArUco database
	    nlohmann::json aruco_database;
	    std::ifstream aruco_database_file_in( aruco_database_path );
	    aruco_database_file_in >> aruco_database;
	
	    //  Set json dictionary to node "DICT_6X6_250"
	    auto aruco_dict_6x6_250 = chipimgproc::aruco::Dictionary::from_json(
	        aruco_database["DICT_6X6_250"]
	    );
	
	    /*
	     *  DICT_6X6_250 means the dictionary of 250 ArUco code with size 6x6 in the json database
	     */
	
	    /*
	     *  +============================+
	     *  | Marker pattern preparation |
	     *  +============================+
	     */
	
	    //  Load marker frame images of both template and mask
	    auto marker_frame_template = cv::imread( marker_frame_template_path, cv::IMREAD_GRAYSCALE );
	    auto marker_frame_mask     = cv::imread( marker_frame_mask_path,     cv::IMREAD_GRAYSCALE );
	    ```
    - ArUco Detection
        - Detect the ArUco marker in the chip image.
        - 利用已知的 marker pattern 去 match 
        
        ```cpp=+
	    /*
	     *  +=================+
	     *  | ArUco detection |
	     *  +=================+
	     */
	
	    // Declare the ArUco marker detector 
	    chipimgproc::aruco::Detector detector;
	    detector.reset(
	        aruco_dict_6x6_250,     //  ArUco database
	        3,                      //  Pyramid level
	        1,                      //  Border bits (illustrated as "b" in Figure 7)
	        1,                      //  Fringe bits (illustrated as "f" in Figure 7)
	        13.4,                   //  Bits width (illustrated as "p" in Figure 7)
	        8.04,                   //  Margin size (illustrated as "m" in Figure 7)
	        marker_frame_template,  //  marker frame template
	        marker_frame_mask,      //  Marker frame mask
	        9,                      //  Number of marker counts
	        268,                    //  Number of radius
	        5,                      //  Cell size (illustrated as "s" in Figure 7)
	        aruco_ids,              //  VArUco IDs
	        std::cout               //  Logger
	    );

	    //  Detecting ArUco marker
	    auto detected_aruco_id_position = detector.detect_markers( raw_image, std::cout );
        ```
        - Parameter Settings
            - ArUco database:
                Database of 250 ArUco code with size 6x6.
			- Pyramid level:
			    A level for down-sampling which to speed up the ArUco marker detection.
			- Border bits:
			    A bit distance between coding region of ArUco and  marker frame template. (Illustrated as "b" in Figure 7)
			- Fringe bits:
			    A bit width of marker frame template. (Illustrated as "f" in Figure 7)
			- Bits width:
			    A bit width of each pixels. (Illustrated as "p" in Figure 7)
			- Margin size:
			    A bit width of marker frame mask. (Illustrated as "m" in Figure 7)
			- Marker frame template:
			    Marker frame inside pattern.
			- Marker frame mask:
			    Marker frame outside border.
			- Number of marker counts:
			    Maximum count of ArUco markers in an FOV.
			- Number of radius:
			    Minimum distance between each ArUco markers.
			- Cell size:
			    A bit with of binary determination region. (Illustrated as "s" in Figure 7)
			- ArUco IDs:
			    Vector of ArUco IDs.
			- Logger:
			    Log output.

        - ![](https://i.imgur.com/HSJ8cuw.png)
          Figure 7 Marker Pattern Description and Corresponding Auxiliary Parameters for Recognition.
        - ![](https://i.imgur.com/2MUd033.png)
          Figure 8 The radius used in NMS algorithm with markers in an FOV (the blue rectangle).

    - Output Files
		```cpp=+
		    /*
		     *  +===================+
		     *  | Output ArUco code |
		     *  +===================+
		     */
		    
		    // Outputing
		    for( auto& [ id, position ] : detected_aruco_id_position )
		        std::cout << id << '\t' << position << std::endl;
		
		    return 0;
		}
		```