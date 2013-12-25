#include <windows.h>
#include <stdio.h>

#include "videoStream.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

//DEBUG: definir esta constante para reproduzir o video o mais rapidamente possível
//#define VIDEO_IGNORE_TIMING

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe VideoStream  =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/

//por omissão
bool VideoStream::avInitialized = false;

void VideoStream::processAVFrame()
{
	AVPacket packet;

	//se não há espaço na queue para o video, posso sair
	if (this->videoQueueActive >= this->videoQueueMax)
		return;

	//a frame pode consistir em vário pacotes
	while (av_read_frame(this->formatContext, &packet) >= 0)
	{
		//se este pacote é do stream de video encontrado previamente
		if (packet.stream_index == this->videoInfo.streamIndex)
		{
			int videoFrameTerminou;

			//descodifico a frame
			avcodec_decode_video2(this->videoInfo.codecContext, this->videoInfo.videoFrameOriginal, &videoFrameTerminou, &packet);

			//se a frame ficou pronta
			if (videoFrameTerminou != 0)
			{
				VideoFrame *frameAnterior, *frameAlvo;
				uint64_t videoFramePTS;

				//preciso saber se tenho alguma frame anterior
				frameAnterior = nullptr;
				if (this->videoQueueActive >= 1)
					frameAnterior = this->videoQueue + this->videoQueueActive - 1;

				//qual a frame para onde vou guardar as coisas
				frameAlvo = this->videoQueue + this->videoQueueActive;
				this->videoQueueActive++;

				//preciso de saber qual o PTS (presentation time stamp) a usar
				videoFramePTS = this->videoInfo.videoFrameOriginal->best_effort_timestamp;
				//videoFramePTS = this->videoInfo.videoFrameOriginal->pkt_pts;
				//videoFramePTS = this->videoInfo.videoFrameOriginal->pkt_dts;
				if (videoFramePTS == AV_NOPTS_VALUE)
					videoFramePTS = 0;

				//converto a frame
				sws_scale(this->videoInfo.pixelConvertContext, this->videoInfo.videoFrameOriginal->data, this->videoInfo.videoFrameOriginal->linesize, 0, this->videoInfo.codecContext->height, frameAlvo->avFrame->data, frameAlvo->avFrame->linesize);

				//preciso de saber o seu PTS (também em segundos)
				frameAlvo->framePTS = videoFramePTS;
				frameAlvo->frameStart = videoFramePTS * av_q2d(this->videoInfo.stream->time_base);

				//calculo a duração desta frame
				frameAlvo->frameDuration = av_q2d(this->videoInfo.codecContext->time_base);
				frameAlvo->frameDuration += this->videoInfo.videoFrameOriginal->repeat_pict * (frameAlvo->frameDuration * 0.5);

				//quando é que ela é suposto acabar
				frameAlvo->frameEnd = frameAlvo->frameStart + frameAlvo->frameDuration;

				//se tiver uma frame anterior
				if (frameAnterior != nullptr)
				{
					double newDuration;

					//crio um novo valor de duração da frame anterior
					newDuration = frameAlvo->frameStart - frameAnterior->frameStart;
					if (newDuration > frameAnterior->frameDuration)
					{
						//guardo a nova duração e ajusto quando é que a frame é suposto acabar
						frameAnterior->frameDuration = newDuration;
						frameAnterior->frameEnd = frameAnterior->frameStart + frameAnterior->frameDuration;
					}
				}
			}

			//liberto o pacote
			av_free_packet(&packet);

			//se a frame ficou pronta, saio, não leio mais nada
			if (videoFrameTerminou != 0)
				break;

			//passo para o próximo
			continue;
		}

		//se este pacote é do stream de audio encontrado previamente
		if ((this->audioInfo.codecContext != nullptr ) && (packet.stream_index == this->audioInfo.streamIndex))
		{
			//liberto o pacote e passo para o próximo
			av_free_packet(&packet);
			continue;
		}

		//chegando aqui não reconheço o pacote, portanto liberto-o e pronto
		av_free_packet(&packet);
	}
}

void VideoStream::recycleVideoFrameQueue()
{
	VideoFrame videoFrameTemp;

	//se não tenho nada
	if (this->videoQueueMax <= 0)
		return;

	//guardo a primeira frame
	memcpy(&videoFrameTemp, this->videoQueue + 0, sizeof(VideoFrame));

	//para cada frame na queue, copio a próxima para a anterior
	for (int i=0; i<(this->videoQueueMax - 1); i++)
		memcpy(this->videoQueue + i, this->videoQueue + i + 1, sizeof(VideoFrame));

	//a primeira frame (que estava no temp), vai para o fim
	memcpy(this->videoQueue + this->videoQueueMax - 1, &videoFrameTemp, sizeof(VideoFrame));
}

VideoStream::VideoFrame* VideoStream::getLatestFrame() const
{
	VideoFrame *frameFinal;

	//preciso ter alguma coisa
	if (this->videoQueueActive <= 0)
		return nullptr;

	//se só tenho um, é fácil
	if (this->videoQueueActive == 1)
		return (this->videoQueue + 0);

	//para cada frame na queue
	frameFinal = (this->videoQueue + 0);
	for(int i = 1; i < this->videoQueueActive; i++)
	{
		//se esta frame é mais avançada, guardo-a
		if (this->videoQueue[i].framePTS > frameFinal->framePTS)
			frameFinal = this->videoQueue + i;
	}

	//já tá
	return frameFinal;
}

VideoStream::VideoStream(const unsigned int maxFramesQueue, const PixelFormat frameTargetPixelFormat, const char * const videoFilePath)
{
	//limpo tudo
	this->formatContext = nullptr;
	this->timerInfo.timestampS = 0.0;
	memset(&this->videoInfo, 0, sizeof(VideoInfo));
	memset(&this->audioInfo, 0, sizeof(AudioInfo));
	this->videoInfo.streamIndex = -1;
	this->audioInfo.streamIndex = -1;

	//preciso de criar a lista para guardar as várias frames
	this->videoQueueMax = (maxFramesQueue < 2) ? 2 : maxFramesQueue;
	this->videoQueueActive = 0;
	this->videoQueue = new VideoFrame[this->videoQueueMax];

	//limpo tudo dentro da queue
	for (int i=0; i<this->videoQueueMax; i++)
		memset(this->videoQueue + i, 0, sizeof(VideoFrame));

	//tento abrir o ficheiro
	if (avformat_open_input(&this->formatContext, videoFilePath, nullptr, nullptr) != 0)
		return;

	//a próxima coisa e tentar tirar a informação do stream
	if (av_find_stream_info(this->formatContext) < 0)
		return;

	//passo por todos os streams
	for (int i = 0; i < this->formatContext->nb_streams; i++)
	{
		//se for de video e ainda não tenho um stream
		if ((this->videoInfo.codecContext == nullptr) && (this->formatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO))
		{
			//guardo o primeiro de video que encontrar
			this->videoInfo.stream = this->formatContext->streams[i];
			this->videoInfo.streamIndex = i;
			this->videoInfo.codecContext = this->videoInfo.stream->codec;
			continue;
		}

		//se for de audio e ainda não tenho um stream
		if ((this->audioInfo.codecContext == nullptr) && (this->formatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO))
		{
			//guardo o primeiro de video que encontrar
			this->audioInfo.streamIndex = i;
			this->audioInfo.codecContext = this->formatContext->streams[i]->codec;
			continue;
		}
	}

	//se não tenho nenhum stream para video
	if (this->videoInfo.codecContext == nullptr)
		return;

	//verifico se o codec de video é suportado
	this->videoInfo.codec = avcodec_find_decoder(this->videoInfo.codecContext->codec_id);
	if (this->videoInfo.codec == nullptr)
		return;

	//preciso de abrir o codec
	if (avcodec_open2(this->videoInfo.codecContext, this->videoInfo.codec, nullptr) < 0)
		return;

	//calculo algumas coisas acerca do video
	this->videoInfo.avgFrameRate = av_q2d(this->videoInfo.stream->avg_frame_rate);

	//preciso de criar um contexto para converter a frame (do formato orignal para o pretendido)
	this->videoInfo.pixelConvertContext = sws_getContext(this->videoInfo.codecContext->width, this->videoInfo.codecContext->height, this->videoInfo.codecContext->pix_fmt, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height, frameTargetPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

	//se tiver audio
	if (this->audioInfo.codecContext != nullptr)
	{
		//verifico se o codec de audio é suportado
		this->audioInfo.codec = avcodec_find_decoder(this->audioInfo.codecContext->codec_id);
		if (this->audioInfo.codec == nullptr)
			return;

		//preciso de abrir o codec
		if (avcodec_open2(this->audioInfo.codecContext, this->audioInfo.codec, nullptr) < 0)
			return;
	}

	//preciso da frame original
	this->videoInfo.videoFrameOriginal = avcodec_alloc_frame();
	if (this->videoInfo.videoFrameOriginal == nullptr)
		return;

	//isto dá jeito
	this->videoInfo.frameBufferSize = avpicture_get_size(frameTargetPixelFormat, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height);

	//para cada frame na queue
	for (int i = 0; i < this->videoQueueMax; i++)
	{
		//crio a frame
		this->videoQueue[i].avFrame = avcodec_alloc_frame();
		if (this->videoQueue[i].avFrame == nullptr)
		{
			//não posso ter esta frame
			this->videoQueueMax--;
			i--;
			continue;
		}

		//ter uma frame não significa ter espaço para guardar os seus dados, por isso crio espaço para ela
		this->videoQueue[i].frameData = (uint8_t *)av_malloc(this->videoInfo.frameBufferSize * sizeof(uint8_t));
		if (this->videoQueue[i].frameData == nullptr)
		{
			//liberto a frame
			av_free(this->videoQueue[i].avFrame);
			this->videoQueue[i].avFrame = nullptr;

			//não posso ter esta frame
			this->videoQueueMax--;
			i--;
			continue;
		}

		//e agora associo o espaço allocado à frame
		avpicture_fill((AVPicture *)this->videoQueue[i].avFrame, this->videoQueue[i].frameData, frameTargetPixelFormat, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height);
	}
}

VideoStream::~VideoStream()
{
	//para cada frame da queue
	for(int i = 0; i < this->videoQueueMax; i++)
	{
		//liberto o buffer e a frame associada
		av_free(this->videoQueue[i].frameData);
		av_free(this->videoQueue[i].avFrame);
	}

	//liberto a lista propriamente dita
	delete[] this->videoQueue;	

	//liberto o contexto que converte as frames
	if (this->videoInfo.pixelConvertContext != nullptr)
		sws_freeContext(this->videoInfo.pixelConvertContext);

	//liberto a frame original
	if (this->videoInfo.videoFrameOriginal != nullptr)
		av_free(this->videoInfo.videoFrameOriginal);

	//fecho os codecs
	if (this->audioInfo.codecContext != nullptr)
		avcodec_close(this->audioInfo.codecContext);
	if (this->videoInfo.codecContext != nullptr)
		avcodec_close(this->videoInfo.codecContext);

	//fecho o ficheiro
	if (this->formatContext != nullptr)
		av_close_input_file(this->formatContext);

	//e para terminar, limpo tudo
	this->formatContext = nullptr;
	this->timerInfo.timestampS = 0.0;
	memset(&this->videoInfo, 0, sizeof(VideoInfo));
	memset(&this->audioInfo, 0, sizeof(AudioInfo));
	this->videoInfo.streamIndex = -1;
	this->audioInfo.streamIndex = -1;
	this->videoQueue = nullptr;
	this->videoQueueActive = 0;
	this->videoQueueMax = 0;
}

void VideoStream::Initialize()
{
	//se já foi inicializado
	if (VideoStream::avInitialized == true)
		return;

	//registo todos os CODECs
	av_register_all();

	//já iniciei
	VideoStream::avInitialized = true;
}

void VideoStream::Process()
{
	//mando precessar frames (o que for lido vai para a queue)
	processAVFrame();
}

bool VideoStream::GoToBeginning()
{
	int streamIndex;
	int64_t seekTarget;

	//preciso de descobrir qual o stream a usar
	streamIndex = -1;
	if (this->videoInfo.streamIndex >= 0)
		streamIndex = this->videoInfo.streamIndex;
	else if (this->audioInfo.streamIndex >= 0)
		streamIndex = this->audioInfo.streamIndex;

	//ajusto o seek
	seekTarget = 0;
	if (streamIndex >= 0)
	{
		//como a linha comentada não compila (só compila em C), é preciso dar a volta em C++
		//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, this->formatContext->streams[streamIndex]->time_base);

		AVRational aux = {1, AV_TIME_BASE};
		seekTarget = av_rescale_q(0, aux, this->formatContext->streams[streamIndex]->time_base);
	}

	//ordeno a reposição propriamente dita
	if (av_seek_frame(this->formatContext, streamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
		return false;

	//faço flush dos codecs
	avcodec_flush_buffers(this->videoInfo.codecContext);
	if (this->audioInfo.codecContext != nullptr)
		avcodec_flush_buffers(this->audioInfo.codecContext);

	//limpo as frames que tenho na queue
	this->videoQueueActive = 0;

	//preciso da primeira frame
	processAVFrame();

	//estando na primeira frame, basta reiniciar o relógio
	this->timerInfo.timer.ReStart();
	this->timerInfo.timestampS = 0.0;

	//correu tudo bem
	return true;
}

bool VideoStream::GoToTime(const double seconds)
{
	int streamIndex;
	int64_t seekTarget;
	VideoFrame *ultimaFrame;

	//preciso de descobrir qual o stream a usar
	streamIndex = -1;
	if (this->videoInfo.streamIndex >= 0)
		streamIndex = this->videoInfo.streamIndex;
	else if (this->audioInfo.streamIndex >= 0)
		streamIndex = this->audioInfo.streamIndex;

	//ajusto o seek
	seekTarget = (seconds < 0) ? 0 : seconds;
	seekTarget *= AV_TIME_BASE;
	if (streamIndex >= 0)
	{
		//como a linha comentada não compila (só em puro C), é preciso dar a volta em C++
		//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, this->formatContext->streams[streamIndex]->time_base);
		
		AVRational aux = {1, AV_TIME_BASE};
		seekTarget = av_rescale_q(seekTarget, aux, this->formatContext->streams[streamIndex]->time_base);
	}

	//a última frame lida
	ultimaFrame = getLatestFrame();

	//ordeno a reposição propriamente dita
	if (av_seek_frame(this->formatContext, streamIndex, seekTarget, ((ultimaFrame != nullptr) && (ultimaFrame->framePTS > seekTarget)) ? AVSEEK_FLAG_BACKWARD : 0) < 0)
		return false;

	//faço flush dos codecs
	avcodec_flush_buffers(this->videoInfo.codecContext);
	if (this->audioInfo.codecContext != nullptr)
		avcodec_flush_buffers(this->audioInfo.codecContext);

	//limpo as frames que tenho na queue
	this->videoQueueActive = 0;

	//preciso da primeira frame
	processAVFrame();

	//tenho de ajustar o relógio interno
	if (this->videoQueueActive > 0)
		this->timerInfo.timer.SetS(this->videoQueue[0].frameStart);
	else
		this->timerInfo.timer.SetS(seconds);
	this->timerInfo.timestampS = this->timerInfo.timer.GetTimeS();

	//correu tudo bem
	return true;
}

const void* VideoStream::GetFrame(bool &clockIsBehind, HorseRadish::hInt64 &frameID, double &frameDurationS)
{
	//por omissão
	clockIsBehind = false;
	frameID = -1;
	frameDurationS = 0.0;

	//leio o tempo actual, que vou usar em todo o processamento
	this->timerInfo.timestampS = this->timerInfo.timer.GetTimeS();

	//enquanto tiver frames para usar
	while(true)
	{
		//se não tenho nada com que trabalhar
		if (this->videoQueueActive <= 0)
		{
			//mando precessar frames (se tiver alguma coisa vai para a queue)
			processAVFrame();

			//se mesmo assim ainda não tenho nada (é porque devo ter chegado ao fim do ficheiro)
			if (this->videoQueueActive <= 0)
				return nullptr;
		}

#ifndef VIDEO_IGNORE_TIMING
		//se o relógio ainda não chegou à primeira frame
		if (this->timerInfo.timestampS < this->videoQueue[0].frameStart)
		{
			clockIsBehind = true;
			return nullptr;
		}

		//se ainda estou na janela da primeira frame, posso já sair
		if (this->timerInfo.timestampS < this->videoQueue[0].frameEnd)
		{
			VideoFrame *frameAlvo;

			//isto dá jeito
			frameAlvo = this->videoQueue + 0;

			//o ID da frame (o seu PTS)
			frameID = frameAlvo->framePTS;

			//e calculo a sua duração
			frameDurationS = frameAlvo->frameEnd - this->timerInfo.timer.GetTimeS();

			//posso devolver a frame
			return frameAlvo->frameData;
		}

		//como já não preciso da primeira, posso recicla-la
		recycleVideoFrameQueue();
		this->videoQueueActive--;
#else
		void* frameData;
		HorseRadish::hUInt64 timerValue;

		//nesta situação, o frameID é sempre diferente e a duração é 0
		this->timerInfo.timer.GetTimeIntMS(timerValue);
		frameID = timerValue;
		frameDurationS = 0.0;

		//os dados da frame
		frameData = this->videoQueue[0].frameData;

		//só mostro uma vez, portanto, removo já a frame
		recycleVideoFrameQueue();
		this->videoQueueActive--;

		//posso devolver a frame
		return frameData;
#endif
	}
}

double VideoStream::GetFrameDuration(const HorseRadish::hInt64 &frameID)
{
	VideoFrame *frameAlvo;

	//se não tenho nada
	if (this->videoQueueActive <= 0)
		return 0.0;

	//isto dá jeito
	frameAlvo = this->videoQueue + 0;

	//tenho de usar a mesma frame
	if (frameAlvo->framePTS != frameID)
		return 0.0;

	//e calculo a sua duração
	return (frameAlvo->frameEnd - this->timerInfo.timer.GetTimeS());
}

bool VideoStream::IsValid() const
{
	//basta verificar isto
	return ((this->videoInfo.videoFrameOriginal != nullptr) && (this->videoQueueMax > 0));
}

bool VideoStream::HasAudio() const
{
	//basta verificar se tenho um codec associado
	return (this->audioInfo.codec != nullptr);
}

double VideoStream::GetTimeStampDelta()
{
	//basta fazer a diferença de tempo
	return (this->timerInfo.timer.GetTimeS() - this->timerInfo.timestampS);
}

int VideoStream::GetVideoFrameDataSize() const
{
	//basta devolver isto
	return this->videoInfo.frameBufferSize;
}

double VideoStream::GetVideoFrameAspectRatio() const
{
	//há vários sitios onde pode estar guardado
	if ((this->videoInfo.stream != nullptr) && (this->videoInfo.stream->sample_aspect_ratio.num != 0))
		return av_q2d(this->videoInfo.stream->sample_aspect_ratio);
	if ((this->videoInfo.codecContext != nullptr) && (this->videoInfo.codecContext->sample_aspect_ratio.num != 0))
		return av_q2d(this->videoInfo.codecContext->sample_aspect_ratio);

	//oops
	return -1.0;
}

void VideoStream::GetVideoDims(int &videoWidth, int &videoHeight) const
{
	//por omissão
	videoWidth = videoHeight = 0;

	//se tiver alguma coisa
	if (this->videoInfo.codecContext != nullptr)
	{
		videoWidth = this->videoInfo.codecContext->width;
		videoHeight = this->videoInfo.codecContext->height;
	}
}

void VideoStream::GetVideoRect(const int winWidth, const int winHeight, const bool maintainAspectRatio, HorseRadish::Primitives2D::Rectangle<int> &rect) const
{
	float videoScale, videoAspectRatio;
	int videoWidth, videoHeight;

	//por omissão
	rect.Reset();

	//se não posso fazer nada
	if (this->videoInfo.codecContext == nullptr)
		return;

	//isto dá jeito
	this->GetVideoDims(videoWidth, videoHeight);

	//se não for para manter o aspecto
	if (maintainAspectRatio == false)
	{
		//os valores para desenhar o quadrado do video
		rect.x = 0;
		rect.y = 0;
		rect.width = winWidth;
		rect.height = winHeight;
		return;
	}

	//as escalas a usar (ajustada pela indicada no video)
	videoScale = ((float)videoWidth) / ((float)videoHeight);
	videoAspectRatio = this->GetVideoFrameAspectRatio();
	if (videoAspectRatio > 0)
		videoScale *= videoAspectRatio;

	//tenho de verificar se eventualmente o video não fica maior do que a área disponível
	if ((winWidth / videoScale) > winHeight)
	{
		float videoNewWidth;

		//tenho de calcular o tamanho do video pela altura
		videoNewWidth = ((float)winHeight) * videoScale;

		//os valores para desenhar o quadrado do video
		rect.x = HorseRadish::Math::ftoi((((float)winWidth) - videoNewWidth) * 0.5f);
		rect.y = 0;
		rect.width = HorseRadish::Math::ftoi(videoNewWidth);
		rect.height = winHeight;
	}
	else
	{
		float videoNewHeight;

		//tenho de calcular o tamanho do video pelo comprimento
		videoNewHeight = ((float)winWidth) / videoScale;

		//os valores para desenhar o quadrado do video
		rect.x = 0;
		rect.y = HorseRadish::Math::ftoi((((float)winHeight) - videoNewHeight) * 0.5f);
		rect.width = winWidth;
		rect.height = HorseRadish::Math::ftoi(videoNewHeight);
	}
}

void VideoStream::GetVideoRectCenter(const float scale, const int winWidth, const int winHeight, HorseRadish::Primitives2D::Rectangle<int> &rect) const
{
	float videoScale, videoAspectRatio;
	int videoWidth, videoHeight;

	//por omissão
	rect.Reset();

	//se não posso fazer nada
	if (this->videoInfo.codecContext == nullptr)
		return;

	//isto dá jeito
	this->GetVideoDims(videoWidth, videoHeight);

	//as escalas a usar (ajustada pela indicada no video)
	videoScale = ((float)videoWidth) / ((float)videoHeight);
	videoAspectRatio = this->GetVideoFrameAspectRatio();
	if (videoAspectRatio > 0)
		videoScale *= videoAspectRatio;

	//o novo tamanho do video
	videoWidth = HorseRadish::Math::ftoi(((float)videoWidth) * scale);
	videoHeight = HorseRadish::Math::ftoi(((float)videoWidth) / videoScale);

	//os valores para desenhar o quadrado do video
	rect.x = HorseRadish::Math::ftoi(((float)(winWidth - videoWidth)) * 0.5f);
	rect.y = HorseRadish::Math::ftoi(((float)(winHeight - videoHeight)) * 0.5f);;
	rect.width = videoWidth;
	rect.height = videoHeight;
}