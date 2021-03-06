#include <cassert>
#include <iostream>
#include "utils/codec/RtmpManager.h"
#include "utils/codec/Rtmp2FlvCodec.h"
#include "utils/Format.h"
#include "utils/Logger.h"

RtmpManager::RtmpManager():
		rtmp_codec_(),
		parsed_status_(RtmpManager::PARSE_FIRST_HEADER),
		shake_hand_status_(RtmpManager::SHAKE_RTMP_C01),
		parsed_length_(0),
		flv_manager_()
{
}

RtmpManager::~RtmpManager()
{
}

ssize_t RtmpManager::ParseData(Buffer* buffer)
{
	bool pack_finish = false;
	ssize_t result = rtmp_codec_.DecodeData(buffer, &pack_finish);
	if (pack_finish)
	{
		// TODO
		RtmpPackPtr pack_ptr = std::make_shared<RtmpPack>(rtmp_codec_.GetLastRtmpPack());
		if (new_rtmp_pack_callback_)
		{
			new_rtmp_pack_callback_(pack_ptr);
		}
	}
	return result;
}

FlvManager* RtmpManager::GetFlvManager()
{
	return &flv_manager_;
}

std::string RtmpManager::GetUrlFromConnectPack() const
{
	RtmpPack connect_pack = connect_pack_;
	std::string body_str = connect_pack.GetBuffer()->ReadAllAsString();
	auto begin_idx = body_str.find("rtmp");
	size_t len = strlen(&body_str[begin_idx]);
	return body_str.substr(begin_idx, len);
}

std::string RtmpManager::GetPasswordFromReleasePack()
{
	const char* release_buffer_begin = release_pack_.GetBuffer()->ReadBegin();
	int len = release_buffer_begin[28];
	return std::string(&release_buffer_begin[29], len);
}

void RtmpManager::SetNewRtmpPackCallback(const NewRtmpPackCallback& callback)
{
	new_rtmp_pack_callback_ = callback;
}

RtmpManager::ShakeHandPackType RtmpManager::ParseShakeHand(Buffer* buffer)
{
	ssize_t parse = 0;
	switch (shake_hand_status_)
	{
		case SHAKE_RTMP_C01:
		{
			if (buffer->ReadableLength() < 1537)
			{
				return SHAKE_DATA_NOT_ENOUGH;
			}
			else
			{
				shake_hand_status_ = SHAKE_RTMP_C2;
				buffer->AddReadIndex(1537);
				return SHAKE_RTMP_C01;
			}

		}
		case SHAKE_RTMP_C2:
		{
			if (buffer->ReadableLength() < 1536)
			{
				return SHAKE_DATA_NOT_ENOUGH;
			}
			else
			{
				buffer->AddReadIndex(1536);

#ifdef MONITOR_MODE
				shake_hand_status_ = SHAKE_RTMP_CONNECT;
#else
				shake_hand_status_ = SHAKE_RTMP_SET_CHUNK_SIZE;
#endif
				return SHAKE_RTMP_C2;
			}
		}
		case SHAKE_RTMP_SET_CHUNK_SIZE:
		{
			RtmpPack set_chunk_size_pack;
			parse = ParseHeaderAndBody(buffer, &set_chunk_size_pack);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_CONNECT;
				return SHAKE_RTMP_SET_CHUNK_SIZE;
			}
			break;
		}
		case SHAKE_RTMP_CONNECT:
		{
			parse = ParseHeaderAndBody(buffer, &connect_pack_);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_RELEASE_STREAM;
				return SHAKE_RTMP_CONNECT;
			}
			break;
		}
		case SHAKE_RTMP_RELEASE_STREAM:
		{
			parse = ParseHeaderAndBody(buffer, &release_pack_);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_FCPUBLISH;
				return SHAKE_RTMP_RELEASE_STREAM;
			}
			break;
		}
		case SHAKE_RTMP_FCPUBLISH:
		{
			RtmpPack fc_publish;
			parse = ParseHeaderAndBody(buffer, &fc_publish);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_CREATE_STREAM;
				return SHAKE_RTMP_FCPUBLISH;
			}
			break;
		}
		case SHAKE_RTMP_CREATE_STREAM:
		{
			if (buffer->ReadableLength() > 0)
			{
				/**
				 * ???????????????c3 ??????????????? ?????????????????????????????? ?????????????????????
				 */
				if ((uint8_t)*buffer->ReadBegin() == 0xc3)
				{
					buffer->AddReadIndex(26);
					shake_hand_status_ = SHAKE_RTMP_PUBLISH;
					return SHAKE_RTMP_CREATE_STREAM;
				}
				else
				{
					RtmpPack create_stream;
					parse = ParseHeaderAndBody(buffer, &create_stream);
					if (parse > 0)
					{
						shake_hand_status_ = SHAKE_RTMP_PUBLISH;
						return SHAKE_RTMP_CREATE_STREAM;
					}
				}
			}
			else
			{
				parse = 0;
			}
			break;
		}
		case SHAKE_RTMP_PUBLISH:
		{
			RtmpPack publish_pack;
			parse = ParseHeaderAndBody(buffer, &publish_pack);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_SUCCESS;
				return SHAKE_RTMP_PUBLISH;
			}
			break;
		}
		case SHAKE_FAILED:
			return SHAKE_FAILED;
		case SHAKE_SUCCESS:
			return SHAKE_SUCCESS;
		case SHAKE_DATA_NOT_ENOUGH:
			return SHAKE_DATA_NOT_ENOUGH;
	}

	if (parse == 0)
	{
		return SHAKE_DATA_NOT_ENOUGH;
	}
	else
	{
		shake_hand_status_ = SHAKE_FAILED;
		return SHAKE_FAILED;
	}
}

ssize_t RtmpManager::ParseHeaderAndBody(Buffer* buffer, RtmpPack* rtmp_pack)
{
	bool pack_finish = false;
	ssize_t parsed = rtmp_codec_.DecodePack(buffer, &pack_finish);
	*rtmp_pack = rtmp_codec_.GetLastRtmpPack();
	if (!pack_finish)
	{
		return 0;
	}
	return parsed;
}

size_t RtmpManager::GetParsedLength() const
{
	return parsed_length_;
}
