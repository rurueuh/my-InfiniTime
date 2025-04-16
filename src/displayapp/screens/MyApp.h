#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MyApp : public Screen {
      public:
        MyApp();
        ~MyApp() override;
      };
    }

    template <>
    struct AppTraits<Apps::MyApp> {
      static constexpr Apps app = Apps::MyApp;
      static constexpr const char* icon = Screens::Symbols::myApp;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::MyApp();
      }
    };
  }
}
