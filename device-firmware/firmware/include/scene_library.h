#pragma once
#include "scene_pack.h"
#include "cJSON.h"
#include <cstdio>
namespace snes::library {
constexpr size_t MAX_ENTRIES=24, MAX_JSON=16384;
struct Entry {char id[32],name[49],file[81],hash[65];};
struct Catalog {uint32_t version=1,count=0,skipped=0;char revision[41]{};Entry entries[MAX_ENTRIES]{};};
inline bool revision_valid(const std::string &s){return s.size()==40&&s.find_first_not_of("0123456789abcdef")==std::string::npos;}
inline const char *str(cJSON *o,const char *key){auto *v=cJSON_GetObjectItemCaseSensitive(o,key);return cJSON_IsString(v)?v->valuestring:"";}
inline bool copy(char *dst,size_t capacity,const char *s){size_t n=std::strlen(s);if(!n||n>=capacity)return false;std::memcpy(dst,s,n+1);return true;}
inline bool parse(const std::string &body,const std::string &revision,Catalog &out,std::string &error){
  auto fail=[&](const char *s){error=s;return false;};
  if(body.size()>MAX_JSON||!revision_valid(revision))return fail("Invalid catalog size or revision");
  // Keep the recursive JSON parser within the bounded worker stack.
  int depth=0;bool quoted=false,escaped=false;
  for(char c:body){
    if(c=='\0')return fail("Invalid JSON string");
    if(quoted){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quoted=false;continue;}
    if(c=='"')quoted=true;
    else if(c=='{'||c=='['){if(++depth>8)return fail("Catalog nesting too deep");}
    else if(c=='}'||c==']'){if(--depth<0)return fail("Invalid JSON nesting");}
  }
  auto *root=cJSON_ParseWithLengthOpts(body.c_str(),body.size()+1,nullptr,true);
  struct Cleanup {cJSON *p;~Cleanup(){cJSON_Delete(p);}} cleanup{root};
  auto *scenes=cJSON_GetObjectItemCaseSensitive(root,"scenes");
  auto *version=cJSON_GetObjectItemCaseSensitive(root,"version");
  if(std::strcmp(str(root,"format"),"SNPK")||!cJSON_IsNumber(version)||version->valuedouble!=1||!cJSON_IsArray(scenes))return fail("Unsupported catalog format");
  // Reset fields in place: a temporary Catalog would consume almost the whole
  // download worker stack on the ESP32.
  out.version=1;out.count=0;out.skipped=0;
  std::memset(out.entries,0,sizeof(out.entries));
  std::memset(out.revision,0,sizeof(out.revision));
  copy(out.revision,sizeof(out.revision),revision.c_str());
  cJSON *item=nullptr;
  cJSON_ArrayForEach(item,scenes){
    auto *engine=cJSON_GetObjectItemCaseSensitive(item,"engine"),*bytes=cJSON_GetObjectItemCaseSensitive(item,"bytes");
    if(!cJSON_IsNumber(engine)||!cJSON_IsNumber(bytes))return fail("Missing engine or size");
    if(engine->valuedouble!=engine->valueint||engine->valueint<1||engine->valueint>5||bytes->valuedouble>snes::packs::MAX_BYTES){++out.skipped;continue;}
    if(bytes->valuedouble!=bytes->valueint||bytes->valueint<int(snes::packs::HEADER_BYTES))return fail("Invalid pack size");
    if(out.count==MAX_ENTRIES)return fail("Catalog exceeds 24 compatible scenes");
    auto &e=out.entries[out.count];
    if(!copy(e.id,sizeof(e.id),str(item,"id"))||!copy(e.name,sizeof(e.name),str(item,"name"))||!copy(e.file,sizeof(e.file),str(item,"file"))||!copy(e.hash,sizeof(e.hash),str(item,"sha256")))return fail("Invalid catalog fields");
    if(std::string(e.id).find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-")!=std::string::npos||!snes::packs::valid_hash(e.hash))return fail("Invalid ID or hash");
    std::string file=e.file;
    if(file.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-._")!=std::string::npos||file.find("..")!=std::string::npos||file.size()<5||file.substr(file.size()-4)!=".scn")return fail("Invalid pack filename");
    for(const unsigned char c:std::string(e.name))if(c<32||c>126)return fail("Invalid scene name");
    if(std::string(e.id)=="mario"||std::string(e.id)=="luigi"||std::string(e.id)=="cape-luigi"||std::string(e.name)=="Mario"||std::string(e.name)=="Luigi Forest"||std::string(e.name)=="Cape Luigi Forest")return fail("Scene conflicts with built-in");
    for(size_t i=0;i<out.count;++i)if(!std::strcmp(e.id,out.entries[i].id)||!std::strcmp(e.name,out.entries[i].name))return fail("Duplicate scene ID or name");
    ++out.count;
  }
  if(!out.count)return fail("Catalog contains no compatible scenes");
  error.clear();return true;
}
inline std::string base(const Catalog &c){return "https://raw.githubusercontent.com/zeekens/snes-mari-oclock/"+std::string(c.revision)+"/packs/";}
}
