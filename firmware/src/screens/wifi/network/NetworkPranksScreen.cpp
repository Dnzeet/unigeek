#include "NetworkPranksScreen.h"
#include "core/ScreenManager.h"
#include "CastBombScreen.h"
#include "BonjourSpamScreen.h"
#include "PrinterPrankScreen.h"

void NetworkPranksScreen::onInit() {
  setItems(_items);
}

void NetworkPranksScreen::onItemSelected(uint8_t index) {
  switch (index) {
    case 0: Screen.push(new CastBombScreen());    break;
    case 1: Screen.push(new BonjourSpamScreen()); break;
    case 2: Screen.push(new PrinterPrankScreen()); break;
  }
}

void NetworkPranksScreen::onBack() {
  Screen.goBack();
}
