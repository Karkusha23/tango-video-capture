#include "VideoEncoderThread.h"

namespace vc
{
    VideoEncoderThread::VideoEncoderThread(const std::string& playlist_path, const std::string& playlist_url, int cam_width, int cam_height, int framerate) :
        MyThread(500 / framerate),
        output_format_(av_guess_format("hls", NULL, NULL)), codec_(avcodec_find_encoder(AV_CODEC_ID_H264)), options_(NULL), format_context_(nullptr), framerate_(framerate),
        frame_count_(0), playlist_path_(playlist_path), playlist_url_(playlist_url), wrote_first_frame_(false), next_key_frame_pts_(2000)
	{
        std::cout << "Initialising FFMPEG video encoder" << std::endl;

        avformat_alloc_output_context2(&format_context_, output_format_, NULL, playlist_path_.c_str());
        av_dict_set(&options_, "hls_time", "2", 0); // Set segment duration to 10 seconds
        av_dict_set(&options_, "hls_init_time", "2", 0);
        av_dict_set(&options_, "hls_base_url", playlist_url_.c_str(), 0);
        av_dict_set(&options_, "hls_list_size", "3", 0);
        av_dict_set(&options_, "hls_flags", "delete_segments", 0);
        av_dict_set(&options_, "segment_format", "mpegts", 0);
        av_dict_set(&options_, "segment_list_type", "m3u8", 0);
        av_dict_set(&options_, "segment_list", playlist_path_.c_str(), 0);
        av_dict_set_int(&options_, "segment_list_size", 0, 0);
        av_dict_set(&options_, "segment_time_delta", "1", 0);
        av_dict_set(&options_, "segment_time", "2", 0);
        av_dict_set(&options_, "segment_list_flags", "cache+live", 0);
        av_dict_set(&options_, "force_key_frames", "expr:gte(t,n_forced*2)", 0);
        stream_ = avformat_new_stream(format_context_, NULL);
        codec_context_ = avcodec_alloc_context3(codec_);
        codec_context_->width = cam_width;
        codec_context_->height = cam_height;
        codec_context_->time_base = av_make_q(1, framerate_);
        codec_context_->framerate = av_make_q(framerate_, 1);
        codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
        codec_context_->codec_id = AV_CODEC_ID_H264;
        codec_context_->codec_type = AVMEDIA_TYPE_VIDEO;
        avcodec_open2(codec_context_, codec_, NULL);
        avcodec_parameters_from_context(stream_->codecpar, codec_context_);
        avformat_write_header(format_context_, &options_);
        av_frame_ = av_frame_alloc();
        av_frame_->format = AV_PIX_FMT_YUV420P;
        av_frame_->width = codec_context_->width;
        av_frame_->height = codec_context_->height;
        av_frame_get_buffer(av_frame_, 0);
        sws_context_ = sws_getContext(
            codec_context_->width, codec_context_->height, AV_PIX_FMT_BGR24,
            codec_context_->width, codec_context_->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL
        );

        std::cout << "FFMPEG initialisation complete" << std::endl;

        start();
	}

    VideoEncoderThread::~VideoEncoderThread()
    {
        stop();

        {
            std::lock_guard<std::mutex> lock(queue_lock_);

            while (!queue_.empty())
            {
                WriteQuery query = queue_.front();
                queue_.pop();
                delete[] query.data;
            }
        }

        av_write_trailer(format_context_);
        av_frame_free(&av_frame_);
        avcodec_free_context(&codec_context_);
        avio_closep(&format_context_->pb);
        avformat_free_context(format_context_);
    }

    void VideoEncoderThread::writeFrame(const cv::Mat& image)
    {
        auto timestamp = std::chrono::high_resolution_clock::now();

        if (!wrote_first_frame_)
        {
            wrote_first_frame_ = true;
            first_frame_time_ = timestamp;
        }

        int64_t pts = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - first_frame_time_).count();

        uint8_t* data = new uint8_t[image.total() * image.elemSize()];
        std::memcpy(data, image.data, image.total() * image.elemSize() * sizeof(uint8_t));

        {
            std::lock_guard<std::mutex> lock(queue_lock_);
            queue_.push({ data, image.rows, image.cols, pts });
        }
    }

    void VideoEncoderThread::write_frame_from_queue_()
    {
        WriteQuery query;

        {
            std::lock_guard<std::mutex> lock(queue_lock_);

            if (queue_.empty())
            {
                return;
            }

            query = queue_.front();
            queue_.pop();
        }

        /*if (std::abs(query.pts - next_key_frame_pts_) < 25)
        {
            query.pts = next_key_frame_pts_;
            std::cout << "Frame with pts " << next_key_frame_pts_ << " prepared " << (query.pts * (format_context_->streams[0]->time_base.den) / 1000) << std::endl;
            next_key_frame_pts_ += 2000;
        }
        else if (query.pts > next_key_frame_pts_)
        {
            WriteQuery keyQuery = query;
            keyQuery.pts = next_key_frame_pts_;
            write_frame_from_(keyQuery);
            std::cout << "Frame with pts " << next_key_frame_pts_ << " written " << (query.pts * (format_context_->streams[0]->time_base.den) / 1000) << std::endl;
            next_key_frame_pts_ += 2000;
        }*/

        write_frame_from_(query);

        delete[] query.data;
    }

    void VideoEncoderThread::write_frame_from_(const WriteQuery& query)
    {
        uint8_t* data[1] = { query.data };
        int linesize[1] = { 3 * query.cols };
        sws_scale(sws_context_, data, linesize, 0, query.rows, av_frame_->data, av_frame_->linesize);
        //av_frame_->pts = frame_count_ * (format_context_->streams[0]->time_base.den) / framerate_;
        av_frame_->pts = query.pts * (format_context_->streams[0]->time_base.den) / 1000;
        av_init_packet(&packet_);
        packet_.data = NULL;
        packet_.size = 0;
        if (avcodec_send_frame(codec_context_, av_frame_) == 0)
        {
            if (avcodec_receive_packet(codec_context_, &packet_) == 0)
            {
                av_interleaved_write_frame(format_context_, &packet_);
                av_packet_unref(&packet_);
            }
        }
    }

    void VideoEncoderThread::update()
    {
        write_frame_from_queue_();
    }
}