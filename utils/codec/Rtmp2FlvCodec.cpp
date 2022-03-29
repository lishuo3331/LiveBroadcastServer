//
// Created by rjd
//

#include "Rtmp2FlvCodec.h"

Rtmp2FlvCodec::Rtmp2FlvCodec()
{
}

void Rtmp2FlvCodec::Transform(const RtmpPackPtr& rtmp_pack_, FlvTagPtr& flv_tag)
{
	flv_tag->SetTagType(static_cast<uint8_t>(rtmp_pack_->GetRtmpPackType()));
	flv_tag->SetDataSize(rtmp_pack_->GetDataSizePtr());
	flv_tag->SetTimeStamp(rtmp_pack_->GetTimeStamp());

	Buffer* rtmp_buffer = rtmp_pack_->GetBuffer();
	Buffer* tag_buffer = flv_tag->GetBody();

	tag_buffer->AppendData(rtmp_buffer->ReadBegin(), rtmp_buffer->ReadableLength());
}