# gob_GC0308

[日本語](README.md)

## Overview
This library complements the GC0308 driver of [esp32-camera](https://github.com/espressif/esp32-camera).  
Some additional camera controls and QR code recognizer added.  
It can also be used as a sample to add your own functions that are not included in the esp32-camera driver.

## Required libraries
* [esp32-camera](https://github.com/espressif/esp32-camera)  
However, If you set platform = espressif32 and framework = arduino in platformio, esp32-camera is included in the package, so you do not need to specify es32-camera in lib_deps.
* [ESP32QRCodeReader](https://github.com/alvarowolfx/ESP32QRCodeReader)  
In platformIO, automatically installed if gob\_GC0308 is registered in lib_deps.

## How to install
Install in an appropriate way depending on your environment.
* git clone and extract into place  
or
* platformio.ini
```ini
lib_deps = https://github.com/GOB52/gob_GC0308
```
## How to use

### Complement
```cpp
#include <esp_camera.h>
#include <gob_GC0308.hpp>

void setup()
{
    camera_config_t ccfg{};
    // Configuration settings...
    esp_camera_init(&ccfg);
    goblib::camera::GC0308::complementDriver(); // Must be call after esp_camera_init()
}
```
### Use camera control
```cpp
void foo()
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_special_effect(s, goblib::camera::SpecialEffect::Sepia);
}
```

### Recognize QR code
```cpp
#include <esp_camera.h>
#include <gob_qr_code_recognizer.hpp>

goblib::QRCodeRecognizer recQR;
void foo()
{
    auto fb = esp_camera_fb_get();
    if(recQR.scan(fb))
    {
        int_fast8_t num = recQR.resultSize();
        for(int_fast8_t i =0; i < num; ++i)
        {
            auto pr = recQR.getResult(i);
            String s(pr->data.payload, pr->data.payload_len);
            printf("QR:%d [%s]", i, s.c_str());
        }
    }
    esp_camera_fb_return(fb);
}
```

##  Additions/changes to esp32-camera GC0308
### Delete
* set\_gain\_ctrl  
Looking at the behavior of other drivers, it seems that turning AGC ON/OFF is the original behavior.  
In esp32-camera, the processing equivalent to set_agc_gain is set, so it should be removed.
### Add
* set\_agc\_gain  
Gain settings. Use esp32-camera's set_agc_gain, which does not rewrite the internal status, so use my own.
* set\_special\_effect  
Camera effect change. The following values can be set.

|goblib::<zero-width space>camera::SpecialEffect|Description|
|---|---|
|NoEffect|No effect|
|Negative|Negative effect|
|Grayscale|Grayscale effect|
|RedTint|Red tint effect|
|GreenTint|Green tint effect|
|BlueTint|Blue tint effect|
|Sepia|Sepia effect|

* set\_wb\_mode  
White balance change. The following values can be set.

|goblib::<zero-width space>camera::WhiteBalance|Description|
|---|---|
|Auto|Automatic|
|Sunny|Sunny|
|Cloudy|Cloudy|
|Office|Fluorescent light|
|Home|Light bulb|

* set\_saturation  
Saturation change.

### Replace
* set\_contrast  
Use esp32-camera's set_contrast, which does not rewrite the internal status, so use my own.

## QR code recognition
ESP32QRCodeReader is a large configuration including camera tasks and can only work with cameras that can output PIXFORMAT\_GRAYSCALE.  
Therefore, the recognition part is independent, and a conversion mechanism to grayscale is included to enable recognition even with cameras that cannot output grayscale (GC0308).  
You can use camera\_fb\_t* with QRCodeRecognizer or your own quirc object to recognize and retrieve.  
The pixel format of the camera supports PIXFORMAT\_JPEG, PIXFORMAT\_RAW <ins>**other than.**</ins>

**Note : <ins>It uses a large amount of stack space</ins>, so be aware of the stack size for tasks when recognizing it within an independent task.**

```
FYI
sizeof(quirc_code) 3960
sizeof(quirc_data) 8920
```

## Note
You can switch to or add your own methods by rewriting the information in sensor\_t obtained by esp\_camera\_sensor\_get().  
See **esp32-camera/driver/include/sensor.h** for details.
