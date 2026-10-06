#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
// Flash-resident indexed tiles and placement commands. Independent of SNPK v1.
namespace snes::native_scene {
struct Asset { const char *id; const char *name; const uint8_t *data; size_t size; };
class Player {
 public:
  bool open(const uint8_t *data,size_t size) {
    data_=nullptr;error_=true;
    if(!data||size<110||size>32*1024*1024||std::memcmp(data,"SNTL",4)||data[4]!=1||(data[5]!=4&&data[5]!=8))return false;
    data_=data;size_=size;tile_=data[5];colours_=u16(6);frames_=u16(8);tiles_=u16(10);period_=u16(12);key_=u16(14);
    tileoff_=u32(16);dict_=u32(20);frameoff_=u32(24);stream_=u32(28);font_=u32(32);slots_=4096/(tile_*tile_);
    if(!colours_||colours_>256||!frames_||frames_>2000||!tiles_||period_!=40||key_!=50||u32(36)!=size||
       tileoff_!=40+colours_*3||uint64_t(tileoff_)+4*(uint64_t(tiles_)+1)!=dict_||dict_>frameoff_||
       uint64_t(frameoff_)+4*uint64_t(frames_)!=stream_||stream_>font_||uint64_t(font_)+70!=size)return fail();
    if(u32(tileoff_)||u32(tileoff_+4*tiles_)!=frameoff_-dict_)return fail();
    for(unsigned n=0;n<tiles_;n++)if(!decode_tile(n))return fail();
    for(unsigned n=0;n<70;n++)if(data_[font_+n]>31)return fail();
    for(unsigned n=0;n<frames_;n++)if(!apply(n))return fail();
    current_=-1;error_=false;return true;
  }
  void clear(){data_=nullptr;error_=false;current_=-1;}
  bool active()const{return data_&&!error_;}
  unsigned frames()const{return frames_;}
  bool render_frame(unsigned target,std::array<uint32_t,4096> &pixels,int hour,int minute,bool valid=true,bool clock=true){
    if(!active()||target>=frames_)return false;
    unsigned key=target/key_*key_;
    if(current_<0||target<unsigned(current_)||unsigned(current_)<key)current_=int(key)-1;
    while(current_<int(target)){if(!apply(unsigned(++current_)))return fail();}
    for(unsigned slot=0;slot<slots_;slot++){
      if(!decode_tile(map_[slot]))return fail();
      const unsigned x=(slot%(64/tile_))*tile_,y=(slot/(64/tile_))*tile_;
      for(unsigned dy=0;dy<tile_;dy++)for(unsigned dx=0;dx<tile_;dx++)pixels[(y+dy)*64+x+dx]=colour(40+3*tile_pixels_[dy*tile_+dx]);
    }
    if(clock)draw_clock(pixels,hour,minute,valid);return true;
  }
  bool render(uint32_t elapsed,std::array<uint32_t,4096>&pixels,int h,int m,bool valid,bool animated){if(!active())return false;return render_frame(animated?(elapsed/period_)%frames_:0,pixels,h,m,valid);}
 private:
  const uint8_t *data_=nullptr;size_t size_=0;uint32_t tileoff_=0,dict_=0,frameoff_=0,stream_=0,font_=0;
  unsigned tile_=0,colours_=0,frames_=0,tiles_=0,period_=40,key_=50,slots_=0;int current_=-1;bool error_=false;
  std::array<uint16_t,256> map_{};std::array<uint8_t,64> tile_pixels_{};std::array<uint8_t,8> style_{};
  uint16_t u16(size_t p)const{return uint16_t(data_[p])|uint16_t(data_[p+1])<<8;}
  uint32_t u32(size_t p)const{return uint32_t(u16(p))|uint32_t(u16(p+2))<<16;}
  uint32_t colour(size_t p)const{return uint32_t(data_[p])<<16|uint32_t(data_[p+1])<<8|data_[p+2];}
  bool fail(){error_=true;data_=nullptr;return false;}
  bool number(size_t &p,uint32_t &v)const{
    v=0;for(unsigned shift=0;shift<21;shift+=7){if(p>=font_)return false;unsigned b=data_[p++];v|=(b&127)<<shift;if(!(b&128))return true;}return false;
  }
  bool decode_tile(unsigned n){
    if(n>=tiles_)return false;uint32_t a=u32(tileoff_+n*4),b=u32(tileoff_+(n+1)*4);
    if(a>=b||b>frameoff_-dict_)return false;size_t p=dict_+a,end=dict_+b;unsigned used=0,total=tile_*tile_,mode=data_[p++];
    if(mode==0){if(end-p!=total)return false;for(unsigned j=0;j<total;j++){unsigned c=data_[p++];if(c>=colours_)return false;tile_pixels_[j]=uint8_t(c);}return true;}
    if(mode!=1)return false;
    while(p<end){unsigned code=data_[p++],count=(code&127)+1;if(count>total-used)return false;
      if(code&128){if(p>=end||data_[p]>=colours_)return false;unsigned c=data_[p++];for(unsigned j=0;j<count;j++)tile_pixels_[used++]=uint8_t(c);}
      else{if(count>end-p)return false;for(unsigned j=0;j<count;j++){unsigned c=data_[p++];if(c>=colours_)return false;tile_pixels_[used++]=uint8_t(c);}}
    }return used==total;
  }
  bool apply(unsigned n){
    uint32_t off=u32(frameoff_+4*n);if(off>font_-stream_||font_-stream_-off<9)return false;size_t p=stream_+off;
    for(unsigned j=0;j<8;j++)style_[j]=data_[p++];if(style_[0]>40||style_[1]>2)return false;
    unsigned cursor=0,written=0;uint32_t skip,count,index;
    while(true){if(!number(p,skip))return false;if(!skip)break;
      if(skip-1>slots_-cursor)return false;cursor+=skip-1;if(!number(p,count)||!count||count>slots_-cursor)return false;
      for(unsigned j=0;j<count;j++){if(!number(p,index)||index>=tiles_)return false;map_[cursor++]=uint16_t(index);written++;}
    }
    return n%key_!=0||written==slots_;
  }
  static void pixel(std::array<uint32_t,4096>&out,int x,int y,uint32_t c){if(x>=0&&x<64&&y>=0&&y<64)out[size_t(y)*64+x]=c;}
  void draw_clock(std::array<uint32_t,4096>&out,int hour,int minute,bool valid){
    valid=valid&&hour>=0&&hour<24&&minute>=0&&minute<60;
    int digits[]={hour/10,hour%10,-1,minute/10,minute%10};
    uint32_t ink=uint32_t(style_[2])<<16|uint32_t(style_[3])<<8|style_[4],shadow=uint32_t(style_[5])<<16|uint32_t(style_[6])<<8|style_[7];
    for(int pass=0;pass<2;pass++){
      int x=6,y=style_[0];
      auto dot=[&](int px,int py){
        if(pass){pixel(out,px,py,ink);return;}
        if(style_[1]==1){for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)pixel(out,px+dx,py+dy,shadow);}
        else{pixel(out,px-1,py,shadow);pixel(out,px+1,py,shadow);pixel(out,px,py-1,shadow);pixel(out,px,py+1,shadow);if(style_[1]==2)pixel(out,px+1,py+1,shadow);}
      };
      for(unsigned d=0;d<5;d++){
        if(d==2){int top=style_[1]==2?4:3;for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){dot(x+dx,y+top+dy);dot(x+dx,y+top+6+dy);}x+=6;continue;}
        for(int row=0;row<7;row++){unsigned bits=valid?data_[font_+digits[d]*7+row]:(row==3?31:0);
          for(int col=0;col<5;col++)if(bits&(1<<(4-col)))for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++)dot(x+col*2+dx,y+row*2+dy);
        }x+=12;
      }
    }
  }
};
static_assert(sizeof(Player)<=1024,"Native scene state must stay below 1 KiB");
} // namespace snes::native_scene
