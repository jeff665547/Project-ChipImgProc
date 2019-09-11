# ChipImgProc
## Outline
[TOC]

## Introduction
This C++ library is a chip image-processing library that helps user correct the chip images, and applies gridding process to extract the biological messages of each probe on microarray.

Centrillion Technologies produce a series of chips, such as Banff, ZION, YZ01, etc. Each sample-hybridized chip will be arranged on the 384 or 96 chip tray and scanned by SUMMIT Reader. During the process of scanning, due to the view field size of the microscope, the chip will be separated into several parts to be scanned. The image of each part is then called the field of view (FOV), and the amount of acquried FOVs is distinct from the types of chips. After SUMMIT Reader finishes the scanning process, the functions in this library will preliminarily correct the acquired FOVs, and all FOVs of a chip will then be stitched to produce a high-resolution image of a chip.

![](https://i.imgur.com/wmcPRZy.png)
Figure 1 From Chip Tray to Chip

![](https://i.imgur.com/ZCvSzra.png =600x520)
Figure 2 From Chip to Marker

![](https://i.imgur.com/2B6f98E.png =400x480)
Figure 3 Marker Composition

SUMMIT Reader applies three different channels, bright field (BF), red light (red), and green light (green), to scan chips. Each channel will make functions in this library recognize its own marker pattern, which helps us locate the chip position in the FOV. In most cases, the ArUco markers are recognized in the BF channel [[Bright-Field Marker Detection](#bright-field-marker-detection)]; the AM3 markers are recognized in the red channel; and the AM1 markers are recognized in the green channel [[Fluorescence Marker Detection](#fluorescence-marker-detection)]. The patterns of the ArUco markers are distinct from marker to marker in a chip, but the AM1 and AM3 markers are consistent to each other with respect to its imaging channel.

On the other hand, because of the placement uncertainty of the chip, the scanning results often require a small-angle rotation calibration to correct the inclination of images. We perform this process with the estimation of the rotation angles judged by makers in each FOV [[Image Rotation Angle Estimation and Calibration](#image-rotation-angle-estimation-and-calibration)].

After finishing image correction, we will crop the image to discard the region of uninterest of the chip. We will also grid the image to build an index for each feature probe location [[Image Gridding](#image-gridding)].

Finally, the intensity of a feature probe is determined by the most representative region defined by the minimum coefficient of variation (minCV) criterion in that region whose size is user-defined, in the corresponding cell [[Image Feature Extraction](#image-feature-extraction)]. The overall workflow of this library is illustrated below.


![](https://i.imgur.com/El5iLaT.png =400x)

Figure 4 The workflow of the ChipImgProc library


## Getting Started

### Installation (From Visual Studio Code)
[Link](http://www.google.com)

### Installation (From Visual Studio 2019)

[Link](http://192.168.1.13:3000/CYUwzAZgrADAnARgLQHYDGAmYSAsM0BGSAhgQcUiAGwgAcGVZxOIGQA=?both)

### A Toy Example
Here is the toy example to help users get the hand of this library.
- Include the library
```cpp=
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include <ChipImgProc/margin.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/stitch/gridline_based.hpp>
#include "./make_layout.hpp"
```
- This is an example code for converting all the raw FOVs of Zion chip into the final heatmap.
```cpp=+
TEST(multi_tiled_mat, basic_test) {
        
    // This is an example using chipimgproc::MultiTiledMat.
    
    // First, we initialize the tools we need.
    chipimgproc::marker::detection::RegMat      reg_mat     ;  // for the fluorescence marker detection
    chipimgproc::rotation::MarkerVec<double>    marker_fit  ;  // for the rotation angle estimation
    chipimgproc::rotation::Calibrate            rot_cali    ;  // for the rotation calibration
    chipimgproc::gridding::RegMat               gridding    ;  // for the image gridding
    chipimgproc::Margin<double>                 margin      ;  // for the image feature extraction.

    /*
        Create the storage for processed FOV.
        We store the gridding result (tiled_mats) and feature extraction result (stat_mats_s)
        seperately.
    */
    std::vector<chipimgproc::TiledMat<>>           tiled_mats  ; 
    std::vector<chipimgproc::stat::Mats<double>>   stat_mats_s ;
    
    // Set the input path for each FOV.
    std::vector<boost::filesystem::path> test_img_paths ({
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-1-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-1-2.tiff"
    });

    for(std::size_t i = 0; i < test_img_paths.size(); i ++ ) {
        // Read each FOV image
        auto& img_path = test_img_paths[i];
        cv::Mat_<std::uint16_t>img = cv::imread(
            img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
        );

        // Create a ZION marker layout for the fluorescence marker detection and 
        // set the micron to pixel rate to 2.68.
        // Attention: The generation of the marker layout varies from the types of chips and SUMMIT readers. 
        // It depends on the particular specification.
        // For more info, you can go to the "Fluorescence Marker Detection" section in this tutorial 
        // or refer to  "unit_test/ChipImgProc/multi_tiled_mat_test.cpp"
        auto mk_layout = make_zion_layout(2.68);

        // Detect the markers
        auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // Estimate the image rotation angle (degree)
        auto theta = marker_fit(mk_regs, std::cout);

        // Rotate the image via estimated rotation angle (Image Calibration)
        rot_cali(img, theta);

        // Since the marker positions have been changed after the image calibration, 
        // we have to re-detect the marker positions.
        mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // Inference for the vacant marker positions.
        mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);

        // Grid the image and output the gridding result. 
        auto gl_res = gridding(img, mk_layout, mk_regs, std::cout);

        // Convert the gridding result into a tile-matrix (Figure 8).
        auto tiled_mat = chipimgproc::TiledMat<>::make_from_grid_res(gl_res, img, mk_layout);

        //  Set the parameters for feature extraction, 
        //  0.6 is the segmentation rate
        chipimgproc::margin::Param<> margin_param { 
            0.6, &tiled_mat, true, nullptr
        };

        // Image Feature Extraction (minCV).
        chipimgproc::margin::Result<double> margin_res = margin(
            "auto_min_cv", margin_param
        );

        // Save the gridding result
        tiled_mats.push_back(tiled_mat);

        // Save the segmentation result
        stat_mats_s.push_back(margin_res.stat_mats);
    }
    
    // Set the stitching positions of top-left marker start point for each FOV.
    // These parameters should be provided by chip & reader specification.
    // They vary from the types of chips.
    // This example only show the parameters of Zion.
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });

    // Set the FOV sequential ID, the order is row major. 
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {0, 1}, {1, 1}
    });

    // Create a matrix with multiple tiles (all FOV) for entire image.
    chipimgproc::MultiTiledMat<double, std::uint16_t> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps, fov_ids
    );

    /* 
        Generate results.
        In this example, we generate the mean value heatmap and the raw image stitching result.
    */

    // Create a empty heatmap with openCV
    cv::Mat heatmap;

    // Convert each tile (.tiff) into heatmap (integer) with openCV
    multi_tiled_mat.dump().convertTo(heatmap, CV_16U, 1);

    // Outputting heatmap data directly may cause an low intensity image which
    // looks like a black and invisable image.
    // To deal with this problem, before saving the image, 
    // we use the chipimgproc::viewable to calibrate the brightness of the image.
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(heatmap));

    // In the heatmap image, a pixel represents a cell, 
    // but we still need raw image to do further analysis.
    // Here, we stitch the images inside the multi_tiled_mat object into a real pixel-level image.
    chipimgproc::stitch::GridlineBased gl_stitcher;
    auto gl_st_img = gl_stitcher(multi_tiled_mat);

    // Again, to prevent from getting an invisible image,
    // we use the chipimgproc::viewable to calibrate the brightness of the image.
    cv::imwrite("stitch.tiff", chipimgproc::viewable(gl_st_img.mat()));
}
```

### Where to next?

Now that you have everything installed, and you have already had some basic knowledge on the operation of our program. See the following sections for further information if you needed it.

For information on detecting markers under the bright field, see [Bright-Field Marker Detection](#bright-field-marker-detection).

For information on detecting markers under the fluorescence, see [Fluorescence Marker Detection](#fluorescence-marker-detection).

For information on estimating the rotation angle and calibrating the chip image, see [Image Rotation Angle Estimation and Calibration](#image-rotation-angle-estimation-and-calibration).

For information on gridding the chip image, see [Image Gridding](#image-gridding).

For information on using minCV to define the probe intensity, see [Image Feature Extraction](#image-feature-extraction).

For information on stitching all FOVs into a complete chip image, see [Image Stitching](#image-stitching).




## Bright-Field Marker Detection

### ArUco Marker Detection

- This example code will help you build the Image-ArUco detection app for Banff and Yz01 chip.
- Input:
    1. A TIFF file of chip image (The raw image from SUMMIT). Its path is specified in variable `image_path` in the example code.
    2. A JSON file of ArUco database (A json database which stores 250 of ArUco code with 6x6 bits size each). Its path is specified in variable `aruco_database_path` in the example code.
    3. A collection of ArUco IDs arranged in an std::vector. The arrangement order of markers is from the top-left to the bottom-right of a chip. It is specified in variable `aruco_ids` in the example code.
    4. Two TIFF files for marker frame template and marker frame mask. Their paths are specified in variables `marker_frame_template_path` and `marker_frame_mask_path` respectively.
    
        ![](https://i.imgur.com/ZoQEm5M.png)
        
        Figure 5 Marker Recognition Principle 
        
        - The marker frame template and the marker frame mask are used to identify the marker locations within an FOV roughly.

- Output:
    - A collection of detected ArUco IDs and their xy-positions in pixel scale.
- Example:
    - Data preparation for ArUco marker detection
	    ```cpp=
	    /*
	     *  +========================+
	     *  | Image data preparation |
	     *  +========================+
	     */
	
	    //  Load raw chip image from path via OpenCV
	    auto raw_image = cv::imread( image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );
	
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
        - Detect and decode the ArUco marker in the chip image one by one.
        - This process will be repeated until all ArUco markers in a FOV are detected, and the detection details are discribed as follows: 
            1. Identify the most similar region to the marker frame in an FOV by using marker frame template (`marker_frame_template`) and marker frame mask (`marker_frame_mask`), and take that region as the true marker position.
            2. Decode the ArUco marker in the marker frame found in the previous step, and store the decoding result into a collection.
            3. To find the position of the next marker frame more accurately, we take the current marker as the center, and use the non-maximum suppression (NMS) algorithm to exclude an user-defined circular region indicating the non-optimal matching locations.
            4. Detect the next marker frame in the chip image without the use of the excluded regions.

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
			    A bit width of each pixels. (Illustrated as "p" in Figure 6) (unit: pixel)
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

        - ![](https://i.imgur.com/2EvpRcl.png)
          Figure 6 Marker Pattern Description and Corresponding Auxiliary Parameters for Recognition.
        - ![](https://i.imgur.com/2MUd033.png)
          Figure 7 The radius used in NMS algorithm with markers in an FOV (the blue rectangle).

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
		```
## Fluorescence Marker Detection

- This example code will help you to build the fluorescence marker detection app for Zion, Banff, Yz01 chips.
    - Data preparation for the fluorescence marker detection.
        - Input:
        	1. A TIFF file of the raw image from SUMMIT. Its path is specified in variable `image_path` in the example code.
        	2. A TSV file of marker pattern.
           - Marker (Marker pattern):
               - Zion:
					```cpp
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
					```
  	           - Banff Cy3 (AM1):
  	 	 			```cpp
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
  	 	 			```
            	- Banff Cy5 (AM3):
            	    ```cpp
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
            	    ``` 
            	- Yz01 Cy3 (AM1):
            	    ```cpp
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
            	    ```
             	- Yz01 Cy5 (AM5):
             	   ```cpp
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
             	   ```

    	- Output:
    	    1. Rotated image with gridded-line.
    	- Example
        ```cpp=
    	/*
    	 *  +========================+
    	 *  | Image data preparation |
    	 *  +========================+
    	 */

    	//  Load raw chip image from path via OpenCV
    	cv::Mat_<std::uint16_t> image = cv::imread( image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );
	
	    /*
	     *  By default, OpenCV read the image with 8 bits RGB color from cv::imread
	     *  We don't want OpenCV reduce our raw chip image to the depth with 8 bits only
	     *  So we give the parameter and set to:
	     * 
	     *      "cv::IMREAD_ANYCOLOR" for RGB color
	     *      "cv::IMREAD_ANYDEPTH" for depth with our raw image (16 bits)
         */

   		 /*
   		  *  +==============================+
   		  *  | Marker detection preparation |
   		  *  +==============================+
   		  */		
	
	    //  Load marker
	    std::ifstream marker_file_in( marker_pattern_path );
	    auto [ marker, mask ] = chipimgproc::marker::Loader::from_txt( marker_file_in, std::cout );
	
	    auto marker_layout = chipimgproc::marker::make_single_pattern_reg_mat_layout(
	        marker, //  Marker
	        mask,   //  Mask
	        4,      //  Row µm of cell
	        4,      //  Column µm of cell
	        1,      //  Spacing µm of cell
	        3,      //  Row number of markers
	        3,      //  Column number of markers
	        81,     //  Spacing x of marker
	        81,     //  Spacing y of marker
	        2.68    //  µm to pixel
	    );
	    ```
        - Parametes:
            - Marker:
                Marker pattern loaded from tsv marker pattern file.
            - Mask:
                Marker mask pattern loaded from tsv marker pattern file which set the ignore region for marker detection.
            - Row µm of cell:
                µm in the row for one cell (a probe).
            - Column µm of cell:
                µm in the column for one cell (a probe).
            - Spacing µm of cell:
                µm between cell and cell (a prboe and a probe). The white line between two cells in the figure 6.
            - Row number of markers:
                Number of markers in one row.
            - Column number of markers:
                Number of markers in one column.
            - Spacing x of marker:
                The spacing (cell counts) between first top-left cell (probe) of markers in X axis.
            - Spacing y of marker:
                The spacing (cell counts) between first top-left cell (probe) of markers in Y axis.
            - µm to pixel:
                A magic number which is dependent with the reader (SUMMIT), this parameter is using to convert the µm to pixel.

         -  Parameters for different chips:
         
		    |       Parameters  | Zion | Banff | Yz01 |
		    |     :------------:|:----:|:-----:|:----:|
		    |      Row µm of cell | 9 | 4 | 3 |
		    |      Column µm of cell | 9 | 4 | 3 |
		    |      Spacing µm of cell | 2 | 1 | 1 |
		    |      Row number of marker | 3 | 3 | 3 |
		    |      Column number of marker | 3 | 3 | 3 |
		    |      Spacing x of marker | 37 | 81 | 101 |
		    |      Spacing y of marker | 37 | 81 | 101 |
		    
     	  - Parameters for different reader
     	  
     		    |      Parameters | DVT-1 | DVT-2 |  DVT-3 |
		    |     :----------:|:-----:|:-----:|:------:|
		    |      µm to pixel | 2.68 |  2.68 | 2.4145 |

    - Marker Detection
        - Input:
        	1.  A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV.
        	2.  A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function.
        	3.  Marker detection with pixel unit (PX) or cell unit (CELL). If we use `chipimgproc::MatUnit::PX` (`chipimgproc::MatUnit::CELL`), then the unit of the value we use to detect marker is PX (CELL), and the output unit of this function will be PX (CELL).
        	4.  An integral specifying perfect marker pattern mode. Default mode is to use perfect marker pattern, that is, the marker pattern we set should perfectly match the chip image, set as 0.
        	5.  A logger - Log output.
        - Output:
        	1. A marker region object.
        - Example
        ```cpp=+
    	/*
    	 *  +=====================================+
    	 *  | Marker detection and image rotation |
    	 *  +=====================================+
    	 */
	
	    // Declare the marker detector and image detector
	    chipimgproc::marker::detection::RegMat marker_detector;
	    chipimgproc::rotation::MarkerVec<float> theta_detector;
	
	    // Declare the image rotator and image gridder
	    chipimgproc::rotation::Calibrate image_rotator;
	    chipimgproc::gridding::RegMat image_gridder;
	    
	    // Detecting marker
	    auto marker_regioins = marker_detector(
	        image,
	        marker_layout,
	        chipimgproc::MatUnit::PX,   //  Marker detection mode 
	        0,                          //  Perfect marker pattern mode
	        std::cout
	        );
        ```

## Image Rotation Angle Estimation and Calibration
- The estimation method of the rotation angle for Zion, Banff and Yz01 chip is illustrated as follows.
  ![](https://i.imgur.com/26SXFYl.png)
    - $N_h$ is the number of the blue line. (In this example $N_h = 3$ )
    - $N_v$ is the number of the orange line. (In this example $N_v = 3$ )
    - $\mathbb E[\theta_h]$ is the average of all $\theta_h$s that are computed from the blue lines in an FOV. (In this example, there are three $\theta_h$s.)
    - $\mathbb E[\theta_v]$ is the average of all $\theta_v$s that are computed from the orange lines in an FOV. (In this example, there are three $\theta_v$s.)


- The flollowing is a brief introduction on the code of the image rotation.
    - Estimate the rotation angle.
        - Input:
            1. A marker region object that is generated from `chipimgproc::marker::detection::RegMat marker_detector` in the AM1, AM3 marker detection section.
            2. A logger - Log output.

        - Output:
            1. The rotation angle (degree).
            
        - Example
			```cpp=+
			// Detecting image rotation theta (degree)
			auto theta = theta_detector( marker_regioins, std::cout );
			// Outputing
	 	   std::cout << theta << std::endl;
	 	   ```
        
    - Rotate the Image.
        - Input:
            1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
            2. The rotation angle (degree).
        - Output:
            It doesn't return anything; rather it rotates and modifies the original image.
        - Example
        ```cpp=+
        // Rotating the image via detected theta (degree)
	    image_rotator( image, theta );
        ```
        
    - Redetect the position of the marker to get the corrected position of the marker after calibrating the image.
        - Input:
            1. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
            2. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.
            3. Marker detection with pixel unit (PX) or cell unit (CELL). If we use `chipimgproc::MatUnit::PX` (`chipimgproc::MatUnit::CELL`), then the unit of the value we use to detect marker is PX (CELL), and the output unit of this function will be PX (CELL).
            4. An integral specifying perfect marker pattern mode. Default mode is to use perfect marker pattern, that is, the marker pattern we set should perfectly match the chip image, set as 0.
            5. A logger - Log output.
        - Output:
            1. A marker region object.
        - Example
        ```cpp=+
	    // Re-detecting the marker
	    marker_regioins = marker_detector( image, marker_layout, chipimgproc::MatUnit::PX, 0, std::cout );
	    ```
        
    - Infer the position of the vacant markers that cannot be recognized by our previous functions. 
        - Input:
            1. A marker region object that is generated from `chipimgproc::marker::detection::RegMat marker_detector`.
            2. Number of markers in one marker row in an FOV.
            3. Number of markers in one marker column in an FOV.
            4. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
        - Output:
            1. A modified marker region object. (fill the vacanct marker positions)
        - Example
        ```cpp=+
	    //  Auto-inference to fill the vacanct marker positions
	    marker_regioins = chipimgproc::marker::detection::reg_mat_infer(
	        marker_regioins,
	        3,                  //  Number of markers in one row in an FOV.
	        3,                  //  Number of markers in one column in an FOV.
	        image
	        );
	    ```
        
## Image Gridding      
- Turning now to the brief introduction on the image rotation.
    - Image Gridding
        - Input:
            1.  A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
            2.  A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.
            3.  A modified marker region object. (The vacanct markers are modified by `chipimgproc::marker::detection::reg_mat_infer`)
            4.  A logger - Log output.
            5.  (Optional) Output file settings.
        - Output:
            1. A grid line object.
        - Example
        ```cpp=+
	    /*
	     *  +================+
	     *  | Image gridding |
	     *  +================+
	     */
	
	    //  Image gridding and output the result via lambda expression
	    auto grid_line = image_gridder( image, marker_layout, marker_regioins, std::cout, [](const auto& m){
	        cv:imwrite( "grid_line.tiff", m );
	    });
        ```
        - The output file name is "grid_line.tiff".


## Image Feature Extraction
- In this section, we determine the intensity of a feature probe by the most representative region defined by the minimum coefficient of variation (minCV) criterion in that cell. The size of the representative region is user-defined.
- Extraction method for Zion, Banff, and Yz01 chips.
  
  1. Let $X_{ij}$ be the intensity pixel value of the $i_{th}$ row and the $j_{th}$ column in a cell, $\mathbb \mu = \frac{\sum_{i,j} X_{ij}}{mn}$,  $\mathbb \sigma^2 = \frac{\sum_{i,j} (X_{ij}-\mu)^2}{mn-1}$,  coefficient of variation $CV = \frac{\sigma}{\mu}$, where m is the number of rows (pixel counts) in a given size region, n is the number of columns (pixel counts) in a given size region.
  2. For each cell, minimize CV subject to all the regions in that cell.
  
    ![](https://i.imgur.com/xaPnVgK.jpg)


    Figure 8 The tile matrix of an FOV (Banff (AM1), Yz01(AM1)).
- The flollowing is a brief introduction on the code of the intensity extraction.
    - Data preparation for image feature extracting
        - Input:
            1. A grid line object generated from image_gridder in the image gridding section.
            2. A loaded raw chip image (The raw image from SUMMIT). It is loaded by OpenCV in the AM1, AM3 marker detection section.
            3. A marker layout object generated from `chipimgproc::marker::make_single_pattern_reg_mat_layout()` function in the AM1, AM3 marker detection section.
        - Output:
            1. A tile matrix object (Figure 8).
        - Example
			```cpp=
			/*
			 *  +==========================+
			 *  | Image feature extracting |
			 *  +==========================+
			 */
			//  Convert the gridding result into a tile-matrix
            auto tile_matrix = chipimgproc::TiledMat<>::make_from_grid_res( grid_line, image, marker_layout );
			```
    - Feature Extraction
        - Input:
            1. A string that set the feature extracting mode. There are two modes can be chosen. If we use "auto_min_cv", that means we ignore the segmentation rate and find the minimum CV of this tile automatically. The default mode for feature extraction is "mid_seg" using only segmentation rate and reducing the tile from outside to inside
            2. A float that stnad for the segmentation rate, that is, the percent of margin segmentation from outside to inside, only use in "mid_seg" mode. Default rate is 0.6.
            3. A reference to the tile matrix.
            4. A boolean value indicating whether segmented tile results replace the tile-matrix or not.
            5. A null pointer.
        - Output:
            1. A feature extracting result object.
        
        - Example:
            ```cpp=+
            //  Set the parameters for feature extraction
            chipimgproc::margin::Param<> feature_extraction_param { 
                0.6,            //  Segmentation rate
                &tile_matrix,
                true,           //  Tile replacement
                nullptr
            };
            
            //  Feature extraction
        	chipimgproc::margin::Result<double> feature_extraction_result = margin(
        	    "auto_min_cv",              //  Feature extracting mode
        	    feature_extraction_param
        	);
            ```
        

## Image Stitching
- After finishing gridding all FOVs, the final section of this tutorial addresses ways of stitching all FOVs into a complete chip image for Zion, Banff, and Yz01 chips.
    - Data preparation for image stitching
        - Input:
            1. A tile matrix object after finising feature extracting in the image feature extraction section.
            2. A segmentation matrix object storing the feature extraction result object generated in the image feature extraction section.
            3. A vector of stitching points {x, y} identifying all the stitching positions in the while chip.
            4. A vector of xy-index for each FOV in the whole chip, the index order is row major.
            
        - Output:
            None
        - Example (Banff):
		```cpp=
		/*
         *  +======================+
         *  | FOV images stitching |
         *  +======================+
         */
    
    	//  Set stitch positions of top-left marker start point for each FOV
    	std::vector<cv::Point_<int>> stitch_positions({
    	    {0,   0}, {162,   0}, {324,   0},
    	    {0, 162}, {162, 162}, {324, 162},
    	    {0, 324}, {162, 324}, {324, 324}
    	});

        /*
     	 *  The point (x,y) of stitch positions are base on the spacing of the markers, for example:
     	 * 
     	 *      $ Spacing (x,y) of markers for Banff are both "81"
     	 * 
     	 *      $ Banff images have nine FOV, and each FOV contain three markers at both X-axis and Y-axis
     	 * 
     	 *      $ The overlapping between two FOV of Banff images are both one column and one row of markers
     	 * 
     	 *      $ Example of entire Banff image:
     	 * 
     	 *               0    81   162  243  324  405  486
     	 *              +--------------------------------+
     	 *            0 |[]   []   []   []   []   []   []|      [] are presenting as markers
     	 *              |                                |
     	 *           81 |[]   []   []   []   []   []   []|
     	 *              |                                |
     	 *          162 |[]   []   []   []   []   []   []|
     	 *              |                                |
     	 *          243 |[]   []   []   []   []   []   []|
     	 *              |                                |
     	 *          324 |[]   []   []   []   []   []   []|
     	 *              |                                |
     	 *          405 |[]   []   []   []   []   []   []|
     	 *              |                                |
     	 *          486 |[]   []   []   []   []   []   []|
     	 *              +--------------------------------+
     	 *                      Entire Banff Image
     	 * 
     	 *      $ Example of overlapping and stitching at three FOVs:
     	 * 
     	 *         StitchingPoint       StitchingPoint      StitchingPoint
     	 *               |                    |                   |
     	 *               |  OverlappingMarker | OverlappingMarker |  OverlappingMarker
     	 *               |          |         |         |         |          |
     	 *               V          V         V         V         V          V
     	 *               0    81   162       162  243  324       324   405  486
     	 *              +------------+      +------------+       +------------+
     	 *            0 |[]   []   []|      |[]   []   []|       |[]   []   []|
     	 *              |            |      |            |       |            |
     	 *           81 |[]   []   []|      |[]   []   []|       |[]   []   []|
     	 *              |            |      |            |       |            |
     	 *          162 |[]   []   []|      |[]   []   []|       |[]   []   []|
     	 *              +------------+      +------------+       +------------+
     	 *                 FOV(0,0)            FOV(1,0)             FOV(2,0)
     	 */
         //  Set the FOV sequencial ID, the order is row major
    	std::vector<cv::Point> fov_ids({
    	    {0, 0}, {1, 0}, {2, 0},
    	    {0, 1}, {1, 1}, {2, 1},
    	    {0, 2}, {1, 2}, {2, 2}
    	});
	
	    //  Create a matrix with multiple tiles (all FOV) for entire image
	    chipimgproc::MultiTiledMat<double, std::uint16_t> multiple_tiled_matrix(
	        tile_matrixs,
	        segmentation_matrixs,
	        stitch_positions,
	        fov_ids
	    );

		```
    - FOV images stitching
        - Input:
            1. An data structure containing the four input arguments specified in data preparation for image stitching.
        - Output:
            1. A stitched image object.
        - Example:
        ```cpp=+
        //  Set stitcher and stitch FOVs in real pixel level of image
    	chipimgproc::stitch::GridlineBased stitcher;
    	auto stitched_image = stitcher( multiple_tiled_matrix );
        ```