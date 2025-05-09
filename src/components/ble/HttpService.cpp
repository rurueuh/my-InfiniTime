#include "components/ble/HttpService.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

namespace {
  int HttpServiceCallback(uint16_t /*conn_handle*/, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto* httpService = static_cast<HttpService*>(arg);
    return httpService->OnHttpRequested(attr_handle, ctxt);
  }
}

HttpService::HttpService() :
    characteristicDefinition {{.uuid = &httpRequestUuid.u,
                               .access_cb = HttpServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &httpRequestHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &httpServiceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
}

void HttpService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int HttpService::OnHttpRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == httpRequestHandle) {
    NRF_LOG_INFO("HTTP request received");
    
    // Parse the request
    HttpRequest request;
    uint8_t* data = context->om->om_data;
    uint16_t length = context->om->om_len;
    
    if (length < 3) { // Minimum size for method + urlLength
      NRF_LOG_ERROR("Invalid request length");
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    
    request.method = data[0];
    request.urlLength = data[1];
    
    if (request.urlLength >= (int) sizeof(request.url)) {
      NRF_LOG_ERROR("URL too long");
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    
    if (length < 3 + request.urlLength) {
      NRF_LOG_ERROR("Invalid request length for URL");
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    
    memcpy(request.url, &data[2], request.urlLength);
    request.url[request.urlLength] = '\0';
    
    // Parse body if present
    if (length > 3 + request.urlLength) {
      request.bodyLength = (data[2 + request.urlLength] << 8) | data[3 + request.urlLength];
      if (request.bodyLength >=  (int)  sizeof(request.body)) {
        NRF_LOG_ERROR("Body too long");
        request.bodyLength = 0;
      } else if (length >= 5 + request.urlLength + request.bodyLength) {
        memcpy(request.body, &data[4 + request.urlLength], request.bodyLength);
      } else {
        request.bodyLength = 0;
      }
    } else {
      request.bodyLength = 0;
    }
    
    NRF_LOG_INFO("HTTP Method: %d, URL: %s", request.method, request.url);
    
    // Process the request
    ProcessHttpRequest(request);
    
    return 0;
  }
  return 0;
}

void HttpService::ProcessHttpRequest(const HttpRequest& request) {
  NRF_LOG_INFO("Processing HTTP request");
  (void)request;
  
  HttpResponse response;
  response.statusCode = 200;
  
  // Set headers
  const char* headers = "Content-Type: text/plain\r\n";
  response.headersLength = strlen(headers);
  if (response.headersLength >= (int)  sizeof(response.headers)) {
    NRF_LOG_ERROR("Headers too long");
    response.headersLength = 0;
  } else {
    memcpy(response.headers, headers, response.headersLength);
  }
  
  // Set body
  const char* body = "Hello from ruru!";
  response.bodyLength = strlen(body);
  if (response.bodyLength >= (int)  sizeof(response.body)) {
    NRF_LOG_ERROR("Body too long");
    response.bodyLength = 0;
  } else {
    memcpy(response.body, body, response.bodyLength);
  }
  
  // Send response
  SendHttpResponse(0, response); // TODO: Get actual connection handle
}

void HttpService::SendHttpResponse(uint16_t connectionHandle, const HttpResponse& response) {
  NRF_LOG_INFO("Sending HTTP response");
  
  // Create response buffer
  uint8_t buffer[2048];
  size_t offset = 0;
  
  // Add status code
  buffer[offset++] = (response.statusCode >> 8) & 0xFF;
  buffer[offset++] = response.statusCode & 0xFF;
  
  // Add headers length
  buffer[offset++] = (response.headersLength >> 8) & 0xFF;
  buffer[offset++] = response.headersLength & 0xFF;
  
  // Add headers
  if (response.headersLength > 0) {
    memcpy(&buffer[offset], response.headers, response.headersLength);
    offset += response.headersLength;
  }
  
  // Add body length
  buffer[offset++] = (response.bodyLength >> 8) & 0xFF;
  buffer[offset++] = response.bodyLength & 0xFF;
  
  // Add body
  if (response.bodyLength > 0) {
    memcpy(&buffer[offset], response.body, response.bodyLength);
    offset += response.bodyLength;
  }
  
  // Send notification
  NotifyHttpResponse(connectionHandle, buffer, offset);

  // Call the callback if set
  if (responseCallback) {
    NRF_LOG_INFO("Calling response callback");
    responseCallback(response);
  }
}

void HttpService::NotifyHttpResponse(uint16_t connectionHandle, const uint8_t* data, size_t length) {
  NRF_LOG_INFO("Notifying HTTP response");
  
  auto* om = ble_hs_mbuf_from_flat(data, length);
  if (om != nullptr) {
    ble_gattc_notify_custom(connectionHandle, httpRequestHandle, om);
  } else {
    NRF_LOG_ERROR("Failed to create mbuf for response");
  }
}

void HttpService::SetResponseCallback(std::function<void(const HttpResponse&)> callback) {
  responseCallback = callback;
} 