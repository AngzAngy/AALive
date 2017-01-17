# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

ADD_DEFINITIONS(
-g -W
-O3 -D__STDC_CONSTANT_MACROS
)

include_directories( # Specifies a path to header files
                     src/main/cpp/ffmpeg/include/
                     )

link_directories(# Specifies a path to external libraries
                  src/main/cpp/ffmpeg/lib/
                  )
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             aalive

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/H264Encoder.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              zip-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              z )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       aalive

                       # Links the target library to the log library
                       # included in the NDK.
                       "-Wl,--whole-archive"
                       ${log-lib}
                       ${zip-lib}
                       libavutil.a
                       libswscale.a
                       libswresample.a
                       libavcodec.a
                       libavformat.a
                       libavfilter.a
                       "-Wl,--no-whole-archive"
                          )