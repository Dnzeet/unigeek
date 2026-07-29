#pragma once

#include "ui/templates/ListScreen.h"

// Noisy but non-destructive LAN toys. Separate from Attacks so the line
// between "safe to demo" and "intercepts other people's traffic" stays visible
// in the menu itself.
class NetworkPranksScreen : public ListScreen
{
public:
  const char* title() override { return "Pranks"; }

  void onInit() override;
  void onItemSelected(uint8_t index) override;
  void onBack() override;

private:
  ListItem _items[3] = {
    {"Cast Bomb"},      // push a video at DIAL/Chromecast devices
    {"Bonjour Spam"},   // phantom mDNS services on the LAN
    {"Printer Prank"},  // short joke jobs over JetDirect
  };
};
