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
    { "rokuyon_threadedRdp", "Threaded RDP; enabled|disabled" },
    { "rokuyon_texFilter", "Texture Filter; disabled|enabled" },
    { nullptr, nullptr }
  };

  envCallback(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)values);
}

static void updateConfig()
{
  Settings::fpsLimiter = fetchVariableBool("rokuyon_fpsLimiter", false);
  Settings::expansionPak = fetchVariableBool("rokuyon_expansionPak", false);
  Settings::threadedRdp = fetchVariableBool("rokuyon_threadedRdp", true);
  Settings::texFilter = fetchVariableBool("rokuyon_texFilter", false);
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
    memset(videoBuffer.data(), 0, videoBuffer.size() * sizeof(videoBuffer[0]));
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

static void convertColors(uint32_t *src, uint32_t *dst, size_t pixels)
{
  for (int i = 0; i < pixels; ++i)
  {
    uint32_t pixel = src[i];

    dst[i] =
      ((pixel & 0xFF000000)) |
      ((pixel & 0x00FF0000) >> 16) |
      ((pixel & 0x0000FF00)) |
      ((pixel & 0x000000FF) << 16);
  }
}

static void drawTexture()
{
  if (_Framebuffer *fb = VI::getFramebuffer())
  {
    updateVideoGeometry(fb->width, fb->height);
    resizeVideoBuffer(fb->width * fb->height);

    convertColors(fb->data, videoBuffer.data(), videoBufferSize);
    videoCallback(videoBuffer.data(), fb->width, fb->height, fb->width * 4);

    delete fb;
  }
  else
  {
    videoCallback(NULL, videoWidth, videoHeight, videoWidth * 4);
  }
}

static void playbackAudio()
{
  static int16_t buffer[1024 * 2];
  uint32_t original[1024];

  AI::fillBuffer(original);

  for (int i = 0; i < 1024; i++)
  {
    buffer[i * 2 + 0] = original[i] >>  0;
    buffer[i * 2 + 1] = original[i] >> 16;
  }

  uint32_t size = sizeof(buffer) / (2 * sizeof(int16_t));
  audioBatchCallback(buffer, size);
}

void retro_get_system_info(retro_system_info* info)
{
  info->need_fullpath = true;
  info->valid_extensions = "z64";
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

  info->timing.fps = 60.0;
  info->timing.sample_rate = 48000.0;
}

void retro_set_environment(retro_environment_t cb)
{
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

  Core::savePath = savePath;
  Core::saveSize = 0;

  if (Core::bootRom(gamePath))
  {
    gameInfo = GameDB::analyze(Core::rom);

    if (!Core::saveSize && gameInfo.saveSize)
    {
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
}

void retro_reset(void)
{
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

  drawTexture();
  playbackAudio();
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
  return gameInfo.region == "PAL" ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
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
