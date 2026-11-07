#include "native_catalog.h"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  if (argc != 2) return 2;
  const std::string root = argv[1];
  std::ifstream input(root + "/catalog.json");
  const std::string body((std::istreambuf_iterator<char>(input)), {});
  auto catalog = std::make_unique<snes::native_catalog::Catalog>();
  std::string error;
  if (!snes::native_catalog::parse(body, std::string(40, 'a'), *catalog, error)) {
    std::cerr << error << '\n';
    return 1;
  }
  for (size_t i = 0; i < catalog->count; ++i) {
    const auto &entry = catalog->entries[i];
    std::ifstream asset(root + "/" + entry.path, std::ios::binary);
    const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(asset)), {});
    if (bytes.size() != entry.bytes ||
        snes::packs::sha256(bytes.data(), bytes.size()) != entry.hash) {
      std::cerr << "Asset mismatch: " << entry.id << '\n';
      return 1;
    }
  }
  std::cout << "Firmware catalog parser and asset hashes passed: "
            << catalog->count << " scenes in " << catalog->libraries << " packs\n";
}
