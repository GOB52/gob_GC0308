/*!
  @file gob_GC0308.hpp
  @brief Complement of missing features and (possibly) correction of mistake in the esp32-camera driver for GC0308

  @mainpage gob_complementGC0308
  Complement of missing features and (possibly) correction of mistake in the esp32-camera driver for GC0308

  @author GOB / GitHub:<a href="https://github.com/GOB52/">GOB52</a> / Twitter:<a href="https://twitter.com/gob_52_gob">@GOB_52_GOB</a>

  @copyright 2023 GOB
  @copyright Licensed under the MIT license. See LICENSE file in the project root for full license information.
*/
#ifndef GOB_GC0308_HPP
#define GOB_GC0308_HPP

#include <cstdint>

namespace goblib { namespace GC0308  {

//! @enum SpecialEffect
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

//! @enum WhiteBalance
enum WhiteBalance : int8_t
{
    Auto,    //!< Automatic
    Sunny,   //!< Manual (Sunny)
    Cloudy,  //!< Manual (Cloudy)
    Office,  //!< Manual (Office)
    Home,    //!< Manual (Home)
};

/*!
  @brief complement esp32-camera GC0308 driver
  @warning Must be call after esp_camera_init() once.
  @note Delete set_gain_ctrl
  @note Add set_agc_gain
  @note Add set_specia_effect
  @note Add set_wb_mode
  @note Replace set_contrast
  @retval true Success
  @retval false Failure
*/
bool complementDriver();

//
}}
#endif
