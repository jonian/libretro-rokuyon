#include "gamedb.h"

// Game codes and matching save types are ported from ares emulator
// https://github.com/ares-emulator/ares/blob/master/mia/medium/nintendo-64.cpp

struct DBEntry {
  std::string id;
  int saveType;
};

static const DBEntry gamedb[] = {
  // EEPROM4K
  { "NTW", 1 }, // 64 de Hakken!! Tamagotchi
  { "NHF", 1 }, // 64 Hanafuda: Tenshi no Yakusoku
  { "NOS", 1 }, // 64 Oozumou
  { "NTC", 1 }, // 64 Trump Collection
  { "NER", 1 }, // Aero Fighters Assault [Sonic Wings Assault (J)]
  { "NAG", 1 }, // AeroGauge
  { "NAB", 1 }, // Air Boarder 64
  { "NS3", 1 }, // AI Shougi 3
  { "NTN", 1 }, // All Star Tennis '99
  { "NBN", 1 }, // Bakuretsu Muteki Bangaioh
  { "NBK", 1 }, // Banjo-Kazooie [Banjo to Kazooie no Daiboken (J)]
  { "NFH", 1 }, // In-Fisherman Bass Hunter 64
  { "NMU", 1 }, // Big Mountain 2000
  { "NBC", 1 }, // Blast Corps
  { "NBH", 1 }, // Body Harvest
  { "NHA", 1 }, // Bomberman 64: Arcade Edition (J)
  { "NBM", 1 }, // Bomberman 64 [Baku Bomberman (J)]
  { "NBV", 1 }, // Bomberman 64: The Second Attack! [Baku Bomberman 2 (J)]
  { "NBD", 1 }, // Bomberman Hero [Mirian Ojo o Sukue! (J)]
  { "NCT", 1 }, // Chameleon Twist
  { "NCH", 1 }, // Chopper Attack
  { "NCG", 1 }, // Choro Q 64 II - Hacha Mecha Grand Prix Race (J)
  { "NP2", 1 }, // Chou Kuukan Night Pro Yakyuu King 2 (J)
  { "NXO", 1 }, // Cruis'n Exotica
  { "NCU", 1 }, // Cruis'n USA
  { "NCX", 1 }, // Custom Robo
  { "NDY", 1 }, // Diddy Kong Racing
  { "NDQ", 1 }, // Disney's Donald Duck - Goin' Quackers [Quack Attack (E)]
  { "NDR", 1 }, // Doraemon: Nobita to 3tsu no Seireiseki
  { "NN6", 1 }, // Dr. Mario 64
  { "NDU", 1 }, // Duck Dodgers starring Daffy Duck
  { "NJM", 1 }, // Earthworm Jim 3D
  { "NFW", 1 }, // F-1 World Grand Prix
  { "NF2", 1 }, // F-1 World Grand Prix II
  { "NKA", 1 }, // Fighters Destiny [Fighting Cup (J)]
  { "NFG", 1 }, // Fighter Destiny 2
  { "NGL", 1 }, // Getter Love!!
  { "NGV", 1 }, // Glover
  { "NGE", 1 }, // GoldenEye 007
  { "NHP", 1 }, // Heiwa Pachinko World 64
  { "NPG", 1 }, // Hey You, Pikachu! [Pikachu Genki Dechu (J)]
  { "NIJ", 1 }, // Indiana Jones and the Infernal Machine
  { "NIC", 1 }, // Indy Racing 2000
  { "NFY", 1 }, // Kakutou Denshou: F-Cup Maniax
  { "NKI", 1 }, // Killer Instinct Gold
  { "NLL", 1 }, // Last Legion UX
  { "NLR", 1 }, // Lode Runner 3-D
  { "NKT", 1 }, // Mario Kart 64
  { "CLB", 1 }, // Mario Party (NTSC)
  { "NLB", 1 }, // Mario Party (PAL)
  { "NMW", 1 }, // Mario Party 2
  { "NML", 1 }, // Mickey's Speedway USA [Mickey no Racing Challenge USA (J)]
  { "NTM", 1 }, // Mischief Makers [Yuke Yuke!! Trouble Makers (J)]
  { "NMI", 1 }, // Mission: Impossible
  { "NMG", 1 }, // Monaco Grand Prix [Racing Simulation 2 (G)]
  { "NMO", 1 }, // Monopoly
  { "NMS", 1 }, // Morita Shougi 64
  { "NMR", 1 }, // Multi-Racing Championship
  { "NCR", 1 }, // Penny Racers [Choro Q 64 (J)]
  { "NEA", 1 }, // PGA European Tour
  { "NPW", 1 }, // Pilotwings 64
  { "NPY", 1 }, // Puyo Puyo Sun 64
  { "NPT", 1 }, // Puyo Puyon Party
  { "NRA", 1 }, // Rally '99 (J)
  { "NWQ", 1 }, // Rally Challenge 2000
  { "NSU", 1 }, // Rocket: Robot on Wheels
  { "NSN", 1 }, // Snow Speeder (J)
  { "NK2", 1 }, // Snowboard Kids 2 [Chou Snobow Kids (J)]
  { "NSV", 1 }, // Space Station Silicon Valley
  { "NFX", 1 }, // Star Fox 64 [Lylat Wars (E)]
  { "NS6", 1 }, // Star Soldier: Vanishing Earth
  { "NNA", 1 }, // Star Wars Episode I: Battle for Naboo
  { "NRS", 1 }, // Star Wars: Rogue Squadron [Shutsugeki! Rogue Chuutai (J)]
  { "NSW", 1 }, // Star Wars: Shadows of the Empire [Teikoku no Kage (J)]
  { "NSC", 1 }, // Starshot: Space Circus Fever
  { "NSA", 1 }, // Sonic Wings Assault (J)
  { "NB6", 1 }, // Super B-Daman: Battle Phoenix 64
  { "NSM", 1 }, // Super Mario 64
  { "NSS", 1 }, // Super Robot Spirits
  { "NTX", 1 }, // Taz Express
  { "NT6", 1 }, // Tetris 64
  { "NTP", 1 }, // Tetrisphere
  { "NTJ", 1 }, // Tom & Jerry in Fists of Fury
  { "NRC", 1 }, // Top Gear Overdrive
  { "NTR", 1 }, // Top Gear Rally (J + E)
  { "NTB", 1 }, // Transformers: Beast Wars Metals 64 (J)
  { "NGU", 1 }, // Tsumi to Batsu: Hoshi no Keishousha (Sin and Punishment)
  { "NIR", 1 }, // Utchan Nanchan no Hono no Challenger: Denryuu Ira Ira Bou
  { "NVL", 1 }, // V-Rally Edition '99
  { "NVY", 1 }, // V-Rally Edition '99 (J)
  { "NWR", 1 }, // Wave Race 64
  { "NWC", 1 }, // Wild Choppers
  { "NAD", 1 }, // Worms Armageddon (U)
  { "NWU", 1 }, // Worms Armageddon (E)
  { "NYK", 1 }, // Yakouchuu II: Satsujin Kouro
  { "NMZ", 1 }, // Zool - Majou Tsukai Densetsu (J)

  // EEPROM16K
  { "NB7", 2 }, // Banjo-Tooie [Banjo to Kazooie no Daiboken 2 (J)]
  { "NGT", 2 }, // City Tour GrandPrix - Zen Nihon GT Senshuken
  { "NFU", 2 }, // Conker's Bad Fur Day
  { "NCW", 2 }, // Cruis'n World
  { "NCZ", 2 }, // Custom Robo V2
  { "ND6", 2 }, // Densha de Go! 64
  { "NDO", 2 }, // Donkey Kong 64
  { "ND2", 2 }, // Doraemon 2: Nobita to Hikari no Shinden
  { "N3D", 2 }, // Doraemon 3: Nobita no Machi SOS!
  { "NMX", 2 }, // Excitebike 64
  { "NGC", 2 }, // GT 64: Championship Edition
  { "NIM", 2 }, // Ide Yosuke no Mahjong Juku
  { "NNB", 2 }, // Kobe Bryant in NBA Courtside
  { "NMV", 2 }, // Mario Party 3
  { "NM8", 2 }, // Mario Tennis
  { "NEV", 2 }, // Neon Genesis Evangelion
  { "NPP", 2 }, // Parlor! Pro 64: Pachinko Jikki Simulation Game
  { "NUB", 2 }, // PD Ultraman Battle Collection 64
  { "NPD", 2 }, // Perfect Dark
  { "NRZ", 2 }, // Ridge Racer 64
  { "NR7", 2 }, // Robot Poncots 64: 7tsu no Umi no Caramel
  { "NEP", 2 }, // Star Wars Episode I: Racer
  { "NYS", 2 }, // Yoshi's Story

  // SRAM256K
  { "NTE", 3 }, // 1080 Snowboarding
  { "NVB", 3 }, // Bass Rush - ECOGEAR PowerWorm Championship (J)
  { "NB5", 3 }, // Biohazard 2 (J)
  { "CFZ", 3 }, // F-Zero X (J)
  { "NFZ", 3 }, // F-Zero X (U + E)
  { "NSI", 3 }, // Fushigi no Dungeon: Fuurai no Shiren 2
  { "NG6", 3 }, // Ganmare Goemon: Dero Dero Douchuu Obake Tenkomori
  { "NGP", 3 }, // Goemon: Mononoke Sugoroku
  { "NYW", 3 }, // Harvest Moon 64
  { "NHY", 3 }, // Hybrid Heaven (J)
  { "NIB", 3 }, // Itoi Shigesato no Bass Tsuri No. 1 Kettei Ban!
  { "NPS", 3 }, // Jikkyou J.League 1999: Perfect Striker 2
  { "NPA", 3 }, // Jikkyou Powerful Pro Yakyuu 2000
  { "NP4", 3 }, // Jikkyou Powerful Pro Yakyuu 4
  { "NJ5", 3 }, // Jikkyou Powerful Pro Yakyuu 5
  { "NP6", 3 }, // Jikkyou Powerful Pro Yakyuu 6
  { "NPE", 3 }, // Jikkyou Powerful Pro Yakyuu Basic Ban 2001
  { "NJG", 3 }, // Jinsei Game 64
  { "CZL", 3 }, // Legend of Zelda: Ocarina of Time [Zelda no Densetsu - Toki no Ocarina (J)]
  { "NZL", 3 }, // Legend of Zelda: Ocarina of Time (E)
  { "NKG", 3 }, // Major League Baseball featuring Ken Griffey Jr.
  { "NMF", 3 }, // Mario Golf 64
  { "NRI", 3 }, // New Tetris, The
  { "NUT", 3 }, // Nushi Zuri 64
  { "NUM", 3 }, // Nushi Zuri 64: Shiokaze ni Notte
  { "NOB", 3 }, // Ogre Battle 64: Person of Lordly Caliber
  { "CPS", 3 }, // Pocket Monsters Stadium (J)
  { "NPM", 3 }, // Premier Manager 64
  { "NRE", 3 }, // Resident Evil 2
  { "NAL", 3 }, // Super Smash Bros. [Nintendo All-Star! Dairantou Smash Brothers (J)]
  { "NT3", 3 }, // Shin Nihon Pro Wrestling - Toukon Road 2 - The Next Generation (J)
  { "NS4", 3 }, // Super Robot Taisen 64
  { "NA2", 3 }, // Virtual Pro Wrestling 2
  { "NVP", 3 }, // Virtual Pro Wrestling 64
  { "NWL", 3 }, // Waialae Country Club: True Golf Classics
  { "NW2", 3 }, // WCW-nWo Revenge
  { "NWX", 3 }, // WWF WrestleMania 2000

  // FLASH1024K
  { "NCC", 4 }, // Command & Conquer
  { "NDA", 4 }, // Derby Stallion 64
  { "NAF", 4 }, // Doubutsu no Mori
  { "NJF", 4 }, // Jet Force Gemini [Star Twins (J)]
  { "NKJ", 4 }, // Ken Griffey Jr.'s Slugfest
  { "NZS", 4 }, // Legend of Zelda: Majora's Mask [Zelda no Densetsu - Mujura no Kamen (J)]
  { "NM6", 4 }, // Mega Man 64
  { "NCK", 4 }, // NBA Courtside 2 featuring Kobe Bryant
  { "NMQ", 4 }, // Paper Mario
  { "NPN", 4 }, // Pokemon Puzzle League
  { "NPF", 4 }, // Pokemon Snap [Pocket Monsters Snap (J)]
  { "NPO", 4 }, // Pokemon Stadium
  { "CP2", 4 }, // Pocket Monsters Stadium 2 (J)
  { "NP3", 4 }, // Pokemon Stadium 2 [Pocket Monsters Stadium - Kin Gin (J)]
  { "NRH", 4 }, // Rockman Dash - Hagane no Boukenshin (J)
  { "NSQ", 4 }, // StarCraft 64
  { "NT9", 4 }, // Tigger's Honey Hunt
  { "NW4", 4 }, // WWF No Mercy
  { "NDP", 4 }, // Dinosaur Planet (Unlicensed)
};

static const DBEntry* findEntry(const std::string& id)
{
  static const size_t dbSize = sizeof(gamedb) / sizeof(gamedb[0]);

  for (size_t i = 0; i < dbSize; ++i)
    if (gamedb[i].id == id) return &gamedb[i];

  return nullptr;
}

static uint32_t getSaveSize(const int32_t& saveType)
{
  switch (saveType)
  {
    case 1:  return 0x00200; // EEPROM 512B
    case 2:  return 0x00800; // EEPROM 8KB
    case 3:  return 0x08000; // SRAM 32KB
    case 4:  return 0x20000; // FLASH 128KB
    default: return 0x00000;
  }
}

GameInfo GameDB::analyze(const uint8_t* data)
{
  GameInfo game = {};

  game.id  = "";
  game.id += (char)data[0x3b];
  game.id += (char)data[0x3c];
  game.id += (char)data[0x3d];

  switch(data[0x3e])
  {
    case 'A': game.region = "NTSC"; break;  // North America + Japan
    case 'D': game.region = "PAL";  break;  // Germany
    case 'E': game.region = "NTSC"; break;  // North America
    case 'F': game.region = "PAL";  break;  // France
    case 'G': game.region = "NTSC"; break;  // Gateway 64 (NTSC)
    case 'I': game.region = "PAL";  break;  // Italy
    case 'J': game.region = "NTSC"; break;  // Japan
    case 'L': game.region = "PAL";  break;  // Gateway 64 (PAL)
    case 'P': game.region = "PAL";  break;  // Europe
    case 'S': game.region = "PAL";  break;  // Spain
    case 'U': game.region = "PAL";  break;  // Australia
    case 'X': game.region = "PAL";  break;  // Europe
    case 'Y': game.region = "PAL";  break;  // Europe
  }

  game.regionCode = data[0x3e];
  game.revision = data[0x3f];

  const DBEntry* entry = findEntry(game.id);
  if (entry) game.saveType = entry->saveType;

  game.saveSize = getSaveSize(game.saveType);

  return game;
}
