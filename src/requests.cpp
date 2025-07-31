//
// Created by atomr on 7/27/2025.
//

#include <vector>
#include <boost/asio/ip/tcp.hpp>
#include <boost/endian/buffers.hpp>

#include "CancellationToken.cuh"
#include "GRMExceptions.h"
#include "conversions.h"
#include "Renderer.cuh"
#include "libgrmproto/Render.pb.h"
constexpr unsigned int MAX_REQUEST_MSG_LENGTH = 60000000;
constexpr unsigned int MAX_BLOB_SIZE = 2000000000;

using boost::asio::ip::tcp;

grm::protobuf::RenderRequest read_request_from_stream(tcp::iostream& sockStream) {
    boost::endian::big_uint32_buf_t len;
    sockStream.read(reinterpret_cast<char*>(&len), 4);
    //sockStream >> len;
    if (len.value() > MAX_REQUEST_MSG_LENGTH) {
        throw RequestException(
            std::format(
                "Request message too large. Max request message size is {} bytes",
                MAX_REQUEST_MSG_LENGTH)
                );
    }
    const auto requestBuffer = std::unique_ptr<char[]>(new char[len.value()]);
    sockStream.read(requestBuffer.get(), len.value());
    grm::protobuf::RenderRequest request;
    if (!request.ParseFromArray(requestBuffer.get(), len.value())) {
        throw RequestException("Ill-formed RenderRequest");
    };

    return request;
}


BlobMap read_blobs_from_stream(tcp::iostream& sockStream, const grm::protobuf::BlobsHeader& bh) {
    auto map = BlobMap(bh.blobs().size());
    auto seenSet = std::unordered_set<uint32_t>(bh.blobs().size());
    for(auto& blob : bh.blobs()) {
        if (blob.bloblength() > MAX_BLOB_SIZE) {
            throw RequestException(
                std::format(
                    "Max blob size of {} exceeded for blob with id {}", MAX_BLOB_SIZE, blob.identifier().id())
                    );
        }
        if (seenSet.contains(blob.identifier().id())) {
            throw RequestException(std::format("Duplicate blob identifier {}", blob.identifier().id()));
        }
        seenSet.insert(blob.identifier().id());
    }
    for(auto& blob : bh.blobs()) {
        std::vector<char> blobBuffer(blob.bloblength());
        sockStream.read(blobBuffer.data(), blob.bloblength());
        map.insert(std::make_pair(blob.identifier().id(), std::move(blobBuffer)));
    }
    return map;
}

void cancel_on_close(tcp::iostream &sockStream, const CancellationToken& ct, CancellationToken& ct_to_cancel) {
    constexpr auto BUFFER_SIZE = 1024;
    // Buffer to store throwaway data
    auto buffer = std::make_unique<char[]>(BUFFER_SIZE);

    while (sockStream.good()) {
        if (ct.cancelled) {
            return;
        }
        // consume the rest of the input stream until eof
        // TODO: the sockstream is sometimes being cleaned up by the other thread and thus becoming invalidated
        // It should be accessed through a shared_ptr on both sides to prevent this
        sockStream.read(&buffer[0], BUFFER_SIZE);
    }

    ct_to_cancel.cancel();
}

constexpr int MAX_RENDER_DIMENSION = 8192;

void do_render_conversation(tcp::iostream& sockStream) {
    auto request = read_request_from_stream(sockStream);
    auto blobs = read_blobs_from_stream(sockStream, request.blobsinfo());

    auto scene = from_protobuf(request.scene(), blobs);
    auto renderConfig = from_protobuf(request.config());

    if (request.device() != grm::protobuf::CPU && request.device() != grm::protobuf::GPU) {
        throw RequestException(std::format("Unknown render device"));
    }
    if (renderConfig.resolution.x * renderConfig.resolution.y == 0) {
        throw RenderException("One of the render resolution dimensions is 0");
    }
    if (renderConfig.resolution.x > MAX_RENDER_DIMENSION || renderConfig.resolution.y > MAX_RENDER_DIMENSION) {
        throw RequestException(std::format("Max render dimension of {} was exceeded", MAX_RENDER_DIMENSION));
    }

    auto renderer = Renderer();
    // these are shared between the watch thread and this thread.
    // This scope and the thread's scope can be exited in any order,
    // so we're using shared pointers to make that shared ownership dynamic easier
    auto render_ct = std::make_shared<CancellationToken>();
    auto socket_watch_ct = std::make_shared<CancellationToken>();
    auto cudaContext = CudaContext();

    auto cancellationThread = std::thread([&sockStream, socket_watch_ct, render_ct] {
        // TODO: ensure this thread actually terminates as expected
        cancel_on_close(sockStream, *socket_watch_ct, *render_ct);
    });

    auto result = request.device() == grm::protobuf::GPU
        ? renderer.renderGPU(scene, renderConfig, cudaContext, *render_ct)
        : renderer.renderCPU(scene, renderConfig, *render_ct);

    socket_watch_ct->cancel();
    cancellationThread.detach();

    auto render_result = grm::protobuf::RenderResult();
    auto render_response = grm::protobuf::RenderResponse();
    if (result.has_value()) {
        auto& render_tex = result.value();
        auto [succ, blob] = to_protobuf(render_tex, 1, grm::protobuf::Texture::RAW_RGB);
        *render_result.mutable_success() = succ;

        *render_response.mutable_result() = render_result;
        auto& mut_blob = *render_response.mutable_blobsinfo()->mutable_blobs()->Add();
        mut_blob.mutable_identifier()->set_id(1);
        mut_blob.set_bloblength(blob.size());

        std::string out;
        render_response.SerializeToString(&out);
        boost::endian::big_uint32_buf_t len;
        len = out.size();
        sockStream.write(reinterpret_cast<char*>(&len), 4);
        sockStream << out;
        sockStream.write(blob.data(), blob.size());
    }else {
        render_result.mutable_error()->set_reason(std::string("Rendering failed. Unknown reason. Potentially canceled"));
        *render_response.mutable_result() = render_result;

        std::string out;
        render_result.SerializeToString(&out);
        boost::endian::big_uint32_buf_t len;
        len = out.size();
        sockStream.write(reinterpret_cast<char*>(&len), 4);
        sockStream << out;
    }
}

void handle_request(tcp::iostream sockStream) {
    try {
        //sockStream.exceptions(std::ios::failbit);
        do_render_conversation(sockStream);
        std::cout
            << std::format("{}: Request handled successfully", toString(sockStream.socket().remote_endpoint()))
            << std::endl;
    }catch (RequestException& e) {
        grm::protobuf::RenderResponse response;
        response.mutable_blobsinfo();
        response.mutable_invalidrequest()->set_reason(std::string(e.what()));

        sockStream.clear(); // clear any iostate errors so we can try to send a response

        std::cout
            << std::format("{}: RequestException: {}", toString(sockStream.socket().remote_endpoint()), e.what())
            << std::endl;

        std::string out;
        response.SerializeToString(&out);
        boost::endian::big_uint32_buf_t len;
        len = out.size();
        sockStream.write(reinterpret_cast<char*>(len.data()), 4);
        sockStream << out;
    }catch (RenderException& e) {
        grm::protobuf::RenderResponse response;
        response.mutable_blobsinfo();
        response.mutable_result()->mutable_error()->set_reason(std::string(e.what()));

        sockStream.clear(); // clear any iostate errors so we can try to send a response

        std::cout
            << std::format("{}: RenderException: {}", toString(sockStream.socket().remote_endpoint()), e.what())
            << std::endl;
        std::string out;
        response.SerializeToString(&out);
        boost::endian::big_uint32_buf_t len;
        len = out.size();
        sockStream.write(reinterpret_cast<char*>(len.data()), 4);
        sockStream << out;
    }
    sockStream.flush();
    sockStream.close();
}