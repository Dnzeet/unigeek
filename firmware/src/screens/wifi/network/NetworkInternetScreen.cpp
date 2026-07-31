#include "NetworkInternetScreen.h"
#include "core/ScreenManager.h"
#include "WorldClockScreen.h"
#include "WikipediaScreen.h"
#include "WigleScreen.h"
#include "DownloadScreen.h"

void NetworkInternetScreen::onInit() {
  setItems(_items);
}

void NetworkInternetScreen::onItemSelected(uint8_t index) {
  switch (index) {
    case 0: Screen.push(new WorldClockScreen()); break;
    case 1: Screen.push(new WikipediaScreen());  break;
    case 2: Screen.push(new WigleScreen());      break;
    case 3: Screen.push(new DownloadScreen());   break;
  }
}

void NetworkInternetScreen::onBack() {
  Screen.goBack();
}
