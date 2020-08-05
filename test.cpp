// FFmpeg_Player.cpp: 定义应用程序的入口点。
//

#include "test.h"
#include "VideoPlayer.h"
#include "AudioPlayer.h"
#include<string>
using namespace std;

char filepath[100] = "";

int audioThread(void* opaque) {
	
	AudioPlayer audio;
	audio.openFile(filepath, AVMEDIA_TYPE_AUDIO);
	audio.audioSetting();
	audio.setAudioSDL();
	audio.play();
	audio.Player_Quit();
	return 0;
}

int videoThread(void* opaque) {
	VideoPlayer video;
	video.openFile(filepath, AVMEDIA_TYPE_VIDEO);
	video.showInfo();
	video.setWindow();
	video.play();
	video.Player_Quit();
	return 0;
}


int main(int argc, char* argv[])
{
	cout << "please input url:";
	cin >> filepath;
	
	avformat_network_init();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
	}
	SDL_Thread* video_tid = SDL_CreateThread(videoThread, NULL, NULL);
	SDL_Thread* audio_tid = SDL_CreateThread(audioThread, NULL, NULL);
	while (true)
	{
	}
	return 0;
	return 0;
}
