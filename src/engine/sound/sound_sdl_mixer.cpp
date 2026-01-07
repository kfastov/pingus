// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "engine/sound/sound_sdl_mixer.hpp"

#include <SDL_mixer.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>

#include <logmich/log.hpp>

#include "pingus/globals.hpp"
#include "pingus/path_manager.hpp"

namespace pingus::sound {

namespace {

constexpr int kAudioRate = 44100;
constexpr int kAudioChannels = 2;
constexpr int kAudioChunkSize = 1024;

float clamp01(float value)
{
  return std::clamp(value, 0.0f, 1.0f);
}

} // namespace

PingusSoundSDLMixer::PingusSoundSDLMixer() :
  m_chunks(),
  m_music(nullptr),
  m_sound_volume(1.0f),
  m_music_volume(1.0f),
  m_master_volume(1.0f)
{
  if (Mix_OpenAudio(kAudioRate, MIX_DEFAULT_FORMAT, kAudioChannels, kAudioChunkSize) < 0)
  {
    throw std::runtime_error(std::string("Mix_OpenAudio failed: ") + Mix_GetError());
  }

  int init_flags = Mix_Init(MIX_INIT_MOD);
  if ((init_flags & MIX_INIT_MOD) != MIX_INIT_MOD)
  {
    log_warn("SDL_mixer: MOD support unavailable: {}", Mix_GetError());
  }

  Mix_AllocateChannels(32);
  apply_volume_changes();
}

PingusSoundSDLMixer::~PingusSoundSDLMixer()
{
  if (m_music)
  {
    Mix_HaltMusic();
    Mix_FreeMusic(m_music);
    m_music = nullptr;
  }

  for (auto& entry : m_chunks)
  {
    Mix_FreeChunk(entry.second);
  }
  m_chunks.clear();

  Mix_CloseAudio();
  Mix_Quit();
}

void
PingusSoundSDLMixer::update(float /*delta*/)
{
}

Mix_Chunk*
PingusSoundSDLMixer::load_chunk(const std::string& path)
{
  auto it = m_chunks.find(path);
  if (it != m_chunks.end())
  {
    return it->second;
  }

  Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
  if (!chunk)
  {
    log_error("SDL_mixer: failed to load sound {}: {}", path, Mix_GetError());
    return nullptr;
  }

  m_chunks.emplace(path, chunk);
  return chunk;
}

void
PingusSoundSDLMixer::real_play_sound(const std::string& name, float volume, float panning)
{
  if (!globals::sound_enabled || m_sound_volume <= 0.0f || m_master_volume <= 0.0f)
  {
    return;
  }

  std::filesystem::path filename = g_path_manager.complete("sounds/" + name + ".wav");
  Mix_Chunk* chunk = load_chunk(filename.string());
  if (!chunk)
  {
    return;
  }

  int channel = Mix_PlayChannel(-1, chunk, 0);
  if (channel < 0)
  {
    log_error("SDL_mixer: failed to play sound {}: {}", filename.string(), Mix_GetError());
    return;
  }

  int channel_volume = to_mix_volume(volume * m_sound_volume * m_master_volume);
  Mix_Volume(channel, channel_volume);
  apply_panning(channel, panning);
}

void
PingusSoundSDLMixer::real_play_music(const std::string& filename, float volume, bool loop)
{
  if (!globals::music_enabled || m_master_volume <= 0.0f)
  {
    return;
  }

  if (m_music)
  {
    Mix_HaltMusic();
    Mix_FreeMusic(m_music);
    m_music = nullptr;
  }

  m_music = Mix_LoadMUS(filename.c_str());
  if (!m_music)
  {
    log_error("SDL_mixer: failed to load music {}: {}", filename, Mix_GetError());
    return;
  }

  m_music_volume = clamp01(volume);
  apply_volume_changes();

  int loops = loop ? -1 : 1;
  if (Mix_PlayMusic(m_music, loops) < 0)
  {
    log_error("SDL_mixer: failed to play music {}: {}", filename, Mix_GetError());
  }
}

void
PingusSoundSDLMixer::real_stop_music()
{
  Mix_HaltMusic();
}

void
PingusSoundSDLMixer::set_sound_volume(float volume)
{
  m_sound_volume = clamp01(volume);
  apply_volume_changes();
}

void
PingusSoundSDLMixer::set_music_volume(float volume)
{
  m_music_volume = clamp01(volume);
  apply_volume_changes();
}

void
PingusSoundSDLMixer::set_master_volume(float volume)
{
  m_master_volume = clamp01(volume);
  apply_volume_changes();
}

float
PingusSoundSDLMixer::get_sound_volume() const
{
  return m_sound_volume;
}

float
PingusSoundSDLMixer::get_music_volume() const
{
  return m_music_volume;
}

float
PingusSoundSDLMixer::get_master_volume() const
{
  return m_master_volume;
}

int
PingusSoundSDLMixer::to_mix_volume(float volume) const
{
  return static_cast<int>(std::lround(clamp01(volume) * MIX_MAX_VOLUME));
}

void
PingusSoundSDLMixer::apply_volume_changes()
{
  int sound_volume = to_mix_volume(m_sound_volume * m_master_volume);
  int music_volume = to_mix_volume(m_music_volume * m_master_volume);

  Mix_Volume(-1, sound_volume);
  Mix_VolumeMusic(music_volume);
}

void
PingusSoundSDLMixer::apply_panning(int channel, float panning) const
{
  float pan = std::clamp(panning, -1.0f, 1.0f);
  Uint8 left = static_cast<Uint8>(std::lround((1.0f - pan) * 127.5f));
  Uint8 right = static_cast<Uint8>(std::lround((1.0f + pan) * 127.5f));
  Mix_SetPanning(channel, left, right);
}

} // namespace pingus::sound

/* EOF */
