cmake_minimum_required(VERSION 3.5)

project(envtools)

# Setting custom modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(Boost COMPONENTS system)

set(Boost_USE_STATIC_RUNTIME OFF)

find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package(TBB)
find_package(PNG)
find_package(TIFF)
find_package(JPEG)

find_package(OpenImageIO)

include_directories(${OIIO_INCLUDE_DIR})
include_directories(${TIFF_INCLUDE_DIR})
include_directories(${TBB_INCLUDE_DIR})

# envremap
add_executable(envremap envremap.cpp)
target_link_libraries(envremap ${PNG_LIBRARY} ${TIFF_LIBRARY} ${JPEG_LIBRARY})

install(TARGETS envremap
	RUNTIME DESTINATION bin
)

# # envIrradiance
add_executable(envIrradiance envIrradiance.cpp Cubemap.cpp)
target_link_libraries(envIrradiance ${TBB_LIBRARIES} ${PNG_LIBRARY} ${TIFF_LIBRARY} ${JPEG_LIBRARY} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS envIrradiance
	RUNTIME DESTINATION bin
)

# cubemapPacker
add_executable(cubemapPacker cubemapPacker.cpp  Cubemap.cpp)
target_link_libraries(cubemapPacker ${TBB_LIBRARIES} ${PNG_LIBRARY} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS cubemapPacker
	RUNTIME DESTINATION bin
)

# extractLights
add_executable(extractLights extractLightsVariance.cpp SummedAreaTable.cpp SummedAreaTableRegion.cpp)
target_link_libraries(extractLights ${TBB_LIBRARIES} ${PNG_LIBRARY} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS extractLights
	RUNTIME DESTINATION bin
)

# # envPrefilter
add_executable(envPrefilter envPrefilter.cpp Cubemap.cpp)
target_link_libraries(envPrefilter ${TBB_LIBRARIES} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS envPrefilter
	RUNTIME DESTINATION bin
)

# envBackground
add_executable(envBackground envBackground.cpp Cubemap.cpp)
target_link_libraries(envBackground ${TBB_LIBRARIES} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS envBackground
	RUNTIME DESTINATION bin
)

# samplesGGX
add_executable(samplesGGX samplesGGX.cpp Cubemap.cpp)
target_link_libraries(samplesGGX ${TBB_LIBRARIES} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS samplesGGX
	RUNTIME DESTINATION bin
)

# envBRDF
add_executable(envBRDF envBRDF.cpp)
target_link_libraries(envBRDF ${TBB_LIBRARIES} ${OIIO_LIBRARY})

install(TARGETS envBRDF
	RUNTIME DESTINATION bin
)

# panoramaPacker
add_executable(panoramaPacker panoramaPacker.cpp)
target_link_libraries(panoramaPacker ${TBB_LIBRARIES} ${OIIO_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS panoramaPacker
	RUNTIME DESTINATION bin
)

# entry: process_environment
install(PROGRAMS process_environment.py prefilter_opencl.py ggx.cl 
	DESTINATION bin
)

# file(COPY testData DESTINATION .)
# file(COPY specularCubemapTest.sh DESTINATION .)
# file(COPY extractLightsTest.py DESTINATION .)
# file(COPY process_environment.py DESTINATION .)
# file(COPY prefilter_opencl.py DESTINATION .)
# file(COPY ggx.cl DESTINATION .)

# # check
# add_custom_target(check)

# add_custom_command(TARGET check
# 	POST_BUILD
# 	COMMAND ./specularCubemapTest.sh
# 	COMMAND ./extractLightsTest.py
# )
