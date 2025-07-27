//
// Created by atomr on 7/27/2025.
//

#include "GRMExceptions.h"

const char* RequestException::what() const noexcept {
    return message_.c_str();
}

const char* RenderException::what() const noexcept {
    return message_.c_str();
}