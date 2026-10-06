#pragma once
#include "reference_sprites.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <string>

namespace snes::packs {
constexpr size_t MAX_BYTES = 16384;
constexpr size_t HEADER_BYTES = 48;
constexpr size_t MAX_PALETTE = 512;
constexpr std::array<uint16_t,5> REQUIRED = {0x000f,0x8000,0x03f0,0x2000,0x7c00};
inline uint16_t u16(const uint8_t *p){return uint16_t(p[0]) | uint16_t(p[1])<<8;}
inline uint32_t u32(const uint8_t *p){return uint32_t(u16(p)) | uint32_t(u16(p+2))<<16;}
inline bool identifier(const std::string &s){
  if(s.empty()||s.size()>31)return false;
  for(char c:s)if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'))return false;
  return true;
}
// Small portable SHA-256, shared by the native validator and the device.
inline uint32_t ror(uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));}
inline std::string sha256(const uint8_t *data,size_t size){
  static constexpr uint32_t k[]={
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  uint32_t h[]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
  const size_t blocks=(size+9+63)/64;
  for(size_t block=0;block<blocks;block++){
    uint32_t w[64]{};
    for(size_t i=0;i<64;i++){
      size_t at=block*64+i;uint8_t b=0;
      if(at<size)b=data[at];else if(at==size)b=0x80;
      else if(at>=blocks*64-8)b=uint8_t((uint64_t(size)*8)>>((blocks*64-1-at)*8));
      w[i/4]|=uint32_t(b)<<(24-8*(i%4));
    }
    for(int i=16;i<64;i++)w[i]=w[i-16]+(ror(w[i-15],7)^ror(w[i-15],18)^(w[i-15]>>3))+w[i-7]+(ror(w[i-2],17)^ror(w[i-2],19)^(w[i-2]>>10));
    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],v=h[7];
    for(int i=0;i<64;i++){
      uint32_t t=v+(ror(e,6)^ror(e,11)^ror(e,25))+((e&f)^(~e&g))+k[i]+w[i];
      uint32_t s=(ror(a,2)^ror(a,13)^ror(a,22))+((a&b)^(a&c)^(b&c));
      v=g;g=f;f=e;e=d+t;d=c;c=b;b=a;a=t+s;
    }
    h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=v;
  }
  std::string out;out.reserve(64);for(uint32_t v:h)for(int i=7;i>=0;i--)out.push_back("0123456789abcdef"[(v>>(i*4))&15]);return out;
}
inline bool valid_hash(const std::string &hash){
  if(hash.size()!=64)return false;
  for(char c:hash)if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;
  return true;
}
// Public HTTPS origins or local HTTP. No credentials, queries, redirects or encoded paths.
inline bool valid_url(const std::string &url){
  if(url.size()>480)return false;
  bool tls=url.compare(0,8,"https://")==0;size_t start=tls?8:7;
  if(!tls&&url.compare(0,7,"http://")!=0)return false;
  size_t slash=url.find('/',start);if(slash==std::string::npos)return false;
  std::string authority=url.substr(start,slash-start),host=authority;
  size_t colon=authority.find(':');
  if(colon!=std::string::npos){
    host=authority.substr(0,colon);auto port=authority.substr(colon+1);unsigned n=0;
    if(port.empty()||port.size()>5)return false;
    for(char c:port){if(c<'0'||c>'9')return false;n=n*10+unsigned(c-'0');}if(!n||n>65535)return false;
  }
  if(host.empty())return false;
  for(char c:host)if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='.'||c=='-'))return false;
  auto path=url.substr(slash);
  if(path.size()<5||path.substr(path.size()-4)!=".scn"||path.find("..")!=std::string::npos)return false;
  for(char c:path)if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='/'||c=='.'||c=='-'||c=='_'))return false;
  if(host=="raw.githubusercontent.com"){
    if(!tls||colon!=std::string::npos)return false;
    size_t a=path.find('/',1),b=a==std::string::npos?a:path.find('/',a+1),c=b==std::string::npos?b:path.find('/',b+1);
    if(a<=1||a==std::string::npos||b==std::string::npos||b==a+1||c==std::string::npos||c-b!=41)return false;
    for(size_t i=b+1;i<c;i++)if(!((path[i]>='0'&&path[i]<='9')||(path[i]>='a'&&path[i]<='f')))return false;
  }
  if(tls)return true;
  if(host=="localhost"||(host.size()>6&&host.substr(host.size()-6)==".local"))return true;
  unsigned oct[4]={},part=0,digits=0;
  for(char c:host){if(c=='.'){if(!digits||part==3)return false;part++;digits=0;}else{if(c<'0'||c>'9'||++digits>3)return false;oct[part]=oct[part]*10+unsigned(c-'0');if(oct[part]>255)return false;}}
  return part==3&&digits&&(oct[0]==10||oct[0]==127||(oct[0]==192&&oct[1]==168)||(oct[0]==172&&oct[1]>=16&&oct[1]<=31));
}
struct Pack {
  std::unique_ptr<uint8_t[]> bytes;
  size_t size=0;
  uint8_t engine=0;
  std::string id,hash;
  std::array<art::Sprite,16> sprites{};
  std::array<uint32_t,MAX_PALETTE> colours{};
  uint16_t present=0;
  static std::unique_ptr<Pack> parse(std::unique_ptr<uint8_t[]> data,size_t size,const std::string &hash,std::string &error){
    auto fail=[&](const char *why)->std::unique_ptr<Pack>{error=why;return nullptr;};
    if(!data||size<HEADER_BYTES||size>MAX_BYTES)return fail("Invalid pack length");
    if(!valid_hash(hash)||sha256(data.get(),size)!=hash)return fail("SHA-256 mismatch");
    const uint8_t *p=data.get();
    if(std::memcmp(p,"SNPK",4)||p[4]!=1||p[5]<1||p[5]>5||p[6]>16||!p[6]||p[7]||u32(p+8)!=size||u32(p+12))return fail("Unsupported pack header");
    if(p[47]||std::memchr(p+16,0,32)==nullptr)return fail("Invalid pack ID");
    std::string id(reinterpret_cast<const char*>(p+16));if(!identifier(id))return fail("Invalid pack ID");
    for(size_t i=16+id.size();i<48;i++)if(p[i])return fail("Noncanonical pack ID");
    std::unique_ptr<Pack> out(new(std::nothrow) Pack);if(!out)return fail("Out of memory");
    out->engine=p[5];out->id=id;out->size=size;out->hash=hash;
    size_t offset=HEADER_BYTES,palette_used=0;
    for(unsigned i=0;i<p[6];i++){
      if(size-offset<8)return fail("Truncated sprite header");
      unsigned slot=p[offset],width=p[offset+1],height=p[offset+2],count=u16(p+offset+4),pixels=u16(p+offset+6);
      if(slot>=16||(out->present&(1u<<slot))||p[offset+3]||!width||width>32||!height||height>32||count<2||count>256||pixels!=width*height||palette_used+count>MAX_PALETTE)return fail("Invalid sprite descriptor");
      offset+=8;size_t need=count*4+pixels;if(need>size-offset)return fail("Truncated sprite data");
      for(unsigned j=0;j<count;j++){uint32_t c=u32(p+offset+j*4);if(c>0xffffff)return fail("Invalid palette colour");out->colours[palette_used+j]=c;}
      if(out->colours[palette_used]!=0)return fail("Invalid transparency entry");
      offset+=count*4;
      for(unsigned j=0;j<pixels;j++)if(p[offset+j]>=count)return fail("Invalid palette index");
      out->sprites[slot]={int(width),int(height),p+offset,out->colours.data()+palette_used};out->present|=uint16_t(1u<<slot);
      offset+=pixels;palette_used+=count;
    }
    if(offset!=size||out->present!=REQUIRED[out->engine-1])return fail("Incorrect scene sprite set");
    out->bytes=std::move(data);error.clear();return out;
  }
};
}  // namespace snes::packs
