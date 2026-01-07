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

#ifndef HEADER_PINGUS_ENGINE_SOUND_SOUND_SDL_MIXER_HPP
#define HEADER_PINGUS_ENGINE_SOUND_SOUND_SDL_MIXER_HPP

#include "engine/sound/sound_impl.hpp"

#include <SDL_mixer.h>

#include <string>
#include <unordered_map>

namespace pingus::sound {

class PingusSoundSDLMixer : public PingusSoundImpl
{
public:
  PingusSoundSDLMixer();
  ~PingusSoundSDLMixer() override;

  void update(float delta) override;
  void real_play_music(const std::string& filename, float volume, bool loop) override;
  void real_stop_music() override;
  void real_play_sound(const std::string& filename, float volume, float panning) override;
  void set_sound_volume(float volume) override;
  void set_music_volume(float volume) override;
  void set_master_volume(float volume) override;
  float get_sound_volume() const override;
  float get_music_volume() const override;
  float get_master_volume() const override;

private:
  Mix_Chunk* load_chunk(const std::string& path);
  void apply_volume_changes();
  int to_mix_volume(float volume) const;
  void apply_panning(int channel, float panning) const;

  std::unordered_map<std::string, Mix_Chunk*> m_chunks;
  Mix_Music* m_music;
  float m_sound_volume;
  float m_music_volume;
  float m_master_volume;

  PingusSoundSDLMixer(PingusSoundSDLMixer const&) = delete;
  PingusSoundSDLMixer& operator=(PingusSoundSDLMixer const&) = delete;
};

} // namespace pingus::sound

#endif
