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

namespace gob
{

namespace GC0308
{

/*!
  @brief complement esp32-camera GC0308 driver
  @warning Must be call after camera_init() once.
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
