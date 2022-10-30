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

#include "common.h"
#if defined(ENABLE_BACKEND_ANDROID)
#include "AndroidSoundDriver.h"
#include "AudioSpec.h"

#include "SoundDriverFactory.h"

#ifndef NDEBUG
#define LOG_TAG "AziAudio"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define LOGV(...)
#else
#define LOGE(...)
#define LOGI(...)
#define LOGV(...)
#endif

#define min(a,b) (((a)<(b))?(a):(b))

constexpr u32 channelCount = 2;
constexpr u32 frameSize = sizeof(int16_t) * channelCount;

AndroidSoundDriver* AndroidSoundDriver::m_Instance;

bool AndroidSoundDriver::ClassRegistered = ValidateDriver() ?
		SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_ANDROID, AndroidSoundDriver::CreateSoundDriver, "Android Driver", 1) :
		false;

aaudio_data_callback_result_t AndroidSoundDriver::audioDataCallback(
	AAudioStream* stream,
	void* userData,
	void* audioData,
	int32_t numFrames) 
{
	AndroidSoundDriver* pSoundDriver = (AndroidSoundDriver*)userData;

	// Write samples directly into the audioData array.
	u32 requestedBytes = numFrames * frameSize;
	u32 readBytes = min(requestedBytes, pSoundDriver->m_MaxBufferSize);
	pSoundDriver->LoadAiBuffer((u8*)audioData, readBytes);

	// Fill with silence, this shouldn't happen... will cause popping if it does.
	if (requestedBytes > readBytes)
	{
		LOGI("Buffer could not be filled! Free bytes: %u", requestedBytes - readBytes);
		memset((u8*)audioData + readBytes, 0, requestedBytes - readBytes);
	}

	return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/*
	Will verify the driver can run in the configured environment
*/
bool AndroidSoundDriver::ValidateDriver()
{
	return true;
}

AndroidSoundDriver::AndroidSoundDriver()
	: m_Stream(nullptr)
	, m_SampleRate(0)
{
	m_Instance = this;
}

AndroidSoundDriver::~AndroidSoundDriver()
{
	DEBUG_OUTPUT("WO Deconstructor");
	Teardown();
	DEBUG_OUTPUT("WO Deconstructor done");
}

void AndroidSoundDriver::Teardown()
{
	m_Mutex.Lock();
	if (m_Stream)
	{
		AAudioStream_close(m_Stream);
		m_Stream = nullptr;
	}
	m_Mutex.Unlock();
}

void AndroidSoundDriver::Setup()
{
	AAudioStreamBuilder* builder;
	aaudio_result_t result = AAudio_createStreamBuilder(&builder);

	if (result == AAUDIO_OK)
	{
		AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
		AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
		AAudioStreamBuilder_setSampleRate(builder, m_SampleRate);
		AAudioStreamBuilder_setChannelCount(builder, channelCount);
		AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
		AAudioStreamBuilder_setBufferCapacityInFrames(builder, m_MaxBufferSize / frameSize);

		AAudioStreamBuilder_setDataCallback(builder, audioDataCallback, this);

		result = AAudioStreamBuilder_openStream(builder, &m_Stream);

		if (result == AAUDIO_OK)
		{
			StartAudio();
		}
		else
		{
			LOGI("Open stream error: %s", AAudio_convertResultToText(result));
		}

		AAudioStreamBuilder_delete(builder);
	}
	else
	{
		LOGI("Create stream builder error: %s", AAudio_convertResultToText(result));
	}
}

Boolean AndroidSoundDriver::Initialize()
{
	DEBUG_OUTPUT("WO: Initialize()\n");
	Teardown();
	m_SampleRate = 0;

	return true;
}

void AndroidSoundDriver::DeInitialize()
{
	DEBUG_OUTPUT("WO: DeInitialize()\n");
	Teardown();
	DEBUG_OUTPUT("WO: DeInitialize() done\n");
}

void AndroidSoundDriver::SetFrequency(u32 Frequency)
{
	DEBUG_OUTPUT("WO: SetFrequency()\n");
	if (m_SampleRate != Frequency)
	{
		Teardown();
		m_SampleRate = Frequency;
		Setup();
	}
}

void AndroidSoundDriver::StopAudio()
{
	DEBUG_OUTPUT("WO: StopAudio()\n");
	if (m_Stream)
	{
		aaudio_result_t result;
		result = AAudioStream_requestPause(m_Stream);

		if (result != AAUDIO_OK)
		{
			LOGI("StopAudio error: %s", AAudio_convertResultToText(result));
		}
	}
	DEBUG_OUTPUT("WO: StopAudio() done\n");
}

void AndroidSoundDriver::StartAudio()
{
	DEBUG_OUTPUT("WO: StartAudio()\n");
	if (m_Stream)
	{
		aaudio_result_t result;
		result = AAudioStream_requestStart(m_Stream);

		if (result != AAUDIO_OK)
		{
			LOGI("StopAudio error: %s", AAudio_convertResultToText(result));
		}
	}
	DEBUG_OUTPUT("WO: StartAudio() done\n");
}

void AndroidSoundDriver::SetVolume(u32 volume)
{
	return;
}

SoundDriverInterface* AndroidSoundDriver::CreateSoundDriver()
{
	return new AndroidSoundDriver();
}

#endif
