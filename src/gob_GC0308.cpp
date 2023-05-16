/*!
  @file gob_GC0308.cpp
  @brief Complement of missing features and (possibly) correction of mistake in the esp32-camera driver for GC0308
  @note  Several features were added.
  @note Please refer to this code to add or modify other features.
  @author GOB @GOB_52_GOB https://twitter.com/GOB_52_GOB
*/
#include <esp_camera.h>
#include "gob_GC0308.hpp"
#include <esp-hal-log.h>
#include <pgmspace.h>

// esp32-camera sccb.c
extern "C"
{
    int SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data); 
    uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg);
}

template<typename T, size_t N> constexpr auto size(const T(&)[N]) noexcept -> size_t { return N; }

enum SpecialEffect : int8_t
{
    SE_NoEffect,
    SE_Negative,
    SE_Grayscale,
    SE_RedTint,
    SE_GreenTint,
    SE_BlueTint,
    SE_Sepia,
};

enum WhieBalance : int8_t
{
    WB_Auto,
    WB_Sunny,
    WB_Cloudy,
    WB_Office,
    WB_Home,
};

namespace
{
constexpr char TAG[] = "GCB_GC0308";

struct RegVal { const uint8_t reg; const uint8_t val; };
struct Params { const size_t sz;   const RegVal* params; };

// For special effect
PROGMEM const RegVal special_no_effect[] =
{
    { 0x23, 0x00 }, // special_effect [7:3] NA [2] edge_map [1] CbCr fixed enable [0] Inverse color
    { 0x20, 0x7f }, // Block_enable_1  [7] BKS_en [6] gamma enable [5] CC enable [4] Edge enhancement enable [3] Interpolation enable [2] Noise removal enable [1] Defect removal enable [0] Lens‐shading correction enabl
    { 0xd2, 0x90 }, // AEC_mode2 [7] AEC_en [6] map measure point [5] color Y mode [4] skin weight mode [3] NA [2] color select [1:0] NA
    { 0x73, 0x00 }, // EINTP mode 2  [7] edge_add_mode2 [6]NA [5] direction mode [4] fixed direction threshold [3:0] reserved
    { 0x77, 0x38 }, // Edge effect [7:4] edge effect1 [3:0] edge effect 2
    { 0xb3, 0x40 }, // Luma_contrast, can be adjusted viacontrast center 2.6bits, 0x40=1.0
    { 0xb4, 0x80 }, // Contrast center value
    { 0xba, 0x00 }, // S7, if fixed CbCr function is enabled, current image Cb value will be replace by this value to achieve special effect
    { 0xbb, 0x00 }, // S7, if fixed CbCr function is enabled, current image Cr value will be replace by this value to achieve special effect
};
PROGMEM const RegVal special_negative[] =
{
    { 0x23, 0x01 },
    { 0x20, 0x7f },
    { 0xd2, 0x90 },
    { 0x73, 0x00 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0x00 },
    { 0xbb, 0x00 },
};

PROGMEM const RegVal special_grayscale[] =
{
    { 0x23, 0x02 },
    { 0x20, 0xff },
    { 0xd2, 0x90 },
    { 0x73, 0x00 },
    { 0x77, 0x54 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0x00 },
    { 0xbb, 0x00 },
};

PROGMEM const RegVal special_red[] =
{
    { 0x23, 0x02 },
    { 0x20, 0x7f },
    { 0xd2, 0x90 },
    { 0x77, 0x00 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0xe0 },
    { 0xbb, 0x50 },
};

PROGMEM const RegVal special_green[] =
{
    { 0x23, 0x02 },
    { 0x20, 0xff },
    { 0xd2, 0x90 },
    { 0x77, 0x88 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0xc0 },
    { 0xbb, 0xc0 },
};

PROGMEM const RegVal special_blue[] =
{
    { 0x23, 0x02 },
    { 0x20, 0xff },
    { 0xd2, 0x90 },
    { 0x77, 0x00 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0x50 },
    { 0xbb, 0xe0 },
};

PROGMEM const RegVal special_sepia[] =
{
    { 0x23, 0x02 },
    { 0x20, 0xff },
    { 0xd2, 0x90 },
    { 0x73, 0x00 },
    { 0xb3, 0x40 },
    { 0xb4, 0x80 },
    { 0xba, 0xd0 },
    { 0xbb, 0x20 },
};

PROGMEM const Params param_special_effect[] =
{
    { size(special_no_effect), special_no_effect },
    { size(special_negative),  special_negative },
    { size(special_grayscale), special_grayscale },
    { size(special_red),       special_red },
    { size(special_green),     special_green },
    { size(special_blue),      special_blue },
    { size(special_sepia),     special_sepia },
};

// For White balance
PROGMEM const RegVal wb_auto[] =
{
    { 0x5a, 0x4c }, // AWB_R_gain 2.6 bits red channel gain from AWB
    { 0x5b, 0x40 }, // AWB_G_gain 2.6 bits green channel gain from AWB
    { 0x5c, 0x56 }, // AWB_B_gain 2.6 bits blue channel gain from AWB
};

PROGMEM const RegVal wb_sunny[] =
{
    { 0x5a, 0x74 },
    { 0x5b, 0x52 },
    { 0x5c, 0x40 },
};

PROGMEM const RegVal wb_cloudy[] =
{
    { 0x5a, 0x8c },
    { 0x5b, 0x50 },
    { 0x5c, 0x40 },
};

PROGMEM const RegVal wb_office[] =
{
    { 0x5a, 0x40 },
    { 0x5b, 0x42 },
    { 0x5c, 0x50 },
};

PROGMEM const RegVal wb_home[] =
{
    { 0x5a, 0x48 },
    { 0x5b, 0x40 },
    { 0x5c, 0x5c },
};

PROGMEM const Params param_wb[] =
{
    { size(wb_auto),   wb_auto   },
    { size(wb_sunny),  wb_sunny  },
    { size(wb_cloudy), wb_cloudy },
    { size(wb_office), wb_office },
    { size(wb_home),   wb_home   },
};

// --------------------------------
// Camera methods
int set_dummy(sensor_t*, int) { return -1; }

int set_contrast(sensor_t *s, int contrast)
{
    if(contrast < 0 || contrast > 255) { return -1; }
    
    s->status.contrast = contrast;
    int ret = SCCB_Write(s->slv_addr, 0xFE, 0); // page 0
    ret |= SCCB_Write(s->slv_addr, 0xb3, contrast); //2.6bits, 0x40=1.x
    return ret;
}

int set_agc_gain(sensor_t* s, int val)
{
    if(val < 0 || val > 63) { return -1; }

    s->status.agc_gain = val;
    int ret = SCCB_Write(s->slv_addr, 0xFE, 0); // page 0
    ret |= SCCB_Write(s->slv_addr, 0x50, val); // [5:0] global_gain, 2.4bits, 0x10 is 1.0x
    return ret;
}

int set_special_effect(sensor_t *s, int effect)
{
    if(effect < SE_NoEffect || effect > SE_Sepia) { return -1; }

    s->status.special_effect = effect;

    int ret = SCCB_Write(s->slv_addr, 0xFE, 0); // page 0
    size_t sz = param_special_effect[effect].sz;
    auto p = param_special_effect[effect].params;
    while(!ret && sz--)
    {
        ret |= SCCB_Write(s->slv_addr, p->reg, p->val);
        ++p;
    }
    return ret;
}

int set_wb_mode(sensor_t *s, int mode)
{
    if(mode < WB_Auto || mode > WB_Home) { return -1; }

    s->status.wb_mode = mode;

    int ret = SCCB_Write(s->slv_addr, 0xFE, 0); // page 0
    
    uint8_t rval = SCCB_Read(s->slv_addr, 0x22); // AAAA_enable  [1] AWB enable
    rval = (rval & ~0x02U) | ((mode == WB_Auto) ? 0x02 : 0x00);
    ret |= SCCB_Write(s->slv_addr, 0x22, rval);
    
    size_t sz = param_wb[mode].sz;
    auto p = param_wb[mode].params;
    while(!ret && sz--)
    {
        ret |= SCCB_Write(s->slv_addr, p->reg, p->val);
        ++p;
    }
    return ret;
}
//
}

//
namespace gc0308 
{

bool complementDriver()
{
    auto s = esp_camera_sensor_get();
    if(!s || s->id.PID != GC0308_PID) { ESP_LOGE(TAG, "%s", "GC0308 not detected"); return false; }

    // Delete gain ctrl
    // In esp32-camera it is set but probably wrong?
    s->set_gain_ctrl = set_dummy;

    // Add agc gain
    s->set_agc_gain = set_agc_gain;
    // Add special effect
    s->set_special_effect = set_special_effect;
    // Add wb_mode
    s->set_wb_mode = set_wb_mode;
    // Change my set_contrast (Because the original does not set a value to the status)
    s->set_contrast = set_contrast;

    return true;
}

//
}
