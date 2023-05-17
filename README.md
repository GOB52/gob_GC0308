# gob_GC0308

[English](README.en.md)

## 概要
[esp32-camera](https://github.com/espressif/esp32-camera) の GC0308 ドライバを補完するライブラリです。  
また esp32-camera のドライバにない機能を独自に追加する際のサンプルとしても活用できます。


 ## 必要なもの
* [esp32-camera](https://github.com/espressif/esp32-camera)  
但し platformio にて  
platform = espressif32  
framework = arduino  
としている場合は、 esp32-camera は同梱されている為 lib_deps への es32-camera の指定は不要です。

## 導入
環境によって適切な方法でインストールしてください
* git clone して所定の位置へ展開する  
または
* platformio.ini
```ini
lib_deps =   https://github.com/GOB52/gob_GC0308.git
```

## 使い方

```cpp
#include <esp_camera.h>
#include <gob_GC0308.hpp>

void setup()
{
    camera_config_t ccfg{};
    //     // Configuration settings...
    esp_camera_init(&ccfg);
    gob::GC0308::complementDriver(); // Must be call after esp_camera_init()
}

void foo()
{
    sensor_t *s = esp_camera_sensor_get();
	s->set_special_effect(s, gob::GC0308::SpecialEffect::Sepia);
}
```

## esp32-camera GC0308 への追加/変更点
### 削除
* set\_gain\_ctrl  
他のドライバの挙動を見るに、AGC の ON/OFF をするのが本来の挙動と思われます。  
esp32-camera では set_agc_gain 相当の処理が設定されている為、削除します。
### 追加
* set\_agc\_gain  
ゲインの設定。 esp32-camera の set_agc_gain は 内部ステータスを書き換えないので独自の物を使用。
* set\_special\_effect  
カメラエフェクト変更。以下の値を設定可能。

|gob::GC0308::SpecialEffect|説明|
|---|---|
|NoEffect|エフェクト無し|
|Negative|ネガポジ変換|
|Grayscale|グレイスケール化|
|RedTint|赤化|
|GreenTint|緑化|
|BlueTint|青化|
|Sepia|セピア化|

* set\_wb\_mode  
ホワイトバランス変更。以下の値を設定可能。

|gob::GC0308::WhiteBalance|説明|
|---|---|
|Auto|自動|
|Sunny|晴天|
|Cloudy|曇天|
|Office|蛍光灯|
|Home|電球|

### 置換
* set\_contrast  
esp32-camera で設定される set_contrast が内部ステータスを書き換えないので独自のものを使用。

## 備考
esp\_camera\_sensor\_get() によって取得した sensor\_t の情報を書き換える事で独自メソッドへの切り替えや追加ができます。  
詳しくは **esp32-camera/driver/include/sensor.h** を参照してください。

