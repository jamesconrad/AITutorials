cmake_minimum_required(VERSION 2.8.9)

macro(make_absolute files)
	set (_out)
	foreach(file ${${files}})
		get_filename_component(file_abs ${file} ABSOLUTE)	
		LIST(APPEND _out ${file_abs})
	endforeach()
	set (${files} ${_out})
endmacro()

macro(make_definitions definitions)
	set (_out)
	foreach(definition ${${definitions}})
		LIST(APPEND _out -D${definition})
	endforeach()
	set (${definitions} ${_out})
endmacro()

######################################################################################################################### Create Variables

# We only want Debug and Release
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set (PLATFORM_WIN_DIR .)

set (EXT 				${PLATFORM_WIN_DIR}/external)
make_absolute(EXT)
set (ENGINE_INCLUDES 	${PLATFORM_WIN_DIR}/include)
make_absolute(ENGINE_INCLUDES)

# GLFW
set (GLFW 			${EXT}/glfw3)
set (GLFW_INCLUDES 	${GLFW}/include)
set (GLFW_LIBS_R	${GLFW}/bin/src/Release/glfw3.lib)
set (GLFW_LIBS_D	${GLFW}/bin/src/Debug/glfw3.lib)

# GL3W
set (GL3W_INCLUDES	${EXT}/gl3w/include)

# 
set (SOIL			${EXT}/soil)
set (SOIL_INCLUDES	${SOIL}/src)

# Loaders
set (TINYOBJLOADER ${EXT}/tinyobjloader)

# GLM
set (GLM 			${EXT}/GLM)
set (GLM_INCLUDES	${GLM}/include)

# IMGUI_SOURCES
set (IMGUI			${EXT}/imgui)

# CMake files
set (GENERATED_FILES_PATH 		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/generated/)
set (GENERATED_FILES_INCLUDES 	${GENERATED_FILES_PATH})

# CRT off
set (DEFINITIONS
	_CRT_SECURE_NO_WARNINGS
)
make_definitions(DEFINITIONS)

# Libraries for linker
set (LIBS
	optimized 	${GLFW_LIBS_R}
	debug 		${GLFW_LIBS_D}
	general		opengl32.lib
)

# Includes for directories
set (INCLUDES
	${GENERATED_FILES_INCLUDES}
	${GL3W_INCLUDES}
	${ENGINE_INCLUDES}
	${GLFW_INCLUDES}
	${SOIL_INCLUDES}
	${TINYOBJLOADER}
	${GLM_INCLUDES}
	${IMGUI}
)

######################################################################################################################### Create a solution

set (PROJ "INFR 4320U")

project(${PROJ})
set(CMAKE_BUILD_TYPE Release)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(_src_root_path "${CMAKE_SOURCE_DIR}/src")

######################################################################################################################### Create a GL3W library

file(
		GLOB GL3W_SOURCES 
		LIST_DIRECTORIES false
		"${EXT}/gl3w/include/GL/*.c*"
		"${EXT}/gl3w/include/GL/*.h*"
	)

add_library(gl3w ${GL3W_SOURCES})
set_target_properties(gl3w PROPERTIES FOLDER "Libraries")
set_target_properties(gl3w PROPERTIES DEBUG_POSTFIX "_debug" )
set_target_properties(gl3w PROPERTIES RELEASE_POSTFIX "" )
set_target_properties(gl3w PROPERTIES INCLUDE_DIRECTORIES "${GL3W_INCLUDES}")
set_target_properties(gl3w PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${EXT}/gl3w/build")

set (LIBS ${LIBS}
	debug "${EXT}/gl3w/build/Debug/gl3w_debug.lib"
	optimized "${EXT}/gl3w/build/Release/gl3w.lib"
	)

######################################################################################################################### Create an imgui library

file(
		GLOB IMGUI_SOURCES 
		LIST_DIRECTORIES false
		"${IMGUI}/*.c*"
		"${IMGUI}/*.h*"
		"${IMGUI}/examples/opengl3_example/imgui_impl_glfw_gl3.*"
	)

add_library(imgui ${IMGUI_SOURCES})
set_target_properties(imgui PROPERTIES FOLDER "Libraries")
set_target_properties(imgui PROPERTIES DEBUG_POSTFIX "_debug" )
set_target_properties(imgui PROPERTIES RELEASE_POSTFIX "" )
set_target_properties(imgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${IMGUI}/build")

set_target_properties(imgui PROPERTIES INCLUDE_DIRECTORIES "${INCLUDES}")
target_link_libraries(imgui	${LIBS})

add_dependencies(imgui gl3w)

file(
		GLOB SOIL_SOURCES 
		LIST_DIRECTORIES false
		"${SOIL}/src/*.c*"
		"${SOIL}/src/*.h*"
	)

add_library(soil ${SOIL_SOURCES})
set_target_properties(soil PROPERTIES FOLDER "Libraries")
set_target_properties(soil PROPERTIES DEBUG_POSTFIX "_debug" )
set_target_properties(soil PROPERTIES RELEASE_POSTFIX "" )
set_target_properties(soil PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${SOIL}/build")

set_target_properties(soil PROPERTIES INCLUDE_DIRECTORIES "${INCLUDES}")

set (LIBS ${LIBS}
	debug "${IMGUI}/build/Debug/imgui_debug.lib"
	optimized "${IMGUI}/build/Release/imgui.lib"
	debug "${SOIL}/build/Debug/soil_debug.lib"
	optimized "${SOIL}/build/Release/soil.lib"
	)
	
set (INCLUDES ${INCLUDES}
	"${IMGUI}"
	"${IMGUI}/examples/opengl3_example"
	)

######################################################################################################################### Add all the source and header files

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

MACRO(SETUPFOLDERS folder)
	# Use the macro to get subdirectorises
	SUBDIRLIST(DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/${folder})

	# Add labs - contained in main file
	foreach(CURR_DIR ${DIRS})

		file(
			GLOB_RECURSE DIR_SOURCES 
			LIST_DIRECTORIES false
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.c*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.h*"
		)
		
		file(
			GLOB_RECURSE DIR_SHADERS 
			LIST_DIRECTORIES false
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.shader*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.glsl*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.cg*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.hlsl*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.vert*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.frag*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.geom*"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.vs"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.gs"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}/*.fs"
		)
		
		set(OUTDIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/Builds/${folder}/${CURR_DIR}")
		
		add_executable("${CURR_DIR}" ${DIR_SOURCES} ${DIR_SHADERS})
		
		source_group("Shaders" FILES ${DIR_SHADERS})
		
		set_target_properties("${CURR_DIR}" PROPERTIES FOLDER ${folder})
		set_target_properties("${CURR_DIR}" PROPERTIES COMPILE_FLAGS ${DEFINITIONS})
		set_target_properties("${CURR_DIR}" PROPERTIES INCLUDE_DIRECTORIES "${INCLUDES}")
		set_target_properties("${CURR_DIR}" PROPERTIES DEBUG_POSTFIX "_debug" )
		set_target_properties("${CURR_DIR}" PROPERTIES RELEASE_POSTFIX "" )
		set_target_properties("${CURR_DIR}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTDIR})
		
		add_dependencies("${CURR_DIR}" imgui)
		add_dependencies("${CURR_DIR}" soil)
		
		# Assets file path
		target_compile_definitions("${CURR_DIR}" PUBLIC ASSETS="data/${folder}_${CURR_DIR}/")
				
		# Post build create and copy assets folder
		add_custom_command(TARGET "${CURR_DIR}" POST_BUILD 
			
			COMMAND (ROBOCOPY
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/bin/data/${folder}_${CURR_DIR}"
			/E /XF *.cpp *.c *.hpp *.h /njh /njs /ndl /nc /ns /np /nfl) ^& (ROBOCOPY
			"${CMAKE_CURRENT_SOURCE_DIR}/src/${folder}/${CURR_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/bin/Builds/${folder}/${CURR_DIR}/$(Configuration)/data/${folder}_${CURR_DIR}"
			/E /XF *.cpp *.c *.hpp *.h /njh /njs /ndl /nc /ns /np /nfl) ^& exit 0
		)
				
		# Link libraries to this project
		target_link_libraries("${CURR_DIR}"	${LIBS})
	endforeach(CURR_DIR)
ENDMACRO()

SUBDIRLIST(FILTERS ${CMAKE_CURRENT_SOURCE_DIR}/src/)

foreach(CURR_FILTER ${FILTERS})
	SETUPFOLDERS(${CURR_FILTER})
endforeach(CURR_FILTER)