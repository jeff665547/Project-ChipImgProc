/**
 *  @file       ChipImgProc/aruco.hpp
 *  @author     Chia-Hua Chang
 *  @brief      ArUco marker detection module.
 *  @details The ArUco database JSON format is:
 * 
 *      @code
 *      {
 *          "dictionary name": {
 *            "coding_bits": <integer>
 *          , "maxcor_bits": <integer>
 *          , "bitmap_list": {
 *                "<marker id>": [
 *                   <ArUco code in bit array>
 *                ]
 *              , "<marker id>": [
 *                   <ArUco code in bit array>
 *                ]
 *              , ...
 *            }
 *          }
 *      }
 *      @endcode
 * 
 *  Here is an example:
 *      @code
 *      {
 *          "DICT_6X6_250": {
 *            "coding_bits": 6
 *          , "maxcor_bits": 5
 *          , "bitmap_list": {
 *                "0": [
 *                    0, 0, 0, 1, 1, 1
 *                  , 1, 0, 0, 0, 1, 1
 *                  , 1, 1, 0, 1, 1, 1
 *                  , 0, 1, 1, 0, 0, 0
 *                  , 0, 0, 1, 0, 1, 0
 *                  , 1, 0, 0, 1, 1, 0
 *                ]
 *              , "1": [
 *                    0, 0, 0, 0, 1, 1
 *                  , 1, 0, 1, 1, 1, 1
 *                  , 1, 0, 1, 1, 1, 0
 *                  , 1, 0, 0, 0, 1, 1
 *                  , 1, 0, 0, 0, 1, 0
 *                  , 0, 1, 0, 0, 0, 1
 *                ]
 *              , ...
 *            }
 *          }
 *      }
 *      @endcode
 * 
 *  The dictionary is an element in database. 
 *  For a chip specification, it only sampling a subset of dictionary, 
 *  which usually called "candidate" in this package source code.
 * 
 *  Consider the "bitmap_list", the element key is marker id and the value is a bitmap array.
 *  The bitmap array will be encode into 64 bits integer in the ArUco module 
 *  and called "code" or "ArUco code" in most function parameter or source code comment.
 *      
 *  Here is the ArUco module use case:
 *  @snippet ChipImgProc/aruco_test.cpp usage
 */
#pragma once
#include "aruco/detector.hpp"
#include "aruco/dictionary.hpp"
#include "aruco/utils.hpp"