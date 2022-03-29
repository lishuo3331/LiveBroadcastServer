//
// Created by rjd
//

#include "live/connection/RtmpPushConnection.h"
#include "utils/Logger.h"
#include "utils/Format.h"

RtmpPushConnection::RtmpPushConnection(const TcpConnectionPtr& connection_ptr) :
		connection_ptr_(connection_ptr),
		rtmp_manager_(),
		flv_manager_(rtmp_manager_.GetFlvManager()),
		last_write_size_(0),
		header_buffer_()
{
	rtmp_manager_.SetNewRtmpPackCallback(
			[this](auto&& PH1){RtmpPushConnection::OnNewRtmpPack(PH1);}
			);
}

RtmpPushConnection::ShakeHandResult RtmpPushConnection::ShakeHand(Buffer* buffer)
{
	while (true)
	{
		RtmpManager::ShakeHandPackType status = rtmp_manager_.ParseShakeHand(buffer);
		LOG_INFO << connection_ptr_->GetConnectionName() << " parse result " << status;
		switch (status)
		{
			case RtmpManager::SHAKE_RTMP_C01:
				connection_ptr_->Send(RTMP_SERVER_S01, sizeof RTMP_SERVER_S01);
				LOG_INFO << connection_ptr_->GetConnectionName() << " send RTMP_SERVER_S01";
				break;
			case RtmpManager::SHAKE_RTMP_C2:
				connection_ptr_->Send(RTMP_SERVER_S2, sizeof RTMP_SERVER_S2);
				LOG_INFO << connection_ptr_->GetConnectionName() << " send SHAKE_RTMP_C2";
				break;
			case RtmpManager::SHAKE_RTMP_SET_CHUNK_SIZE:
				break;
			case RtmpManager::SHAKE_RTMP_CONNECT:
			{
				connection_ptr_->Send(RTMP_SERVER_ACKNOWLEDGE_SIZE, sizeof RTMP_SERVER_ACKNOWLEDGE_SIZE);
				connection_ptr_->Send(RTMP_SERVER_PEER_BANDWIDTH, sizeof RTMP_SERVER_PEER_BANDWIDTH);
				connection_ptr_->Send(RTMP_SERVER_SET_CHUNK_SIZE, sizeof RTMP_SERVER_SET_CHUNK_SIZE);
				connection_ptr_->Send(RTMP_SERVER_CONNECT_RESULT, sizeof RTMP_SERVER_CONNECT_RESULT);
#ifdef MONITOR_MODE
				connection_ptr_->Send(RTMP_SERVER_RESULT, sizeof RTMP_SERVER_RESULT);
#endif
				break;
			}
			/** 解析完release包后 进行用户校验*/
			case RtmpManager::SHAKE_RTMP_RELEASE_STREAM:
			{
				return SHAKE_AUTHENTICATE;
			}
			case RtmpManager::SHAKE_RTMP_FCPUBLISH:
				break;
			case RtmpManager::SHAKE_RTMP_CREATE_STREAM:
#ifdef MONITOR_MODE
#else
			connection_ptr_->Send(RTMP_SERVER_RESULT, sizeof RTMP_SERVER_RESULT);
#endif

				break;
			case RtmpManager::SHAKE_RTMP_PUBLISH:
				connection_ptr_->Send(RTMP_SERVER_START, sizeof RTMP_SERVER_START);
				break;
			case RtmpManager::SHAKE_SUCCESS:
			{
				return RtmpPushConnection::SHAKE_SUCCESS;
			}
			case RtmpManager::SHAKE_FAILED:
			{
				/**
				 * 出错时返回
				 */
				return SHAKE_FAILED;
			}
			case RtmpManager::SHAKE_DATA_NOT_ENOUGH:
				/**
				 * 数据不足时返回
				 */
				return SHAKE_DATA_NOT_ENOUGH;
		}
	}
}

ssize_t RtmpPushConnection::ParseData(Buffer* buffer)
{
	ssize_t result = rtmp_manager_.ParseData(buffer);
	DebugParseSize(100 * 1000 * 1000);

	return result;
}

void RtmpPushConnection::DebugParseSize(size_t division)
{
	size_t read_m = rtmp_manager_.GetParsedLength() / division;
	if (read_m != last_write_size_)
	{
		last_write_size_ = read_m;
		LOG_INFO << "connection: " << connection_ptr_->GetConnectionName()
				 << ", sum write " << rtmp_manager_.GetParsedLength()
				 << " bytes";
	}
}

const Buffer& RtmpPushConnection::GetHeaderDataBuffer()
{
	if (header_buffer_.ReadableLength() == 0)
	{
		RtmpPackPtr first_script_pack = rtmp_manager_.rtmp_codec_.first_script_pack_;
		RtmpPackPtr first_video_pack = rtmp_manager_.rtmp_codec_.first_video_pack_;
		RtmpPackPtr first_audio_pack = rtmp_manager_.rtmp_codec_.first_audio_pack_;

		FlvTagPtr first_script_tag = std::make_shared<FlvTag>();
		FlvTagPtr first_video_tag = std::make_shared<FlvTag>();
		FlvTagPtr first_audio_tag = std::make_shared<FlvTag>();

		rtmp_to_flv_codec_.Transform(first_script_pack, first_script_tag);
		rtmp_to_flv_codec_.Transform(first_video_pack, first_video_tag);
		if (first_audio_pack)
		{
			rtmp_to_flv_codec_.Transform(first_audio_pack, first_audio_tag);
		}

		header_buffer_.AppendData(FlvHeader::DEFAULT_HEADER, FlvHeader::FLV_HEADER_LENGTH);

		header_buffer_.AppendData(htonl(0));
		header_buffer_.AppendData(first_script_tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		header_buffer_.AppendData(first_script_tag->GetBody());

		header_buffer_.AppendData(htonl(first_script_tag->GetCurrentTagSize()));
		header_buffer_.AppendData(first_video_tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		header_buffer_.AppendData(first_video_tag->GetBody());

		if (first_audio_pack)
		{
			header_buffer_.AppendData(htonl(first_video_tag->GetCurrentTagSize()));
			header_buffer_.AppendData(first_audio_tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
			header_buffer_.AppendData(first_audio_tag->GetBody());
		}
	}
	return header_buffer_;
}

void RtmpPushConnection::OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpPushConnection::ShakeHandResult result = ShakeHand(buffer);
	switch (result)
	{
		case RtmpPushConnection::SHAKE_AUTHENTICATE:
		{
			if (!Authenticate())
			{
				LOG_WARN << "connection: " << connection_ptr->GetConnectionName() << " authenticate failed";
				connection_ptr->Shutdown();
				/**
				 * 校验出错时返回
				 */
				return;
			}
			return;
		}
		case RtmpPushConnection::SHAKE_SUCCESS:
		{
			LOG_INFO << "connection: " << connection_ptr->GetConnectionName()
					 << " shake hand success";

			if (shake_hand_success_callback_)
			{
				shake_hand_success_callback_(shared_from_this());
			}

			connection_ptr->SetNewMessageCallback(
					[this](auto&& PH1, auto&& PH2, auto&& PH3)
					{
						OnBodyData(PH1, PH2, PH3);
					}
					);
			/**
			 * 握手成功时返回
			 */
			return;
		}
		case RtmpPushConnection::SHAKE_FAILED:
		{
			LOG_WARN << "connection: %s " << connection_ptr->GetConnectionName()
					 << "shake hand failed";
			connection_ptr->Shutdown();
			/**
			 * 出错时返回
			 */
			return;
		}
		case RtmpPushConnection::SHAKE_DATA_NOT_ENOUGH:
			/**
			 * 数据不足时返回
			 */
			return;
	}
}

void RtmpPushConnection::OnBodyData(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	ParseData(buffer);
}

void RtmpPushConnection::AddClientConnection(
		const PullConnectionPtr& client_connection_ptr)
{
	client_connection_map_[client_connection_ptr->GetConnectionName()]
		= client_connection_ptr;

	LOG_INFO << "server: " << connection_ptr_->GetConnectionName()
			 << ", add a client: " << GetConnectionName();

	client_connection_ptr->SetCloseConnectionCallback(
			[this](auto&& PH1){OnConnectionClose(PH1);});

	SendHeaderToClientConnection(client_connection_ptr);
}

void RtmpPushConnection::SetShakeHandSuccessCallback(const ShakeHandSuccessCallback& callback)
{
	shake_hand_success_callback_ = callback;
}

std::string RtmpPushConnection::GetRtmpUrl() const
{
	return rtmp_manager_.GetUrlFromConnectPack();
}

std::string RtmpPushConnection::GetRtmpPath() const
{
	return Format::GetPathFromUrl(GetRtmpUrl());
}

std::string RtmpPushConnection::GetConnectionName() const
{
	return connection_ptr_->GetConnectionName();
}

void RtmpPushConnection::SetAuthenticationCallback(const AuthenticationCallback& callback)
{
	authentication_callback_ = callback;
}

void RtmpPushConnection::SendHeaderToClientConnection(
		const PullConnectionPtr& client_connection_ptr)
{
	const Buffer& header_buffer = GetHeaderDataBuffer();

	client_connection_ptr->SendHeaderOnConnection(header_buffer);

	/**
	 * 头部之后第一个 Tag的PreviousTagSize 需要设置为 头部中最后一个Tag的CurrentSize
	 */
	// last_flv_tag_ptr_->SetPreviousTagSize(GetLastHeaderTagCurrentSize());
}

void RtmpPushConnection::OnNewFlvTag(const FlvTagPtr& tag_ptr)
{
	for (auto& [connection_name, connection_ptr] : client_connection_map_)
	{
		connection_ptr->AddFlvTag(tag_ptr);
	}
}

void RtmpPushConnection::OnNewRtmpPack(const RtmpPackPtr& rtmp_pack_ptr)
{
	FlvTagPtr flv_tag_ptr = std::make_shared<FlvTag>();
	rtmp_to_flv_codec_.Transform(rtmp_pack_ptr, flv_tag_ptr);
	OnNewFlvTag(flv_tag_ptr);
}

uint32_t RtmpPushConnection::GetLastHeaderTagCurrentSize() const
{
	return flv_manager_->GetVideoAudioTags()[0].GetCurrentTagSize();
}

void RtmpPushConnection::OnConnectionClose(const TcpConnectionPtr& connection_ptr)
{
	client_connection_map_.erase(connection_ptr->GetConnectionName());
	LOG_INFO << "client: " << connection_ptr->GetConnectionName()
			 << ", remove from server: " << connection_ptr_->GetConnectionName();
}

bool RtmpPushConnection::Authenticate()
{
	std::string username = GetRtmpPath();
	std::string password = rtmp_manager_.GetPasswordFromReleasePack();

	if (authentication_callback_)
	{
		if (!authentication_callback_(username, password))
		{
			return false;
		}
	}
	return true;
}
