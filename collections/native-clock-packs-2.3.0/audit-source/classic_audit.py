from pathlib import Path
import subprocess
R=Path(__file__).resolve().parents[2];O=R/'build/scene-audit-2026-11-07';F=R/'build/github-pack-publication/device-firmware/firmware/include'
s=(F/'clock_core.h').read_text()
s=s.replace('  void native(const art::Sprite&s,double x,double y,bool flip=false){','  std::vector<std::array<int,4096>> audit_masks;\n  void record_sprite(const art::Sprite&s,int x,int y,bool flip){std::array<int,4096>mask{};for(int yy=0;yy<s.height;yy++)for(int xx=0;xx<s.width;xx++)if(s.indices[yy*s.width+xx]){int px=x+(flip?s.width-1-xx:xx),py=y+yy;if(px>=0&&px<64&&py>=0&&py<64)mask[py*64+px]=1;}audit_masks.push_back(mask);}\n  void native(const art::Sprite&s,double x,double y,bool flip=false){record_sprite(s,iround(x),iround(y),flip);')
s=s.replace('  void reference_sprite(const art::Sprite &s,int x,int y){','  void reference_sprite(const art::Sprite &s,int x,int y){record_sprite(s,x,y,false);')
(O/'classic_instrumented.h').write_text(s)
subprocess.run(['clang++','-std=c++17','-O1','-fsanitize=address,undefined','-I'+str(F),'-I'+str(O),str(R/'tools/scene-audit/classic.cpp'),'-o',str(O/'classic-audit')],check=True)
result=subprocess.run([str(O/'classic-audit'),str(O)],check=True,capture_output=True,text=True)
(O/'classic-validation.log').write_text(result.stdout)
print(result.stdout)
