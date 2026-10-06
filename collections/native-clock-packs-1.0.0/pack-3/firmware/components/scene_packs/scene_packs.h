#pragma once
#include "esphome/core/component.h"
#include "scene_pack.h"
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
namespace snes {class Clock;}
namespace esphome::scene_packs {
class ScenePacks : public Component {
 public:
  void setup() override;
  void loop() override;
  void on_shutdown() override { cancel(); }
  void set_clock(snes::Clock *clock){clock_=clock;}
  bool request(const std::string &url,const std::string &sha);
  void cancel();
  bool select_builtin(const std::string &name);
  const std::string &state()const{return state_;}
  const std::string &error()const{return error_;}
  std::string active_id()const;
  size_t active_bytes()const;
  uint32_t worker_stack_free()const{return worker_stack_free_.load();}
 private:
  struct Request {uint32_t generation;char url[481];char sha[65];};
  struct Result {uint32_t generation;snes::packs::Pack *pack;char error[96];};
  QueueHandle_t requests_=nullptr,results_=nullptr;
  snes::Clock *clock_=nullptr;
  std::unique_ptr<snes::packs::Pack> active_;
  std::atomic<uint32_t> generation_{0},worker_stack_free_{0};
  std::string state_="builtin",error_;
  static void worker_entry(void *arg){static_cast<ScenePacks*>(arg)->worker();}
  void worker();
  Result download(const Request &request);
};
}
