/*!
  @file gob_qr_code_recognizer.cpp
  @brief Scan the camera's frame buffer to identify the QR code and retrieve information
  @note Wrapper of the quirc library.
  @note Using quirc library in ESP32QRCodeReader
*/
#include "gob_qr_code_recognizer.hpp"
#include "quirc/quirc_internal.h"
#include <cstring> // memcpy
#include <esp32-hal-log.h>

#pragma GCC optimize ("O3")

namespace
{
inline uint_fast8_t rgb_to_grayscale8(const uint_fast8_t& r, const uint_fast8_t& g, const uint_fast8_t& b)
{
    return (((r << 1) + (g << 2) + g + b) >> 3);   // Compute fast grayscale approximations
}

void copy_RGB565_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_RGB565) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint16_t* src = (uint16_t*)fb->buf;
    while(sz--)
    {
        uint_fast16_t clr = __builtin_bswap16(*src++);
#if 0
        uint_fast8_t r = (clr & 0xF800) >> 8;
        uint_fast8_t g = (clr & 0x07E0) >> 3;
        uint_fast8_t b = (clr & 0x001F) << 3;
#else
        // With normalize
        uint_fast8_t r = (clr & 0xF800) >> 8 | ((clr & 0xF800) >> 13);
        uint_fast8_t g = (clr & 0x07E0) >> 3 | ((clr & 0x07E0) >> 9);
        uint_fast8_t b = (clr & 0x001F) << 3 | ((clr & 0x001F) >> 2);
#endif
        *dst++ = rgb_to_grayscale8(r, g, b);
    }
}

void copy_YUV422_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_YUV422) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint16_t* src = (uint16_t*)fb->buf;
    while(sz--)
    {
        // Using Y factor only [Y0U0Y1V0...]
        *dst++ = *src++ & 0xFF;
    }
}

void copy_YUV420_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_YUV420) { return; }
    uint32_t sz = fb->width * fb->height;
    // Using Y factor only [Y[width*height], UV...]
    std::memcpy(dst, fb->buf, sz);
}

void copy_grayscale_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_GRAYSCALE) { return; }
    std::memcpy(dst, fb->buf, fb->len);
}

void copy_RGB888_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_RGB888) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint8_t* src = fb->buf;
    while(sz--)
    {
        *dst++ = rgb_to_grayscale8(src[0], src[1], src[2]);
        src += 3;
    }
}

void copy_RGB444_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_RGB444) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint16_t* src = (uint16_t*)fb->buf;
    while(sz--)
    {
        uint_fast16_t clr = *src++;
#if 0
        uint_fast8_t r = (clr & 0x000F) << 4;
        uint_fast8_t g = (clr & 0xF000);
        uint_fast8_t b = (clr & 0x0F00) << 4;
#else
        // With normalize
        uint_fast8_t r =  (clr & 0x000F) * 0x11;
        uint_fast8_t g = ((clr & 0xF000) >> 12) * 0x11;
        uint_fast8_t b = ((clr & 0x0F00) >>  8) * 0x11;

#endif
        *dst++ = rgb_to_grayscale8(r, g, b);
    }
}

void copy_RGB555_to_grayscale(uint8_t* dst, const camera_fb_t* fb)
{
    if(!dst || !fb || fb->format != PIXFORMAT_RGB555) { return; }
    uint32_t sz = fb->width * fb->height;
    const uint16_t* src = (uint16_t*)fb->buf;
    while(sz--)
    {
        uint_fast16_t clr = __builtin_bswap16(*src++);
#if 0
        uint_fast8_t r = (clr & 0x7C00) >> 7;
        uint_fast8_t g = (clr & 0x03E0) >> 2;
        uint_fast8_t b = (clr & 0x001F) << 3;
#else
        // With normalize
        uint_fast8_t r = (clr & 0x7C00) >> 7 | ((clr & 0x7C00) >> 12);
        uint_fast8_t g = (clr & 0x03E0) >> 2 | ((clr & 0x03E0) >> 7);
        uint_fast8_t b = (clr & 0x001F) << 3 | ((clr & 0x001F) >> 2);
#endif
        *dst++ = rgb_to_grayscale8(r, g, b);
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

using copy_function = void(*)(uint8_t* dst, const camera_fb_t* fb);
copy_function functionTable[/*pixelformat_t*/] =
{
    copy_RGB565_to_grayscale,
    copy_YUV422_to_grayscale,
    copy_YUV420_to_grayscale,
    copy_grayscale_to_grayscale,
    nullptr, // PIXFORMAT_JPEG
    copy_RGB888_to_grayscale,
    nullptr, // PIXFORMAT_RAW
    copy_RGB444_to_grayscale,    
    copy_RGB555_to_grayscale,
};
//
}

namespace goblib { namespace camera {

QRCodeRecognizer::QRCodeRecognizer()
{
}

QRCodeRecognizer:: ~QRCodeRecognizer()
{
    if(_quirc) { quirc_destroy(_quirc); }
}

bool QRCodeRecognizer::scan(const camera_fb_t* fb)
{
    log_e("%zu : %zu", sizeof(quirc_code), sizeof(quirc_data));

    _results.clear();
    if(!fb || !functionTable[fb->format]) { log_e("Invalid fb:%d", fb ? fb->format : -1); return false; }

    if(!_quirc)
    {
        _quirc = quirc_new();
        if(!_quirc)
        {
            log_e("Failed to new");
            return false;
        }
    }

    if(recognizeQR(_quirc, fb))
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


bool recognizeQR(::quirc* q, const camera_fb_t* fb)
{
    if(!q->image || q->w != fb->width || q->h != fb->height)
    {
        free(q->image);
        q->image = nullptr;
        if(quirc_resize(q, fb->width, fb->height) != 0) // Allocate quirc.image
       {
           log_e("Failed to resize");
           return false;
       }
    }
    auto buf = quirc_begin(q, nullptr, nullptr);
    if(!buf) { log_e("Failed to begin"); return false; }
    // Copy from the frame buffer to the quirc buffer, converting to grayscale 8bit.
    functionTable[fb->format](buf, fb);
    quirc_end(q);
    return quirc_count(q) > 0;
}

//
}}
