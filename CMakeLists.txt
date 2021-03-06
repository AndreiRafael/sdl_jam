cmake_minimum_required(VERSION 3.1)

set(GAME_TITLE sdl_jam)

project(${GAME_TITLE} C)

#debug/errorlog on/off
add_definitions(-DHFE_DEBUG)

#output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(${GAME_TITLE} ./src/main.c)
else()
	if(WIN32)
		add_executable(${GAME_TITLE} WIN32 ./src/main.c)
	else()
		add_executable(${GAME_TITLE} ./src/main.c)
	endif()
endif()

# Make compiler scream out every possible warning
if(MSVC)
	target_compile_options(${GAME_TITLE} PRIVATE /W4 /WX)
else()
	target_compile_options(${GAME_TITLE} PRIVATE -Wconversion -Wall -Wextra -Wpedantic -Werror)
endif()

# Add sources from Sourcefile (snip from Sonic Robo Blast 2)
function(target_sourcefile file prjct)
	file(STRINGS ${file} list)
	list(TRANSFORM list PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
	target_sources(${prjct} PRIVATE ${list})
endfunction()

#includes
include_directories(${GAME_TITLE} PUBLIC ${CMAKE_SOURCE_DIR}/include)

#libs
#sdl
if(WIN32)
	if(MSVC)
		#SDL link
		target_link_libraries(${GAME_TITLE} PUBLIC
			${CMAKE_SOURCE_DIR}/lib/sdl/msvc/SDL2main.lib
			${CMAKE_SOURCE_DIR}/lib/sdl/msvc/SDL2.lib
		)#target_link_libraries
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl/msvc/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) 

		# TODO: SDL IMG LINK
	else()
		#SDL link
		target_link_libraries(${GAME_TITLE} PUBLIC
			mingw32 ${CMAKE_SOURCE_DIR}/lib/sdl/mingw/libSDL2.dll.a
		)#target_link_libraries
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl/mingw/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		
		#SDL_IMG link
		target_link_libraries(${GAME_TITLE} PUBLIC
			mingw32 ${CMAKE_SOURCE_DIR}/lib/sdl_img/mingw/libSDL2_image.dll.a
		)#target_link_libraries
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl_img/mingw/SDL2_image.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl_img/mingw/libpng16-16.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl_img/mingw/LICENSE.png.txt DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl_img/mingw/zlib1.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl_img/mingw/LICENSE.zlib.txt DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	endif()
else()
	target_link_libraries(${GAME_TITLE} PUBLIC ${CMAKE_DL_LIBS} m pthread)
	find_package(SDL2 REQUIRED)
	target_link_libraries(${GAME_TITLE} PUBLIC ${SDL2_LIBRARIES})
endif()

#copy all resources
file(GLOB_RECURSE resources ${CMAKE_SOURCE_DIR}/res/*.*)
file(COPY ${resources} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)

add_subdirectory(./src)