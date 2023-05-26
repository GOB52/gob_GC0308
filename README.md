# gob_GC0308

[English](README.en.md)

## 概要
[esp32-camera](https://github.com/espressif/esp32-camera) の GC0308 ドライバを補完するライブラリです。  
いくつかのカメラ制御機能と、QR コード識別機能が追加されます。  
また esp32-camera のドライバにない機能を独自に追加する際のサンプルとしても活用できます。


 ## 依存ライブラリ
* [esp32-camera](https://github.com/espressif/esp32-camera)  
但し platformio にて  
platform = espressif32  
framework = arduino  
としている場合は、 esp32-camera は同梱されている為 lib_deps への es32-camera の指定は不要です。
* [ESP32QRCodeReader](https://github.com/alvarowolfx/ESP32QRCodeReader)  
platformio では lib\_deps に gob_GC0308 を登録していれば自動的にインストールされます。

## 導入
環境によって適切な方法でインストールしてください
* git clone して所定の位置へ展開する  
または
* platformio.ini
```ini
lib_deps =   https://github.com/GOB52/gob_GC0308.git
```

## 使い方

### 機能の補完
```cpp
#include <esp_camera.h>
#include <gob_GC0308.hpp>

void setup()
{
    camera_config_t ccfg{};
    //     // Configuration settings...
    esp_camera_init(&ccfg);
    goblib::camera::GC0308::complementDriver(); // Must be call after esp_camera_init()
}
```

### 機能の使用
```cpp
#include <esp_camera.h>
void foo()
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_special_effect(s, goblib::camera::SpecialEffect::Sepia);
}
```

### QR コード識別
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

|goblib::<zero-width space>camera::SpecialEffect|説明|
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

|goblib::<zero-width space>camera::WhiteBalance|説明|
|---|---|
|Auto|自動|
|Sunny|晴天|
|Cloudy|曇天|
|Office|蛍光灯|
|Home|電球|

* set\_saturation  
彩度変更。

### 置換
* set\_contrast  
esp32-camera で設定される set_contrast が内部ステータスを書き換えないので独自のものを使用。


## QR 識別
ESP32QRCodeReader はカメラタスクを含めた大掛かりな構成で、PIXFORMAT\_GRAYSCALE を出力できるカメラでないと動作できません。  
そこで識別部分を独立、グレイスケールへの変換機構を内包し、グレイスケール出力できないカメラ(GC0308)でも識別可能としました。
camera\_fb\_t* と QRCodeRecognizer または自前の quirc オブジェクトと組み合わせて、識別と取得を行うこどができます。  
カメラのピクセルフォーマットは PIXFORMAT\_JPEG PIXFORMAT\_RAW <ins>**以外**</ins>に対応しています。

**注意 : <ins>スタック領域をかなり使用するので</ins>、独立したタスク内で識別する場合はタスク用のスタックサイズに留意のこと。**
```
参考
sizeof(quirc_code) 3960
sizeof(quirc_data) 8920
```

## 備考
esp\_camera\_sensor\_get() によって取得した sensor\_t の情報を書き換える事で独自メソッドへの切り替えや追加ができます。  
詳しくは **esp32-camera/driver/include/sensor.h** を参照してください。

