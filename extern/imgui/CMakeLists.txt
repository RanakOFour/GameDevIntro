file(GLOB CORE_SRCS *.cpp)
file(GLOB stringSupport_SRCS misc/cpp/*.cpp)

target_sources(RanakEngine PUBLIC ${CORE_SRCS} ${stringSupport_SRCS})