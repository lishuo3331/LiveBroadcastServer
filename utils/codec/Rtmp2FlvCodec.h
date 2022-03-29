//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_RTMP2FLVCODEC_H
#define LIVEBROADCASTSERVER_RTMP2FLVCODEC_H

#include <functional>

#include "utils/codec/FlvCodec.h"
#include "utils/codec/RtmpCodec.h"

class Rtmp2FlvCodec
{
public:

	Rtmp2FlvCodec();

	void Transform(const RtmpPackPtr& rtmp_pack_, FlvTagPtr& flv_tag);

private:

	uint32_t base_timestamp_ = 0;
};


#endif //LIVEBROADCASTSERVER_RTMP2FLVCODEC_H
