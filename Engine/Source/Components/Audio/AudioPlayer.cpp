#include "AudioPlayer.h"
#ifndef EDITOR
#include "Game.h"
#endif

void AudioPlayer::Awake()
{
	if (audioClip == nullptr)
		return;

	std::filesystem::path path;
#ifndef EDITOR
	if (exeParent.empty())
		path = "Resources/Assets/" + audioClip->GetPath();
	else
		path = std::filesystem::path(exeParent) / "Resources" / "Assets" / audioClip->GetPath();
#endif

	if (this->audioClip->LoadedInMemory())
	{
		sound = new Raylib::Sound();
		*sound = Raylib::LoadSoundAlias(*this->audioClip->GetRaylibSound());
	}
	else
	{
		music = new Raylib::Music();
		*music = Raylib::LoadMusicStream(path.string().c_str());
		music->looping = loop;
	}
}

void AudioPlayer::Start()
{
	if (playOnStart && audioClip != nullptr)
		Play();
}

void AudioPlayer::Update()
{
	if (audioClip != nullptr && !audioClip->LoadedInMemory())
		Raylib::UpdateMusicStream(*music);

	if (playingSound && !Raylib::IsSoundPlaying(*sound))
	{
		if (loop)
			Raylib::PlaySound(*sound);
		else
			playingSound = false;
	}

	for (auto it = musicStreams.begin(); it != musicStreams.end();)
	{
		if (!Raylib::IsMusicStreamPlaying(*it)) // Removes the music from the vector if its finished playing
		{
			Raylib::UnloadMusicStream(*it);
			it = musicStreams.erase(it);
		}
		else
		{
			Raylib::UpdateMusicStream(*it);
			++it;
		}
	}

	for (auto it = sounds.begin(); it != sounds.end();)
	{
		if (!Raylib::IsSoundPlaying(*it)) // Removes the sound from the vector if its finished playing
		{
			Raylib::UnloadSoundAlias(*it);
			it = sounds.erase(it);
		}
	}
}

void AudioPlayer::Destroy()
{
	Stop();

	if (sound)
		Raylib::UnloadSoundAlias(*sound);
	else if (music)
		Raylib::UnloadMusicStream(*music);

	for (Raylib::Music& musicStream : musicStreams)
	{
		Raylib::StopMusicStream(musicStream);
		Raylib::UnloadMusicStream(musicStream);
	}

	for (Raylib::Sound& sound : sounds)
	{
		Raylib::StopSound(sound);
		Raylib::UnloadSoundAlias(sound);
	}

	if (this->audioClip != nullptr)
		delete this->audioClip;
}

void AudioPlayer::SetAudioClip(AudioClip audioClip)
{
	if (this->audioClip != nullptr)
	{
		if (sound)
		{
			Raylib::UnloadSoundAlias(*sound);
			sound = nullptr;
		}
		else if (music)
		{
			Raylib::UnloadMusicStream(*music);
			music = nullptr;
		}

		Stop();
		delete this->audioClip;
	}

	std::filesystem::path path;
#ifndef EDITOR
	if (exeParent.empty())
		path = "Resources/Assets/" + audioClip.GetPath();
	else
		path = std::filesystem::path(exeParent) / "Resources" / "Assets" / audioClip.GetPath();
#endif

	if (path.string().empty() || path.string() == "nullptr") // It may be "nullptr" if no audio clip was set in the properties
	{
		ConsoleLogger::WarningLog(gameObject->GetName() + ":Audio Player - SetAudioClip(AudioClip audioClip) failed to set audio clip. The selected audio clip has an invalid audio file path.", false);
		return;
	}

	this->audioClip = new AudioClip(audioClip.GetPath());

	if (this->audioClip->LoadedInMemory())
	{
		sound = new Raylib::Sound();
		*sound = Raylib::LoadSoundAlias(*this->audioClip->GetRaylibSound());
	}
	else
	{
		music = new Raylib::Music();
		*music = Raylib::LoadMusicStream(path.string().c_str());
		music->looping = loop;
	}
}

bool AudioPlayer::IsPlaying() const
{
	if (audioClip == nullptr)
		return false;

	if (audioClip->LoadedInMemory())
		return Raylib::IsSoundPlaying(*sound);
	else
		return Raylib::IsMusicStreamPlaying(*music);
}

void AudioPlayer::Play()
{
	if (audioClip == nullptr)
	{
		ConsoleLogger::WarningLog(gameObject->GetName() + ":Audio Player - Play() failed to play audio. No audio clip selected. Either use SetAudioClip() before Play(), set an audio clip in the properties, or Play(AudioClip audioClip).", false);
		return;
	}

	if (audioClip->LoadedInMemory())
	{
		Raylib::PlaySound(*sound);
		playingSound = true;
	}
	else
	{
		music->looping = loop;
		Raylib::PlayMusicStream(*music);
	}
	paused = false;
}

void AudioPlayer::Play(AudioClip audioClip)
{
	std::filesystem::path path;
#ifndef EDITOR
	if (exeParent.empty())
		path = "Resources/Assets/" + audioClip.GetPath();
	else
		path = std::filesystem::path(exeParent) / "Resources" / "Assets" / audioClip.GetPath();
#endif
	if (audioClip.GetPath().empty() || audioClip.GetPath() == "nullptr" || !std::filesystem::exists(path)) // It may be "nullptr" if no audio clip was set in the properties
	{
		ConsoleLogger::WarningLog(gameObject->GetName() + ":Audio Player - Play(AudioClip audioClip) failed to play audio. The selected audio clip has an invalid audio file path.", false);
		return;
	}
	// Todo: Ensure audio clip works and send errror if not (like check if the path exists)

	if (audioClip.LoadedInMemory())
	{
		sounds.push_back(Raylib::LoadSoundAlias(*audioClip.GetRaylibSound()));
		Raylib::PlaySound(sounds.back());
	}
	else
	{
		musicStreams.push_back(Raylib::LoadMusicStream((path).string().c_str()));
		Raylib::PlayMusicStream(musicStreams.back());
	}
}

void AudioPlayer::Stop()
{
	if (!IsPlaying())
		return;

	if (audioClip->LoadedInMemory())
	{
		Raylib::StopSound(*sound);
		playingSound = false;
	}
	else
		Raylib::StopMusicStream(*music);
}

void AudioPlayer::Pause()
{
	paused = true;
	if (audioClip == nullptr)
		return;

	if (audioClip->LoadedInMemory())
		Raylib::PauseSound(*sound);
	else
		Raylib::PauseMusicStream(*music);
}

void AudioPlayer::Unpause()
{
	paused = false;
	if (audioClip == nullptr)
		return;

	if (audioClip->LoadedInMemory())
		Raylib::ResumeSound(*sound);
	else
		Raylib::ResumeMusicStream(*music);
}

bool AudioPlayer::IsPaused() const
{
	return paused;
}

void AudioPlayer::SetLooping(bool loop)
{
	if (this->loop == loop)
		return;

	this->loop = loop;

	if (music)
		music->looping = loop;
}

bool AudioPlayer::IsLooping() const
{
	return loop;
}
