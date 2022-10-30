/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2019 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once

#include "SoundDriver.h"
#include <aaudio/AAudio.h>
#include <android/log.h>

class AndroidSoundDriver :
	public SoundDriver
{
public:
	AndroidSoundDriver();
	~AndroidSoundDriver();

	// Setup and Teardown Functions
	Boolean Initialize();
	void DeInitialize();
	void Setup();
	void Teardown();

	// Buffer Functions for the Audio Code
	void SetFrequency(u32 Frequency);           // Sets the Nintendo64 Game Audio Frequency

	// Management functions
	void AiUpdate(Boolean Wait) {  };
	void StopAudio();							// Stops the Audio PlayBack (as if paused)
	void StartAudio();							// Starts the Audio PlayBack (as if unpaused)

	void SetVolume(u32 volume);

	static SoundDriverInterface* CreateSoundDriver();
	static bool ValidateDriver();
	static bool ClassRegistered;

	static aaudio_data_callback_result_t audioDataCallback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames);

protected:
	static AndroidSoundDriver* m_Instance;

	AAudioStream* m_Stream;
	u32 m_SampleRate;
};
