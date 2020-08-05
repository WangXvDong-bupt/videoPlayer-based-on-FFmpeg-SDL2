#include "AudioPlayer.h"

#define RATE 48000

AudioPlayer::AudioPlayer()
{

}


AudioPlayer::~AudioPlayer()
{
	SDL_CloseAudio();//关闭音频设备 
	swr_free(&swrCtx);
}


void AudioPlayer::audioSetting()
{
	//重采样设置选项-----------------------------------------------------------start
	//输入的采样格式
	in_sample_fmt = pCodeCtx->sample_fmt;
	//输出的采样格式 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//输入的采样率
	in_sample_rate = pCodeCtx->sample_rate;
	//输出的采样率
	out_sample_rate = RATE;
	//输入的声道布局
	in_ch_layout = pCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pCodeCtx->channels);
	}
	//输出的声道布局
	out_ch_layout = AV_CH_LAYOUT_MONO;

	//frame->16bit 44100 PCM 统一音频采样格式与采样率
	swrCtx = swr_alloc();
	swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt,
		in_sample_rate, 0, NULL);
	swr_init(swrCtx);
	//重采样设置选项-----------------------------------------------------------end
}

int AudioPlayer::setAudioSDL()
{
	//获取输出的声道个数
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channel_nb;
	wanted_spec.silence = 0;
	wanted_spec.samples = pCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//回调函数
	wanted_spec.userdata = pCodeCtx;
	if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
		printf("can't open audio.\n");
		return -1;
	}
}

static Uint8* audio_chunk;
//设置音频数据长度
static Uint32 audio_len;
static Uint8* audio_pos;

void  AudioPlayer::fill_audio(void* udata, Uint8* stream, int len) {
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		//有数据才播放
		return;
	len = (len > audio_len ? audio_len : len);

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int AudioPlayer::play()
{
	//编码数据
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	//av_init_packet(packet);		//初始化
	//解压缩数据
	AVFrame* frame = av_frame_alloc();
	//存储pcm数据
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * RATE);

	int ret, got_frame, framecount = 0;
	//一帧一帧读取压缩的音频数据AVPacket

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == index) {
			//解码AVPacket->AVFrame
			ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);

			//ret = avcodec_send_packet(pCodeCtx, packet);
			//if (ret != 0) { printf("%s/n", "error"); }
			//got_frame = avcodec_receive_frame(pCodeCtx, frame);			//output=0》success, a frame was returned
			/*while ((got_frame = avcodec_receive_frame(pCodeCtx, frame)) == 0) {
					//读取到一帧音频或者视频
					//处理解码后音视频 frame
					swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
			}*/


			if (ret < 0) {
				printf("%s", "解码完成");
			}

			//非0，正在解码
			int out_buffer_size;
			if (got_frame) {
				//printf("解码%d帧", framecount++);
				swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
				//获取sample的size
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples,
					out_sample_fmt, 1);
				//设置音频数据缓冲,PCM数据
				audio_chunk = (Uint8*)out_buffer;
				//设置音频数据长度
				audio_len = out_buffer_size;
				audio_pos = audio_chunk;
				//回放音频数据 
				SDL_PauseAudio(0);
				while (audio_len > 0)//等待直到音频数据播放完毕! 
					SDL_Delay(10);
				packet->data += ret;
				packet->size -= ret;
			}
		}
		//av_packet_unref(packet);
	}

	av_free(out_buffer);
	av_frame_free(&frame);
	//av_free_packet(packet);
	av_packet_unref(packet);
	SDL_CloseAudio();//关闭音频设备 
	swr_free(&swrCtx);
	return 0;
}