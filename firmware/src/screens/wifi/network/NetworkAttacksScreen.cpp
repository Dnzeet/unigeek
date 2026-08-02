#include "NetworkAttacksScreen.h"
#include "core/ScreenManager.h"
#include "NetworkMitmScreen.h"
#include "DhcpAttackScreen.h"
#ifdef HAS_NET_TOOLS
#include "ResponderScreen.h"
#endif

void NetworkAttacksScreen::onInit() {
  setItems(_items);
}

void NetworkAttacksScreen::onItemSelected(uint8_t index) {
  switch (index) {
    case 0: Screen.push(new NetworkMitmScreen()); break;
    case 1: Screen.push(new DhcpAttackScreen());  break;
#ifdef HAS_NET_TOOLS
    case 2: Screen.push(new ResponderScreen());   break;
#endif
  }
}

void NetworkAttacksScreen::onBack() {
  Screen.goBack();
}
