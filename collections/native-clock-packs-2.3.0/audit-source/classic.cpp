#include "classic_instrumented.h"
#include <fstream>
#include <iostream>
#include <cassert>
int main(int argc,char**argv){
 assert(argc==2);
 for(auto scene:{"mario","luigi","cape-luigi","ghost-house","tide-pool","lava-fortress","star-road","bonus-room"}){
  snes::Clock c;assert(c.set_scene(scene));unsigned overlaps=0;std::ofstream out(std::string(argv[1])+"/"+scene+"-current.rgb",std::ios::binary);
  for(unsigned k=0;k<1560;k++){
   unsigned ms=k*40;c.audit_masks.clear();auto&f=c.render(ms,12,k<1500?34:35);
   if(k<300||k>=1500)for(auto v:f){char rgb[]={char(v>>16),char(v>>8),char(v)};out.write(rgb,3);}
   for(unsigned a=0;a<c.audit_masks.size();a++)for(unsigned b=a+1;b<c.audit_masks.size();b++){
    bool hit=false;for(unsigned p=0;p<4096;p++)hit|=c.audit_masks[a][p]&&c.audit_masks[b][p];if(hit)overlaps++;
   }
  }
  if(std::string(scene)!="lava-fortress")assert(overlaps==0);
  c.animated=false;auto frozen=c.render(15000,12,35);assert(c.render(18000,12,35)==frozen);
  std::cout<<scene<<" 1560 frames; sprite-pair overlap frames="<<overlaps<<"\n";
 }
}
