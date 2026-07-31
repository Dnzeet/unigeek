# DHCP Attack

Drains the real DHCP server's address pool and serves leases in its place, so clients take the ESP32 as their gateway and DNS server. Accessed from **WiFi > Network > Attacks > DHCP Attack**. Must be connected to a WiFi network first.

> [!warn]
> Exhausting a DHCP pool, sending deauthentication frames, or running a rogue DHCP server on a network you do not own is illegal in most jurisdictions. Only test on networks and clients you control.

Previously these lived inside the MITM screen. They were split out when MITM was rebuilt around ARP spoofing — the two attacks pursue the same goal (become the victim's gateway) by different means, and ARP spoofing does it without waiting for a lease renewal.

## Setup

1. Connect to the target network via **WiFi > Network**
2. Go to **WiFi > Network > Attacks > DHCP Attack**
3. Toggle the components:
   - **DHCP Starvation** — Exhaust the real router's pool
   - **Rogue DHCP** — Serve leases naming ourselves as gateway and DNS
   - **Deauth Burst** — 10 s of deauth after starvation to force clients to re-request
4. **Start**

## Components

### DHCP Starvation

Sends a stream of DISCOVER/REQUEST pairs with randomised MACs drawn from real vendor OUIs, each with a fresh transaction id. Considers the pool exhausted after 50 NAKs.

### Rogue DHCP

Binds UDP 67 on the STA interface and answers DISCOVER with OFFER and REQUEST with ACK, handing out option 3 (router) and option 6 (DNS) pointing at the ESP32, plus option 252 (WPAD). Pool of 50 addresses starting at `.100`.

### Deauth Burst

Devices already on the network hold their lease for hours and renew by **unicast directly to the real server**, which we never see. Something has to make them re-request, so this sends a 10-second deauth burst, then reconnects with a static IP and starts the rogue server.

## Attack flow

**Full chain:** starvation exhausts the pool → deauth burst forces clients off → ESP32 reconnects with a static IP → rogue DHCP serves the reconnecting clients.

**Without deauth:** starvation runs, then rogue DHCP waits for natural lease renewals. Much slower.

**Rogue DHCP alone:** starts immediately and races the real server. Usually loses — a dedicated router answers faster than an ESP32 in an Arduino loop.

## Why starvation often fails

`DhcpStarvation` puts the ESP32's **real MAC** in the `chaddr` field and only varies option 61 (Client Identifier). This is a hardware constraint, not an oversight: the WiFi driver will not deliver replies addressed to a fabricated MAC.

The consequence is that starvation only works against servers that key leases on **option 61**. Most consumer and ISP routers key on `chaddr`, see the same client every time, and simply re-offer the same address — the pool never empties. In that case `ack` climbs, `nak` stays at zero, `isExhausted()` never trips, and the run ends at `isStuck()` (20 consecutive timeouts) reporting `Starvation stuck`.

dnsmasq-based firmware (OpenWrt, pfSense) does honour option 61 and can be starved.

## During the attack

Status bar shows starvation counters (`A:` ack, `N:` nak, `T:` timeout, `CT:` consecutive timeouts), the deauth countdown, or the rogue server's client count. Press **BACK** to stop.

## Notes and limits

- No traffic forwarding. Clients that accept a rogue lease point their default route at the ESP32, which does not route — they lose internet access rather than being transparently intercepted. Only what the device serves locally (DNS, portals) reaches them. For actual interception use **MITM Attack**.
- 802.11w (PMF) clients and APs ignore unprotected deauth frames. WPA3 mandates PMF, and WPA2 with PMF is now common, so the burst may not disconnect anyone.
- Networks with DHCP snooping or 802.1X will resist this.
- Devices on other VLANs are unaffected.
