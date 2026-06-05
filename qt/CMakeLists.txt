cmake_minimum_required(VERSION 3.16)
project(goattrk2-qt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL REQUIRED sdl)
pkg_check_modules(JACK REQUIRED jack)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Core data + edit logic (no rendering calls verified by grep)
set(CORE_SRC
    ${SRC_DIR}/gsong.c
    ${SRC_DIR}/gorder.c
    ${SRC_DIR}/gpattern.c
    ${SRC_DIR}/ginstr.c
    ${SRC_DIR}/gtable.c
    ${SRC_DIR}/gplay.c
    ${SRC_DIR}/gsound.c
    ${SRC_DIR}/gsid.cpp
)

file(GLOB RESID_SRC ${SRC_DIR}/resid/*.cpp)
list(REMOVE_ITEM RESID_SRC ${SRC_DIR}/resid/version.cpp)
file(GLOB RESIDFP_SRC ${SRC_DIR}/resid-fp/*.cpp)
list(REMOVE_ITEM RESIDFP_SRC ${SRC_DIR}/resid-fp/versionfp.cpp)
file(GLOB ASM_SRC ${SRC_DIR}/asm/*.c)

set(BME_SRC
    ${SRC_DIR}/bme/bme.c
    ${SRC_DIR}/bme/bme_snd.c
    ${SRC_DIR}/bme/bme_io.c
    ${SRC_DIR}/bme/bme_end.c
)

set(QT_SRC
    main.cpp
    MainWindow.cpp
    PatternView.cpp
    OrderView.cpp
    InstrumentView.cpp
    TablesView.cpp
    SongNameView.cpp
    qt_globals.c
    qt_stubs.c
)

add_executable(goattrk2-qt
    ${QT_SRC}
    ${CORE_SRC}
    ${RESID_SRC}
    ${RESIDFP_SRC}
    ${ASM_SRC}
    ${BME_SRC}
)

target_include_directories(goattrk2-qt PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bme
    ${SRC_DIR}/asm
    ${SDL_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
)

target_compile_options(goattrk2-qt PRIVATE
    -fcommon
    -Wno-write-strings
    -Wno-narrowing
    ${SDL_CFLAGS_OTHER}
    ${JACK_CFLAGS_OTHER}
)

target_link_libraries(goattrk2-qt PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    ${SDL_LIBRARIES}
    ${JACK_LIBRARIES}
    m pthread
)
