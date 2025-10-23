cmake_minimum_required(VERSION 3.10)
project(hw)

# Define o padrão C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adiciona flags de otimização e warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# Cria o executável
add_executable(hw main.cpp)

# Configura a instalação
install(TARGETS hw RUNTIME DESTINATION bin)
