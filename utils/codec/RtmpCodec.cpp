#include <sstream>
#include "utils/codec/RtmpCodec.h"
#include "utils/codec/FlvCodec.h"
#include "utils/Logger.h"


RtmpCodec::RtmpCodec():
		decode_data_status(PARSE_FIRST_HEADER),
		decode_pack_status(PARSE_RTMP_HEADER)
{

}

ssize_t RtmpCodec::DecodeData(Buffer* buffer, bool* pack_finish)
{
	size_t before_readable_length = buffer->ReadableLength();
	bool error = false;
	bool data_enough = true;
	while (data_enough && !error)
	{
		ssize_t parsed = 0;
		if (decode_data_status == PARSE_FIRST_HEADER)
		{
			bool first_header_finish = false;
			parsed = ParseFirstHeader(buffer, &first_header_finish);
			if (first_header_finish)
			{
				decode_data_status = PARSE_DATA_PACK;
			}
			if (parsed < 0)
			{
				LOG_ERROR << "ParseFirstHeader error";
				error = true;
			}
		}
		else
		{
			parsed = DecodePack(buffer, pack_finish);
			if (parsed < 0)
			{
				LOG_ERROR << "DecodePack error";
				error = true;
			}
		}
		if (parsed == 0)
		{
			data_enough = false;
		}
	}
	if (error)
	{
		return -1;
	}
	return static_cast<ssize_t>(before_readable_length - buffer->ReadableLength());
}

ssize_t RtmpCodec::DecodePack(Buffer* buffer, bool* pack_finish)
{
	if (buffer->ReadableLength() == 0)
	{
		return 0;
	}
	ssize_t read_length = 0;
	if (decode_pack_status == PARSE_RTMP_HEADER)
	{
		ssize_t parsed = DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength());
		read_length = parsed;
		if (parsed > 0)
		{
			buffer->AddReadIndex(parsed);
			decode_pack_status = PARSE_RTMP_BODY;
		}
		else if (parsed < 0)
		{
			LOG_ERROR << "ParseHeader error";
		}
	}
	if (decode_pack_status == PARSE_RTMP_BODY)
	{
		bool body_finish = false;
		size_t parsed_length = DecodeBody(buffer->ReadBegin(), buffer->ReadableLength(), &body_finish);
		buffer->AddReadIndex(parsed_length);
		read_length += static_cast<ssize_t>(parsed_length);
		if (body_finish)
		{
			//	FlvTagPtr tag_ptr = std::make_shared<FlvTag>();
			//	rtmp_codec_.EncodeHeaderAndSwapBuffer(&current_rtmp_pack_, tag_ptr.get());
			//	ProcessNewFlvTag(tag_ptr);
			decode_pack_status = PARSE_RTMP_HEADER;

			if (chunk_over_)
			{
				last_rtmp_pack_ = current_rtmp_pack_;
				current_rtmp_pack_.GetBuffer()->DropAllData();
				LOG_INFO << current_rtmp_pack_.GetHeaderDebugMessage();
				*pack_finish = true;
			}
		}
	}
	return read_length;
}

RtmpPack RtmpCodec::GetLastRtmpPack() const
{
	return last_rtmp_pack_;
}

ssize_t RtmpCodec::DecodeHeader(const char* data, size_t length)
{
	return current_rtmp_pack_.DecodeHeader(data, length);
}

size_t RtmpCodec::DecodeBody(const char* data, size_t length, bool* body_finish)
{
	// 只有在读满一个chunk分块4096字节后 返回解析一个新的header的时候
	// 当remain小于等于RTMP_CHUNK_SIZE的时候说明 此chunk分块结束了
	// LOG_INFO << "GetBodyRemainSize " << current_rtmp_pack_.GetBodyRemainSize() << ",read_chunk_size_ " << read_chunk_size_;

	chunk_over_ = false;
	if (current_rtmp_pack_.GetBodyRemainSize() <=
		rtmp_chunk_size_ && (read_chunk_size_ == 0))
	{
		chunk_over_ = true;
	}

	size_t remain = current_rtmp_pack_.GetBodyRemainSize();
	size_t read_length = 0;
	*body_finish = false;
	if (chunk_over_)
	{
		// 当前chunk没有分块 或者最后一个chunk分块被接收
		if (length < remain)
		{
			current_rtmp_pack_.AppendData(data, length);
			read_length = length;
		}
		else
		{
			current_rtmp_pack_.AppendData(data, remain);
			read_length = remain;
			*body_finish = true;
		}
	}
	else
	{
		// 当前chunk分块没有全部接受
		size_t current_chunk_remain = rtmp_chunk_size_ - read_chunk_size_;
		if (length < current_chunk_remain)
		{
			current_rtmp_pack_.AppendData(data, length);
			read_length = length;
			read_chunk_size_ += length;
		}
		else
		{
			current_rtmp_pack_.AppendData(data, current_chunk_remain);
			read_length = current_chunk_remain;
			/* chunk 结束 清除当前chunk已读字节数*/
			read_chunk_size_ = 0;
			*body_finish = true;
		}
	}
	return read_length;
}

ssize_t RtmpCodec::ParseFirstHeader(Buffer* buffer, bool* first_header_finish)
{
	bool has_audio = true;
	*first_header_finish = false;
	if (buffer->ReadableLength() < RTMP_START_PARSE_LENGTH)
	{
		return 0;
	}
	/**
	 * 解析脚本包并保存到flv_manager_
	 */

	ssize_t sum_parsed_length = 0;
	while ((has_audio && !first_audio_pack_) || !first_video_pack_ || !first_script_pack_)
	{
		bool pack_finish = false;
		ssize_t parsed_length = DecodePack(buffer, &pack_finish);
		if (pack_finish)
		{
			switch (last_rtmp_pack_.GetRtmpPackType())
			{
			case RtmpPack::RTMP_AUDIO:
				first_audio_pack_ = std::make_shared<RtmpPack>(last_rtmp_pack_);
				break;
			case RtmpPack::RTMP_VIDEO:
				first_video_pack_ = std::make_shared<RtmpPack>(last_rtmp_pack_);
				break;
			case RtmpPack::RTMP_SCRIPT:
				first_script_pack_ = std::make_shared<RtmpPack>(last_rtmp_pack_);
				break;
			default:
				LOG_ERROR << "ParseFirstHeader error pack type " << last_rtmp_pack_.GetRtmpPackType();
			}
		}
		if (parsed_length < 0)
		{
			return -1;
		}
		else if (parsed_length == 0)
		{
			break;
		}
		else if (parsed_length > 0)
		{
			sum_parsed_length += parsed_length;
		}
	}
	*first_header_finish = (!has_audio || first_audio_pack_) && first_video_pack_ && first_script_pack_;
	return sum_parsed_length;
}