#include "native_library.h"
#include "clock_core.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "mbedtls/sha256.h"
#include "esphome/components/api/api_server.h"
#include <algorithm>
#include <cstdio>
namespace esphome::native_library {
namespace {
const char*names[]={"Mario","Luigi Forest","Cape Luigi Forest","Ghost House","Tide Pool","Lava Fortress","Star Road","Bonus Room"};
const char*ids[]={"mario","luigi","cape-luigi","ghost-house","tide-pool","lava-fortress","star-road","bonus-room"};
std::string origin(const char*rev){return "https://raw.githubusercontent.com/zeekens/snes-mari-oclock/"+std::string(rev)+"/";}
struct HTTP {
 esp_http_client_handle_t c=nullptr;std::string error;int64_t length=0,deadline=0;
 ~HTTP(){if(c){esp_http_client_close(c);esp_http_client_cleanup(c);}}
 bool open(const std::string&url,size_t limit){
  esp_http_client_config_t cfg{};cfg.url=url.c_str();cfg.timeout_ms=2000;cfg.disable_auto_redirect=true;cfg.buffer_size=1024;cfg.buffer_size_tx=512;cfg.crt_bundle_attach=esp_crt_bundle_attach;cfg.user_agent="SNES-SNTL/1";
  c=esp_http_client_init(&cfg);deadline=esp_timer_get_time()+60000000;
  if(!c||esp_http_client_open(c,0)!=ESP_OK){error="Connection or TLS failed";return false;}
  length=esp_http_client_fetch_headers(c);if(esp_http_client_get_status_code(c)!=200){error="HTTP error or GitHub rate limit";return false;}
  if(length>int64_t(limit)){error="Download exceeds size limit";return false;}return true;
 }
 int read(char*dst,size_t size){if(esp_timer_get_time()>deadline){error="Download timed out";return -1;}return esp_http_client_read(c,dst,size);}
};
}
bool Store::open(unsigned bank,bool write){const char*name=bank?"sntl_b":"sntl_a";return nvs_open(name,write?NVS_READWRITE:NVS_READONLY,&handle)==ESP_OK;}
uint8_t Store::read(size_t p){
 if(p>=size){good=false;return 0;}size_t index=p/CHUNK;
 if(cache[last].index==index){cache[last].touched=++tick;return cache[last].bytes[p%CHUNK];}
 unsigned oldest=0;for(unsigned i=0;i<CACHES;i++){if(cache[i].index==index){last=i;cache[i].touched=++tick;return cache[i].bytes[p%CHUNK];}if(cache[i].touched<cache[oldest].touched)oldest=i;}
 last=oldest;auto &page=cache[last];page.touched=++tick;char key[12];std::snprintf(key,sizeof(key),"p%u",unsigned(index));size_t length=std::min(CHUNK,size-index*CHUNK);
 if(nvs_get_blob(handle,key,page.bytes,&length)!=ESP_OK||length!=std::min(CHUNK,size-index*CHUNK)){good=false;return 0;}page.index=index;return page.bytes[p%CHUNK];
}
void NativeLibrary::setup(){
 catalog_.reset(new snes::native_catalog::Catalog{});catalog_pref_=global_preferences->make_preference<snes::native_catalog::Catalog>(0x534e5401);
 if(!catalog_pref_.load(catalog_.get())||catalog_->version!=1||catalog_->count>snes::native_catalog::MAX_SCENES||catalog_->libraries>snes::native_catalog::MAX_LIBRARIES){std::memset(catalog_.get(),0,sizeof(*catalog_));catalog_->version=1;}
 else library_status_="Cached: "+std::to_string(catalog_->libraries)+" libraries";
 saved_pref_=global_preferences->make_preference<Saved>(0x534e5402);
 if(!saved_pref_.load(&saved_)||saved_.version!=1||saved_.bank<0||saved_.bank>1){std::memset(&saved_,0,sizeof(saved_));saved_.version=1;std::strcpy(saved_.library,"classic");std::strcpy(saved_.scene,"mario");
  char old[32]{};auto pref=global_preferences->make_preference<char[32]>(0x534e5301);if(pref.load(&old)){old[31]=0;for(auto id:ids)if(std::strcmp(id,old)==0)std::strcpy(saved_.scene,id);}}
 saved_.library[19]=0;saved_.scene[31]=0;
 requests_=xQueueCreate(1,sizeof(Request));results_=xQueueCreate(1,sizeof(Result));
 if(!requests_||!results_||xTaskCreate(worker_entry,"sntl",16384,this,1,nullptr)!=pdPASS){mark_failed();error_="Cannot start SNTL worker";}
}
void NativeLibrary::save(){saved_pref_.save(&saved_);global_preferences->sync();}
void NativeLibrary::attach(NativeSelect*s,bool library){if(library)library_select_=s;else scene_select_=s;options();}
void NativeLibrary::options(){
 if(library_select_){FixedVector<const char*>opts;opts.init(1+catalog_->libraries);opts.push_back("Classic");const char*selected="Classic";for(size_t i=0;i<catalog_->libraries;i++){auto&g=catalog_->groups[i];opts.push_back(g.name);if(std::strcmp(g.id,saved_.library)==0)selected=g.name;}library_select_->traits.set_options(opts);library_select_->publish_state(selected);}
 if(scene_select_){FixedVector<const char*>opts;opts.init(8+catalog_->count);const char*selected_name=nullptr;
  if(std::strcmp(saved_.library,"classic")==0){for(size_t i=0;i<8;i++){opts.push_back(names[i]);if(std::strcmp(saved_.scene,ids[i])==0)selected_name=names[i];}}
  else for(size_t i=0;i<catalog_->count;i++){auto&e=catalog_->entries[i];if(std::strcmp(e.library,saved_.library)==0){opts.push_back(e.name);if(std::strcmp(e.id,saved_.scene)==0)selected_name=e.name;}}
  if(opts.size()==0){std::strcpy(saved_.library,"classic");std::strcpy(saved_.scene,"mario");options();return;}
  scene_select_->traits.set_options(opts);scene_select_->publish_state(selected_name?selected_name:opts[0]);
 }
}
const snes::native_catalog::Entry*NativeLibrary::selected()const{for(size_t i=0;i<catalog_->count;i++){auto&e=catalog_->entries[i];if(std::strcmp(e.id,saved_.scene)==0&&std::strcmp(e.library,saved_.library)==0)return &e;}return nullptr;}
void NativeLibrary::choose(const std::string&value,bool library){
 if(library){
  if(value=="Classic"){select_builtin("Mario");return;}
  for(size_t i=0;i<catalog_->libraries;i++){auto&g=catalog_->groups[i];if(value==g.name){std::strcpy(saved_.library,g.id);for(size_t j=0;j<catalog_->count;j++)if(std::strcmp(catalog_->entries[j].library,g.id)==0){std::strcpy(saved_.scene,catalog_->entries[j].id);break;}options();restore_scene();for(const auto&c:api::global_api_server->active_clients())c->on_disconnect_response();return;}}
 }else{
  if(std::strcmp(saved_.library,"classic")==0){select_builtin(value);return;}
  for(size_t i=0;i<catalog_->count;i++){auto&e=catalog_->entries[i];if(std::strcmp(e.library,saved_.library)==0&&value==e.name){std::strcpy(saved_.scene,e.id);options();request_scene(e);return;}}
 }
}
bool NativeLibrary::select_builtin(const std::string&name){
 for(size_t i=0;i<8;i++)if(name==names[i]||name==ids[i]){if(!clock_)return false;bool changed=std::strcmp(saved_.library,"classic")!=0;cancel();clock_->set_scene(names[i]);active_.reset();std::strcpy(saved_.library,"classic");std::strcpy(saved_.scene,ids[i]);state_="builtin";error_.clear();save();options();if(changed)for(const auto&c:api::global_api_server->active_clients())c->on_disconnect_response();return true;}return false;
}
void NativeLibrary::restore_scene(){if(!clock_)return;if(std::strcmp(saved_.library,"classic")==0){std::string id=saved_.scene;select_builtin(id);return;}if(auto*e=selected())request_scene(*e);}
void NativeLibrary::request_scene(const snes::native_catalog::Entry&e){
 if(is_failed())return;
 if(active_&&std::strcmp(active_meta_.hash,e.hash)==0){cancel();state_="ready";save();return;}
 Request r{};r.generation=++generation_;r.bank=saved_.bank;r.entry=e;std::strcpy(r.revision,catalog_->revision);state_="loading";error_.clear();xQueueOverwrite(requests_,&r);
}
std::string NativeLibrary::active_id()const{if(active_)return active_meta_.id;if(!clock_)return "mario";return clock_->extra_scene?ids[clock_->extra_scene+2]:clock_->cape?"cape-luigi":clock_->luigi?"luigi":"mario";}
bool NativeLibrary::refresh_library(bool ready){
 if(!ready){library_status_="Error: Wi-Fi or time not ready";return false;}if(is_failed()||heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)<65000){library_status_="Error: insufficient memory";return false;}
 Request r{};r.generation=++generation_;r.refresh=true;library_status_="Refreshing";state_=active_?"ready":"builtin";xQueueOverwrite(requests_,&r);return true;
}
bool NativeLibrary::fetch(const std::string&url,size_t limit,uint32_t gen,std::string&body,std::string&error){
 HTTP http;if(!http.open(url,limit)){error=http.error;return false;}body.clear();char buf[512];
 while(!esp_http_client_is_complete_data_received(http.c)){if(gen!=generation_){error="Cancelled";return false;}int n=http.read(buf,sizeof(buf));if(n<=0){if(n==0&&esp_http_client_is_complete_data_received(http.c))break;error="Truncated or stalled catalog";return false;}if(body.size()+size_t(n)>limit){error="Catalog too large";return false;}body.append(buf,n);vTaskDelay(1);}return true;
}
NativeLibrary::Result NativeLibrary::work(const Request&r){
 Result out{};out.generation=r.generation;out.refresh=r.refresh;std::string error;
 auto fail=[&](const std::string&s){std::snprintf(out.error,sizeof(out.error),"%s",s.c_str());return out;};
 if(r.refresh){std::string body;if(!fetch("https://api.github.com/repos/zeekens/snes-mari-oclock/git/ref/heads/main",2048,r.generation,body,error))return fail(error);auto*json=cJSON_Parse(body.c_str());std::string rev=snes::native_catalog::str(cJSON_GetObjectItemCaseSensitive(json,"object"),"sha");cJSON_Delete(json);if(!snes::native_catalog::revision_valid(rev))return fail("Invalid GitHub revision");if(!fetch(origin(rev.c_str())+"libraries/sntl-v1/catalog.json",snes::native_catalog::MAX_JSON,r.generation,body,error))return fail(error);auto cat=std::unique_ptr<snes::native_catalog::Catalog>(new(std::nothrow) snes::native_catalog::Catalog{});if(!cat)return fail("Catalog allocation failed");if(!snes::native_catalog::parse(body,rev,*cat,error))return fail(error);out.catalog=cat.release();return out;}
 auto store=std::unique_ptr<Store>(new(std::nothrow) Store);if(!store)return fail("Cache allocation failed");Metadata meta{};size_t length=sizeof(meta);bool cached=store->open(r.bank)&&nvs_get_blob(store->handle,"meta",&meta,&length)==ESP_OK&&length==sizeof(meta)&&meta.version==2&&meta.bytes==r.entry.bytes&&std::strcmp(meta.hash,r.entry.hash)==0&&std::strcmp(meta.id,r.entry.id)==0&&std::strcmp(meta.library,r.entry.library)==0;
 int bank=r.bank;
 if(!cached){store.reset(new Store);bank=1-r.bank;if(!store->open(bank,true))return fail("Cannot open flash cache");
  if(nvs_erase_all(store->handle)!=ESP_OK||nvs_commit(store->handle)!=ESP_OK)return fail("Cannot clear inactive flash cache");
  HTTP http;if(!http.open(origin(r.revision)+r.entry.path,snes::native_catalog::MAX_ASSET))return fail(http.error);
  if(http.length!=r.entry.bytes)return fail("Asset length mismatch");
  auto bytes=std::unique_ptr<uint8_t[]>(new(std::nothrow) uint8_t[Store::CHUNK]);if(!bytes)return fail("Download allocation failed");size_t used=0;
  while(used<r.entry.bytes){if(r.generation!=generation_)return fail("Cancelled");size_t need=std::min(Store::CHUNK,size_t(r.entry.bytes)-used),got=0;while(got<need){int n=http.read(reinterpret_cast<char*>(bytes.get()+got),need-got);if(n<=0)return fail("Truncated asset download");got+=n;}char key[12];std::snprintf(key,sizeof(key),"p%u",unsigned(used/Store::CHUNK));if(nvs_set_blob(store->handle,key,bytes.get(),need)!=ESP_OK)return fail("Flash cache full or write failed");used+=need;vTaskDelay(1);}
  if(nvs_commit(store->handle)!=ESP_OK)return fail("Flash commit failed");meta.version=2;meta.bytes=r.entry.bytes;std::strcpy(meta.hash,r.entry.hash);std::strcpy(meta.id,r.entry.id);std::strcpy(meta.library,r.entry.library);
 }
 store->size=r.entry.bytes;
 // Hash the persisted bytes, not just the network buffer, before activation.
 mbedtls_sha256_context sha;mbedtls_sha256_init(&sha);mbedtls_sha256_starts(&sha,0);uint8_t chunk[256],digest[32];
 for(size_t p=0;p<store->size;p+=sizeof(chunk)){if(r.generation!=generation_){mbedtls_sha256_free(&sha);return fail("Cancelled");}size_t n=std::min(sizeof(chunk),store->size-p);for(size_t i=0;i<n;i++)chunk[i]=store->read(p+i);mbedtls_sha256_update(&sha,chunk,n);if(p%4096==0)vTaskDelay(1);}
 mbedtls_sha256_finish(&sha,digest);mbedtls_sha256_free(&sha);char hash[65];for(size_t i=0;i<32;i++)std::snprintf(hash+i*2,3,"%02x",digest[i]);if(!store->good||std::strcmp(hash,r.entry.hash))return fail("Stored asset SHA-256 mismatch");
 auto player=std::unique_ptr<snes::native_scene::Player>(new snes::native_scene::Player);if(!player->open_reader(store.get(),store->size,Store::byte)||!store->good)return fail("Invalid SNTL asset");
 if(!cached&&(nvs_set_blob(store->handle,"meta",&meta,sizeof(meta))!=ESP_OK||nvs_commit(store->handle)!=ESP_OK))return fail("Cannot save verified asset metadata");
 out.bank=bank;out.metadata=meta;out.store=store.release();out.player=player.release();return out;
}
void NativeLibrary::worker(){for(;;){Request r{};if(xQueueReceive(requests_,&r,portMAX_DELAY)!=pdTRUE)continue;Result result=work(r);stack_free_=uxTaskGetStackHighWaterMark(nullptr);while(r.generation==generation_){if(xQueueSend(results_,&result,pdMS_TO_TICKS(20))==pdTRUE){result.store=nullptr;result.player=nullptr;result.catalog=nullptr;break;}}delete result.store;delete result.player;delete result.catalog;}}
void NativeLibrary::loop(){
 if(!clock_||!results_)return;Result r{};if(xQueueReceive(results_,&r,0)!=pdTRUE)return;
 std::unique_ptr<Store>store(r.store);std::unique_ptr<snes::native_scene::Player>player(r.player);std::unique_ptr<snes::native_catalog::Catalog>cat(r.catalog);
 if(r.generation!=generation_)return;
 if(r.error[0]){if(r.refresh)library_status_=std::string("Error: ")+r.error;else{state_="error";error_=r.error;}return;}
 if(cat){if(!catalog_pref_.save(cat.get())||!global_preferences->sync()){library_status_="Error: catalog storage failed";return;}catalog_=std::move(cat);options();library_status_="Ready: "+std::to_string(catalog_->libraries)+" libraries / "+std::to_string(catalog_->count)+" scenes";for(const auto&c:api::global_api_server->active_clients())c->on_disconnect_response();return;}
 if(store&&player){clock_->activate_native(*player,millis());active_=std::move(store);active_meta_=r.metadata;saved_.bank=r.bank;state_="ready";error_.clear();save();}
}
}
