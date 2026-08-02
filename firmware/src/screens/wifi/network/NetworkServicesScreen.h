#pragma once

#include "ui/templates/ListScreen.h"

// Things the device *hosts* for the rest of the LAN — the mirror image of the
// Internet menu, which is about what the device consumes.
class NetworkServicesScreen : public ListScreen
{
public:
  const char* title() override { return "Services"; }

  void onInit() override;
  void onItemSelected(uint8_t index) override;
  void onBack() override;

private:
#ifdef HAS_NET_TOOLS
  ListItem _items[2] = {
#else
  ListItem _items[1] = {
#endif
    {"Web File Manager"},  // browse device storage from a browser
#ifdef HAS_NET_TOOLS
    {"SOCKS4 Proxy"},      // pivot through the device
#endif
  };
};
