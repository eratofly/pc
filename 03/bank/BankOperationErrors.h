#pragma once
#include <stdexcept>

class BankOperationError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};