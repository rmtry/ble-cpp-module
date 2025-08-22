cmake_minimum_required(VERSION 3.20)
project(ble_cpp_macos LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

# Your library sources
add_library(ble_core
    src/core/BleClient.cpp
    src/core/EventBus.cpp
    src/core/Errors.cpp
  # ... add remaining core files
)
target_include_directories(ble_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Apple adapter (ObjC++)
add_library(ble_adapter_apple
  macos/BleAdapter.mm
)
target_include_directories(ble_adapter_apple PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ble_adapter_apple PRIVATE "-framework Foundation" "-framework CoreBluetooth")

# Test app (as a macOS bundle so Info.plist is embedded)
add_executable(TestBle MACOSX_BUNDLE macos/TestBle.mm)
target_link_libraries(TestBle PRIVATE ble_core ble_adapter_apple "-framework Foundation" "-framework CoreBluetooth")
set_target_properties(TestBle PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/macos/Info.plist"
)
