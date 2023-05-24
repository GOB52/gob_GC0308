/*!
  @file gob_qr_code_recognizer.cpp
  @brief Scan the camera's frame buffer to identify the QR code and retrieve information
  @note Wrapper of the quirc library
 */
#include "gob_qr_code_recognizer.hpp"
#include "quirc/quirc_internal.h"
#include <cstring> // memcpy
#include <esp32-hal-log.h>

#pragma GCC optimize ("O3")

namespace
{
// Copying while grayscaling
void copyWhileGrayscaling(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_RGB565) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint16_t* src = (uint16_t*)fb->buf;
    while(sz--)
    {
        uint_fast16_t clr = __builtin_bswap16(*src++);
        uint_fast8_t r = (clr & 0xF800) >> 8;
        uint_fast8_t g = (clr & 0x07E0) >> 3;
        uint_fast8_t b = (clr & 0x001F) << 3;
        *dst++ = (((r << 1) + (g << 2) + g + b) >> 3);   // Compute fast grayscale approximations from RGB565
    }
}

inline int grid_bit(const struct quirc_code *code, int x, int y)
{
	int p = y * code->size + x;
	return (code->cell_bitmap[p >> 3] >> (p & 7)) & 1;
}

// ESP32QRCoderReader does not have quirc_flip, so it was ported from the original.
void quirc_flip(struct quirc_code *code)
{
    struct quirc_code flipped{};
    unsigned int offset = 0;
    for (int y = 0; y < code->size; y++) {
        for (int x = 0; x < code->size; x++) {
            if (grid_bit(code, y, x)) {
                flipped.cell_bitmap[offset >> 3u] |= (1u << (offset & 7u));
            }
            offset++;
        }
    }
    memcpy(&code->cell_bitmap, &flipped.cell_bitmap, sizeof(flipped.cell_bitmap));
}
//
}

namespace goblib {

QRCodeRecognizer::QRCodeRecognizer()
{
}

QRCodeRecognizer:: ~QRCodeRecognizer()
{
    if(_quirc) { quirc_destroy(_quirc); }
}

bool QRCodeRecognizer::scan(const camera_fb_t* fb)
{
    _results.clear();
    if(!fb || fb->format == PIXFORMAT_JPEG) { log_e("Invalid fb:%d", fb ? fb->format : -1); return false; }

    if(!_quirc)
    {
        _quirc = quirc_new();
        if(!_quirc)
        {
            log_e("Failed to new");
            return false;
        }
    }
    if(!_quirc->image || _quirc->w != fb->width || _quirc->h != fb->height)
    {
        free(_quirc->image);
        _quirc->image = nullptr;
        if(quirc_resize(_quirc, fb->width, fb->height) != 0) // Allocate quirc.image
       {
           log_e("Failed to resize");
           return false;
       }
    }

    auto buf = quirc_begin(_quirc, nullptr, nullptr);
    if(!buf) { log_e("Failed to begin"); return false; }
    copyWhileGrayscaling(buf, fb);
    quirc_end(_quirc); // Detect

    if(quirc_count(_quirc))
    {
        int_fast8_t num = quirc_count(_quirc);
        for(int_fast8_t i = 0; i < num; ++i)
        {
            /* ***************************************************
              CAUTION
               code and data are large, beware of stack overflow.
            ****************************************************** */
            struct quirc_code code; 
            struct quirc_data data; 

            quirc_extract(_quirc, i, &code);
            auto err = quirc_decode(&code, &data);
            // Support H-Mirror image
            if (err == QUIRC_ERROR_DATA_ECC)
            {
                quirc_flip(&code);
                err = quirc_decode(&code, &data);
            }
            if(!err)
            {
                _results.emplace_back(code, data);
            }
            else
            {
                log_d("decode error %d:%u", i, err);
            }
        }
    }
    log_d("Decoded:%zu", _results.size());
    return !_results.empty();
}
//
}
