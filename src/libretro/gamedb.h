#ifndef GAMEDB_H
#define GAMEDB_H

#include <cstdint>
#include <string>

namespace GameDB
{
  std::string getGameID(const std::string& romPath);
  uint32_t getSaveSize(const std::string& gameId);
}

#endif // GAMEDB_H
