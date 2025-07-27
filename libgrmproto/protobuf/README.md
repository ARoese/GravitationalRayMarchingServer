Render protocol:

A client sends a request, which contains scene and render information.
large data such as textures is sent out-of-band from the protobuf protocol
because protobuf shouldn't really handle blobs larger than 64Mib

{len} - always a network-endian 4-byte uint32, the length of the following protobuf message

Following a blobsHeader, a concat of binary blobs will be sent. 
These should be chunked and associated using information in the preceding BlobsHeader.
These blobs are referenced in other messages, and should be made available 
for association on deserialization-time.

For example, a Scene message contains Materials which may or may not contain a Texture.
The full binary image could be too large to reasonably fit inside a protobuf, so Texture
merely references a binary blob ID, and expects that it will be available when necessary.

A BlobsHeader must be sent even if there are no blobs required--that is how it is conveyed
that no blobs will be sent.

At any time, the connection may be killed by the client. If this happens, the server will
cancel rendering and resume waiting for connections.

Client -> 
    {len}{message RenderRequest}{blob1}{blob2}
<- Server
    {len}{message RenderResponse}{blob1}