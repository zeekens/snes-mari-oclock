#pragma once
#include <string>
namespace snes {
// Download credentials are scoped to this exact HTTPS host and release directory.
inline bool firmware_url_allowed(const std::string &url) {
  const std::string prefix = "https://git.local.weylandyutani.se/clock-firmware/releases/";
  if (url.compare(0, prefix.size(), prefix) != 0) return false;
  const auto filename = url.substr(prefix.size());
  return filename.size() > 4 && filename.substr(filename.size()-4) == ".bin" &&
         filename.find("..") == std::string::npos &&
         filename.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.") == std::string::npos;
}
}
