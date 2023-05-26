/*!
  @file gob_camera.hpp
  @brief Definition that does not take any type of camera
 */
#ifndef GOB_CAMERA_HPP
#define GOB_CAMERA_HPP

#include <cstdint>

/*!
  @namespace goblib
  @brief Top level of my library
  
 */
namespace goblib {

/*!
  @namespace camera
  @brief For esp32-camera
 */
namespace camera {

/*!
  @enum SpecialEffect
  @brief For set_special_effect
 */
enum SpecialEffect : int8_t
{
    NoEffect,  //!< @brief No effect
    Negative,  //!< @brief Negative effect
    Grayscale, //!< @brief Grayscale effect
    RedTint,   //!< @brief Red tint effect
    GreenTint, //!< @brief Green tint effect
    BlueTint,  //!< @brief Blue tint effect
    Sepia,     //!< @brief Sepia effect
};

/*!
  @enum WhiteBalance
  @brief For set_wb_mode
*/
enum WhiteBalance : int8_t
{
    Auto,    //!< @brief Automatic
    Sunny,   //!< @brief Manual (Sunny)
    Cloudy,  //!< @brief Manual (Cloudy)
    Office,  //!< @brief Manual (Office)
    Home,    //!< @brief Manual (Home)
};

//
}}
#endif
