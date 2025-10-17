#pragma once

#include "Components/Component.h"
#include "Resources/AudioClip.h"
#include <vector>


class AudioPlayer : public Component
{
public:
    AudioPlayer(GameObject* obj, int id) : Component(obj, id)
	{
		name = "AudioPlayer";
		iconUnicode = "\xef\x80\xa8";

#if defined(EDITOR)
        std::string variables = R"(
        [
            0,
            [
                [
                    "AudioClip",
                    "audioClip",
                    "nullptr",
                    "Audio Clip",
                    {
                        "Extensions": [".mp3", ".wav", ".ogg", ".flac", ".qoa", ".xm", ".mod"]
                    }
                ],
                [
                    "bool",
                    "loop",
                    false,
                    "Loop"
                ],
                [
                    "bool",
                    "playOnStart",
                     false,
                    "Play On Start"
                ]
            ]
        ]
    )";
        exposedVariables = nlohmann::json::parse(variables);
#endif
	}

    void Awake() override;
	void Start() override;
	void Update() override;
    void Destroy() override;

    bool IsPlaying() const;
    void Play();
    void Play(AudioClip audioClip);
    void Stop();
    void Pause();
    void Unpause();
    bool IsPaused() const;
    void SetLooping(bool loop);
    bool IsLooping() const;

    // Get clip length -- Should this be in AudioClip? But then audioClip would need to be puclic or create a GetAudioClip() const. ALthough people should have their audioclip unless they dragged it into AudioPlayer.
    //   It should stay private since SetAudioClip() does important stuff
    // Get current time elapsed?

    void SetAudioClip(AudioClip audioClip);

private:
    Raylib::Music* music; // Used for streaming sounds
    Raylib::Sound* sound; // Used for streaming sounds
    std::vector<Raylib::Music> musicStreams; // Used for Play(audioClip)
    std::vector<Raylib::Sound> sounds; // Used for Play(audioClip)
    AudioClip* audioClip = nullptr;
    bool loop = false;
    bool paused = false;
    bool playOnStart = false;
    bool playingSound = false;
};