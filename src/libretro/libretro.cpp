#include <cstdarg>
#include <cstring>
#include <cstdint>
#include <vector>
#include <regex>
#include <fstream>

#include "gamedb.h"
#include "libretro.h"

#include "../ai.h"
#include "../pif.h"
#include "../vi.h"
#include "../memory.h"
#include "../core.h"
#include "../settings.h"

#ifndef VERSION
#define VERSION "0.1"
#endif

static retro_environment_t envCallback;
static retro_video_refresh_t videoCallback;
static retro_audio_sample_batch_t audioBatchCallback;
static retro_input_poll_t inputPollCallback;
static retro_input_state_t inputStateCallback;

static struct retro_log_callback logging;
static retro_log_printf_t logCallback;

static bool cropBorders;

static std::string systemPath;
static std::string savesPath;

static std::string gamePath;
static std::string savePath;

static GameInfo gameInfo = {};

static std::vector<uint32_t> videoBuffer;
static uint32_t videoBufferSize;

static int videoWidth = 640;
static int videoHeight = 480;

static int keymap[] = {
  RETRO_DEVICE_ID_JOYPAD_A,
  RETRO_DEVICE_ID_JOYPAD_B,
  RETRO_DEVICE_ID_JOYPAD_L2,
  RETRO_DEVICE_ID_JOYPAD_START,
  RETRO_DEVICE_ID_JOYPAD_UP,
  RETRO_DEVICE_ID_JOYPAD_DOWN,
  RETRO_DEVICE_ID_JOYPAD_LEFT,
  RETRO_DEVICE_ID_JOYPAD_RIGHT,
  0, 0,
  RETRO_DEVICE_ID_JOYPAD_L,
  RETRO_DEVICE_ID_JOYPAD_R
};

static int32_t clampValue(int32_t value, int32_t min, int32_t max)
{
  return std::max(min, std::min(max, value));
}

static std::string normalizePath(std::string path, bool addSlash = false)
{
  std::string newPath = path;
  if (addSlash && newPath.back() != '/') newPath += '/';
#ifdef WINDOWS
  std::replace(newPath.begin(), newPath.end(), '\\', '/');
#endif
  return newPath;
}

static std::string getNameFromPath(std::string path)
{
  std::string base = path.substr(path.find_last_of("/\\") + 1);
  for (const auto& delim : {".zip#", ".7z#", ".apk#"})
  {
    size_t delimPos = base.find(delim);
    if (delimPos != std::string::npos) base = base.substr(0, delimPos);
  }
  return base.substr(0, base.rfind("."));
}

static void logFallback(enum retro_log_level level, const char *fmt, ...)
{
  (void)level;
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
}

static std::string fetchVariable(std::string key, std::string def)
{
  struct retro_variable var = { nullptr };
  var.key = key.c_str();

  if (!envCallback(RETRO_ENVIRONMENT_GET_VARIABLE, &var) || var.value == nullptr)
  {
    logCallback(RETRO_LOG_WARN, "Fetching variable %s failed.", var.key);
    return def;
  }

  return std::string(var.value);
}

static bool fetchVariableBool(std::string key, bool def)
{
  return fetchVariable(key, def ? "enabled" : "disabled") == "enabled";
}

static std::string getSaveDir()
{
  char* dir = nullptr;
  if (!envCallback(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) || dir == nullptr)
  {
    logCallback(RETRO_LOG_INFO, "No save directory provided by LibRetro.");
    return std::string("rokuyon");
  }
  return std::string(dir);
}

static std::string getSystemDir()
{
  char* dir = nullptr;
  if (!envCallback(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) || dir == nullptr)
  {
    logCallback(RETRO_LOG_INFO, "No system directory provided by LibRetro.");
    return std::string("rokuyon");
  }
  return std::string(dir);
}

static bool getButtonState(unsigned id)
{
  return inputStateCallback(0, RETRO_DEVICE_JOYPAD, 0, id);
}

static float getAxisState(unsigned index, unsigned id)
{
  return inputStateCallback(0, RETRO_DEVICE_ANALOG, index, id);
}

static void initInput(void)
{
  static const struct retro_controller_description controllers[] = {
    { "Controller", RETRO_DEVICE_JOYPAD },
    { NULL, 0 },
  };

  static const struct retro_controller_info ports[] = {
    { controllers, 1 },
    { controllers, 1 },
    { controllers, 1 },
    { controllers, 1 },
    { NULL, 0 },
  };

  envCallback(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);

  struct retro_input_descriptor desc[] = {
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "Z" },
    { 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Stick X" },
    { 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Stick Y" },
    { 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X, "C-Pad X" },
    { 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y, "C-Pad Y" },
    { 0 },
  };

  envCallback(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, &desc);
}

static void initConfig()
{
  static const retro_variable values[] = {
    { "rokuyon_fpsLimiter", "FPS Limiter; disabled|enabled" },
    { "rokuyon_expansionPak", "Expansion Pak; disabled|enabled" },
    { "rokuyon_threadedRdp", "Threaded RDP; disabled|enabled" },
    { "rokuyon_texFilter", "Texture Filter; disabled|enabled" },
    { "rokuyon_cropBorders", "Crop Borders; disabled|enabled" },
    { nullptr, nullptr }
  };

  envCallback(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)values);
}

static void updateConfig()
{
  Settings::fpsLimiter = fetchVariableBool("rokuyon_fpsLimiter", false);
  Settings::expansionPak = fetchVariableBool("rokuyon_expansionPak", false);
  Settings::threadedRdp = fetchVariableBool("rokuyon_threadedRdp", false);
  Settings::texFilter = fetchVariableBool("rokuyon_texFilter", false);

  cropBorders = fetchVariableBool("rokuyon_cropBorders", false);
}

static void checkConfigVariables()
{
  bool updated = false;
  envCallback(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated);

  if (updated) updateConfig();
}

static void resizeVideoBuffer(uint32_t newSize)
{
  if (newSize != videoBufferSize)
  {
    videoBufferSize = newSize;
    videoBuffer.resize(videoBufferSize);
    memset(videoBuffer.data(), 0, videoBufferSize * sizeof(uint32_t));
  }
}

static void updateVideoGeometry(int width, int height)
{
  if (width != videoWidth || height != videoHeight)
  {
    videoWidth = width;
    videoHeight = height;

    retro_system_av_info info;
    retro_get_system_av_info(&info);
    envCallback(RETRO_ENVIRONMENT_SET_GEOMETRY, &info);
  }
}

static void copyScreen(uint32_t *src, uint32_t *dst, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh)
{
  if (sw > dw || sh > dh)
  {
    int offsetX = (sw - dw) / 2;
    int offsetY = (sh - dh) / 2;

    for (size_t y = 0; y < dh; y++)
    {
      size_t srcIndex = ((y + offsetY) * sw) + offsetX;
      size_t dstIndex = y * dw;

      memcpy(dst + dstIndex, src + srcIndex, dw * sizeof(uint32_t));
    }
  }
  else
  {
    memcpy(dst, src, sw * sh * sizeof(uint32_t));
  }
}

static void renderVideo()
{
  if (_Framebuffer *fb = VI::getFramebuffer())
  {
    int fbBorder = cropBorders ? 8 : 0;
    int fbWidth = clampValue(fb->width, 256, 640) - (fbBorder * 2);
    int fbHeight = clampValue(fb->height, 224, 480) - (fbBorder * 2);

    updateVideoGeometry(fbWidth, fbHeight);
    resizeVideoBuffer(fbWidth * fbHeight);

    copyScreen(fb->data, videoBuffer.data(), fb->width, fb->height, videoWidth, videoHeight);
    videoCallback(videoBuffer.data(), videoWidth, videoHeight, videoWidth * 4);

    delete fb;
  }
  else
  {
    videoCallback(NULL, videoWidth, videoHeight, videoWidth * 4);
  }
}

static void renderAudio()
{
  static int16_t buffer[1024 * 2];
  AI::fillBuffer((uint32_t*)buffer);

  uint32_t size = sizeof(buffer) / (2 * sizeof(int16_t));
  audioBatchCallback(buffer, size);
}

static uint8_t* convertRom(const void* data, const size_t& size)
{
  const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
  uint8_t* output = new uint8_t[size];

  // Extract the first four bytes
  uint32_t header = (input[0] << 24) | (input[1] << 16) | (input[2] << 8) | input[3];

  // Known ROM format headers
  const uint32_t N64_HEADER = 0x40123780; // Little Endian (.n64)
  const uint32_t V64_HEADER = 0x37804012; // Byte Swapped (.v64)
  const uint32_t Z64_HEADER = 0x80371240; // Big Endian (.z64)

  switch (header)
  {
    case N64_HEADER:
      // Reverse bytes in 4-byte chunks
      logCallback(RETRO_LOG_INFO, "Detected ROM format: N64\n");
      for (size_t i = 0; i < size; i += 4)
      {
        output[i + 0] = input[i + 3];
        output[i + 1] = input[i + 2];
        output[i + 2] = input[i + 1];
        output[i + 3] = input[i + 0];
      }
      break;
    case V64_HEADER:
      // Swap adjacent bytes in 2-byte chunks
      logCallback(RETRO_LOG_INFO, "Detected ROM format: V64\n");
      for (size_t i = 0; i < size; i += 2)
      {
        output[i + 0] = input[i + 1];
        output[i + 1] = input[i + 0];
      }
      break;
    case Z64_HEADER:
      // No change needed, copy the data as is
      logCallback(RETRO_LOG_INFO, "Detected ROM format: Z64\n");
      memcpy(output, input, size);
      break;
    default:
      // Unknown format, copy the data as is
      logCallback(RETRO_LOG_INFO, "Detected ROM format: Unknown\n");
      memcpy(output, input, size);
      break;
  }

  return output;
}

void retro_get_system_info(retro_system_info* info)
{
  info->need_fullpath = false;
  info->valid_extensions = "z64|n64|v64";
  info->library_version = VERSION;
  info->library_name = "Rokuyon";
  info->block_extract = false;
}

void retro_get_system_av_info(retro_system_av_info* info)
{
  info->geometry.base_width = videoWidth;
  info->geometry.base_height = videoHeight;

  info->geometry.max_width = info->geometry.base_width;
  info->geometry.max_height = info->geometry.base_height;
  info->geometry.aspect_ratio = 4.0 / 3.0;

  info->timing.fps = gameInfo.ntsc ? 60.0 : 50.0;
  info->timing.sample_rate = 48000.0;
}

void retro_set_environment(retro_environment_t cb)
{
  const struct retro_system_content_info_override contentOverrides[] = {
    { "z64|n64|v64", false, false },
    {}
  };

  cb(RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE, (void*)contentOverrides);

  envCallback = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
  videoCallback = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
  audioBatchCallback = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
}

void retro_set_input_poll(retro_input_poll_t cb)
{
  inputPollCallback = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
  inputStateCallback = cb;
}

void retro_init(void)
{
  enum retro_pixel_format xrgb888 = RETRO_PIXEL_FORMAT_XRGB8888;
  envCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &xrgb888);

  if (envCallback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
    logCallback = logging.log;
  else
    logCallback = logFallback;

  systemPath = normalizePath(getSystemDir(), true);
  savesPath = normalizePath(getSaveDir(), true);
}

void retro_deinit(void)
{
  logCallback = nullptr;
}

bool retro_load_game(const struct retro_game_info* info)
{
  gamePath = normalizePath(info->path);
  savePath = normalizePath(savesPath + getNameFromPath(info->path) + ".sav");

  initConfig();
  updateConfig();
  initInput();

  Core::stop();

  Core::rom = convertRom(info->data, info->size);
  Core::romSize = (uint32_t)info->size;

  Core::savePath = savePath;
  Core::saveSize = 0;

  if (Core::bootRom(gamePath))
  {
    gameInfo = GameDB::analyze(Core::rom);

    if (!Core::saveSize && gameInfo.saveSize)
    {
      Core::stop();
      Core::resizeSave(gameInfo.saveSize);
      Core::bootRom(gamePath);
    }

    return true;
  }

  return false;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info* info, size_t info_size)
{
  return false;
}

void retro_unload_game(void)
{
  Core::stop();

  Core::romSize = 0;
  if (Core::rom) delete[] Core::rom;

  Core::saveSize = 0;
  if (Core::save) delete[] Core::save;
}

void retro_reset(void)
{
  Core::stop();
  Core::bootRom(gamePath);
}

void retro_run(void)
{
  checkConfigVariables();
  inputPollCallback();

  float stickX = getAxisState(RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
  float stickY = getAxisState(RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

  stickX = (stickX / +32767) * 80;
  stickY = (stickY / -32767) * 80;

  if (stickX && stickY)
  {
    stickX = stickX * 60 / 80;
    stickY = stickY * 60 / 80;
  }

  PIF::setStick(stickX, stickY);

  float cpadX = getAxisState(RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X);
  float cpadY = getAxisState(RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y);

  cpadX = (cpadX / +32767);
  cpadY = (cpadY / -32767);

  if (cpadX == -1) PIF::pressKey(14); // C-Pad Left
  else PIF::releaseKey(14);

  if (cpadX == +1) PIF::pressKey(15); // C-Pad Right
  else PIF::releaseKey(15);

  if (cpadY == +1) PIF::pressKey(12); // C-Pad Up
  else PIF::releaseKey(12);

  if (cpadY == -1) PIF::pressKey(13); // C-Pad Down
  else PIF::releaseKey(13);

  for (int i = 0; i < sizeof(keymap) / sizeof(*keymap); ++i)
  {
    if (getButtonState(keymap[i]))
      PIF::pressKey(i);
    else
      PIF::releaseKey(i);
  }

  Core::start();

  renderVideo();
  renderAudio();

  Core::stop();
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

size_t retro_serialize_size(void)
{
  size_t size = 0;
  return size;
}

bool retro_serialize(void* data, size_t size)
{
  return false;
}

bool retro_unserialize(const void* data, size_t size)
{
  return false;
}

unsigned retro_get_region(void)
{
  return gameInfo.ntsc ? RETRO_REGION_NTSC : RETRO_REGION_PAL;
}

unsigned retro_api_version()
{
  return RETRO_API_VERSION;
}

size_t retro_get_memory_size(unsigned id)
{
  if (id == RETRO_MEMORY_SYSTEM_RAM)
  {
    return Settings::expansionPak ? 0x800000 : 0x400000;
  }
  return 0;
}

void* retro_get_memory_data(unsigned id)
{
  if (id == RETRO_MEMORY_SYSTEM_RAM)
  {
    static uint32_t data = Memory::read<uint32_t>(0);
    return &data;
  }
  return NULL;
}

void retro_cheat_set(unsigned index, bool enabled, const char* code)
{
}

void retro_cheat_reset(void)
{
}
