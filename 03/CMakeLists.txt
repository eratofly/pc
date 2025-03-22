#cmake_minimum_required(VERSION 3.22)
#project(bank)
#
#set(CMAKE_CXX_STANDARD 20)
#
#add_executable(bank
#        main.cpp
#        Bank.h
#        CharacterBase.h
#        Heroes.h
#)

cmake_minimum_required(VERSION 3.22)
project(bank)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_library(catch2 INTERFACE)
target_include_directories(
        catch2 INTERFACE lib/catch2
)

add_executable(bank
        bank/main.cpp
        bank/Bank.h
        bank/CharacterBase.h
        bank/Heroes.h
)

add_executable(bank_tests
        bank/Bank.h
        bank/BankOperationErrors.h
        bank_tests/BankTests.cpp
)

target_link_libraries(bank_tests PUBLIC catch2)