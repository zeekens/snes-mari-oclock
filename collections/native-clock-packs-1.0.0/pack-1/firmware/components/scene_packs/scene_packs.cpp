#include "scene_packs.h"
#include "clock_core.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include <algorithm>
#include <cstdio>

namespace esphome::scene_packs {
void ScenePacks::setup(){
  requests_=xQueueCreate(1,sizeof(Request));results_=xQueueCreate(1,sizeof(Result));
  if(!requests_||!results_||xTaskCreate(worker_entry,"scene_packs",8192,this,1,nullptr)!=pdPASS){
    error_="Could not start pack worker";state_="error";mark_failed();
  }
}
bool ScenePacks::request(const std::string &url,const std::string &sha){
  if(is_failed()||!clock_){error_="Pack loader unavailable";return false;}
  if(!snes::packs::valid_url(url)||!snes::packs::valid_hash(sha)){error_="Invalid pack URL or SHA-256";return false;}
  if(active_&&active_->hash==sha){cancel();clock_->activate_pack(*active_);state_="ready";error_.clear();return true;}
  // Include TLS and worker costs in the live gate, not just payload length.
  if(heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)<65536 ||
     heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)<snes::packs::MAX_BYTES){
    error_="Insufficient free heap for download";return false;
  }
  Request req{};req.generation=++generation_;std::memcpy(req.url,url.c_str(),url.size()+1);std::memcpy(req.sha,sha.c_str(),65);
  error_.clear();state_="loading";xQueueOverwrite(requests_,&req);return true;
}
void ScenePacks::cancel(){++generation_;state_=clock_&&clock_->active_pack?"ready":"builtin";}
bool ScenePacks::select_builtin(const std::string &name){
  if(!clock_||!clock_->set_scene(name))return false;
  cancel();active_.reset();error_.clear();return true;
}
std::string ScenePacks::active_id()const{
  if(!clock_)return "mario";
#ifdef SNES_NATIVE_SCENES
  if(clock_->native_asset)return clock_->native_asset->id;
#endif
  if(clock_->active_pack)return clock_->active_pack->id;
  return clock_->cape?"cape-luigi":clock_->luigi?"luigi":"mario";
}
size_t ScenePacks::active_bytes()const{
#ifdef SNES_NATIVE_SCENES
  if(clock_&&clock_->native_asset)return clock_->native_asset->size;
#endif
  return clock_&&clock_->active_pack?clock_->active_pack->size:0;
}
void ScenePacks::loop(){
  if(!results_||!clock_)return;
  Result result{};
  if(xQueueReceive(results_,&result,0)==pdTRUE){
    std::unique_ptr<snes::packs::Pack> candidate(result.pack);
    if(result.generation!=generation_)return;
    if(!candidate){error_=result.error;state_="error";return;}
    clock_->activate_pack(*candidate);active_=std::move(candidate);state_="ready";error_.clear();
  }
}
ScenePacks::Result ScenePacks::download(const Request &req){
  Result out{};out.generation=req.generation;
  auto fail=[&](const char *error){std::snprintf(out.error,sizeof(out.error),"%s",error);return out;};
  if(req.generation!=generation_)return fail("Cancelled");
  esp_http_client_config_t config{};config.url=req.url;config.timeout_ms=2000;
  config.disable_auto_redirect=true;config.buffer_size=1024;config.buffer_size_tx=512;
  config.crt_bundle_attach=esp_crt_bundle_attach;config.user_agent="SNES-Clock-Packs/1";
  auto client=esp_http_client_init(&config);if(!client)return fail("HTTP initialization failed");
  struct Cleanup{esp_http_client_handle_t c;~Cleanup(){esp_http_client_close(c);esp_http_client_cleanup(c);}} cleanup{client};
  const int64_t deadline=esp_timer_get_time()+15000000;
  if(esp_http_client_open(client,0)!=ESP_OK)return fail("Connection or TLS verification failed");
  int64_t length=esp_http_client_fetch_headers(client);
  if(esp_http_client_get_status_code(client)!=200)return fail("HTTP status is not 200 (redirects disabled)");
  // Static hosts provide Content-Length. Reject unknown/chunked bodies to bound allocations.
  if(length<static_cast<int64_t>(snes::packs::HEADER_BYTES)||length>static_cast<int64_t>(snes::packs::MAX_BYTES)||esp_http_client_is_chunked_response(client))return fail("Invalid HTTP content length");
  std::unique_ptr<uint8_t[]> bytes(new(std::nothrow) uint8_t[size_t(length)]);if(!bytes)return fail("Pack allocation failed");
  size_t used=0;
  while(used<size_t(length)){
    if(req.generation!=generation_)return fail("Cancelled");
    if(esp_timer_get_time()>deadline)return fail("Download timed out");
    int n=esp_http_client_read(client,reinterpret_cast<char*>(bytes.get()+used),int(std::min(size_t(1024),size_t(length)-used)));
    if(n<=0)return fail("Truncated or stalled download");used+=size_t(n);
    vTaskDelay(1);  // Keep the renderer and network service runnable between chunks.
  }
  if(req.generation!=generation_)return fail("Cancelled");
  std::string error;auto pack=snes::packs::Pack::parse(std::move(bytes),used,req.sha,error);
  if(!pack)return fail(error.c_str());
  out.pack=pack.release();return out;
}
void ScenePacks::worker(){
  for(;;){
    Request req{};if(xQueueReceive(requests_,&req,portMAX_DELAY)!=pdTRUE)continue;
    Result result=download(req);worker_stack_free_=uxTaskGetStackHighWaterMark(nullptr);
    while(req.generation==generation_){
      if(xQueueSend(results_,&result,pdMS_TO_TICKS(20))==pdTRUE){result.pack=nullptr;break;}
    }
    delete result.pack;
  }
}
}
