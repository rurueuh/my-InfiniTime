#include "displayapp/screens/MyApp.h"
#include <nrf_log.h>

using namespace Pinetime::Applications::Screens;

MyApp::MyApp(Pinetime::Controllers::HttpService& httpService) : httpService(httpService) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "My App");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

  // Create a button to trigger HTTP request
  lv_obj_t* btn = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_align(btn, lv_scr_act(), LV_ALIGN_CENTER, 0, 20);
  
  lv_obj_t* label = lv_label_create(btn, nullptr);
  lv_label_set_text_static(label, "Make Request");
  
  lv_obj_set_event_cb(btn, [](lv_obj_t* obj, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
      auto* app = static_cast<MyApp*>(obj->user_data);
      app->MakeHttpRequest();
    }
  });
  
  btn->user_data = this;

  // Create a container for the response
  lv_obj_t* responseContainer = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(responseContainer, LV_HOR_RES - 20, 100);
  lv_obj_align(responseContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_cont_set_fit(responseContainer, LV_FIT_TIGHT);
  lv_cont_set_layout(responseContainer, LV_LAYOUT_COL_M);

  // Create response label inside the container
  responseLabel = lv_label_create(responseContainer, nullptr);
  lv_label_set_text_static(responseLabel, "Response will appear here");
  lv_label_set_align(responseLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_long_mode(responseLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(responseLabel, LV_HOR_RES - 30);
  lv_obj_set_style_local_text_color(responseLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Set up HTTP response callback
  httpService.SetResponseCallback([this](const Pinetime::Controllers::HttpResponse& response) {
    HandleHttpResponse(response);
  });
}

void MyApp::MakeHttpRequest() {
  // Create a simple GET request
  Pinetime::Controllers::HttpRequest request;
  request.method = 0; // GET method
  const char* url = "http://panel.armeenapoleon.rurueuh.fr/";
  request.urlLength = strlen(url);
  memcpy(request.url, url, request.urlLength);
  request.url[request.urlLength] = '\0';
  request.bodyLength = 0;
  
  // Process the request
  httpService.ProcessHttpRequest(request);
}

void MyApp::HandleHttpResponse(const Pinetime::Controllers::HttpResponse& response) {
  // Create a buffer to store the response text
  char responseText[256];
  
  // Format the response with a maximum length
  int maxBodyLength = std::min(response.bodyLength, static_cast<uint16_t>(100));
  snprintf(responseText, sizeof(responseText), 
           "Status: %d\nBody: %.*s", 
           response.statusCode,
           maxBodyLength,
           response.body);
  
  // Update the label with the response
  lv_label_set_text(responseLabel, responseText);
}

void MyApp::Refresh() {
  // Refresh implementation if needed
}

MyApp::~MyApp() {
  lv_obj_clean(lv_scr_act());
}
