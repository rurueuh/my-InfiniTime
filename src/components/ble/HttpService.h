#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include <cstdint>
#include <array>
#include <cstring>
#include <functional>

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    struct HttpRequest {
      uint8_t method;
      uint8_t urlLength;
      char url[256];
      uint16_t bodyLength;
      uint8_t body[1024];
    };

    struct HttpResponse {
      uint16_t statusCode;
      uint16_t headersLength;
      char headers[512];
      uint16_t bodyLength;
      char body[2048];
    };

    class HttpService {
    public:
      HttpService();
      void Init();
      int OnHttpRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void NotifyHttpResponse(uint16_t connectionHandle, const uint8_t* data, size_t length);

      static constexpr ble_uuid128_t httpServiceUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xee, 0x00, 0x00, 0x00}};

      static constexpr ble_uuid128_t httpRequestUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xee, 0x01, 0x00, 0x00}};

      uint16_t httpRequestHandle;

      ble_gatt_chr_def characteristicDefinition[2];
      ble_gatt_svc_def serviceDefinition[2];

      void ProcessHttpRequest(const HttpRequest& request);
      void SendHttpResponse(uint16_t connectionHandle, const HttpResponse& response);
      void SetResponseCallback(std::function<void(const HttpResponse&)> callback);
    private:
      std::function<void(const HttpResponse&)> responseCallback;
    };
  }
} 