#ifndef GAMEDB_H
#define GAMEDB_H

#include <cstdint>
#include <string>

struct GameInfo {
  std::string id;
  std::string region;
  std::string regionCode;

  bool ntsc = true;

  uint32_t revision = 0;
  uint32_t saveType = 0;
  uint32_t saveSize = 0;
};

namespace GameDB
{
  GameInfo analyze(const uint8_t* data);
}

#endif // GAMEDB_H
