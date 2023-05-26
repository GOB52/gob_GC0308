/*!
  @file gob_qr_code_recognizer.hpp
  @brief Scan the camera's frame buffer to identify the QR code and retrieve information
  @note Wrapper of the quirc library.
  @note Using quirc library in ESP32QRCodeReader
*/
#ifndef GOB_QR_CODE_RECOGNIZER_HPP
#define GOB_QR_CODE_RECOGNIZER_HPP

#include "quirc/quirc.h"
#include <esp_camera.h>
#include <vector>

namespace goblib { namespace camera {

/*!
  @class QRCodeRecognizer
  @brief Scan the camera's frame buffer to identify the QR code and retrieve information
 */
class QRCodeRecognizer
{
  public:
    /*!
      @struct Result
      @brief QR code information
     */
    struct Result
    {
        Result(const quirc_code& c, const quirc_data& d) : corners(c.corners), data(d) {}
	struct quirc_point corners[4]{}; //!< @brief  The four corners of the QR-code, from top right, clockwise
        quirc_data data{}; //!< @brief QR data (See also ESP32QRCodeReader quirc.h)
    };

    QRCodeRecognizer();
    virtual ~QRCodeRecognizer();

    /*! @brief Gets the quirc object pointer */
    inline ::quirc* quirc() { return _quirc; }

    /*!
      @brief Scan the frame buffer
      @retval true There exists a QR Code that has been extracted and decoded.
      @retval false Not exists or error
      @note The pixel format of the camera supports PIXFORMAT_JPEG, PIXFORMAT_RAW other than.
    */
    bool scan(const camera_fb_t* fb);

    /*! @brief Gets the number of results */
    inline size_t resultSize() const { return _results.size(); }
    /*! @brief Gets the result of the specified index */
    const Result* getResult(const uint32_t idx) { return idx < _results.size() ? &_results[idx] : nullptr; }
    
  private:
    const framesize_t _frameSize{};
    ::quirc* _quirc{};
    std::vector<Result> _results{};
};


/*!
  @brief Scan the frame buffer with quirc object.
  @param q quirc object
  @param fb frame buffer
  @retval true Detected
  @retval false Not exists or error
  @note The pixel format of the camera supports PIXFORMAT_JPEG, PIXFORMAT_RAW other than.
  @warning Since it is only detection, you need to do extract and decode by yourself.
*/
bool recognizeQR(::quirc* q, const camera_fb_t* fb);

//
}}
#endif
