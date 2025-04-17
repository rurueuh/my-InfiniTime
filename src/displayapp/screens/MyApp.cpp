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

void MyApp::Refresh() {
  // Refresh implementation if needed
}

MyApp::~MyApp() {
  lv_obj_clean(lv_scr_act());
}
