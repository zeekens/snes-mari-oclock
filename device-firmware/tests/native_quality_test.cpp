#include "fixtures/native_scene_v1.h"
#include "../firmware/include/native_scene.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>
using snes::native_scene::Player;
std::vector<uint8_t> load(const char*p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
unsigned word(const std::vector<uint8_t>&b,unsigned p){return b[p]|unsigned(b[p+1])<<8;}
int main(int argc,char**argv){
 assert(argc>=3);auto old=load(argv[1]),now=load(argv[2]);golden_native_scene::Player gold;Player full,delta;
 assert(gold.open(old.data(),old.size()));assert(full.open(now.data(),now.size()));assert(delta.open(now.data(),now.size()));
 std::array<uint32_t,4096>a{},b{},c{};
 std::ofstream raw,clocked;if(argc==5){raw.open(argv[3],std::ios::binary);clocked.open(argv[4],std::ios::binary);}
 auto write=[](std::ofstream&f,const auto&px){for(auto v:px){char rgb[]={char(v>>16),char(v>>8),char(v)};f.write(rgb,3);}};
 for(unsigned i=0;i<full.frames();++i){
  assert(gold.render_frame(i,a,12,34,true,false));assert(full.render_frame(i,b,12,34,true,false));assert(a==b);
  if(raw.is_open())write(raw,b);
  assert(full.render_frame(i,b,12,34));if(clocked.is_open())write(clocked,b);
 }
 // Cross keyframes, loops, arbitrary seeks, time validity and removal/reapplication of clock overlays.
 for(unsigned i=0;i<2000;i++){
  unsigned k=i<600?i%full.frames():(i*73)%full.frames();bool valid=i%91!=0,clock=i%37!=0;int h=i/77%24,m=i/7%60;
  assert(full.render_frame(k,b,h,m,valid,clock));assert(delta.render_frame(k,c,h,m,valid,clock,true));assert(b==c);
  if(i%113==0){c.fill(0);assert(delta.render_frame(k,c,h,m,valid,clock,true,true));assert(b==c);}
 }
 unsigned t=now.size()-16,mode=word(now,t+4),idle=word(now,t+6),count=word(now,t+8),action=word(now,t+10),actions=word(now,t+12),period=word(now,12);
 Player p;assert(p.open(now.data(),now.size()));
 auto is_idle=[&](unsigned f){return f>=idle&&f<idle+count;};
 if(mode){
  assert(is_idle(p.frame_at(0,12,34,true,true)));assert(is_idle(p.frame_at(12000,12,34,true,true)));
  assert(p.frame_at(20000,12,35,true,true)==action);
  assert(p.frame_at(20000+(actions-1)*period,12,35,true,true)==action+actions-1);
  assert(is_idle(p.frame_at(20000+actions*period,12,35,true,true)));
  assert(is_idle(p.frame_at(40000,15,55,true,true))); // clock correction
  assert(p.frame_at(41000,15,56,true,true)==action);
  assert(p.frame_at(42000,15,56,true,false)==idle); // disabled cancels action
  assert(is_idle(p.frame_at(43000,15,56,true,true)));
  p.frame_at(44000,15,57,false,true);assert(is_idle(p.frame_at(45000,15,57,true,true)));
  p.frame_at(0xfffffff0u,23,59,true,true);assert(p.frame_at(0xfffffff8u,0,0,true,true)==action);
  assert(p.frame_at(0xfffffff8u+period,0,0,true,true)==action+1);
 }else{assert(p.frame_at(period*10,12,34,true,true)==10);assert(p.frame_at(12000,12,35,true,false)==0);}
 // Reject malformed trailer, out-of-range clips, zero periods and unsupported modes before reading pixels.
 for(unsigned offset:{t,t+4,t+8,t+12,t+14}){auto bad=now;bad[offset]=255;bad[offset+1]=255;Player invalid;assert(!invalid.open(bad.data(),bad.size()));}
 std::cout<<argv[2]<<": source pixels, incremental redraws, clock overlays, timing, bounds OK\n";
}
