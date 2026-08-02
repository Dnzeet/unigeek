#pragma once

#include "ui/templates/ListScreen.h"

// Traffic interception and credential capture. Kept apart from Pranks on
// purpose: everything here is disruptive or captures other people's data.
class NetworkAttacksScreen : public ListScreen
{
public:
  const char* title() override { return "Attacks"; }

  void onInit() override;
  void onItemSelected(uint8_t index) override;
  void onBack() override;

private:
#ifdef HAS_NET_TOOLS
  ListItem _items[3] = {
#else
  ListItem _items[2] = {
#endif
    {"MITM Attack"},    // ARP poisoning + forwarding + capture + DNS spoof
    {"DHCP Attack"},    // starvation, rogue leases, deauth burst
#ifdef HAS_NET_TOOLS
    {"Responder"},      // LLMNR/NBT-NS poisoning, NetNTLMv2 capture
#endif
  };
};
