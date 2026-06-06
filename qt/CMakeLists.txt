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

# libresidfp 2.x backend (vendored under src/residfp/)
set(RESIDFP_SRC
    ${SRC_DIR}/residfp/Dac.cpp
    ${SRC_DIR}/residfp/EnvelopeGenerator.cpp
    ${SRC_DIR}/residfp/ExternalFilter.cpp
    ${SRC_DIR}/residfp/Filter.cpp
    ${SRC_DIR}/residfp/Filter6581.cpp
    ${SRC_DIR}/residfp/Filter8580.cpp
    ${SRC_DIR}/residfp/FilterModelConfig.cpp
    ${SRC_DIR}/residfp/FilterModelConfig6581.cpp
    ${SRC_DIR}/residfp/FilterModelConfig8580.cpp
    ${SRC_DIR}/residfp/Integrator6581.cpp
    ${SRC_DIR}/residfp/Integrator8580.cpp
    ${SRC_DIR}/residfp/OpAmp.cpp
    ${SRC_DIR}/residfp/SID.cpp
    ${SRC_DIR}/residfp/Spline.cpp
    ${SRC_DIR}/residfp/version.cc
    ${SRC_DIR}/residfp/WaveformCalculator.cpp
    ${SRC_DIR}/residfp/WaveformGenerator.cpp
    ${SRC_DIR}/residfp/resample/SincResampler.cpp
    ${SRC_DIR}/residfp/residfp/residfp.cpp
)
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
    OrderMiniMap.cpp
    InstrumentQuickList.cpp
    StatusStrip.cpp
    UndoStack.cpp
    Rpc.cpp
    qt_globals.c
    qt_stubs.c
)

set(QT_HEADERS
    Theme.h
    SdlKeyMap.h
)

add_executable(goattrk2-qt
    ${QT_SRC}
    ${CORE_SRC}
    ${RESIDFP_SRC}
    ${ASM_SRC}
    ${BME_SRC}
)

target_include_directories(goattrk2-qt PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bme
    ${SRC_DIR}/asm
    ${SRC_DIR}/residfp
    ${SRC_DIR}/residfp/residfp
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

# ---------------------------------------------------------------------------
# goattrk2-qt-stereo — dual-SID / 6-channel variant.
#
# Same source tree as the mono build, conditionally compiled via the
# GOATTRK2_STEREO define. gcommon.h widens MAX_CHN to 6, gsid.cpp keeps a
# second reSIDfp::residfp alongside the first, and sid_getlevels reports
# levels for all six voices. Audio path is currently mono-output of SID1
# only — SID2 envelopes show up in the VU meters and the editor sees 6
# channels, but sound rendering still single SID. That gap is the next
# follow-up commit. Stereo songs that contain channel-4..6 data will
# survive a round-trip through load+save.
# ---------------------------------------------------------------------------

add_executable(goattrk2-qt-stereo
    ${QT_SRC}
    ${CORE_SRC}
    ${RESIDFP_SRC}
    ${ASM_SRC}
    ${BME_SRC}
)

target_include_directories(goattrk2-qt-stereo PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bme
    ${SRC_DIR}/asm
    ${SRC_DIR}/residfp
    ${SRC_DIR}/residfp/residfp
    ${SDL_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
)

target_compile_options(goattrk2-qt-stereo PRIVATE
    -fcommon
    -Wno-write-strings
    -Wno-narrowing
    ${SDL_CFLAGS_OTHER}
    ${JACK_CFLAGS_OTHER}
)

target_compile_definitions(goattrk2-qt-stereo PRIVATE GOATTRK2_STEREO=1)

target_link_libraries(goattrk2-qt-stereo PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    ${SDL_LIBRARIES}
    ${JACK_LIBRARIES}
    m pthread
)

# ---------------------------------------------------------------------------
# gt2reloc — standalone commandline relocator/packer (backport from v2.73)
#
# Plain CLI tool, no Qt. Reuses the core tracker objects (gsong, greloc, ...)
# plus the bme and reSIDfp backends so the same loadsong() / relocator() code
# paths run headlessly. The GT2RELOC #define stubs out the interactive UI
# code paths inside greloc.c.
# ---------------------------------------------------------------------------

set(GT2RELOC_SRC
    ${SRC_DIR}/gt2reloc.c
    ${SRC_DIR}/goatdata.c
    ${SRC_DIR}/gsong.c
    ${SRC_DIR}/gorder.c
    ${SRC_DIR}/gpattern.c
    ${SRC_DIR}/ginstr.c
    ${SRC_DIR}/gtable.c
    ${SRC_DIR}/gplay.c
    ${SRC_DIR}/gdisplay.c
    ${SRC_DIR}/gfile.c
    ${SRC_DIR}/ghelp.c
    ${SRC_DIR}/gsound.c
    ${SRC_DIR}/gconsole.c
    ${SRC_DIR}/gsid.cpp
)

set(BME_FULL_SRC
    ${SRC_DIR}/bme/bme.c
    ${SRC_DIR}/bme/bme_snd.c
    ${SRC_DIR}/bme/bme_io.c
    ${SRC_DIR}/bme/bme_end.c
    ${SRC_DIR}/bme/bme_gfx.c
    ${SRC_DIR}/bme/bme_win.c
    ${SRC_DIR}/bme/bme_mou.c
    ${SRC_DIR}/bme/bme_kbd.c
)

# goatdata.c is a generated data blob from the upstream build and is not
# vendored here, so only configure gt2reloc when it is actually present.
if(EXISTS ${SRC_DIR}/goatdata.c)
add_executable(gt2reloc
    ${GT2RELOC_SRC}
    ${RESIDFP_SRC}
    ${ASM_SRC}
    ${BME_FULL_SRC}
)

target_include_directories(gt2reloc PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bme
    ${SRC_DIR}/asm
    ${SRC_DIR}/residfp
    ${SRC_DIR}/residfp/residfp
    ${SDL_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
)

target_compile_options(gt2reloc PRIVATE
    -fcommon
    -fgnu89-inline
    -Wno-write-strings
    -Wno-narrowing
    ${SDL_CFLAGS_OTHER}
)

target_link_libraries(gt2reloc PRIVATE
    ${SDL_LIBRARIES}
    ${JACK_LIBRARIES}
    m pthread
)
endif()
