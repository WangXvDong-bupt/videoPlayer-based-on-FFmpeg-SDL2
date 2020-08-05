#pragma once
#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
#include "Player.h"

class VideoPlayer :public Player
{
public:
	VideoPlayer();
	~VideoPlayer();
private:
	struct SwsContext* img_convert_ctx;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Thread* video_tid;
	SDL_Event event;
	SDL_Window* screen;
public:
	void showInfo();
	int setWindow();
	virtual int play();
	static int sfp_refresh_thread(void* opaque);
};
#endif
