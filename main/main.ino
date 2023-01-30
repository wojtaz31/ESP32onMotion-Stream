#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define CONCATE_(X, Y) X##Y
#define CONCATE(X, Y) CONCATE_(X, Y)

#define ALLOW_ACCESS(CLASS, MEMBER, ...) \
  template<typename Only, __VA_ARGS__ CLASS::*Member> \
  struct CONCATE(MEMBER, __LINE__) { friend __VA_ARGS__ CLASS::*Access(Only*) { return Member; } }; \
  template<typename> struct Only_##MEMBER; \
  template<> struct Only_##MEMBER<CLASS> { friend __VA_ARGS__ CLASS::*Access(Only_##MEMBER<CLASS>*); }; \
  template struct CONCATE(MEMBER, __LINE__)<Only_##MEMBER<CLASS>, &CLASS::MEMBER>

#define ACCESS(OBJECT, MEMBER) \     
 (OBJECT).*Access((Only_##MEMBER<std::remove_reference<decltype(OBJECT)>::type>*)nullptr)

const char* ssid = "your ssid";
const char* password = "your password";
const char * udpAddress = "192.168.1.4";
char buffer_size[8192];
const int udpPort = 6000;
const int pirPin = 14;
WiFiUDP udp;
ALLOW_ACCESS(WiFiUDP, tx_buffer, char*);
ALLOW_ACCESS(WiFiUDP, tx_buffer_len, size_t);


static esp_err_t stream_handler()
{
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    static int64_t last_frame = 0;
    if (!last_frame)
    {
        last_frame = esp_timer_get_time();
    }
    if (res != ESP_OK)
    {
        return res;
    }

    {

        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        }
        else
        {
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        ESP_LOGE(TAG, "JPEG compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
        }    
            ACCESS(udp, tx_buffer) = (char*)_jpg_buf;
            udp.beginPacket(udpAddress,udpPort);
            ACCESS(udp, tx_buffer_len) = _jpg_buf_len;
            Serial.println(_jpg_buf_len);
            udp.endPacket();
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        frame_time /= 1000;
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
#endif
        ESP_LOGI(TAG, "MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)"
                 ,
                 (uint32_t)(_jpg_buf_len),
                 (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
                 avg_frame_time, 1000.0 / avg_frame_time
        );
    }
    return res;
}

camera_fb_t *GetJPEG() {
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return NULL;
  }
  return fb;
}

void setup() {
  pinMode(pirPin, INPUT);
  Serial.begin(115200);
  Serial.setDebugOutput(true);

camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }


  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {

  if(digitalRead(pirPin)){
    stream_handler();
  }
}
