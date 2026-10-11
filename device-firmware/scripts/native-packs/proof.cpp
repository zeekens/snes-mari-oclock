#include "../../firmware/include/native_scene.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
int main(int argc,char**argv){
 if(argc!=6)return 2;std::ifstream f(argv[1],std::ios::binary);std::vector<uint8_t>b((std::istreambuf_iterator<char>(f)),{});
 snes::native_scene::Player p;std::array<uint32_t,4096>pixels{};
 if(!p.open(b.data(),b.size())||!p.render_frame(std::stoul(argv[2]),pixels,std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5])!=0))return 3;
 uint32_t hash=2166136261u;for(auto pixel:pixels)hash=(hash^pixel)*16777619u;
 std::cout<<std::hex<<std::setw(8)<<std::setfill('0')<<hash<<'\n';
}
