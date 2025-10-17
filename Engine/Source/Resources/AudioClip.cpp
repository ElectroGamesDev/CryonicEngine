#include "AudioClip.h"

std::unordered_map<std::filesystem::path, Raylib::Sound> AudioClip::sounds;

bool AudioClip::LoadedInMemory() const
{
	return loadInMemory;
}

std::string AudioClip::GetPath() const
{
	return path;
}

Raylib::Sound* AudioClip::GetRaylibSound()
{
	return sound;
}
