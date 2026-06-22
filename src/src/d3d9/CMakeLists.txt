# CMakeLists.txt for newD3d9.dll — D3D9 proxy for War3
#
# This is a separate DLL from yd_japi.dll. It acts as a D3D9 proxy
# that War3 loads instead of the real d3d9.dll.
#
# Output: newD3d9.dll

set(D3D9_SOURCES
    src/d3d9/dllmain.cpp
    src/d3d9/CDirect3D9.cpp
    src/d3d9/CDirect3DDevice9.cpp
    src/d3d9/renderer.cpp
    src/d3d9/api.cpp
)

add_library(newD3d9 SHARED ${D3D9_SOURCES})

target_include_directories(newD3d9 PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/d3d9"
)

target_compile_definitions(newD3d9 PRIVATE
    WIN32
    _WINDOWS
    _CRT_SECURE_NO_WARNINGS
)

# Link against d3d9.lib only (import library for d3d9.dll)
# D3DX9 functions are loaded dynamically via LoadLibrary in renderer.cpp
target_link_libraries(newD3d9 PRIVATE
    d3d9
)

if(MSVC)
    target_compile_options(newD3d9 PRIVATE /W3 /EHsc)
    target_link_options(newD3d9 PRIVATE /SAFESEH:NO)
endif()

set_target_properties(newD3d9 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    PREFIX ""
    SUFFIX ".dll"
)
