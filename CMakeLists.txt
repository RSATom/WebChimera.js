cmake_minimum_required( VERSION 2.8 )

project( WebChimera.js )

if( MSVC )
    foreach( flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
        if( ${flag_var} MATCHES "/MD" )
            string( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
        endif()
    endforeach()
else()
    add_definitions( -std=c++11 )
endif()

file( GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/[^.]*.cpp
    src/[^.]*.h
    README.md
    package.json
    )

include_directories( ${CMAKE_JS_INC} deps )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/deps/libvlc_wrapper )

add_library( ${PROJECT_NAME} SHARED ${SOURCE_FILES} )
set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node" )
target_link_libraries( ${PROJECT_NAME} ${CMAKE_JS_LIB} libvlc_wrapper )

#get_cmake_property( _variableNames VARIABLES )
#foreach( _variableName ${_variableNames} )
#    message( STATUS "${_variableName}=${${_variableName}}" )
#endforeach()
