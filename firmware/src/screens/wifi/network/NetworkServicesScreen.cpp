#include "NetworkServicesScreen.h"
#include "core/ScreenManager.h"
#include "WebFileManagerScreen.h"
#ifdef HAS_NET_TOOLS
#include "Socks4ProxyScreen.h"
#endif

void NetworkServicesScreen::onInit() {
  setItems(_items);
}

void NetworkServicesScreen::onItemSelected(uint8_t index) {
  switch (index) {
    case 0: Screen.push(new WebFileManagerScreen()); break;
#ifdef HAS_NET_TOOLS
    case 1: Screen.push(new Socks4ProxyScreen());    break;
#endif
  }
}

void NetworkServicesScreen::onBack() {
  Screen.goBack();
}
