#include "fixtures/native_scene_v1.h"
#define SNES_NATIVE_STREAM
#include "../firmware/include/clock_core.h"
#include "../firmware/include/native_catalog.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>
struct Reader {std::vector<uint8_t> bytes;size_t reads=0;static uint8_t read(void*p,size_t at){auto&r=*static_cast<Reader*>(p);assert(at<r.bytes.size());++r.reads;return r.bytes[at];}};
int main(int argc,char**argv){
 assert(argc==2);std::ifstream file(argv[1],std::ios::binary);Reader reader;reader.bytes.assign(std::istreambuf_iterator<char>(file),{});
 golden_native_scene::Player direct;snes::native_scene::Player stream;assert(direct.open(reader.bytes.data(),reader.bytes.size()));assert(stream.open_reader(&reader,reader.bytes.size(),Reader::read));
 std::array<uint32_t,4096>a{},b{};for(unsigned i=0;i<direct.frames();++i){assert(direct.render_frame(i,a,21,57));assert(stream.render_frame(i,b,21,57,true,true,true));assert(a==b);}
 for(unsigned i:{0u,50u,299u,1u}){i%=direct.frames();assert(direct.render_frame(i,a,0,0));assert(stream.render_frame(i,b,0,0,true,true,true));assert(a==b);}
 snes::Clock clock;clock.activate_native(stream,0);clock.render(80,21,57);auto clear=clock.pixels;
 assert(clock.queue.submit("HELLO","textbox",3,1,"test",80));clock.render(80,21,57);assert(clear!=clock.pixels);clock.queue.clear_all();clock.render(80,21,57);assert(clear==clock.pixels);
 clock.enabled=false;clock.render(100,21,57);for(auto p:clock.pixels)assert(p==0);clock.enabled=true;assert(clock.set_scene("Mario"));assert(!clock.native_mode);
 std::ifstream cf("libraries/sntl-v1/catalog.json");std::string json((std::istreambuf_iterator<char>(cf)),{}),error;auto cat=std::make_unique<snes::native_catalog::Catalog>();assert(snes::native_catalog::parse(json,std::string(40,'a'),*cat,error));assert(cat->count==41&&cat->libraries==3);assert(!snes::native_catalog::parse(json,"main",*cat,error));assert(!snes::native_catalog::parse(std::string(100,'['),std::string(40,'a'),*cat,error));
 for(size_t n:{0u,39u,109u})assert(!stream.open_reader(&reader,n,Reader::read));
 std::cout<<"SNTL streamed/direct frames, seeks, messages, display off, Classic return, catalog bounds passed: "<<direct.frames()<<" frames\n";
}
