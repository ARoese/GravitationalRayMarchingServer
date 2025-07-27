//
// Created by atomr on 7/27/2025.
//

#include "conversions.h"

#include <vector_functions.h>

#include "GRMExceptions.h"

Camera from_protobuf(const grm::protobuf::Camera& p_Camera) {
    const auto p_fov = p_Camera.fov();
    const auto p_camPos = p_Camera.campos();
    const auto p_camRot = p_Camera.camrot();
    return Camera(
        make_float2(p_fov.x(), p_fov.y()),
        make_float3(p_camPos.x(), p_camPos.y(), p_camPos.z()),
        make_float3(p_camRot.x(), p_camRot.y(), p_camRot.z())
    );
}

Texture from_protobuf(const grm::protobuf::Texture& p_Texture, const BlobMap& bm) {
    auto data_blob_id = p_Texture.blobident().id();
    if (!bm.contains(data_blob_id)) {
        throw RequestException(std::format("Blob id {} mentioned in texture but not sent", data_blob_id));
    }
    const auto& data = bm.at(data_blob_id);
    switch (p_Texture.encoding()) {
        case grm::protobuf::Texture_ImageEncodingType_RAW_RGB:
            return from_raw_rgb(data_blob_id, p_Texture.width(), p_Texture.height(), data);
    }
    throw RequestException("Cannot decode unknown texture encoding");
}

Material from_protobuf(const grm::protobuf::Material &p_Material, const BlobMap& bm) {
    if (p_Material.has_color()) {
        auto& color = p_Material.color();
        return Material(make_uchar3(color.x(), color.y(), color.z()));
    }
    if (p_Material.has_texture()) {
        return Material(from_protobuf(p_Material.texture(), bm));
    }
    throw RequestException(std::format("Unknown material type. Not color nor Texture."));
}

float3 from_protobuf(const grm::protobuf::Float3& f3) {
    return make_float3(f3.x(), f3.y(), f3.z());
}

uint2 from_protobuf(const grm::protobuf::UInt2& i2) {
    return make_uint2(i2.x(), i2.y());
}

Body from_protobuf(grm::protobuf::Body p_Body, const BlobMap& bm) {
    return Body(
        p_Body.radius(),
        p_Body.mass(),
        from_protobuf(p_Body.position()),
        from_protobuf(p_Body.rotation()),
        from_protobuf(p_Body.material(), bm));
}

UniversalConstants from_protobuf(const grm::protobuf::UniversalConstants& p_Constants) {
    return {
        .G = p_Constants.g(),
        .C = p_Constants.c()
    };
}

Scene from_protobuf(const grm::protobuf::Scene& p_Scene, const BlobMap& bm) {
    auto constants = p_Scene.has_constants()
        ? from_protobuf(p_Scene.constants())
        : real_universal_constants;
    auto numBodies = p_Scene.bodies().size();
    auto& bodies = p_Scene.bodies();
    auto bodies_arr = new Body[numBodies];
    for (int i = 0; i < numBodies; i++) {
        new(&bodies_arr[i]) Body(from_protobuf(bodies[i], bm));
    }

    return Scene(
        from_protobuf(p_Scene.cam()),
        Buffer<Body>(bodies_arr, numBodies),
        from_protobuf(p_Scene.nohit(), bm),
        constants
        );
}

D_MarchConfig from_protobuf(const grm::protobuf::MarchConfig& p_MarchConfig) {
    return {
        .marchSteps = static_cast<int>(p_MarchConfig.marchsteps()),
        .marchStepDeltaTime = p_MarchConfig.marchstepdeltatime()
    };
}

RenderConfig from_protobuf(const grm::protobuf::RenderConfig& p_RenderConfig) {
    auto marchConfig = p_RenderConfig.has_marchconfig()
        ? from_protobuf(p_RenderConfig.marchconfig())
        : default_MarchConfig;

    return RenderConfig(
        from_protobuf(p_RenderConfig.resolution()),
        marchConfig
        );
}

std::vector<char> texture_to_blob(
    const Texture& tex,
    const grm::protobuf::Texture::ImageEncodingType& encodingType
) {
    if (encodingType == grm::protobuf::Texture_ImageEncodingType_RAW_RGB) {
        const auto num_pixels = tex.size.x * tex.size.y;
        const auto texture_size_bytes = num_pixels * 3;

        auto blob_buffer = std::vector<char>(texture_size_bytes);
        for (int i = 0; i < num_pixels; i++) {
            const auto&[x, y, z] = tex.data[i];
            blob_buffer[i * 3 + 0] = x;
            blob_buffer[i * 3 + 1] = y;
            blob_buffer[i * 3 + 2] = z;
        }
        return blob_buffer;
    }

    throw RequestException("Cannot encode blob with requested type");
}

// returns the Texture protobuf and referenced blob with the given id
std::pair<grm::protobuf::Texture, std::vector<char>> to_protobuf(
    const Texture &result,
    const uint64_t &blobIdent,
    const grm::protobuf::Texture::ImageEncodingType &encodingType
    ) {
    auto tex = grm::protobuf::Texture();
    tex.set_height(result.size.y);
    tex.set_width(result.size.x);
    tex.mutable_blobident()->set_id(blobIdent);
    tex.set_encoding(encodingType);

    auto blob = texture_to_blob(result, encodingType);
    return std::make_pair(tex, blob);
}

std::string toString(const boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> &endp) {
    return std::format("{}:{}", endp.address().to_string(), endp.port());
}

Texture from_raw_rgb(uint32_t blob_id, uint32_t width, uint32_t height, const std::vector<char>& data) {
    const auto numPixels = width * height;
    const auto requiredBlobLength = numPixels * 3; //flat length of (w,h,3) tensor
    if (requiredBlobLength != data.size()) {
        throw RequestException(
            std::format(
                "data blob {} is not expected length of {} for {}x{} raw_RGB texture",
                blob_id, requiredBlobLength, width, height
                ));
    }
    const auto raw_data = new uchar3[numPixels];
    // NOTE: This is probably really slow
    for (int i = 0; i < numPixels; i++) {
        raw_data[i] = make_uchar3(data[i*3+0], data[i*3+1], data[i*3+2]);
    }
    return Texture(make_uint2(width, height), raw_data);
}