#include <iostream>

#include "libgrmproto/Material.pb.h"

int main() {
    auto material = grm::protobuf::Material();
    auto const mc = material.mutable_color();
    mc->set_x(1);
    mc->set_y(2);
    mc->set_z(3);
    material.SerializeToOstream(&std::cout);
    std::cout << std::endl;
    std::cout << material.DebugString() << std::endl;
}