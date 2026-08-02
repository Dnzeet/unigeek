//
// PcapWriter — buffered capture file writer shared by the MITM tooling.
//
// Both capture paths feed this: the relay (decrypted Ethernet frames,
// LINKTYPE_ETHERNET) and the 802.11 monitor fallback (raw radio frames,
// LINKTYPE_IEEE802_11). Frames are staged in a 4 KB buffer so the SD card
// sees one bulk write per block instead of one write per frame — at 4 MHz SPI
// the per-write overhead is what decides whether frames get dropped.
//

#pragma once

#include <Arduino.h>
#include <FS.h>

#include "core/IStorage.h"

class PcapWriter {
public:
  static constexpr uint32_t LINKTYPE_ETHERNET   = 1;
  static constexpr uint32_t LINKTYPE_IEEE802_11 = 105;

  // Opens <dir>/<ssid>_<MM-DD-YYYY>.pcap. Appends when the file already exists
  // so a second run on the same network and day keeps the same name — a PCAP is
  // a global header followed by records, so appending stays valid.
  bool begin(IStorage* fs, const char* dir, const String& ssid,
             uint32_t linktype, uint32_t snapLen);
  void end();

  bool writeFrame(const uint8_t* data, uint16_t len, uint16_t origLen,
                  uint32_t tsSec, uint32_t tsUsec);
  bool flush();

  bool          ok()       const { return _ok; }
  const char*   error()    const { return _error; }
  const String& filename() const { return _filename; }
  uint32_t      frames()   const { return _frames; }
  uint64_t      bytes()    const { return _bytes; }

  // Syncs the clock over NTP when it was never set (these boards have no
  // battery-backed RTC) and formats MM-DD-YYYY in UTC. Returns false and
  // writes "00-00-0000" when no time could be obtained.
  static bool   resolveDate(char* out, size_t n);
  static String sanitize(const String& s);

  // Epoch seconds / millis snapshot taken at begin(), so producers can stamp
  // frames without calling time() on every packet.
  uint32_t baseEpoch() const { return _baseEpoch; }
  uint32_t baseMs()    const { return _baseMs; }

private:
  static constexpr size_t WBUF_SIZE = 4096;

  IStorage*   _fs    = nullptr;
  fs::File    _file;
  String      _filename;
  // Allocated on begin(), released on end(). As a plain member it sat in the
  // MITM screen's object for the whole session even with capture off, and heap
  // is the binding constraint once the portal server starts serving files.
  uint8_t*    _buf   = nullptr;
  size_t      _len       = 0;
  uint32_t    _frames    = 0;
  uint64_t    _bytes     = 0;
  uint32_t    _baseEpoch = 0;
  uint32_t    _baseMs    = 0;
  bool        _ok        = false;
  const char* _error     = nullptr;
};
