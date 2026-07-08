cmake_minimum_required(VERSION 3.10)

project(rknn_yolov8_pose_demo)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

if (ENABLE_ASAN)
	message(STATUS "BUILD WITH ADDRESS SANITIZER")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif ()

set(rknpu_yolov8-pose_file rknpu2/yolov8-pose.cc)
if (TARGET_SOC STREQUAL "rv1106" OR TARGET_SOC STREQUAL "rv1103")
    add_definitions(-DRV1106_1103)
    set(rknpu_yolov8-pose_file rknpu2/yolov8-pose_rv1106_1103.cc)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/allocator/dma)
elseif(TARGET_SOC STREQUAL "rk1808" OR TARGET_SOC STREQUAL "rv1109" OR TARGET_SOC STREQUAL "rv1126")
    add_definitions(-DRKNPU1)
    set(rknpu_yolov8-pose_file rknpu1/yolov8-pose.cc)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/ 3rdparty.out)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../utils/ utils.out)

set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")

# # ========== YOLOv5 目标检测进程 ==========
#     yolov5_postprocess.cc
#     rknpu2_yolov5/yolov5.cc
# )

# # 设置链接库
# target_link_libraries(object_detector
#     imageutils      # 来自 utils.out
#     fileutils       # 来自 utils.out
#     imagedrawing    # 来自 utils.out
#     /usr/lib/librknnrt.so
#     pthread
#     rt
#     m
# )

# # 设置头文件搜索路径
# target_include_directories(object_detector PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}                # 当前目录，用于找 yolov5_postprocess.h 等
#     ${CMAKE_CURRENT_SOURCE_DIR}/rknpu2_yolov5  # 找 yolov5.cc 中可能引用的内部头文件
#     ${CMAKE_SOURCE_DIR}/utils                  # 关键！添加utils目录，让编译器能找到 common.h 和 image_utils.h
# )

# 摄像头采集进程
add_executable(gst_producer
    gst_cam.c
)
target_link_libraries(gst_producer
    ${GST_LIBRARIES}
    pthread
    rt
    m
    rga
)
target_include_directories(gst_producer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GST_INCLUDE_DIRS}
)

# 疲劳检测进程
add_executable(behavior_engine
    behavior_engine.cc
    postprocess.cc
    retinaface.cc
    ${rknpu_yolov8-pose_file}
)
target_link_libraries(behavior_engine
    imageutils
    fileutils
    imagedrawing
    #${LIBRKNNRT}
    /usr/lib/librknnrt.so 
    dl
    m
    pthread
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(behavior_engine Threads::Threads)
endif()
target_include_directories(behavior_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBRKNNRT_INCLUDES}
)

# UI显示进程
add_executable(ui_display
    ui_display.c
)
target_link_libraries(ui_display
    imageutils
    fileutils
    imagedrawing
    pthread
    rt
    m
    rga
)
target_include_directories(ui_display PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 温湿度监控进程（使用 i2ctransfer 命令，不依赖 LibDriver）
add_executable(env_monitor env_monitor.c)
target_link_libraries(env_monitor pthread m)

# 风扇控制进程
add_executable(fan_control
    fan/fan_control.c
    fan/pid_controller.c
)

# 如果需要链接数学库和线程库
target_link_libraries(fan_control
    m
    pthread
)

# （可选）指定头文件搜索路径，如果 pid_controller.h 中有额外包含
target_include_directories(fan_control PRIVATE fan)
target_include_directories(fan_control PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# 安装
install(TARGETS behavior_engine ui_display DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../model/bus.jpg DESTINATION ./model)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../model/yolov8_pose_labels_list.txt DESTINATION ./model)
file(GLOB RKNN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../model/*.rknn")
install(FILES ${RKNN_FILES} DESTINATION model)
