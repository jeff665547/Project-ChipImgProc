/**
 *  @file       ChipImgProc/aruco.hpp
 *  @author     Chia-Hua Chang
 *  @brief      ArUco marker detection module.
 *  @details    To detect ArUco marker in the image, there are serval material have to prepared:
 * 
 *  1. ArUco marker database, and candidate marker id list
 *  2. Border template pattern image.
 *  3. Border template mask pattern image.
 *  4. Numerical specification, e.g. bit pixel width, margin size etc.
 * 
 *  The ArUco database JSON format is:
 * 
 *  @code
 *  {
 *      "dictionary name": {
 *        "coding_bits": <integer>
 *      , "maxcor_bits": <integer>
 *      , "bitmap_list": {
 *            "<marker id>": [
 *               <ArUco code in bit array>
 *            ]
 *          , "<marker id>": [
 *               <ArUco code in bit array>
 *            ]
 *          , ...
 *        }
 *      }
 *  }
 *  @endcode
 * 
 *  Here is an example:
 * 
 *  @code
 *  {
 *      "DICT_6X6_250": {
 *        "coding_bits": 6
 *      , "maxcor_bits": 5
 *      , "bitmap_list": {
 *            "0": [
 *                0, 0, 0, 1, 1, 1
 *              , 1, 0, 0, 0, 1, 1
 *              , 1, 1, 0, 1, 1, 1
 *              , 0, 1, 1, 0, 0, 0
 *              , 0, 0, 1, 0, 1, 0
 *              , 1, 0, 0, 1, 1, 0
 *            ]
 *          , "1": [
 *                0, 0, 0, 0, 1, 1
 *              , 1, 0, 1, 1, 1, 1
 *              , 1, 0, 1, 1, 1, 0
 *              , 1, 0, 0, 0, 1, 1
 *              , 1, 0, 0, 0, 1, 0
 *              , 0, 1, 0, 0, 0, 1
 *            ]
 *          , ...
 *        }
 *      }
 *  }
 *  @endcode
 * 
 *  The dictionary is an element in database. 
 *  For the chip specification, it only sampling a subset of dictionary, 
 *  which usually called "candidate" in this package source code.
 * 
 *  Consider the "bitmap_list", the element key is "marker id" and the value is a bitmap array.
 *  The bitmap array will be encode into 64 bits integer in the ArUco module 
 *  and called "code" or "ArUco code" in most function parameter or source code comment.
 *      
 *  Here is the ArUco module use case:
 *  @snippet ChipImgProc/aruco_test.cpp usage
 * 
 *  Here is an ArUco regular marker match example:
 *  @snippet ChipImgProc/marker/detection/aruco_reg_mat_test.cpp usage
 */
#pragma once
#include "aruco/detector.hpp"
#include "aruco/detector_v2.hpp"
#include "aruco/dictionary.hpp"
#include "aruco/utils.hpp"
#include "aruco/location_mark_creator.hpp"