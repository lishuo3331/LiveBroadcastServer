//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_RTMP2FLVCODEC_H
#define LIVEBROADCASTSERVER_RTMP2FLVCODEC_H

#include "utils/codec/FlvCodec.h"
#include "utils/codec/RtmpCodec.h"

class Rtmp2FlvCodec
{
public:

	static void Transform(RtmpPack* rtmp_pack_, FlvTag* flv_tag);

private:

	/**
	 * @brief 将RtmpPack的头部部分 转换到 FlvTag的头部
	 * @return
	*/
	void EncodeHeaderToFlvTag(RtmpPack* rtmp_pack_, FlvTag* flv_tag);

	static void SwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag);

	void EncodeHeaderAndSwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag);
};


#endif //LIVEBROADCASTSERVER_RTMP2FLVCODEC_H
