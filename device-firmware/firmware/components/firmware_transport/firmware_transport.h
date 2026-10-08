#pragma once
#include "esphome/components/http_request/http_request.h"
#include "esphome/core/alloc_helpers.h"
#include "esphome/core/log.h"
#include "firmware_url.h"

namespace esphome::firmware_transport {
class FirmwareTransport : public http_request::HttpRequestComponent {
 public:
  void set_transport(http_request::HttpRequestComponent *transport) { transport_ = transport; }
  void set_credentials(const std::string &user, const std::string &password) {
    const auto text = user + ":" + password;
    authorization_ = "Basic " + base64_encode(reinterpret_cast<const uint8_t *>(text.data()), text.size());
  }
 protected:
  std::shared_ptr<http_request::HttpContainer> perform(
      const std::string &url, const std::string &method, const std::string &body,
      const std::vector<http_request::Header> &headers,
      const std::vector<std::string> &collect_headers) override {
    if (method != "GET" || !snes::firmware_url_allowed(url)) {
      ESP_LOGE("firmware_transport", "Rejected firmware download outside configured release host");
      return nullptr;
    }
    auto authenticated = headers;
    authenticated.push_back({"Authorization", authorization_});
    // Never put credentials in the URL: upstream logs URLs on HTTP errors.
    // The backing transport verifies TLS and has redirects disabled.
    return transport_->start(url, method, body, authenticated, collect_headers);
  }
  http_request::HttpRequestComponent *transport_{nullptr};
  std::string authorization_;
};
}
