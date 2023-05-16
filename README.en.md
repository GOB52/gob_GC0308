# gob_GC0308

[日本語](README.md)

## Overview
This library complements the GC0308 driver of [esp32-camera](https://github.com/espressif/esp32-camera).  
It can also be used as a sample to add your own functions that are not included in the esp32-camera driver.

## Required libraries
* [esp32-camera](https://github.com/espressif/esp32-camera)  
However, If you set platform = espressif32 and framework = arduino in platformio, esp32-camera is included in the package, so you do not need to specify es32-camera in lib_deps.

## How to install
Install in an appropriate way depending on your environment.
* git clone and extract into place  
or
* platformio.ini
```ini
lib_deps = https://github.com/GOB52/gob_GC0308
```
## How to use

```cpp
#include <esp_camera.h>
#include <gob_GC0308.hpp>

void setup()
{
    camera_config_t ccfg{};
    // Configuration settings...
    esp_camera_init(&ccfg);
    gob::GC0308::complementDriver(); // Must be call after esp_camera_init()
}

void foo()
{
    sensor_t *s = esp_camera_sensor_get();
	s->set_special_effect(s, gob::GC0308::SpecialEffect::Sepia);
}
```

## Changes since esp32-camera GC0308
### Delete
* set\_gain\_ctrl  
Looking at the behavior of other drivers, it seems that turning AGC ON/OFF is the original behavior.  
In esp32-camera, the processing equivalent to set_agc_gain is set, so it should be removed.
### Add
* set\_agc\_gain  
Gain settings. Use esp32-camera's set_agc_gain, which does not rewrite the internal status, so use my own.
* set\_special\_effect  
Camera effect change. The following values can be set.

|gob::GC0308::SpecialEffect|Description|
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

|gob::GC0308::WhiteBalance|Description|
|---|---|
|Auto|Automatic|
|Sunny|Sunny|
|Cloudy|Cloudy|
|Office|Fluorescent light|
|Home|Light bulb|

### Replace
* set\_contrast  
Use esp32-camera's set_contrast, which does not rewrite the internal status, so use my own.

## Note
You can switch to or add your own methods by rewriting the information in sensor\_t obtained by esp\_camera\_sensor\_get().  
See **esp32-camera/driver/include/sensor.h** for details.


