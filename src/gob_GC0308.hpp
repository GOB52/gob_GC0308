/*!
  @file gob_GC0308.hpp
  @brief Complement of missing features and (possibly) correction of mistake in the esp32-camera driver for GC0308

  @mainpage gob_complementGC0308
  This library complements the GC0308 driver of [esp32-camera](https://github.com/espressif/esp32-camera).  
  Some additional camera controls and QR code recognizer added.

  @author GOB / [GitHub](https://github.com/GOB52/) / [Twitter](https://twitter.com/gob_52_gob)

  @copyright 2023 GOB
  @copyright Licensed under the MIT license. See LICENSE file in the project root for full license information.
*/
#ifndef GOB_GC0308_HPP
#define GOB_GC0308_HPP

#include "gob_camera.hpp"

namespace goblib { namespace camera {
/*!
  @namespace GC0308
  @brief Functions for GC0308
 */
namespace GC0308
{

/*!
  @brief complement esp32-camera GC0308 driver
  @warning Must be call after esp_camera_init() once.
  @note Delete set_gain_ctrl
  @note Add set_agc_gain
  @note Add set_specia_effect
  @note Add set_wb_mode
  @note Add set_saturation
  @note Replace set_contrast
  @retval true Success
  @retval false Failure
*/
bool complementDriver();

//
}
//
}}
#endif
