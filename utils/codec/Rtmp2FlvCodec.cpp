//
// Created by rjd
//

#include "Rtmp2FlvCodec.h"

void Rtmp2FlvCodec::Transform(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{

}

void Rtmp2FlvCodec::EncodeHeaderAndSwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	EncodeHeaderToFlvTag(rtmp_pack_, flv_tag);
	SwapBuffer(rtmp_pack_, flv_tag);
}

void Rtmp2FlvCodec::SwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	Buffer* rtmp_buffer = rtmp_pack_->GetBuffer();
	Buffer* tag_buffer = flv_tag->GetBody();

	rtmp_buffer->SwapBuffer(tag_buffer);
}

void Rtmp2FlvCodec::EncodeHeaderToFlvTag(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	flv_tag->SetTagType(static_cast<uint8_t>(rtmp_pack_->GetRtmpPackType()));
	flv_tag->SetDataSize(rtmp_pack_->GetDataSizePtr());
	flv_tag->SetTimeStamp(rtmp_pack_->GetTimeStamp());
}