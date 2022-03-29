#ifndef SERVER_RTMPMANAGER_H
#define SERVER_RTMPMANAGER_H

#include <functional>
#include "utils/codec/RtmpCodec.h"
#include "utils/codec/FlvManager.h"

constexpr int RTMP_START_PARSE_LENGTH = 1000;
// constexpr int RTMP_CHUNK_SIZE = 4096;
constexpr int RTMP_CHUNK_SIZE = 0x5a0;

typedef std::function<void(const RtmpPackPtr&)> NewRtmpPackCallback;
class RtmpManager
{
public:

	enum ParseStatus
	{
		PARSE_FIRST_HEADER,
		PARSE_RTMP_HEADER,
		PARSE_RTMP_BODY
	};

	/**
	 *
	 */
	enum ShakeHandPackType
	{
		SHAKE_RTMP_C01,
		SHAKE_RTMP_C2,
		SHAKE_RTMP_SET_CHUNK_SIZE,
		SHAKE_RTMP_CONNECT,
		SHAKE_RTMP_RELEASE_STREAM,
		SHAKE_RTMP_FCPUBLISH,
		SHAKE_RTMP_CREATE_STREAM,
		SHAKE_RTMP_PUBLISH,
		SHAKE_SUCCESS,
		SHAKE_FAILED,
		SHAKE_DATA_NOT_ENOUGH
	};


	RtmpManager();
	~RtmpManager();

	ssize_t ParseData(Buffer* buffer);

	FlvManager* GetFlvManager();

	/**
	 * 解析握手数据
	 * @param buffer
	 * @return 此轮解析时的状态
	 */
	ShakeHandPackType ParseShakeHand(Buffer* buffer);

	size_t GetParsedLength() const;

	std::string GetUrlFromConnectPack() const;

	std::string GetPasswordFromReleasePack();

	void SetNewRtmpPackCallback(const NewRtmpPackCallback& callback);

	RtmpCodec rtmp_codec_;

private:

	ParseStatus parsed_status_;
	ShakeHandPackType shake_hand_status_;

	size_t parsed_length_;

	FlvManager flv_manager_;

	FlvTagPtr last_flv_ptr_;

	RtmpPack connect_pack_;

	RtmpPack release_pack_;

	NewRtmpPackCallback new_rtmp_pack_callback_;
	/**
	 * 解析头部后 解析body  如果解析头部或body时长度不足则返回0 不移动读指针
	 * 如果完成解析则移动读指针
	 * @param buffer
	 * @param rtmp_pack
	 * @return 解析成功返回解析长度 长度不足返回0 失败返回-1
	 */
	ssize_t ParseHeaderAndBody(Buffer* buffer, RtmpPack* rtmp_pack);

	void ProcessNewFlvTag(const FlvTagPtr& tag);
};

#endif