#pragma once
#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "esphome/core/preferences.h"
#include "native_catalog.h"
#include "native_scene.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>
namespace snes {class Clock;}
namespace esphome::native_library {
class NativeSelect;
struct Store {
 static constexpr size_t CHUNK=512,CACHES=16;
 nvs_handle_t handle=0;size_t size=0;bool good=true;unsigned last=0;uint32_t tick=0;
 struct Page {size_t index=SIZE_MAX;uint32_t touched=0;uint8_t bytes[CHUNK];};Page cache[CACHES];
 ~Store(){if(handle)nvs_close(handle);}
 bool open(unsigned bank,bool write=false);
 uint8_t read(size_t p);
 static uint8_t byte(void*ctx,size_t p){return static_cast<Store*>(ctx)->read(p);}
};
class NativeLibrary:public Component {
 public:
 void setup() override;void loop() override;void on_shutdown() override {++generation_;}
 void set_clock(snes::Clock*c){clock_=c;set_timeout("restore",10000,[this]{restore_scene();});}
 void attach(NativeSelect*s,bool library);void choose(const std::string&value,bool library);
 bool refresh_library(bool ready=true);void cancel(){++generation_;state_=active_?"ready":"builtin";}
 bool select_builtin(const std::string &name);
 const std::string&state()const{return state_;}const std::string&error()const{return error_;}
 const std::string&library_status()const{return library_status_;}
 std::string active_id()const;size_t active_bytes()const{return active_?active_->size:0;}
 uint32_t worker_stack_free()const{return stack_free_;}
 private:
 struct Saved {uint32_t version;int bank;char library[20],scene[32];};
 struct Metadata {uint32_t version,bytes;char library[20],id[32],hash[65];};
 struct Request {uint32_t generation;bool refresh;int bank;char revision[41];snes::native_catalog::Entry entry;};
 struct Result {uint32_t generation;bool refresh;int bank;Store *store; snes::native_scene::Player*player;snes::native_catalog::Catalog*catalog;Metadata metadata;char error[128];};
 Saved saved_{};Metadata active_meta_{};ESPPreferenceObject saved_pref_,catalog_pref_;
 std::unique_ptr<snes::native_catalog::Catalog> catalog_;std::unique_ptr<Store> active_;
 NativeSelect *library_select_=nullptr,*scene_select_=nullptr;snes::Clock*clock_=nullptr;
 QueueHandle_t requests_=nullptr,results_=nullptr;std::atomic<uint32_t> generation_{0},stack_free_{0};
 std::string state_="builtin",error_,library_status_="Not refreshed";bool restoring_=true;
 static void worker_entry(void*p){static_cast<NativeLibrary*>(p)->worker();}void worker();
 Result work(const Request&r);bool fetch(const std::string&url,size_t limit,uint32_t gen,std::string&body,std::string&error);
 void options();void restore_scene();void request_scene(const snes::native_catalog::Entry&e);
 const snes::native_catalog::Entry*selected()const;void save();
};
class NativeSelect:public select::Select,public Component {
 public:void set_parent(NativeLibrary*p,bool l){parent_=p;library_=l;}void setup()override{parent_->attach(this,library_);}float get_setup_priority()const override{return -10;}
 protected:void control(const std::string&v)override{parent_->choose(v,library_);}NativeLibrary*parent_;bool library_;
};
}
