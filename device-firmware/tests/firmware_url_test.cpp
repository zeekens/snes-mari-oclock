#include "../firmware/include/firmware_url.h"
#include <cassert>
int main() {
  const std::string base="https://git.local.weylandyutani.se/clock-firmware/releases/";
  assert(snes::firmware_url_allowed(base+"sntl-2.1.1.bin"));
  for (auto path:{"../secret.bin", "%2e%2e/secret.bin", "x.bin?token=a", "x.bin#x", "sub/x.bin", "x.bin/", "x.txt"})
    assert(!snes::firmware_url_allowed(base+path));
  assert(!snes::firmware_url_allowed("http://git.local.weylandyutani.se/clock-firmware/releases/x.bin"));
  assert(!snes::firmware_url_allowed("https://git.local.weylandyutani.se.evil/clock-firmware/releases/x.bin"));
  assert(!snes::firmware_url_allowed("https://user:password@git.local.weylandyutani.se/clock-firmware/releases/x.bin"));
}
