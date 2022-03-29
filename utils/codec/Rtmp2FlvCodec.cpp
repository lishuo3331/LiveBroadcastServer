//
// Created by rjd
//

#include "Rtmp2FlvCodec.h"
#include "utils/Logger.h"

Rtmp2FlvCodec::Rtmp2FlvCodec():
		base_timestamp_(0)
{
}

void Rtmp2FlvCodec::Transform(const RtmpPackPtr& rtmp_pack_, FlvTagPtr& flv_tag)
{
	flv_tag->SetTagType(static_cast<uint8_t>(rtmp_pack_->GetRtmpPackType()));
	flv_tag->SetDataSize(rtmp_pack_->GetDataSizePtr());

	// TODO 考虑大端序机器
	uint32_t timestamp;
	uint8_t* time_ptr = reinterpret_cast<uint8_t*>(&timestamp);
	time_ptr[0] = rtmp_pack_->GetTimeStamp()[2];
	time_ptr[1] = rtmp_pack_->GetTimeStamp()[1];
	time_ptr[2] = rtmp_pack_->GetTimeStamp()[0];

	base_timestamp_ += timestamp;

	flv_tag->SetTimeStamp(base_timestamp_);

	LOG_INFO << "add: " << timestamp <<",timestamp: " << base_timestamp_;

	Buffer* rtmp_buffer = rtmp_pack_->GetBuffer();
	Buffer* tag_buffer = flv_tag->GetBody();

	tag_buffer->AppendData(rtmp_buffer->ReadBegin(), rtmp_buffer->ReadableLength());
}