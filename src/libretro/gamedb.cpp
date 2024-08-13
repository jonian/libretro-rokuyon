#include <unordered_set>
#include <fstream>

#include "gamedb.h"

// Game codes and matching save types are ported from ares emulator
// https://github.com/ares-emulator/ares/blob/master/mia/medium/nintendo-64.cpp

static const std::unordered_set<std::string> eeprom4k = {
  "NTW", // 64 de Hakken!! Tamagotchi
  "NHF", // 64 Hanafuda: Tenshi no Yakusoku
  "NOS", // 64 Oozumou
  "NTC", // 64 Trump Collection
  "NER", // Aero Fighters Assault [Sonic Wings Assault (J)]
  "NAG", // AeroGauge
  "NAB", // Air Boarder 64
  "NS3", // AI Shougi 3
  "NTN", // All Star Tennis '99
  "NBN", // Bakuretsu Muteki Bangaioh
  "NBK", // Banjo-Kazooie [Banjo to Kazooie no Daiboken (J)]
  "NFH", // In-Fisherman Bass Hunter 64
  "NMU", // Big Mountain 2000
  "NBC", // Blast Corps
  "NBH", // Body Harvest
  "NHA", // Bomberman 64: Arcade Edition (J)
  "NBM", // Bomberman 64 [Baku Bomberman (J)]
  "NBV", // Bomberman 64: The Second Attack! [Baku Bomberman 2 (J)]
  "NBD", // Bomberman Hero [Mirian Ojo o Sukue! (J)]
  "NCT", // Chameleon Twist
  "NCH", // Chopper Attack
  "NCG", // Choro Q 64 II - Hacha Mecha Grand Prix Race (J)
  "NP2", // Chou Kuukan Night Pro Yakyuu King 2 (J)
  "NXO", // Cruis'n Exotica
  "NCU", // Cruis'n USA
  "NCX", // Custom Robo
  "NDY", // Diddy Kong Racing
  "NDQ", // Disney's Donald Duck - Goin' Quackers [Quack Attack (E)]
  "NDR", // Doraemon: Nobita to 3tsu no Seireiseki
  "NN6", // Dr. Mario 64
  "NDU", // Duck Dodgers starring Daffy Duck
  "NJM", // Earthworm Jim 3D
  "NFW", // F-1 World Grand Prix
  "NF2", // F-1 World Grand Prix II
  "NKA", // Fighters Destiny [Fighting Cup (J)]
  "NFG", // Fighter Destiny 2
  "NGL", // Getter Love!!
  "NGV", // Glover
  "NGE", // GoldenEye 007
  "NHP", // Heiwa Pachinko World 64
  "NPG", // Hey You, Pikachu! [Pikachu Genki Dechu (J)]
  "NIJ", // Indiana Jones and the Infernal Machine
  "NIC", // Indy Racing 2000
  "NFY", // Kakutou Denshou: F-Cup Maniax
  "NKI", // Killer Instinct Gold
  "NLL", // Last Legion UX
  "NLR", // Lode Runner 3-D
  "NKT", // Mario Kart 64
  "CLB", // Mario Party (NTSC)
  "NLB", // Mario Party (PAL)
  "NMW", // Mario Party 2
  "NML", // Mickey's Speedway USA [Mickey no Racing Challenge USA (J)]
  "NTM", // Mischief Makers [Yuke Yuke!! Trouble Makers (J)]
  "NMI", // Mission: Impossible
  "NMG", // Monaco Grand Prix [Racing Simulation 2 (G)]
  "NMO", // Monopoly
  "NMS", // Morita Shougi 64
  "NMR", // Multi-Racing Championship
  "NCR", // Penny Racers [Choro Q 64 (J)]
  "NEA", // PGA European Tour
  "NPW", // Pilotwings 64
  "NPY", // Puyo Puyo Sun 64
  "NPT", // Puyo Puyon Party
  "NRA", // Rally '99 (J)
  "NWQ", // Rally Challenge 2000
  "NSU", // Rocket: Robot on Wheels
  "NSN", // Snow Speeder (J)
  "NK2", // Snowboard Kids 2 [Chou Snobow Kids (J)]
  "NSV", // Space Station Silicon Valley
  "NFX", // Star Fox 64 [Lylat Wars (E)]
  "NS6", // Star Soldier: Vanishing Earth
  "NNA", // Star Wars Episode I: Battle for Naboo
  "NRS", // Star Wars: Rogue Squadron [Shutsugeki! Rogue Chuutai (J)]
  "NSW", // Star Wars: Shadows of the Empire [Teikoku no Kage (J)]
  "NSC", // Starshot: Space Circus Fever
  "NSA", // Sonic Wings Assault (J)
  "NB6", // Super B-Daman: Battle Phoenix 64
  "NSM", // Super Mario 64
  "NSS", // Super Robot Spirits
  "NTX", // Taz Express
  "NT6", // Tetris 64
  "NTP", // Tetrisphere
  "NTJ", // Tom & Jerry in Fists of Fury
  "NRC", // Top Gear Overdrive
  "NTR", // Top Gear Rally (J + E)
  "NTB", // Transformers: Beast Wars Metals 64 (J)
  "NGU", // Tsumi to Batsu: Hoshi no Keishousha (Sin and Punishment)
  "NIR", // Utchan Nanchan no Hono no Challenger: Denryuu Ira Ira Bou
  "NVL", // V-Rally Edition '99
  "NVY", // V-Rally Edition '99 (J)
  "NWR", // Wave Race 64
  "NWC", // Wild Choppers
  "NAD", // Worms Armageddon (U)
  "NWU", // Worms Armageddon (E)
  "NYK", // Yakouchuu II: Satsujin Kouro
  "NMZ", // Zool - Majou Tsukai Densetsu (J)
};

static const std::unordered_set<std::string> eeprom16k = {
  "NB7", // Banjo-Tooie [Banjo to Kazooie no Daiboken 2 (J)]
  "NGT", // City Tour GrandPrix - Zen Nihon GT Senshuken
  "NFU", // Conker's Bad Fur Day
  "NCW", // Cruis'n World
  "NCZ", // Custom Robo V2
  "ND6", // Densha de Go! 64
  "NDO", // Donkey Kong 64
  "ND2", // Doraemon 2: Nobita to Hikari no Shinden
  "N3D", // Doraemon 3: Nobita no Machi SOS!
  "NMX", // Excitebike 64
  "NGC", // GT 64: Championship Edition
  "NIM", // Ide Yosuke no Mahjong Juku
  "NNB", // Kobe Bryant in NBA Courtside
  "NMV", // Mario Party 3
  "NM8", // Mario Tennis
  "NEV", // Neon Genesis Evangelion
  "NPP", // Parlor! Pro 64: Pachinko Jikki Simulation Game
  "NUB", // PD Ultraman Battle Collection 64
  "NPD", // Perfect Dark
  "NRZ", // Ridge Racer 64
  "NR7", // Robot Poncots 64: 7tsu no Umi no Caramel
  "NEP", // Star Wars Episode I: Racer
  "NYS", // Yoshi's Story
};

static const std::unordered_set<std::string> sram256k = {
  "NTE", // 1080 Snowboarding
  "NVB", // Bass Rush - ECOGEAR PowerWorm Championship (J)
  "NB5", // Biohazard 2 (J)
  "CFZ", // F-Zero X (J)
  "NFZ", // F-Zero X (U + E)
  "NSI", // Fushigi no Dungeon: Fuurai no Shiren 2
  "NG6", // Ganmare Goemon: Dero Dero Douchuu Obake Tenkomori
  "NGP", // Goemon: Mononoke Sugoroku
  "NYW", // Harvest Moon 64
  "NHY", // Hybrid Heaven (J)
  "NIB", // Itoi Shigesato no Bass Tsuri No. 1 Kettei Ban!
  "NPS", // Jikkyou J.League 1999: Perfect Striker 2
  "NPA", // Jikkyou Powerful Pro Yakyuu 2000
  "NP4", // Jikkyou Powerful Pro Yakyuu 4
  "NJ5", // Jikkyou Powerful Pro Yakyuu 5
  "NP6", // Jikkyou Powerful Pro Yakyuu 6
  "NPE", // Jikkyou Powerful Pro Yakyuu Basic Ban 2001
  "NJG", // Jinsei Game 64
  "CZL", // Legend of Zelda: Ocarina of Time [Zelda no Densetsu - Toki no Ocarina (J)]
  "NZL", // Legend of Zelda: Ocarina of Time (E)
  "NKG", // Major League Baseball featuring Ken Griffey Jr.
  "NMF", // Mario Golf 64
  "NRI", // New Tetris, The
  "NUT", // Nushi Zuri 64
  "NUM", // Nushi Zuri 64: Shiokaze ni Notte
  "NOB", // Ogre Battle 64: Person of Lordly Caliber
  "CPS", // Pocket Monsters Stadium (J)
  "NPM", // Premier Manager 64
  "NRE", // Resident Evil 2
  "NAL", // Super Smash Bros. [Nintendo All-Star! Dairantou Smash Brothers (J)]
  "NT3", // Shin Nihon Pro Wrestling - Toukon Road 2 - The Next Generation (J)
  "NS4", // Super Robot Taisen 64
  "NA2", // Virtual Pro Wrestling 2
  "NVP", // Virtual Pro Wrestling 64
  "NWL", // Waialae Country Club: True Golf Classics
  "NW2", // WCW-nWo Revenge
  "NWX", // WWF WrestleMania 2000
};

static const std::unordered_set<std::string> flash1024k = {
  "NCC", // Command & Conquer
  "NDA", // Derby Stallion 64
  "NAF", // Doubutsu no Mori
  "NJF", // Jet Force Gemini [Star Twins (J)]
  "NKJ", // Ken Griffey Jr.'s Slugfest
  "NZS", // Legend of Zelda: Majora's Mask [Zelda no Densetsu - Mujura no Kamen (J)]
  "NM6", // Mega Man 64
  "NCK", // NBA Courtside 2 featuring Kobe Bryant
  "NMQ", // Paper Mario
  "NPN", // Pokemon Puzzle League
  "NPF", // Pokemon Snap [Pocket Monsters Snap (J)]
  "NPO", // Pokemon Stadium
  "CP2", // Pocket Monsters Stadium 2 (J)
  "NP3", // Pokemon Stadium 2 [Pocket Monsters Stadium - Kin Gin (J)]
  "NRH", // Rockman Dash - Hagane no Boukenshin (J)
  "NSQ", // StarCraft 64
  "NT9", // Tigger's Honey Hunt
  "NW4", // WWF No Mercy
  "NDP", // Dinosaur Planet (Unlicensed)
};

std::string GameDB::getGameID(const std::string& romPath)
{
  std::ifstream romFile(romPath, std::ios::binary);
  if (!romFile) return "";

  romFile.seekg(0x3B, std::ios::beg);

  char gameId[4] = {0};
  romFile.read(gameId, 3);

  romFile.close();

  return std::string(gameId);
}

uint32_t GameDB::getSaveSize(const std::string& gameId)
{
  if (eeprom4k.find(gameId) != eeprom4k.end())
    return 0x00200; // EEPROM 512B

  if (eeprom16k.find(gameId) != eeprom16k.end())
    return 0x00800; // EEPROM 8KB

  if (sram256k.find(gameId) != sram256k.end())
    return 0x08000; // SRAM 32KB

  if (flash1024k.find(gameId) != flash1024k.end())
    return 0x20000; // FLASH 128KB

  return 0x00000;
}
