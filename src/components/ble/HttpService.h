#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include <cstdint>
#include <array>

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class HttpService {
    public:
      HttpService(NimbleController& nimble);
      void Init();
      int OnHttpRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void NotifyHttpResponse(uint16_t connectionHandle, const uint8_t* data, size_t length);

      static constexpr uint16_t httpServiceId {0xFEED};
      static constexpr ble_uuid16_t httpServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = httpServiceId};

    private:
      NimbleController& nimble;
      static constexpr uint16_t httpRequestId {0x0001};
      static constexpr ble_uuid16_t httpRequestUuid {.u {.type = BLE_UUID_TYPE_16}, .value = httpRequestId};

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t httpRequestHandle;

      // Structure pour le protocole de communication
      struct HttpRequest {
        uint8_t method; // 0=GET, 1=POST, 2=PUT, 3=DELETE
        uint8_t urlLength;
        char url[256];
        uint16_t bodyLength;
        uint8_t body[512];
      };

      struct HttpResponse {
        uint16_t statusCode;
        uint16_t headersLength;
        char headers[512];
        uint16_t bodyLength;
        uint8_t body[1024];
      };

      void ProcessHttpRequest(const HttpRequest& request);
      void SendHttpResponse(uint16_t connectionHandle, const HttpResponse& response);
    };
  }
} 