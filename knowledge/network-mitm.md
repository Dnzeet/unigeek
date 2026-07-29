# Network MITM Attack

Man-in-the-Middle against the network you are joined to. ARP cache poisoning pulls the other hosts' traffic through the ESP32, which forwards it on to the real gateway while capturing it. Accessed from **WiFi > Network > Attacks > MITM Attack**. Must be connected to a WiFi network first.

> [!warn]
> ARP poisoning and traffic capture on a network you do not own is illegal in most jurisdictions. Only test on networks and clients you control.

## Setup

1. Connect to the target network via **WiFi > Network**
2. Go to **WiFi > Network > Attacks > MITM Attack**
3. Toggle the components you want:
   - **ARP Spoof** — Sweep the subnet and poison every host found
   - **Network Sniffer** — Write the traffic to a PCAP on the SD card
4. **Start**

## Attack Components

### ARP Spoof

On start, sweeps the whole subnet with broadcast ARP requests (up to 254 hosts) and builds a table from the replies. It then poisons continuously in both directions:

- the victim is told the **gateway** is at our MAC
- the gateway is told the **victim** is at our MAC

Both halves are required — with only the victim poisoned you see one direction of every conversation. Hosts that appear later (gratuitous ARP, new joiners) are picked up automatically and added to the rotation.

What actually keeps a victim poisoned is not the periodic gratuitous reply — a host following the RFC 4861 neighbour state machine leaves an unsolicited reply as stale and only relearns from the answer to its **own** request. So the attack answers every request it sees for the gateway address, immediately and again about 30 ms later. The delayed copy is the one that matters: the real router answers the same broadcast, and the cache keeps whichever reply arrives last.

On stop, the correct mappings are broadcast three times and then sent per-target twice. Every restore frame carries our own MAC as the Ethernet source — we are an associated station, so the driver encrypts with our pairwise key and a frame claiming someone else's address is one the AP silently discards. The correction lives in the ARP sender fields, which is what a cache keys on. Stopping any other way (power loss, crash) leaves victims offline until their ARP cache ages out.

### Network Sniffer

Writes captured frames to `/unigeek/wifi/sniffer/<SSID>_<MM-DD-YYYY>.pcap`, ready for Wireshark. Running again on the same network and day appends to the same file.

The capture mode depends on ARP Spoof:

| ARP Spoof | Link type | What you get |
|---|---|---|
| On | `LINKTYPE_ETHERNET` (1) | Victims' traffic, **already decrypted** by the WiFi driver — HTTP, FTP, DNS readable with no Wireshark key setup |
| Off | `LINKTYPE_IEEE802_11` (105) | Every frame on the current channel, but **CCMP-encrypted** on a WPA2 network |

The 802.11 fallback is the wider net (all networks on the channel, management frames, handshakes) but the payloads stay opaque unless the capture also contains that client's 4-way handshake and you supply the passphrase to Wireshark.

## Why there is no DNS spoofing here

There used to be a third toggle that forged DNS answers in transit. It was removed, and the reason is worth recording because it looks like it should work.

Serving the forged pages needs an HTTP server. On a board without PSRAM — the Cardputer ADV among them — the WiFi driver allocates its TX buffers from the same heap that AsyncTCP allocates from. Serving one portal page drove the free heap to **1 KB**, at which point the driver refused **54%** of all ARP frames. The poison stopped being refreshed, the victim's cache went to `Probe` and then back to the real gateway within seconds, and the interception the DNS spoof depended on was gone. Relaying traffic and serving pages do not both fit in this device.

> [!tip]
> For DNS spoofing with a captive portal, use **WiFi > Access Point** instead. There the clients are yours and point at the device as their resolver, so there is no relay competing for the heap.

## How forwarding works

`CONFIG_LWIP_IP_FORWARD` is compiled out of the Arduino ESP32 lwIP build, so the network stack will not route transit packets. `MitmRelay` does it in application code instead:

1. `esp_wifi_internal_reg_rxcb()` intercepts inbound frames **after** the driver decrypts them
2. Frames addressed to us go to `esp_netif_receive()` so the device keeps its own connectivity
3. Transit frames get their destination MAC rewritten and go back out through `esp_wifi_internal_tx()`, which re-encrypts them on the normal data path

Without step 3 the ARP spoof would be a denial of service, not an interception. This is why the previous version of this screen never actually intercepted anything.

Three details in that path are load-bearing:

- **The gateway's own address is a valid destination.** It is held separately from the target table and never enters it, so a lookup miss plus "this is a local address" used to black-hole it. That path ate every DNS query in the common setup where the router is also the resolver.
- **The RX hook is re-armed every two seconds.** `esp_wifi_internal_reg_rxcb()` is a single slot that `esp_netif` also uses, and anything that re-attaches the station interface puts its own callback back. Nothing reports that — the relay just stops seeing frames while every other counter keeps climbing.
- **Radio power save is turned off for the session.** Arduino leaves the station in `WIFI_PS_MIN_MODEM`, which queues our traffic at the AP between beacons. A relay cannot be duty-cycled.

## During the attack

During discovery the status bar shows sweep progress. Once poisoning starts it shows hosts, gateway state and forwarded frames, plus the PCAP count when the sniffer is on. On the right it shows free heap and the low-water mark, as `20k/14k` — the second number is the one to watch, because the WiFi driver takes its TX buffers from that same heap.

Every ten seconds two fuller lines go to the log:

| Field | Meaning |
|---|---|
| `S` | frames the RX hook was handed at all |
| `F` | frames forwarded — the proof this is interception and not a denial of service |
| `X` | forwards the driver refused |
| `L` | transit frames dropped instead of being looped back |

`S` and `F` together separate two failures that look identical from the victim. `S` flat while a host is poisoned means its traffic is not reaching us; `S` climbing with `F` stuck means it reaches us and the relay will not forward it.

| Field | Meaning |
|---|---|
| `A` | ARP frames sent, then refused after `!` |
| `R` | deferred gateway claims sent, then lost after `!` and dropped after `d` |
| `G` | times the router broadcast its own address, correcting every cache at once |

A climbing `!` on `A` means the heap is gone and the poison is about to start failing. `R…!` is a refresh the real router won. `d` is the retry queue overflowing, which needs more slots rather than more retries.

Press **BACK** to stop and restore the ARP tables.

Both MAC addresses are printed in the log when the attack starts — `Us` (the device) and `GW` (the gateway's real address) — so a poisoned victim's ARP entry can be compared against them without leaving the screen.

## Storage

```
/unigeek/wifi/sniffer/<SSID>_<date>.pcap   Capture file
```

## Notes and limits

- **Throughput.** Every victim packet passes through the ESP32 and then to the SD card. Expect a few Mbps at best — victims will notice the network is slow. MITM here is not discreet.
- **HTTPS stays opaque.** Interception gives you the TCP stream, but TLS is end-to-end.
- **Heap is the real ceiling.** On a board without PSRAM the driver's TX buffers compete with everything else running. If the low-water mark on the status bar falls under ~15 KB, ARP frames start being refused and victims drift back to the real gateway. Run the MITM screen on its own.
- **IPv6 escapes entirely.** ARP is IPv4-only; IPv6 uses NDP. A device preferring IPv6 bypasses the attack.
- **Defended networks resist it.** Dynamic ARP Inspection, client isolation, and AP-level ARP protection all break the poisoning. Assume enterprise networks are out.
- The sweep covers at most 254 addresses. Subnets larger than a /24 are only partially covered.
- Devices on other VLANs are unreachable.

## Achievements

| Achievement | Tier |
|------------|------|
| **Man in the Middle** | Silver |
