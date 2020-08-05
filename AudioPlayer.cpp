#include "AudioPlayer.h"

#define RATE 48000

AudioPlayer::AudioPlayer()
{

}


AudioPlayer::~AudioPlayer()
{
	SDL_CloseAudio();//�ر���Ƶ�豸 
	swr_free(&swrCtx);
}


void AudioPlayer::audioSetting()
{
	//�ز�������ѡ��-----------------------------------------------------------start
	//����Ĳ�����ʽ
	in_sample_fmt = pCodeCtx->sample_fmt;
	//����Ĳ�����ʽ 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//����Ĳ�����
	in_sample_rate = pCodeCtx->sample_rate;
	//����Ĳ�����
	out_sample_rate = RATE;
	//�������������
	in_ch_layout = pCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pCodeCtx->channels);
	}
	//�������������
	out_ch_layout = AV_CH_LAYOUT_MONO;

	//frame->16bit 44100 PCM ͳһ��Ƶ������ʽ�������
	swrCtx = swr_alloc();
	swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt,
		in_sample_rate, 0, NULL);
	swr_init(swrCtx);
	//�ز�������ѡ��-----------------------------------------------------------end
}

int AudioPlayer::setAudioSDL()
{
	//��ȡ�������������
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channel_nb;
	wanted_spec.silence = 0;
	wanted_spec.samples = pCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//�ص�����
	wanted_spec.userdata = pCodeCtx;
	if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
		printf("can't open audio.\n");
		return -1;
	}
}

static Uint8* audio_chunk;
//������Ƶ���ݳ���
static Uint32 audio_len;
static Uint8* audio_pos;

void  AudioPlayer::fill_audio(void* udata, Uint8* stream, int len) {
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		//�����ݲŲ���
		return;
	len = (len > audio_len ? audio_len : len);

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int AudioPlayer::play()
{
	//��������
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	//av_init_packet(packet);		//��ʼ��
	//��ѹ������
	AVFrame* frame = av_frame_alloc();
	//�洢pcm����
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * RATE);

	int ret, got_frame, framecount = 0;
	//һ֡һ֡��ȡѹ������Ƶ����AVPacket

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == index) {
			//����AVPacket->AVFrame
			ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);

			//ret = avcodec_send_packet(pCodeCtx, packet);
			//if (ret != 0) { printf("%s/n", "error"); }
			//got_frame = avcodec_receive_frame(pCodeCtx, frame);			//output=0��success, a frame was returned
			/*while ((got_frame = avcodec_receive_frame(pCodeCtx, frame)) == 0) {
					//��ȡ��һ֡��Ƶ������Ƶ
					//������������Ƶ frame
					swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
			}*/


			if (ret < 0) {
				printf("%s", "�������");
			}

			//��0�����ڽ���
			int out_buffer_size;
			if (got_frame) {
				//printf("����%d֡", framecount++);
				swr_convert(swrCtx, &out_buffer, 2 * 44100, (const uint8_t**)frame->data, frame->nb_samples);
				//��ȡsample��size
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples,
					out_sample_fmt, 1);
				//������Ƶ���ݻ���,PCM����
				audio_chunk = (Uint8*)out_buffer;
				//������Ƶ���ݳ���
				audio_len = out_buffer_size;
				audio_pos = audio_chunk;
				//�ط���Ƶ���� 
				SDL_PauseAudio(0);
				while (audio_len > 0)//�ȴ�ֱ����Ƶ���ݲ������! 
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
	SDL_CloseAudio();//�ر���Ƶ�豸 
	swr_free(&swrCtx);
	return 0;
}