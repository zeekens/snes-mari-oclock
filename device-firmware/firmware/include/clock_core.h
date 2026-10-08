#pragma once
#ifdef SNES_NATIVE_STREAM
#include "native_scene.h"
#endif
#include <array>
#include <cmath>
#include "reference_sprites.h"
#include "luigi_sprites.h"
#ifndef SNES_REMOTE_SELECTED_ONLY
#include "selected_sprites.h"
#endif
#include "scene_pack.h"
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace snes {
// Time differences remain valid across the uint32_t millis() rollover.
inline uint32_t elapsed(uint32_t now, uint32_t then) { return now - then; }
inline bool decode(const std::string &s, std::vector<uint32_t> &out) {
  out.clear();
  for (size_t i=0; i<s.size();) {
    uint8_t b=s[i++]; uint32_t c=b; int n=0; uint32_t min=0;
    if(b>=0xc2 && b<=0xdf){c=b&31;n=1;min=128;}
    else if(b>=0xe0 && b<=0xef){c=b&15;n=2;min=2048;}
    else if(b>=0xf0 && b<=0xf4){c=b&7;n=3;min=65536;}
    else if(b>=128) return false;
    for(int j=0;j<n;j++){if(i==s.size() || (uint8_t(s[i])&192)!=128)return false;c=(c<<6)|(uint8_t(s[i++])&63);}
    if(c<min || c>0x10ffff || (c>=0xd800 && c<=0xdfff))return false;
    if(c<32 && c!='\n')return false;
    if(c==127)return false;
    out.push_back(c);
  }
  return true;
}
struct Message {
  std::string text, id; bool fullscreen=false; uint32_t duration=15000, queued_at=0; int priority=0; bool textbox=false;
};
class Queue {
 public:
  static constexpr size_t CAPACITY=4; // active plus pending
  static constexpr uint32_t PENDING_TTL=60000;
  std::vector<Message> pending;
  Message active;
  bool showing=false;
  uint32_t started=0;
  std::string last_completed, last_error;
  size_t size() const { return pending.size()+(showing?1:0); }
  void tick(uint32_t now) {
    pending.erase(std::remove_if(pending.begin(),pending.end(),[now](const Message&m){return elapsed(now,m.queued_at)>=PENDING_TTL;}),pending.end());
    if(showing && elapsed(now,started)>=active.duration){last_completed=active.id;showing=false;}
    if(!showing && !pending.empty()){active=pending.front();pending.erase(pending.begin());started=now;showing=true;}
  }
  bool submit(const std::string &text,const std::string &mode,int seconds,int priority,const std::string &id,uint32_t now){
    std::vector<uint32_t> decoded;
    last_error.clear();
    if(text.empty() || text.size()>256 || !decode(text,decoded) || std::all_of(decoded.begin(),decoded.end(),[](uint32_t c){return c==' '||c=='\n';})) last_error="Text must be 1-256 UTF-8 bytes and contain visible characters";
    else if(mode!="banner" && mode!="fullscreen" && mode!="textbox")last_error="Unknown mode";
    else if(seconds<1 || seconds>300)last_error="Duration must be 1-300 seconds";
    else if(priority<0 || priority>1)last_error="Priority must be 0 or 1";
    else if(id.empty() || id.size()>48 || !std::all_of(id.begin(),id.end(),[](unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_';}))last_error="ID must be 1-48 letters, digits, hyphens or underscores";
    if(!last_error.empty())return false;
    tick(now);
    Message m{text,id,mode=="fullscreen",uint32_t(seconds)*1000,now,priority,mode=="textbox"};
    if(showing && active.id==id){active=m;started=now;return true;}
    auto existing=std::find_if(pending.begin(),pending.end(),[&](const Message &p){return p.id==id;});
    // Priority promotion also removes any pending duplicate.
    if(!showing || priority>active.priority){
      if(existing!=pending.end())pending.erase(existing);
      active=m;showing=true;started=now;return true;
    }
    if(existing!=pending.end()){*existing=m;return true;}
    if(size()>=CAPACITY){last_error="Queue is full";return false;}
    pending.push_back(m);return true;
  }
  void clear(const std::string &id,uint32_t now){
    pending.erase(std::remove_if(pending.begin(),pending.end(),[&](const Message&m){return m.id==id;}),pending.end());
    if(showing && active.id==id)showing=false;
    tick(now);
  }
  void clear_all(){pending.clear();showing=false;last_error.clear();}
};
// Compact hand-authored 5x7 font. Lowercase maps to uppercase; unsupported glyphs
// render as '?'. Swedish letters retain their diacritics.
inline std::array<uint8_t,7> glyph(uint32_t c){
  if(c>='a'&&c<='z')c-=32;
  if(c==0xe5)c=0xc5;if(c==0xe4)c=0xc4;if(c==0xf6)c=0xd6;
  switch(c){
#define G(ch,a,b,d,e,f,g,h) case ch:return {{a,b,d,e,f,g,h}};
G('A',14,17,17,31,17,17,17) G('B',30,17,17,30,17,17,30)
G('C',14,17,16,16,16,17,14) G('D',30,17,17,17,17,17,30)
G('E',31,16,16,30,16,16,31) G('F',31,16,16,30,16,16,16)
G('G',14,17,16,23,17,17,15) G('H',17,17,17,31,17,17,17)
G('I',14,4,4,4,4,4,14) G('J',7,2,2,2,18,18,12)
G('K',17,18,20,24,20,18,17) G('L',16,16,16,16,16,16,31)
G('M',17,27,21,21,17,17,17) G('N',17,25,21,19,17,17,17)
G('O',14,17,17,17,17,17,14) G('P',30,17,17,30,16,16,16)
G('Q',14,17,17,17,21,18,13) G('R',30,17,17,30,20,18,17)
G('S',15,16,16,14,1,1,30) G('T',31,4,4,4,4,4,4)
G('U',17,17,17,17,17,17,14) G('V',17,17,17,17,17,10,4)
G('W',17,17,17,21,21,21,10) G('X',17,17,10,4,10,17,17)
G('Y',17,17,10,4,4,4,4) G('Z',31,1,2,4,8,16,31)
G('0',14,17,19,21,25,17,14) G('1',4,12,4,4,4,4,14)
G('2',14,17,1,2,4,8,31) G('3',30,1,1,14,1,1,30)
G('4',2,6,10,18,31,2,2) G('5',31,16,16,30,1,1,30)
G('6',14,16,16,30,17,17,14) G('7',31,1,2,4,8,8,8)
G('8',14,17,17,14,17,17,14) G('9',14,17,17,15,1,1,14)
G(' ',0,0,0,0,0,0,0) G(':',0,4,4,0,4,4,0)
G('!',4,4,4,4,4,0,4) G('.',0,0,0,0,0,0,4)
G(',',0,0,0,0,0,4,8) G('-',0,0,0,31,0,0,0)
G('?',14,17,1,2,4,0,4) G('/',1,2,2,4,8,8,16)
G('(',2,4,8,8,8,4,2) G(')',8,4,2,2,2,4,8)
G(0xc5,4,10,14,17,31,17,17) G(0xc4,10,0,14,17,31,17,17)
G(0xd6,10,0,14,17,17,17,14)
#undef G
    default:return {{14,17,1,2,4,0,4}};
  }
}
using Frame=std::array<uint32_t,4096>;
class Clock {
 public:
  Queue queue; Frame pixels{}; bool enabled=true, animated=true, luigi=false, cape=false;
  bool smooth_background=true;
  // One reusable layer with an extra right-edge sample; no frame-sized stack buffers.
  std::array<uint8_t,65*64> scenery_layer{};
  std::array<uint32_t,256> scenery_palette{};
  uint16_t scenery_colors=1;
  bool drawing_layer=false;
  static constexpr uint32_t transparent=0xff000000;
  static uint32_t mix_color(uint32_t a,uint32_t b,uint32_t weight,uint32_t total){
    uint32_t out=0;
    for(int shift=0;shift<=16;shift+=8)
      out|=((((a>>shift)&255)*(total-weight)+((b>>shift)&255)*weight+total/2)/total)<<shift;
    return out;
  }
  void begin_scenery(){
    drawing_layer=smooth_background&&animated;
    if(drawing_layer){scenery_layer.fill(0);scenery_colors=1;}
  }
  void end_scenery(uint32_t now,uint32_t period){
    if(!drawing_layer)return;
    drawing_layer=false;
    const uint32_t weight=now%period;
    for(int y=0;y<64;y++)for(int x=0;x<64;x++){
      const uint32_t under=pixels[y*64+x];
      const uint8_t ia=scenery_layer[y*65+x],ib=scenery_layer[y*65+x+1];
      const uint32_t a=ia?scenery_palette[ia]:under,b=ib?scenery_palette[ib]:under;
      pixels[y*64+x]=mix_color(a,b,weight,period);
    }
  }
  int last_minute=-1, old_minute=0;
  uint32_t jump_started=0;
  bool jumping=false;
  static const art::Sprite &cape_walk_pose(uint32_t now){
    // Complete left-to-right cape walking row from the source sheet.
    // Eight distinct poses over 800ms, with no repeated idle hold at the seam.
    static const art::Sprite *const walk[]={
      &art::cape_luigi_walk4,&art::cape_luigi_walk6,&art::cape_luigi_idle,
      &art::cape_luigi_walk_up,&art::cape_luigi_step_up,
      &art::cape_luigi_step_back,&art::cape_luigi_step_low,&art::cape_luigi_walk_down
    };
    return *walk[(now/100)%8];
  }
  void reference_sprite(const art::Sprite &s,int x,int y){
    for(int row=0;row<s.height;row++)for(int col=0;col<s.width;col++){
      auto index=s.indices[row*s.width+col];
      if(index)pixel(x+col,y+row,s.palette[index]);
    }
  }
  void pixel(int x,int y,uint32_t color){
    if(y<0||y>=64||x<0)return;
    if(drawing_layer){
      if(x>=65)return;
      uint16_t index=1;
      while(index<scenery_colors&&scenery_palette[index]!=color)index++;
      // Scenery layers use at most a dozen colors; preserve their exact RGB values.
      if(index==scenery_colors){
        if(scenery_colors==256)return;
        scenery_palette[scenery_colors++]=color;
      }
      scenery_layer[y*65+x]=uint8_t(index);
    }
    else if(x<64)pixels[y*64+x]=color;
  }
  void rect(int x,int y,int w,int h,uint32_t color){for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)pixel(i,j,color);}
  void text(const std::vector<uint32_t>&s,int x,int y,uint32_t color,int scale=1){
    for(auto c:s){auto g=glyph(c);for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(g[row]&(1<<(4-col)))rect(x+col*scale,y+row*scale,scale,scale,color);x+=6*scale;}
  }
  void message_text(const std::vector<uint32_t>&s,int x,int y){
    for(auto c:s){
      auto g=glyph(c);
      // Compact mixed-case message alphabet, enlarged to a 6x9 pixel cell.
#define LOWER(ch,a,b,d,e,f,h,i) case ch:g={{a,b,d,e,f,h,i}};break;
      switch(c){
LOWER('a',0,0,14,1,15,17,15) LOWER('b',16,16,30,17,17,17,30)
LOWER('c',0,0,14,16,16,17,14) LOWER('d',1,1,15,17,17,17,15)
LOWER('e',0,0,14,17,31,16,14) LOWER('f',6,9,8,28,8,8,8)
LOWER('g',0,14,17,17,15,1,14) LOWER('h',16,16,30,17,17,17,17)
LOWER('i',4,0,12,4,4,4,14) LOWER('j',2,0,6,2,2,18,12)
LOWER('k',16,16,18,20,24,20,18) LOWER('l',12,4,4,4,4,4,14)
LOWER('m',0,0,26,21,21,21,21) LOWER('n',0,0,30,17,17,17,17)
LOWER('o',0,0,14,17,17,17,14) LOWER('p',0,30,17,17,30,16,16)
LOWER('q',0,15,17,17,15,1,1) LOWER('r',0,0,22,25,16,16,16)
LOWER('s',0,0,15,16,14,1,30) LOWER('t',8,8,28,8,8,9,6)
LOWER('u',0,0,17,17,17,19,13) LOWER('v',0,0,17,17,17,10,4)
LOWER('w',0,0,17,17,21,21,10) LOWER('x',0,0,17,10,4,10,17)
LOWER('y',0,17,17,17,15,1,14) LOWER('z',0,0,31,2,4,8,31)
LOWER(0xe4,10,0,14,1,15,17,15) LOWER(0xe5,4,0,14,1,15,17,15)
LOWER(0xf6,10,0,14,17,17,17,14)
        default:break;
      }
#undef LOWER
      for(int row=0;row<9;row++)for(int col=0;col<6;col++)
        if(g[row*7/9]&(1<<(4-col*5/6)))pixel(x+col,y+row,0xffffff);
      x+=7;
    }
  }
  void label(const std::string&s,int x,int y,uint32_t color,int scale=1){std::vector<uint32_t> v;decode(s,v);text(v,x,y,color,scale);}
  static std::vector<std::vector<uint32_t>> lines(const std::string&s,size_t columns=9){
    columns=std::max(size_t(1),columns);
    std::vector<uint32_t> codes;decode(s,codes);
    std::vector<std::vector<uint32_t>> result(1);
    // Configurable glyphs per line. Word-wrap when a word fits; hard-wrap longer tokens.
    for(size_t i=0;i<codes.size();){
      if(codes[i]=='\n'){result.emplace_back();i++;continue;}
      if(codes[i]==' '){if(!result.back().empty()&&result.back().size()<columns)result.back().push_back(' ');i++;continue;}
      size_t end=i;while(end<codes.size()&&codes[end]!=' '&&codes[end]!='\n')end++;
      size_t length=end-i;
      if(length<=columns && !result.back().empty() && result.back().size()+length>columns){while(!result.back().empty()&&result.back().back()==' ')result.back().pop_back();result.emplace_back();}
      while(i<end){if(result.back().size()==columns)result.emplace_back();result.back().push_back(codes[i++]);}
    }
    for(auto &line:result)while(!line.empty()&&line.back()==' ')line.pop_back();
    return result;
  }
  void forest(uint32_t now){
    // Forest of Illusion foliage, recomposed for the 64x64 clock viewport.
    // Scrolling speeds match the mountain/bush layers in the Mario scene.
    rect(0,18,64,37,0x94d9d7);
    const int far=animated?int(now/320)%96:0;
    const int near=animated?int(now/180)%80:0;
    begin_scenery();
    for(int base=-96;base<160;base+=96){
      for(int lobe=0;lobe<8;lobe++){
        const int cx=base+lobe*12-far;
        const int cy=26+(lobe%3)*4;
        for(int dx=-9;dx<=9;dx++){
          const int top=cy-int(std::sqrt(float(81-dx*dx)));
          rect(cx+dx,top,1,55-top,0xa0b000);
          rect(cx+dx,top+1,1,54-top,0x608000);
        }
        pixel(cx-2,cy+4,0xc0c828);pixel(cx-1,cy+5,0xc0c828);
        pixel(cx+2,cy+7,0xc0c828);
      }
    }
    end_scenery(now,320);
    begin_scenery();
    for(int base=-80;base<144;base+=80){
      const int bx=base+8-near;
      // Broad background trunks and dark, overlapping foreground leaves.
      rect(bx,32,9,23,0x504820);rect(bx+1,32,3,23,0x887030);
      rect(bx+6,34,1,21,0x302818);
      for(int k=0;k<5;k++){
        const int cx=bx+13+k*14,cy=48+(k%2)*3;
        for(int dx=-10;dx<=10;dx++){
          const int top=cy-int(std::sqrt(float(100-dx*dx)));
          rect(cx+dx,top,1,55-top,0x507800);
          rect(cx+dx,top+1,1,54-top,0x205800);
        }
        pixel(cx-3,cy,0x789000);pixel(cx-2,cy-1,0x789000);
        pixel(cx+1,cy+1,0x789000);
      }
    }
    end_scenery(now,180);
  }
// Included inside Clock. Live scenes: native indexed sprites, no stored movie frames.
  int extra_scene=0;
  const packs::Pack *active_pack=nullptr;
#ifdef SNES_NATIVE_STREAM
  native_scene::Player native_player;
  bool native_mode=false,native_repaint=true,native_message=false;
  uint32_t native_started=0;
  void activate_native(const native_scene::Player &player,uint32_t now){native_player=player;native_mode=true;native_repaint=true;native_started=now;active_pack=nullptr;last_minute=-1;}
#endif
  void activate_pack(const packs::Pack &pack){
    active_pack=&pack;extra_scene=pack.engine;luigi=cape=false;last_minute=-1;jumping=false;
  }
  const art::Sprite &selected_sprite(unsigned slot) const {
    if(active_pack)return active_pack->sprites[slot];
#ifndef SNES_REMOTE_SELECTED_ONLY
    static const art::Sprite *const builtins[]={
      &art::selected::boo0,&art::selected::boo1,&art::selected::shy0,&art::selected::shy1,
      &art::selected::bones0,&art::selected::bones1,&art::selected::torch0,&art::selected::torch1,
      &art::selected::torch2,&art::selected::torch3,&art::selected::coin0,&art::selected::coin1,
      &art::selected::coin2,&art::selected::star0,&art::selected::flower,&art::selected::fish};
    return *builtins[slot];
#else
    static const art::Sprite empty{0,0,nullptr,nullptr};return empty;
#endif
  }
  bool set_scene(const std::string &name){
#ifdef SNES_NATIVE_STREAM
    native_mode=false;
#endif
    int next=0;
    if(name=="ghost-house"||name=="Ghost House")next=1;
    else if(name=="tide-pool"||name=="Tide Pool")next=2;
    else if(name=="lava-fortress"||name=="Lava Fortress")next=3;
    else if(name=="star-road"||name=="Star Road")next=4;
    else if(name=="bonus-room"||name=="Bonus Room")next=5;
    else if(name!="mario"&&name!="Mario"&&name!="luigi"&&name!="Luigi Forest"&&name!="cape-luigi"&&name!="Cape Luigi Forest")return false;
#ifdef SNES_REMOTE_SELECTED_ONLY
    if(next)return false;  // New scenes are only activated after validating their downloaded pack.
#endif
    active_pack=nullptr;
    bool nextCape=name=="cape-luigi"||name=="Cape Luigi Forest";
    bool nextLuigi=nextCape||name=="luigi"||name=="Luigi Forest";
    if(extra_scene!=next||luigi!=nextLuigi||cape!=nextCape){last_minute=-1;jumping=false;}
    extra_scene=next;luigi=nextLuigi;cape=nextCape;return true;
  }
  static int iround(double v){return int(std::lround(v));}
  static double wave(double t,double period=6,double amp=2,double phase=0){return std::sin(t/period*6.283185307179586+phase)*amp;}
  void native(const art::Sprite&s,double x,double y,bool flip=false){
    int xx=iround(x),yy=iround(y);
    for(int row=0;row<s.height;row++)for(int col=0;col<s.width;col++){
      auto i=s.indices[row*s.width+col];if(i)pixel(xx+(flip?s.width-1-col:col),yy+row,s.palette[i]);
    }
  }
  void stroke(int x,int y,int x2,int y2,uint32_t c,int width=1){
    int dx=std::abs(x2-x),sx=x<x2?1:-1,dy=-std::abs(y2-y),sy=y<y2?1:-1,err=dx+dy;
    for(;;){rect(x,y,width,width,c);if(x==x2&&y==y2)break;int e=err*2;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}
  }
  void polygon(std::initializer_list<std::pair<int,int>> points,uint32_t color){
    // Integer ray crossings within the polygon bounds avoid software double
    // division on every screen pixel on the classic ESP32. Preserve edge rules.
    if(points.size()<3)return;
    int x0=63,x1=0,y0=63,y1=0;
    for(auto point:points){x0=std::min(x0,point.first);x1=std::max(x1,point.first);y0=std::min(y0,point.second);y1=std::max(y1,point.second);}
    for(int y=std::max(0,y0);y<=std::min(63,y1);y++)for(int x=std::max(0,x0);x<=std::min(63,x1);x++){
      bool inside=false;auto prev=points.end()-1;
      for(auto cur=points.begin();cur!=points.end();prev=cur++){
        if((cur->second>y)!=(prev->second>y)){
          int dy=prev->second-cur->second;
          int lhs=(x-cur->first)*dy,rhs=(prev->first-cur->first)*(y-cur->second);
          if(dy>0?lhs<rhs:lhs>rhs)inside=!inside;
        }
      }
      if(inside)pixel(x,y,color);
    }
    auto prev=points.end()-1;for(auto cur=points.begin();cur!=points.end();prev=cur++)stroke(prev->first,prev->second,cur->first,cur->second,color);
  }
  void bubble(int x,int y,uint32_t c){pixel(x+1,y,c);pixel(x,y+1,c);pixel(x+2,y+1,c);pixel(x+1,y+2,c);}
  void scene_glyph(char ch,int x,int y,uint32_t c,int scale=1){auto g=glyph(ch);for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(g[yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,c);}
  void scene_time(int hour,int minute,bool valid,int y,uint32_t c,bool stacked=false){
    int ds[]={hour/10,hour%10,minute/10,minute%10};
    for(int i=0;i<4;i++){
      int x=stacked?21+(i%2)*12:6+i*12+(i>=2?6:0),yy=stacked?(i<2?13:32):y;
      char ch=valid?char('0'+ds[i]):'-';scene_glyph(ch,x+1,yy+1,0x080e1c,2);scene_glyph(ch,x,yy,c,2);
    }
    if(!stacked){rect(31,y+5,2,2,0x080e1c);rect(31,y+11,2,2,0x080e1c);rect(30,y+4,2,2,c);rect(30,y+10,2,2,c);}
  }
  void scene_blocks(int hour,int minute,bool valid,int y,double age,bool bonus){
    int ds[]={hour/10,hour%10,minute/10,minute%10};int xs[]={3,17,34,48};uint32_t cs[]={0xf1d268,0x94d27b,0x80bce5,0xe99bac};
    for(int i=0;i<4;i++){
      double a=age-i*.08;int yy=y-(a>0&&a<.6?iround(3*std::sin(a/.6*3.14159265)):0),x=xs[i];
      rect(x+1,yy,11,13,0x111722);rect(x,yy+1,13,11,0x111722);rect(x+1,yy+1,11,11,bonus?cs[i]:0xb3a6ae);
      rect(x+2,yy+1,9,1,0xfff0ca);rect(x+1,yy+2,1,8,0xe4d9b9);rect(x+2,yy+11,9,1,0x665768);scene_glyph(valid?char('0'+ds[i]):'-',x+4,yy+3,0x151823);
    }pixel(31,y+4,0xebe2c3);pixel(31,y+8,0xebe2c3);
  }
  void bricks(uint32_t c,uint32_t seam){for(int y=0;y<64;y+=8)for(int x=y%16?-8:0;x<64;x+=16){rect(x,y,15,7,c);rect(x+1,y+1,13,1,seam);}}
  void selected_scene(uint32_t now,int hour,int minute,bool valid){
    const auto &boo0=selected_sprite(0);
    const auto &boo1=selected_sprite(1);
    const auto &shy0=selected_sprite(2);
    const auto &shy1=selected_sprite(3);
    const auto &bones0=selected_sprite(4);
    const auto &bones1=selected_sprite(5);
    const auto &torch0=selected_sprite(6);
    const auto &torch1=selected_sprite(7);
    const auto &torch2=selected_sprite(8);
    const auto &torch3=selected_sprite(9);
    const auto &coin0=selected_sprite(10);
    const auto &coin1=selected_sprite(11);
    const auto &coin2=selected_sprite(12);
    const auto &star0=selected_sprite(13);
    const auto &flower=selected_sprite(14);
    const auto &fish=selected_sprite(15);
    const double t=animated?double(now%12000)/1000:0;
    const double age=jumping?double(elapsed(now,jump_started))/1000:-10;
    auto r=[&](int x,int y,int w,int h,uint32_t c){rect(x,y,w,h,c);};
    if(extra_scene==1){
      pixels.fill(0x131420);bricks(0x242334,0x191a29);
      for(int x:{5,47}){
        polygon({{x,51},{x,29},{x+2,25},{x+6,23},{x+10,25},{x+12,29},{x+12,51}},0x46405c);
        polygon({{x+2,50},{x+2,29},{x+5,26},{x+8,26},{x+10,29},{x+10,50}},0x11172a);
        r(x+4,30,5,18,0x2c2c4c);r(x+6,28,1,22,0x5b5273);r(x+2,36,9,1,0x5b5273);r(x-1,51,15,2,0x625573);
      }
      r(0,59,64,5,0x605a76);r(0,59,64,1,0xb4b2d0);for(int x=0;x<64;x+=8)r(x,60,1,6,0x33344e);
      // Source Boos face right. Flip only when moving left.
      native(age>=0?shy0:boo0,9+wave(t,12,5),32+wave(t),std::cos(t/12*6.283185307)<0);
      native(age>=0?shy1:boo1,38-wave(t,12,4),40+wave(t,6,2,2),std::cos(t/12*6.283185307)>0);
      r(3,3,58,19,0x10121f);r(4,21,56,1,age>=0&&age<.7?0xc2b9e8:0x77708f);scene_time(hour,minute,valid,5,0xe7e3f7);
    }else if(extra_scene==2){
      pixels.fill(0x082941);r(0,24,64,18,0x08465e);r(0,42,64,22,0x086375);
      for(int x=-12;x<76;x+=12){int xx=x+int(t*2)%12;stroke(xx,2,xx+3,1,0x57b9c4);stroke(xx+3,1,xx+8,1,0x57b9c4);stroke(xx+8,1,xx+11,2,0x57b9c4);}
      int bx[]={7,31,51},bo[]={0,11,21};for(int i=0;i<3;i++)bubble(bx[i],58-int(std::fmod(t*3+bo[i],33)),0x6fb5bf);
      native(fish,20+wave(t,12,17),28+wave(t,4,.5),std::cos(t/12*6.283185307)>0);
      native(fish,25-wave(t,12,18),42+wave(t,4,.5,2),std::cos(t/12*6.283185307)<0);
      int sx[]={3,8,55,61},sh[]={16,10,15,20};for(int i=0;i<4;i++){int x=sx[i],w=iround(wave(t,4,1,x));stroke(x,61,x,54,0x3ca264,2);stroke(x,54,x+w,61-sh[i],0x3ca264,2);stroke(x,54,x-2+w,50,0x78c76c);}
      r(0,60,64,4,0xccb886);r(0,59,64,1,0xf1dfaa);for(int x:{12,31,48})r(x,62,2,1,0x8e785c);
      scene_time(hour,minute,valid,6,0xd0ffeb);if(age>=0&&age<1)for(int x:{15,24,40})bubble(x,27-int(age*9),0x99e1da);
    }else if(extra_scene==3){
      pixels.fill(0x1e1724);bricks(0x302532,0x42303b);
      const art::Sprite*torches[]={&torch0,&torch1,&torch2,&torch3};for(int x:{2,47}){r(x+3,23,12,23,0x402936);r(x+5,26,8,16,0x53313b);r(x+7,34,2,13,0x76505a);native(*torches[int(t*6)%4],x,25);}
      r(0,51,64,5,0x6b5361);r(0,51,64,1,0xb9999a);for(int x=0;x<64;x+=8)r(x,52,1,4,0x3c303f);for(int x:{2,27,53})r(x,56,8,8,0x49343f);
      r(0,59,64,5,0xd73529);for(int x=-8;x<72;x+=8){int xx=x+int(t*4)%8;r(xx,58,5,2,0xffc256);r(xx+3,61,4,1,0xff7e36);}
      double half=std::fmod(t,6),u=std::max(0.,std::min(1.,(half-.3)/5.4)),travel=u*u*(3-2*u)*18;bool right=t<6;
      int pose=half>.3&&half<5.7?int(travel/2)%2:0;native(pose?bones1:bones0,right?12+travel:30-travel,19,right);
      for(int i=0;i<2;i++){int y=57-int(std::fmod(t*4+i*10,19));if(y>36)pixel(i?44:10,y,0xca7245);}
      scene_blocks(hour,minute,valid,5,age,false);
    }else if(extra_scene==4){
      pixels.fill(0x10142c);int xs[]={3,11,6,12,4,12,52,60,53,61,49,31},ys[]={5,11,21,28,48,52,5,12,42,49,52,4};
      for(int i=0;i<12;i++){double glow=std::pow(.5+.5*std::sin(t/4*6.283185307+i*1.7),3);pixel(xs[i],ys[i],mix_color(0x33344f,0xf7eac6,iround(glow*1000),1000));if(glow>.38){auto c=mix_color(0x10142c,0xa79c8f,iround((glow-.38)/.62*1000),1000);pixel(xs[i]-1,ys[i],c);pixel(xs[i]+1,ys[i],c);pixel(xs[i],ys[i]-1,c);pixel(xs[i],ys[i]+1,c);}}
      r(17,10,30,39,0x171c39);r(18,9,28,1,0x544b7a);r(18,49,28,1,0x544b7a);
      scene_time(hour,minute,valid,0,0xffe699,true);
      for(int i=2;i<4;i++){int ds=i==2?minute/10:minute%10;scene_glyph(valid?char('0'+ds):'-',21+(i%2)*12,32,0xfff2cf,2);}
      native(star0,0,32+wave(t));native(star0,48,20-wave(t));
      for(int i=0;i<4;i++){int x=i*16,y=i%2?58:56;r(x,y,16,6,0x7263a0);r(x,y,16,1,0xcab0e1);r(x+2,y+2,12,2,0x9380b8);}
      if(age>=0&&age<1.2){int x=int(age/1.2*76)-6,y=2+int(age*4);stroke(x-6,y-1,x-1,y,0x8178a6);r(x,y-1,1,3,0xfff1bc);r(x-1,y,3,1,0xfff1bc);}
    }else{
      pixels.fill(0x202442);for(int y=0;y<64;y+=8)for(int x=0;x<64;x+=8)if((x+y)%16==0)r(x,y,8,8,0x292d4e);
      scene_blocks(hour,minute,valid,9,age,true);int bump=age>0&&age<.8?iround(5*std::sin(age/.8*3.14159265)):0;
      native(star0,7,34+wave(t,4,1)-bump);native(flower,25,34+wave(t,4,1,2)-bump);const art::Sprite*coins[]={&coin0,&coin1,&coin2,&coin1};native(*coins[int(t*8)%4],44,34+wave(t,4,1,4)-bump);
      for(int x:{6,24,43}){r(x,52,17,2,0x9a759b);r(x+2,54,13,1,0x524362);}r(0,58,64,6,0x6c527d);r(0,58,64,1,0xcaa4bc);for(int x=0;x<64;x+=8)r(x,60,7,4,0x927099);
      for(int i=0;i<4;i++)r(7+i*16,3,2,2,int(t*2)%4==i?0xf8e5ae:0x8f7a82);
      if(age>=0&&age<1){int xs[]={5,22,40,57},os[]={0,3,1,4};for(int i=0;i<4;i++)r(xs[i],26+int(age*14)+os[i],1,2,0xf5d78b);}
    }
  }

  const Frame&render(uint32_t now,int hour,int minute,bool valid=true){
    queue.tick(now);
#ifdef SNES_NATIVE_STREAM
    if(!native_mode||!enabled)pixels.fill(0);
    if(!enabled)native_repaint=true;
#else
    pixels.fill(0);
#endif
    const int current=hour*60+minute;
    const bool can_jump=enabled&&animated&&valid&&
      !(queue.showing&&(queue.active.fullscreen||queue.active.textbox));
    if(valid&&last_minute>=0&&current!=last_minute&&can_jump){
      old_minute=last_minute;jump_started=now;jumping=true;
    }
    last_minute=valid?current:-1;
    if(!can_jump||elapsed(now,jump_started)>=1200)jumping=false;
    const uint32_t jump_age=jumping?elapsed(now,jump_started):1200;
    if(!enabled)return pixels;
#ifdef SNES_NATIVE_STREAM
    if(native_mode){native_player.render(elapsed(now,native_started),pixels,hour,minute,valid,animated,true,native_repaint||queue.showing||native_message);native_repaint=false;native_message=queue.showing;}
    else
#endif
    if(extra_scene)selected_scene(now,hour,minute,valid);
    else {
    pixels.fill(0x94d9d7);
    if(luigi)forest(now);
    else {
    // Layered scenery adapted to 64x64 from the supplied SMW reference.
    int hillShift=animated?int(now/320)%112:0;
    begin_scenery();
    for(int base=-112;base<176;base+=112){
      int center=base+28-hillShift;
      for(int dx=-34;dx<=34;dx++){
        int top=27+std::max(0,std::abs(dx)-5)*3/4;
        for(int y=top;y<55;y++){
          uint32_t color=dx<0?0xd8dbc2:0xa8ad96;
          // Alternating diagonal rock faces replace the rounded prototype hills.
          int facet=(y-top+std::max(0,dx))/10;
          if(facet%2==0)color=dx<0?0xc2c7ad:0xb8bea4;
          if(y==top)color=0xe3e6cc;
          pixel(center+dx,y,color);
        }
      }
      int small=center+57;
      for(int dx=-23;dx<=23;dx++){
        int top=37+std::max(0,std::abs(dx)-4)*3/4;
        if(top<55)rect(small+dx,top,1,55-top,dx<0?0xd8dbc2:0xa8ad96);
      }
    }
    end_scenery(now,320);
    begin_scenery();
    // Rounded floating Boo form from the supplied reference, at panel scale.
    int drift=animated?int(now/600)%96:0;
    for(int cloud=-96;cloud<160;cloud+=96){
      int cx=cloud+44-drift;
      rect(cx+3,20,8,14,0x101810);rect(cx+1,22,12,10,0x101810);
      rect(cx,24,14,6,0x101810);
      rect(cx+3,21,8,11,0xffffff);rect(cx+2,23,10,8,0xffffff);
      rect(cx+1,24,12,5,0xffffff);
      rect(cx+4,31,2,2,0xffffff);rect(cx+8,31,2,2,0xffffff);
      rect(cx+5,24,1,3,0x101810);rect(cx+8,24,1,3,0x101810);
      rect(cx+3,29,8,1,0xe80079);pixel(cx+3,28,0xe80079);pixel(cx+10,28,0xe80079);
    }
    end_scenery(now,600);
    begin_scenery();
    int bushShift=animated?int(now/180)%80:0;
    for(int base=-80;base<144;base+=80){
      int bx=base+12-bushShift;
      for(int dx=-14;dx<=14;dx++){
        int top=47+dx*dx/35+(std::abs(dx)%4==0?1:0);
        if(top<55){rect(bx+dx,top,1,55-top,0x14561e);rect(bx+dx,top+1,1,54-top,dx<0?0x08bb21:0x009c19);}
      }
    }
    end_scenery(now,180);
    }
    begin_scenery();
    rect(0,55,65,1,0x17301d);rect(0,56,65,2,0x08cf28);
    rect(0,58,65,1,0x18371b);rect(0,59,65,5,0xc4a62d);
    int groundShift=animated?int(now/100)%16:0;
    for(int gx=-16;gx<80;gx+=16){
      int x=gx-groundShift;
      rect(x,56,3,1,0x82f54d);rect(x+7,56,3,1,0x82f54d);
      rect(x+3,58,2,1,0x08cf28);pixel(x+3,59,0x18371b);
      rect(x+11,58,2,1,0x08cf28);pixel(x+11,59,0x18371b);
      rect(x+1,61,1,2,0xffe878);pixel(x+5,60,0xf9d647);
      rect(x+9,62,1,2,0xffe878);pixel(x+14,61,0xf9d647);
    }
    end_scenery(now,100);
    // Odd-sized 13x13 blocks center the 5x7 glyphs exactly.
    static const int blockX[]={3,17,34,48};
    const int displayed=jumping&&jump_age<780?old_minute:current;
    int digits[]={displayed/60/10,displayed/60%10,displayed%60/10,displayed%10};
    for(int i=0;i<4;i++){
      int bx=blockX[i];
      // Rounded black border, ochre bevel and white upper-left glint.
      rect(bx+2,3,9,13,0x101810);rect(bx,5,13,9,0x101810);
      rect(bx+1,4,11,11,0x101810);
      rect(bx+2,4,9,11,0xb58a19);rect(bx+1,5,11,9,0xc99f20);
      rect(bx+2,5,9,9,0xf8d82b);
      rect(bx+3,4,7,1,0xffed81);rect(bx+1,6,1,6,0xffed81);
      rect(bx+2,5,3,1,0xffffff);rect(bx+1,6,1,2,0xffffff);
      rect(bx+3,14,7,1,0x947016);rect(bx+11,6,1,7,0x947016);
      auto g=glyph(valid?uint32_t('0'+digits[i]):uint32_t('-'));
      for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(g[row]&(1<<(4-col)))
        pixel(bx+4+col,6+row,0x101810);
    }
    pixel(31,8,0x38231c);pixel(31,12,0x38231c);
    if(jumping){
      const float progress=std::min(1.0f,std::max(0.0f,(float(jump_age)-600)/360));
      const int width=std::max(1,int(std::lround(13*std::abs(std::cos(3.14159265f*progress)))));
      const int bump=int(std::lround(2*std::sin(3.14159265f*progress)));
      for(int bx:blockX){
        uint32_t block[13*13];
        for(int y=0;y<13;y++)for(int x=0;x<13;x++)block[y*13+x]=pixels[(y+3)*64+bx+x];
        rect(bx,1,13,15,0x94d9d7);
        for(int y=0;y<13;y++)for(int x=0;x<width;x++)
          pixel(bx+(13-width)/2+x,3-bump+y,block[y*13+std::min(12,int((x+.5f)*13/width))]);
      }
    }
    {
      const art::Sprite *pose=luigi?&art::luigi_idle:&art::mario_idle;
      if(jumping)pose=luigi?&art::luigi_jump:&art::mario_jump;
      else if(animated){
        const art::Sprite *walk[]={luigi?&art::luigi_idle:&art::mario_idle,luigi?&art::luigi_walk4:&art::mario_walk4,luigi?&art::luigi_walk6:&art::mario_walk6,luigi?&art::luigi_idle:&art::mario_idle};
        pose=walk[(now/200)%4];
      }
      if(luigi&&cape){
        pose=jumping?&art::cape_luigi_jump:animated?&cape_walk_pose(now):&art::cape_luigi_idle;
      }
      const float progress=float(jump_age)/1200;
      const int lift=jumping?int(std::lround(40*progress*(1-progress))):0;
      // Cape extends behind Luigi; anchor his front instead of centering the cape.
      reference_sprite(*pose,luigi&&cape?40-pose->width:(64-pose->width)/2,57-pose->height-lift);
    }
    } // legacy Mario/Luigi scene
    if(!queue.showing)return pixels;
    auto &m=queue.active;auto age=elapsed(now,queue.started);
    if(m.textbox){
      rect(0,0,64,28,0x000000);
      auto wrapped=lines(m.text,8);size_t pages=(wrapped.size()+1)/2;size_t page=(age/4000)%pages;
      for(size_t i=0;i<2 && page*2+i<wrapped.size();i++)message_text(wrapped[page*2+i],4,3+int(i)*11);
      // Tiny page dots leave the two large text lines and Mario unobstructed.
      if(pages>1)for(size_t i=0;i<std::min(pages,size_t(20));i++)pixel(3+int(i)*3,26,i==page?0xffffff:0x555555);
    }else if(!m.fullscreen){
      rect(0,47,64,17,0x000000);
      std::vector<uint32_t> v;decode(m.text,v);for(auto&c:v)if(c=='\n')c=' ';
      int width=int(v.size())*7-1;int offset=width<=60?(64-width)/2:64-int(age/60 % uint32_t(width+80));
      message_text(v,offset,51);
    }else{
      pixels.fill(0x000000);
      auto wrapped=lines(m.text,8);size_t pages=(wrapped.size()+4)/5;size_t page=(age/4000)%pages;
      for(size_t i=0;i<5 && page*5+i<wrapped.size();i++){auto &line=wrapped[page*5+i];message_text(line,4,4+int(i)*10);}
      if(pages>1){std::string status=std::to_string(page+1)+"/"+std::to_string(pages);label(status,23,57,0xffffff);}
    }
    return pixels;
  }
};
} // namespace snes
