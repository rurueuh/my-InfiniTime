#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include <string_view>

namespace Pinetime::Applications::Screens {

class MyApp : public Screen {
public:
  MyApp();
  ~MyApp() override;

  static MyApp* Self;                       // singleton pour le callback

  static void SendUrl(std::string_view url);          // Notify -> tel
  void OnHttpResponse(const char* data, size_t len);  // Write  <- tel

private:
  lv_obj_t* title {};
};

} // namespace Pinetime::Applications::Screens


/* ---------------- AppTraits : indispensable ---------------- */
namespace Pinetime::Applications {
  template <>
  struct AppTraits<Apps::MyApp> {
    static constexpr Apps app = Apps::MyApp;
    static constexpr const char* icon = Screens::Symbols::info;
    static Screens::Screen* Create(AppControllers&) {
      return new Screens::MyApp();
    }
  };
}
