#pragma once
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H
#include "Player.h"

class AudioPlayer :public Player
{
public:
	AudioPlayer();
	~AudioPlayer();
private:
	SwrContext* swrCtx;

	//重采样设置选项-----------------------------------------------------------start

	//输入的采样格式
	enum AVSampleFormat in_sample_fmt;
	//输出的采样格式 16bit PCM
	enum AVSampleFormat out_sample_fmt;
	//输入的采样率
	int in_sample_rate;
	//输出的采样率
	int out_sample_rate;
	//输入的声道布局
	uint64_t in_ch_layout;
	//输出的声道布局
	uint64_t out_ch_layout;
	//重采样设置选项-----------------------------------------------------------end

	//SDL_AudioSpec
	SDL_AudioSpec wanted_spec;
	//获取输出的声道个数
	int out_channel_nb;

public:
	void audioSetting();
	int setAudioSDL();
	int play();

private:
	static void fill_audio(void* udata, Uint8* stream, int len);
};


#endif
