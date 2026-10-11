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
  using ReadByte=uint8_t(*)(void*,size_t);
  bool open(const uint8_t *data,size_t size) {
    return open_reader(const_cast<uint8_t*>(data),size,[](void *ctx,size_t p){return static_cast<uint8_t*>(ctx)[p];});
  }
  bool open_reader(void *ctx,size_t size,ReadByte read) {
    context_=ctx;read_=read;
    present_=false;error_=true;current_=-1;event_minute_=-1;action_running_=false;mode_=0;
    if(!ctx||!read||size<110||size>32*1024*1024||(at(0)!='S'||at(1)!='N'||at(2)!='T'||at(3)!='L')||(at(4)!=1&&at(4)!=2)||(at(5)!=4&&at(5)!=8))return false;
    present_=true;size_=size;tile_=at(5);colours_=u16(6);frames_=u16(8);tiles_=u16(10);period_=u16(12);key_=u16(14);
    tileoff_=u32(16);dict_=u32(20);frameoff_=u32(24);stream_=u32(28);font_=u32(32);slots_=4096/(tile_*tile_);
    if(!colours_||colours_>256||!frames_||frames_>2000||!tiles_||(period_<40||period_>1000||period_%20)||key_!=50||u32(36)!=size||
       tileoff_!=40+colours_*3||uint64_t(tileoff_)+4*(uint64_t(tiles_)+1)!=dict_||dict_>frameoff_||
       uint64_t(frameoff_)+4*uint64_t(frames_)!=stream_||stream_>font_||uint64_t(font_)+70+(at(4)==2?16:0)!=size)return fail();
    idle_start_=0;idle_count_=1;action_start_=0;action_count_=frames_;idle_period_=period_;
    if(at(4)==2){
      size_t t=font_+70;
      if(at(t)!='T'||at(t+1)!='I'||at(t+2)!='M'||at(t+3)!='E')return fail();
      mode_=u16(t+4);idle_start_=u16(t+6);idle_count_=u16(t+8);
      action_start_=u16(t+10);action_count_=u16(t+12);idle_period_=u16(t+14);
      if(mode_>1||!idle_count_||idle_start_+idle_count_>frames_||!action_count_||
         action_start_+action_count_>frames_||idle_period_<40||idle_period_>1000||idle_period_%20||
         uint32_t(action_count_)*period_>30000)return fail();
    }
    if(u32(tileoff_)||u32(tileoff_+4*tiles_)!=frameoff_-dict_)return fail();
    for(unsigned n=0;n<tiles_;n++)if(!decode_tile(n))return fail();
    for(unsigned n=0;n<70;n++)if(at(font_+n)>31)return fail();
    for(unsigned n=0;n<frames_;n++)if(!apply(n))return fail();
    current_=-1;error_=false;return true;
  }
  void clear(){present_=false;error_=false;current_=-1;}
  bool active()const{return present_&&!error_;}
  unsigned frames()const{return frames_;}
  // Describe the exact composited frame for an independent host-side pixel audit.
  std::array<int,4> frame_info()const{return {current_,last_hour_,last_minute_,last_valid_?1:0};}
  bool render_frame(unsigned target,std::array<uint32_t,4096> &pixels,int hour,int minute,bool valid=true,bool clock=true,bool incremental=false,bool force=false){
    if(!active()||target>=frames_)return false;
    const auto old_style=style_;
    dirty_.fill(!incremental||force||current_<0);
    bool clock_changed=hour!=last_hour_||minute!=last_minute_||valid!=last_valid_||clock!=last_clock_;
    unsigned key=target/key_*key_;
    if(current_<0||target<unsigned(current_)||unsigned(current_)<key)current_=int(key)-1;
    while(current_<int(target)){if(!apply(unsigned(++current_)))return fail();}
    if(clock_changed||old_style!=style_)for(unsigned slot=0;slot<slots_;slot++){
      unsigned y=(slot/(64/tile_))*tile_;
      if((int(y)+int(tile_)>int(old_style[0])-1&&y<old_style[0]+16u)||(int(y)+int(tile_)>int(style_[0])-1&&y<style_[0]+16u))dirty_[slot]=true;
    }
    for(unsigned slot=0;slot<slots_;slot++){
      if(!dirty_[slot])continue;
      if(!decode_tile(map_[slot]))return fail();
      const unsigned x=(slot%(64/tile_))*tile_,y=(slot/(64/tile_))*tile_;
      for(unsigned dy=0;dy<tile_;dy++)for(unsigned dx=0;dx<tile_;dx++)pixels[(y+dy)*64+x+dx]=colour(40+3*tile_pixels_[dy*tile_+dx]);
    }
    if(clock)draw_clock(pixels,hour,minute,valid);last_hour_=hour;last_minute_=minute;last_valid_=valid;last_clock_=clock;return true;
  }
  // Time corrections, startup and invalid time never fire an action. Only the next minute does.
  unsigned frame_at(uint32_t elapsed,int h,int m,bool valid,bool animated){
    valid=valid&&h>=0&&h<24&&m>=0&&m<60;
    int minute=valid?h*60+m:-1;
    if(!animated||!valid)action_running_=false;
    else if(mode_==1&&event_minute_>=0&&minute!=event_minute_){
      if(minute==(event_minute_+1)%1440){action_started_=elapsed;action_running_=true;}
      else action_running_=false;
    }
    event_minute_=minute;
    if(!animated)return mode_==1?idle_start_:0;
    if(mode_==0)return (elapsed/period_)%frames_;
    if(action_running_){
      uint32_t frame=(elapsed-action_started_)/period_;
      if(frame<action_count_)return action_start_+frame;
      action_running_=false;
    }
    // Ping-pong ambient idle frames to avoid a visible wrap in short source excerpts.
    unsigned cycle=idle_count_>1?2*idle_count_-2:1;
    unsigned f=(elapsed/idle_period_)%cycle;
    return idle_start_+(f<idle_count_?f:cycle-f);
  }
  bool render(uint32_t elapsed,std::array<uint32_t,4096>&pixels,int h,int m,bool valid,bool animated,bool incremental=false,bool force=false){
    if(!active())return false;
    return render_frame(frame_at(elapsed,h,m,valid,animated),pixels,h,m,valid,true,incremental,force);
  }
 private:
  unsigned mode_=0,idle_start_=0,idle_count_=1,action_start_=0,action_count_=0,idle_period_=120;
  int event_minute_=-1;uint32_t action_started_=0;bool action_running_=false;
  bool present_=false;void *context_=nullptr;ReadByte read_=nullptr;size_t size_=0;uint32_t tileoff_=0,dict_=0,frameoff_=0,stream_=0,font_=0;
  unsigned tile_=0,colours_=0,frames_=0,tiles_=0,period_=40,key_=50,slots_=0;int current_=-1;bool error_=false;
  std::array<bool,256>dirty_{};int last_hour_=-1,last_minute_=-1;bool last_valid_=false,last_clock_=false;
  std::array<uint16_t,256> map_{};std::array<uint8_t,64> tile_pixels_{};std::array<uint8_t,8> style_{};
  uint8_t at(size_t p)const{return read_(context_,p);}
  uint16_t u16(size_t p)const{return uint16_t(at(p))|uint16_t(at(p+1))<<8;}
  uint32_t u32(size_t p)const{return uint32_t(u16(p))|uint32_t(u16(p+2))<<16;}
  uint32_t colour(size_t p)const{return uint32_t(at(p))<<16|uint32_t(at(p+1))<<8|at(p+2);}
  bool fail(){error_=true;present_=false;return false;}
  bool number(size_t &p,uint32_t &v)const{
    v=0;for(unsigned shift=0;shift<21;shift+=7){if(p>=font_)return false;unsigned b=at(p++);v|=(b&127)<<shift;if(!(b&128))return true;}return false;
  }
  bool decode_tile(unsigned n){
    if(n>=tiles_)return false;uint32_t a=u32(tileoff_+n*4),b=u32(tileoff_+(n+1)*4);
    if(a>=b||b>frameoff_-dict_)return false;size_t p=dict_+a,end=dict_+b;unsigned used=0,total=tile_*tile_,mode=at(p++);
    if(mode==0){if(end-p!=total)return false;for(unsigned j=0;j<total;j++){unsigned c=at(p++);if(c>=colours_)return false;tile_pixels_[j]=uint8_t(c);}return true;}
    if(mode!=1)return false;
    while(p<end){unsigned code=at(p++),count=(code&127)+1;if(count>total-used)return false;
      if(code&128){if(p>=end||at(p)>=colours_)return false;unsigned c=at(p++);for(unsigned j=0;j<count;j++)tile_pixels_[used++]=uint8_t(c);}
      else{if(count>end-p)return false;for(unsigned j=0;j<count;j++){unsigned c=at(p++);if(c>=colours_)return false;tile_pixels_[used++]=uint8_t(c);}}
    }return used==total;
  }
  bool apply(unsigned n){
    uint32_t off=u32(frameoff_+4*n);if(off>font_-stream_||font_-stream_-off<9)return false;size_t p=stream_+off;
    for(unsigned j=0;j<8;j++)style_[j]=at(p++);if(style_[0]>40||style_[1]>2)return false;
    unsigned cursor=0,written=0;uint32_t skip,count,index;
    while(true){if(!number(p,skip))return false;if(!skip)break;
      if(skip-1>slots_-cursor)return false;cursor+=skip-1;if(!number(p,count)||!count||count>slots_-cursor)return false;
      for(unsigned j=0;j<count;j++){if(!number(p,index)||index>=tiles_)return false;if(map_[cursor]!=index)dirty_[cursor]=true;map_[cursor++]=uint16_t(index);written++;}
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
        if(d==2){int top=3;for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){dot(x+1+dx,y+top+dy);dot(x+1+dx,y+top+6+dy);}x+=6;continue;}
        for(int row=0;row<7;row++){unsigned bits=valid?at(font_+digits[d]*7+row):(row==3?31:0);
          for(int col=0;col<5;col++)if(bits&(1<<(4-col)))for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++)dot(x+col*2+dx,y+row*2+dy);
        }x+=12;
      }
    }
  }
};
static_assert(sizeof(Player)<=1024,"Native scene state must stay below 1 KiB");
} // namespace snes::native_scene
