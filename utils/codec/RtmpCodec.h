#ifndef UTILS_CODEC_RTMPCODEC_H
#define UTILS_CODEC_RTMPCODEC_H

#include <cstdint>
#include <cstring>
#include <string>

#include "utils/Buffer.h"
#include "utils/codec/RtmpPack.h"

class RtmpCodec
{
public:

	RtmpCodec();
	~RtmpCodec() = default;


	ssize_t DecodeData(Buffer* buffer, bool* pack_finish);

	ssize_t DecodePack(Buffer* buffer, bool* pack_finish);

	RtmpPack GetLastRtmpPack() const;

private:

	const uint32_t RTMP_START_PARSE_LENGTH = 1000;

	enum DecodeDataStatus
	{
		PARSE_FIRST_HEADER,
		PARSE_DATA_PACK
	};

	enum DecodePackStatus
	{
		PARSE_RTMP_HEADER,
		PARSE_RTMP_BODY
	};

	DecodeDataStatus decode_data_status;
	DecodePackStatus decode_pack_status;

	uint32_t rtmp_chunk_size_ = 4096;
	// uint32_t rtmp_chunk_size_ = 0x5a0;
	/* 由于chunk的分块存在 导致 当body大于4096字节时, 每读取4096个字节 需要重新解析一次header故在此记录*/
	uint32_t read_chunk_size_;

	// 正在解析的数据包
	RtmpPack current_rtmp_pack_;

	// 为false时 说明rtmp数据包还不完整 只有为true时一个rtmp数据包才收完
	bool chunk_over_;

	// 上一个解析完成的数据包
	RtmpPack last_rtmp_pack_;

	RtmpPackPtr first_script_pack_;
	RtmpPackPtr first_video_pack_;
	RtmpPackPtr first_audio_pack_;

	/**
	 * @brief 解析头部
	 * @param data 数据指针
	 * @param length 数据长度
	 * @return 成功返回解析字节数 失败返回-1 长度不足返回0
	*/
	ssize_t DecodeHeader(const char* data, size_t length);

	/**
	 * 解析body部分
	 * @param data 数据指针
	 * @param length 数据长度
	 * @param body_finish body是否结束 true结束 false数据不足
	 * @return true body结束
	 */
	size_t DecodeBody(const char* data, size_t length, bool* body_finish);

	ssize_t ParseFirstHeader(Buffer* buffer, bool* first_header_finish);
};

#endif // !UTILS_CODEC_RTMPCODEC_H
