#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "components/ble/HttpService.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MyApp : public Screen {
      public:
        MyApp(Pinetime::Controllers::HttpService& httpService);
        ~MyApp() override;
        void Refresh() override;

      private:
        Pinetime::Controllers::HttpService& httpService;
        void MakeHttpRequest();
      };
    }

    template <>
    struct AppTraits<Apps::MyApp> {
      static constexpr Apps app = Apps::MyApp;
      static constexpr const char* icon = Screens::Symbols::info;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::MyApp(controllers.httpService);
      }
    };
  }
}
