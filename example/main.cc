#include <iostream>
#include <thread>
#include <chrono>
#include <camera/camera.h>
#include <camera/photography_settings.h>
#include <camera/device_discovery.h>
#include <regex>
#include <stream/stream_delegate.h>
#include <opencv2/opencv.hpp>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

// Estrutura para armazenar recursos de decodificação de vídeo
struct VideoDecoder {
    const AVCodec* codec;       // Codec de vídeo
    AVCodecContext* codec_ctx;  // Contexto do codec
    AVFrame* frame;             // Frame de vídeo
    AVPacket* avpkt;            // Pacote AV
    SwsContext* sws_ctx;        // Contexto de conversão de escala
};

// Função para inicializar um decodificador de vídeo
void initialize_decoder(VideoDecoder& decoder) {
    // Encontra o codec de vídeo H.264
    decoder.codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!decoder.codec) {
        std::cerr << "Codec not found" << std::endl;
        exit(1);
    }

    // Aloca o contexto do codec
    decoder.codec_ctx = avcodec_alloc_context3(decoder.codec);
    if (!decoder.codec_ctx) {
        std::cerr << "Could not allocate video codec context" << std::endl;
        exit(1);
    }

    // Abre o codec
    if (avcodec_open2(decoder.codec_ctx, decoder.codec, NULL) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        exit(1);
    }

    // Aloca o frame de vídeo
    decoder.frame = av_frame_alloc();
    if (!decoder.frame) {
        std::cerr << "Could not allocate video frame" << std::endl;
        exit(1);
    }

    // Aloca o pacote AV
    decoder.avpkt = av_packet_alloc();
    if (!decoder.avpkt) {
        std::cerr << "Could not allocate AVPacket" << std::endl;
        exit(1);
    }

    // Inicializa o contexto de conversão de escala como nulo
    decoder.sws_ctx = nullptr;
}

// Função para liberar os recursos de um decodificador de vídeo
void free_decoder(VideoDecoder& decoder) {
    avcodec_free_context(&decoder.codec_ctx);
    av_frame_free(&decoder.frame);
    av_packet_free(&decoder.avpkt);
    if (decoder.sws_ctx) {
        sws_freeContext(decoder.sws_ctx);
    }
}

// Classe para gerenciar a captura de streams da câmera
class TestStreamDelegate : public ins_camera::StreamDelegate {
public:
    TestStreamDelegate() {
        // Inicializa os decodificadores de vídeo
        initialize_decoder(decoder0);
        initialize_decoder(decoder1);
    }

    ~TestStreamDelegate() {
        // Libera os decodificadores de vídeo
        free_decoder(decoder0);
        free_decoder(decoder1);
    }

    // Método chamado ao receber dados de áudio (não usado aqui)
    void OnAudioData(const uint8_t* data, size_t size, int64_t timestamp) override {}

    // Método chamado ao receber dados de vídeo
    void OnVideoData(const uint8_t* data, size_t size, int64_t timestamp, uint8_t streamType, int stream_index = 0) override {
        if (stream_index == 0) {
            process_video_data(data, size, "Video0", decoder0);
        } else if (stream_index == 1) {
            process_video_data(data, size, "Video1", decoder1);
        }
    }

    // Método para processar os dados de vídeo e exibir o frame usando OpenCV
    void process_video_data(const uint8_t* data, size_t size, const std::string& window_name, VideoDecoder& decoder) {
        // Desreferencia o pacote AV
        av_packet_unref(decoder.avpkt);
        decoder.avpkt->data = (uint8_t*)data;
        decoder.avpkt->size = size;

        // Envia o pacote para decodificação
        int ret = avcodec_send_packet(decoder.codec_ctx, decoder.avpkt);
        if (ret < 0) {
            std::cerr << "Error sending a packet for decoding" << std::endl;
            return;
        }

        // Recebe o frame decodificado
        while (ret >= 0) {
            ret = avcodec_receive_frame(decoder.codec_ctx, decoder.frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                return;
            } else if (ret < 0) {
                std::cerr << "Error during decoding" << std::endl;
                return;
            }

            // Exibe o frame decodificado
            display_frame(decoder.frame, window_name, decoder.sws_ctx);
        }
    }

    // Método para exibir o frame decodificado usando OpenCV
    void display_frame(AVFrame* frame, const std::string& window_name, SwsContext*& sws_ctx) {
        // Inicializa o contexto de conversão de escala se for nulo
        if (!sws_ctx) {
            sws_ctx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                     frame->width, frame->height, AV_PIX_FMT_BGR24,
                                     SWS_BILINEAR, NULL, NULL, NULL);
        }

        // Converte o frame para formato BGR
        uint8_t* bgr_data[1];
        int bgr_linesize[1];
        int bgr_stride = frame->width * 3;
        bgr_data[0] = (uint8_t*)malloc(bgr_stride * frame->height);
        bgr_linesize[0] = bgr_stride;

        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, bgr_data, bgr_linesize);

        // Cria uma imagem OpenCV a partir dos dados convertidos
        cv::Mat img(frame->height, frame->width, CV_8UC3, bgr_data[0]);
        cv::resize(img, img, cv::Size(640, 360)); // Redimensiona a imagem para 640x360
        cv::imshow(window_name, img);             // Exibe a imagem na janela especificada
        cv::waitKey(1);                           // Aguarda por 1ms para permitir a atualização da janela

        // Libera a memória alocada para os dados BGR
        free(bgr_data[0]);
    }

    // Método chamado ao receber dados de giroscópio (não usado aqui)
    void OnGyroData(const std::vector<ins_camera::GyroData>& data) override {}

    // Método chamado ao receber dados de exposição (não usado aqui)
    void OnExposureData(const ins_camera::ExposureData& data) override {}

private:
    VideoDecoder decoder0;  // Decodificador para a primeira stream de vídeo
    VideoDecoder decoder1;  // Decodificador para a segunda stream de vídeo
};

int main(int argc, char* argv[]) {
    ins_camera::DeviceDiscovery discovery;

    // Descobre dispositivos de câmera disponíveis
    auto list = discovery.GetAvailableDevices();
    for (int i = 0; i < list.size(); ++i) {
        auto desc = list[i];
        std::cout << "Serial:" << desc.serial_number << "\t"
                  << "Tipo da câmera:" << int(desc.camera_type) << "\t"
                  << "Tipo da lente:" << int(desc.lens_type) << std::endl;
    }

    // Verifica se há dispositivos disponíveis
    if (list.size() <= 0) {
        std::cerr << "Nenhum dispositivo encontrado" << std::endl;
        return -1;
    }

    // Cria e abre uma conexão com a câmera
    std::shared_ptr<ins_camera::Camera> cam = std::make_shared<ins_camera::Camera>(list[0].info);
    if (!cam->Open()) {
        std::cerr << "Falha ao abrir a câmera" << std::endl;
        return -1;
    }

    // Imprime a URL HTTP base da câmera
    std::cout << "HTTP URL BASE:" << cam->GetHttpBaseUrl() << std::endl;

    // Cria e define o delegate para captura de streams
    std::shared_ptr<ins_camera::StreamDelegate> delegate = std::make_shared<TestStreamDelegate>();
    cam->SetStreamDelegate(delegate);

    // Libera os descritores de dispositivos
    discovery.FreeDeviceDescriptors(list);

    std::cout << "Sucesso ao conectar a câmera" << std::endl;

    // Configura os parâmetros para live streaming
    ins_camera::LiveStreamParam param;
    param.video_resolution = ins_camera::VideoResolution::RES_720_360P30;
    param.lrv_video_resulution = ins_camera::VideoResolution::RES_720_360P30;
    param.video_bitrate = 1024 * 1024 / 2;
    param.enable_audio = false;
    param.using_lrv = false;

    // Inicia o live streaming
    if (cam->StartLiveStreaming(param)) {
        std::cout << "Iniciou a live stream com sucesso" << std::endl;
    }

    auto start_time = std::chrono::steady_clock::now();

    // Loop principal para manter o streaming por 50 segundos
    while (true) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

        // Para o streaming após 50 segundos
        if (elapsed_time >= 50) {
            if (cam->StopLiveStreaming()) {
                std::cout << "Sucesso ao finalizar live stream!" << std::endl;
            } else {
                std::cerr << "Falha ao finalizar live stream." << std::endl;
            }
            break;
        }
    }

    // Fecha a conexão com a câmera e destrói as janelas do OpenCV
    cam->Close();
    cv::destroyAllWindows();
    return 0;
}
