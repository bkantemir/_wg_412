# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("pa")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(rootDir "../../../../../")
set(platform "${rootDir}/../p_android")
set(engine "${rootDir}/../engine")
set(modeler "${engine}/modeler")
set(subjs "${modeler}/subjs")
set(gltf "${engine}/gltf")
set(geom "${engine}/geom")
set(ui "${engine}/ui")
set(rr "${rootDir}/rr")

add_library( # Sets the name of the library.
        pa

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).

        ${rr}/WheelPair.cpp
        ${rr}/Gear.cpp
        ${rr}/Coupler.cpp
        ${rr}/OnRails.cpp
        ${rr}/RollingStock.cpp
        ${rr}/ModelLoaderRR.cpp
        ${rr}/Rail.cpp
        ${rr}/RailMap45.cpp
        ${rr}/Gangway.cpp
        ${rr}/RRUI.cpp
        ${rr}/ExhaustLoco.cpp

        ${ui}/ScreenFrame.cpp
        ${ui}/ButtonClose.cpp
        ${ui}/ButtonMap.cpp
        ${ui}/ButtonEdit.cpp
        ${ui}/ButtonMove.cpp
        ${ui}/ButtonUncouple.cpp
        ${ui}/UISubj.cpp
        ${ui}/ProgressBar.cpp
        ${ui}/TouchScreen.cpp
        ${ui}/ScreenLine.cpp
        ${ui}/ButtonTableDrag.cpp
        ${ui}/MeltingSign.cpp
        ${ui}/UIhint.cpp

        ${gltf}/GLTFanimation.cpp
        ${gltf}/GLTFskin.cpp
        ${gltf}/GLTFanim2nodesMap.cpp
        ${gltf}/GLTFskinShader.cpp
        ${gltf}/GLTFparser.cpp
        ${gltf}/GLTFnode.cpp

        ${subjs}/Fan.cpp
        ${subjs}/Interior.cpp
        ${subjs}/Switcher.cpp
        ${subjs}/DirLight.cpp
        ${subjs}/Smoke.cpp
        ${subjs}/Exhaust.cpp

        ${modeler}/CoordOnLine.cpp
        ${modeler}/TexCoords.cpp
        ${modeler}/ModelBuilder.cpp
        ${modeler}/ModelBuilder1base.cpp
        ${modeler}/GroupTransform.cpp
        ${modeler}/ModelLoader.cpp
        ${modeler}/ModelLoaderCmd.cpp

        ${geom}/ArcXY.cpp
        ${geom}/LineXY.cpp
        ${geom}/MyPolygonRib.cpp
        ${geom}/MyPolygon.cpp
        ${geom}/Line3D.cpp
        ${geom}/Gabarites.cpp
        ${geom}/Chorda.cpp

        # ${engine}/stb_image.h
        ${engine}/MySound.cpp 
        ${engine}/TheTable.cpp
        ${engine}/SceneSubj.cpp
        ${engine}/XMLparser.cpp
        ${engine}/Coords.cpp
        ${engine}/Camera.cpp
        ${engine}/CameraMan.cpp
        ${engine}/MyColor.cpp
        ${engine}/Texture.cpp
        ${engine}/Shadows.cpp
        ${engine}/DrawJob.cpp
        ${engine}/FileLoader.cpp
        ${engine}/Shader.cpp
        ${engine}/Material.cpp
        ${engine}/MaterialAdjust.cpp
        ${engine}/ui/UISubj.cpp
        ${engine}/ui/ProgressBar.cpp
        ${engine}/utils.cpp
        ${platform}/platform.cpp
        ${platform}/input.cpp

        ${rootDir}/model_car/CarWheel.cpp
        ${rootDir}/TheApp.cpp
        main.cpp
        )
target_include_directories(pa
        PUBLIC
        ${rootDir}
        ${platform}
        ${engine}
        ${modeler}
        ${ui}
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Searches for a package provided by the game activity dependency

find_package(game-activity REQUIRED CONFIG)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        pa

        android

        # The game activity
        game-activity::game-activity

        # EGL, required for configuring the display context
        EGL

        # GL ES 3, used for the sample renderer
        GLESv3

        # for AImageDecoder, to load images from resources
        jnigraphics

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
