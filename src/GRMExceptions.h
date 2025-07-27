//
// Created by atomr on 7/27/2025.
//

#ifndef GRMEXCEPTIONS_H
#define GRMEXCEPTIONS_H
#include <string>


class RequestException : public std::exception {
private:
    std::string message_;

public:
    // Constructor to initialize the error message
    explicit RequestException(const std::string& msg) : message_(msg) {}

    // Override the virtual what() method to provide a description
    const char* what() const noexcept override;
};

class RenderException : public std::exception {
private:
    std::string message_;

public:
    // Constructor to initialize the error message
    explicit RenderException(const std::string& msg) : message_(msg) {}

    // Override the virtual what() method to provide a description
    const char* what() const noexcept override;
};


#endif //GRMEXCEPTIONS_H
