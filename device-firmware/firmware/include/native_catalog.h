#pragma once
#include "cJSON.h"
#include "scene_pack.h"
#include <cstdio>
namespace snes::native_catalog {
constexpr size_t MAX_SCENES=48,MAX_LIBRARIES=8,MAX_ASSET=160*1024,MAX_JSON=24000;
struct Library {char id[20],name[40];};
struct Entry {char id[32],name[64],library[20],path[112],hash[65];uint32_t bytes;};
struct Catalog {uint32_t version,count,libraries;char revision[41];Library groups[MAX_LIBRARIES];Entry entries[MAX_SCENES];};
inline const char *str(cJSON *o,const char*k){auto*v=cJSON_GetObjectItemCaseSensitive(o,k);return cJSON_IsString(v)?v->valuestring:"";}
inline bool copy(char*d,size_t n,const char*s){auto len=std::strlen(s);if(!len||len>=n)return false;std::memcpy(d,s,len+1);return true;}
inline bool revision_valid(const std::string&s){return s.size()==40&&s.find_first_not_of("0123456789abcdef")==std::string::npos;}
inline bool parse(const std::string &body,const std::string &revision,Catalog &out,std::string &error){
 auto fail=[&](const char*s){error=s;return false;};
 if(body.size()>MAX_JSON||!revision_valid(revision))return fail("Invalid catalog size or revision");
 bool quoted=false,escaped=false;int depth=0;
 for(char c:body){if(!c)return fail("Invalid JSON");if(quoted){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quoted=false;}else if(c=='"')quoted=true;else if(c=='['||c=='{'){if(++depth>8)return fail("JSON nesting too deep");}else if(c==']'||c=='}'){if(--depth<0)return fail("Invalid JSON");}}
 auto*root=cJSON_ParseWithLengthOpts(body.c_str(),body.size()+1,nullptr,true);struct Cleanup{cJSON*p;~Cleanup(){cJSON_Delete(p);}} cleanup{root};
 auto*version=cJSON_GetObjectItemCaseSensitive(root,"version");auto*libs=cJSON_GetObjectItemCaseSensitive(root,"libraries"),*scenes=cJSON_GetObjectItemCaseSensitive(root,"scenes");
 if(std::strcmp(str(root,"format"),"SNTL")||!cJSON_IsNumber(version)||version->valuedouble!=1||!cJSON_IsArray(libs)||!cJSON_IsArray(scenes))return fail("Unsupported library format");
 std::memset(&out,0,sizeof(out));out.version=1;copy(out.revision,sizeof(out.revision),revision.c_str());cJSON*item=nullptr;
 cJSON_ArrayForEach(item,libs){if(out.libraries==MAX_LIBRARIES)return fail("Too many libraries");auto &l=out.groups[out.libraries];if(!copy(l.id,sizeof(l.id),str(item,"id"))||!copy(l.name,sizeof(l.name),str(item,"name"))||!packs::identifier(l.id)||std::string(l.name)=="Classic")return fail("Invalid library");for(size_t i=0;i<out.libraries;i++)if(!std::strcmp(out.groups[i].id,l.id)||!std::strcmp(out.groups[i].name,l.name))return fail("Duplicate library");++out.libraries;}
 cJSON_ArrayForEach(item,scenes){if(out.count==MAX_SCENES)return fail("Too many scenes");auto&e=out.entries[out.count];auto*b=cJSON_GetObjectItemCaseSensitive(item,"bytes");
 if(!copy(e.id,sizeof(e.id),str(item,"id"))||!copy(e.name,sizeof(e.name),str(item,"name"))||!copy(e.library,sizeof(e.library),str(item,"library"))||!copy(e.path,sizeof(e.path),str(item,"path"))||!copy(e.hash,sizeof(e.hash),str(item,"sha256"))||!packs::identifier(e.id)||!packs::valid_hash(e.hash)||!cJSON_IsNumber(b)||b->valuedouble!=b->valueint||b->valueint<110||b->valueint>int(MAX_ASSET))return fail("Invalid scene metadata or scene too large");
 bool found=false;for(size_t i=0;i<out.libraries;i++)if(!std::strcmp(out.groups[i].id,e.library))found=true;if(!found)return fail("Unknown library");
 std::string path=e.path;if(path.front()=='/'||path.find("..")!=std::string::npos||path.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_./")!=std::string::npos||path.size()<6||path.substr(path.size()-5)!=".sntl")return fail("Invalid asset path");
 for(size_t i=0;i<out.count;i++)if(!std::strcmp(out.entries[i].library,e.library)&&(!std::strcmp(out.entries[i].id,e.id)||!std::strcmp(out.entries[i].name,e.name)))return fail("Duplicate scene");e.bytes=b->valueint;++out.count;
 }
 if(!out.count||!out.libraries)return fail("Empty library");for(size_t l=0;l<out.libraries;l++){bool found=false;for(size_t i=0;i<out.count;i++)if(!std::strcmp(out.groups[l].id,out.entries[i].library))found=true;if(!found)return fail("Empty library group");}error.clear();return true;
}
}
