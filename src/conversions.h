//
// Created by atomr on 7/27/2025.
//

#ifndef PROTOBUF_CONVERSIONS_H
#define PROTOBUF_CONVERSIONS_H
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "Body.cuh"
#include "camera.cuh"
#include "Material.cuh"
#include "RenderConfig.cuh"
#include "Scene.cuh"
#include "libgrmproto/Body.pb.h"
#include "libgrmproto/Camera.pb.h"
#include "libgrmproto/RenderConfig.pb.h"
#include "libgrmproto/Scene.pb.h"

typedef std::unordered_map<uint32_t, std::vector<char>> BlobMap;

Camera from_protobuf(const grm::protobuf::Camera& p_Camera);
Texture from_protobuf(const grm::protobuf::Texture& p_Texture, const BlobMap& bm);
Material from_protobuf(const grm::protobuf::Material &p_Material, const BlobMap& bm);
float3 from_protobuf(const grm::protobuf::Float3& f3);
uint2 from_protobuf(const grm::protobuf::UInt2& i2);
Body from_protobuf(grm::protobuf::Body p_Body, const BlobMap& bm);
UniversalConstants from_protobuf(const grm::protobuf::UniversalConstants& p_Constants);
Scene from_protobuf(const grm::protobuf::Scene& p_Scene, const BlobMap& bm);
D_MarchConfig from_protobuf(const grm::protobuf::MarchConfig& p_MarchConfig);
RenderConfig from_protobuf(const grm::protobuf::RenderConfig& p_RenderConfig);
std::pair<grm::protobuf::Texture, std::vector<char>> to_protobuf(
    const Texture &result,
    const uint64_t &blobIdent,
    const grm::protobuf::Texture::ImageEncodingType &encodingType
    );
std::string toString(const boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> &endp);
Texture from_raw_rgb(uint32_t blob_id, uint32_t width, uint32_t height, const std::vector<char>& data);

#endif //PROTOBUF_CONVERSIONS_H
