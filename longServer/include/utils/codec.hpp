#pragma once

#include "noncopyable.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <memory>
#include <string>

namespace Uranus::Utils
{
//  数据包封包解包
class Codec : public Noncopyable
{
public:
    Codec()  = default;
    ~Codec() = default;

    // 编码
    auto EnCode(const google::protobuf::Message &msg) -> std::string
    {
        std::string data;

        auto typeName       = msg.GetTypeName();
        std::string strData = msg.SerializeAsString();

        // 除消息头4位之外的其他数据的长度
        std::uint32_t len  = mHeaderLen + typeName.size() + strData.size();
        std::uint32_t be32 = htonl(len);
        data.append(reinterpret_cast<char *>(&be32), sizeof be32);  // len

        // type name len
        unsigned int nNameLen = htonl(static_cast<uint32_t>(typeName.size()));
        data.append(reinterpret_cast<char *>(&nNameLen), sizeof nNameLen);  // nameLen

        data.append(typeName.data(), typeName.size());  // message typename

        data.append(strData.data(), strData.size());  // protobuf message data

        return data;
    }

    // 解码  这个是解码消息体部分
    std::shared_ptr<google::protobuf::Message> DeCode(const std::string &data)
    {
        if (data.size() >= mMinDataLen) {
            // body len
            auto nLen = data.size();

            // typeNameLen
            std::string sTypeNameLen(data.data(), mHeaderLen);
            auto nTypeNameLen = Utility::ToUInt(sTypeNameLen.data());

            // typeName
            std::string sTypeName(data, mHeaderLen, nTypeNameLen);

            // protoData
            std::int32_t nDataLen = nLen - mHeaderLen - nTypeNameLen;
            std::string sData(data, mHeaderLen + nTypeNameLen, nDataLen);

            auto result = createMsg(sTypeName);
            if (result) {
                result->ParseFromString(sData);
                return result;
            }
        }
        return nullptr;
    }

private:
    //  根据 protobuf 的 typename  创建 message
    auto createMsg(const std::string &typeName) -> std::shared_ptr<google::protobuf::Message>
    {
        auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
        if (descriptor) {
            auto ptototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
            if (ptototype) {
                std::shared_ptr<google::protobuf::Message> result(ptototype->New());
                return result;
            }
        }
        return nullptr;
    }

    // 数据包头
    const std::uint32_t mHeaderLen{sizeof(unsigned int)};
    // 最小的数据包长度
    const std::uint32_t mMinDataLen{2 * mHeaderLen};
};
}  // namespace Uranus::Utils
