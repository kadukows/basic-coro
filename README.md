# basic-coro - c++ coroutine library
Library that implements helper types for using c++ coroutines. Please be aware that this is a training project for me - I wanted to learn more about CMake, gtest and git submodules.

## Usage
### Prerequisites
* g++-10

### Installing
```
mkdir build && cd build
cmake -D CMAKE_CXX_COMPILER=g++-10 ..
make install
```
This will install appropriate headers into `./include/` and static linked library into `./lib/`.

## Acknowledgments
* [CMake C++ Project Template](https://github.com/kigster/cmake-project-template) as this project is based on this template
* Lewis Baker has excellent [articles](https://lewissbaker.github.io/) on topic of coroutines and assymetric transfer. This project is mostly based on information (and code snippets) contained in those articles.
