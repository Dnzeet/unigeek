//
// DhcpAttackScreen — the DHCP half of the old MITM screen, on its own.
//
// Starvation and Rogue DHCP live together because they are one technique: the
// starvation exists to drain the real server's pool so our OFFER is the only
// one left when a client asks. Deauth Burst is the trigger — devices already
// on the network hold their lease for hours and renew by unicast to the real
// server, which we never see, so something has to make them re-DHCP.
//
// Note on effectiveness: DhcpStarvation puts the ESP32's real MAC in `chaddr`
// (the driver will not accept replies to a fabricated one) and only varies
// option 61. Servers that key leases on chaddr — most consumer routers — hand
// back the same address every time and never exhaust. dnsmasq-based firmware
// honours option 61 and does.
//

#pragma once

#include "ui/templates/ListScreen.h"
#include "ui/views/LogView.h"
#include "utils/network/DhcpStarvation.h"
#include "utils/network/RogueDhcpServer.h"
#include "utils/network/WifiAttackUtil.h"

class DhcpAttackScreen : public ListScreen {
public:
  const char* title()    override { return "DHCP Attack"; }
  bool inhibitPowerOff() override { return _state == STATE_RUNNING; }

  ~DhcpAttackScreen();

  void onInit() override;
  void onUpdate() override;
  void onRender() override;
  void onItemSelected(uint8_t index) override;
  void onBack() override;

private:
  enum State { STATE_MENU, STATE_RUNNING };
  State _state = STATE_MENU;

  bool _starvEnabled = false;
  bool _rogueEnabled = false;
  bool _deauthBurst  = false;

  String _starvSub, _rogueSub, _deauthSub;
  ListItem _menuItems[4];

  DhcpStarvation  _starv;
  RogueDhcpServer _rogue;
  WifiAttackUtil* _attacker = nullptr;

  bool _starvRunning  = false;
  bool _rogueRunning  = false;
  bool _deauthRunning = false;
  unsigned long _deauthStart = 0;
  unsigned long _lastDraw    = 0;

  // Saved for the post-deauth reconnect.
  String    _savedSSID;
  String    _savedPassword;
  uint8_t   _savedBSSID[6] = {};
  uint8_t   _savedChannel  = 0;
  IPAddress _savedIP, _savedGateway, _savedSubnet;

  LogView _log;

  void _showMenu();
  void _start();
  void _stop();
  void _startRogue();
  void _startDeauthBurst();
  void _stopDeauthBurst();
  void _reconnectStaticIP();
  void _drawLog();

  static DhcpAttackScreen* _instance;
  static void _onDhcpClient(const char* mac, const char* ip);
};
