
set(TRIG_INCLUDE_DIR "/usr/include/trigonometry")
set(TRIG_LIB_DIR "/usr/lib/trigonometry")

file(GLOB TRIG_FOLDERS RELATIVE "${TRIG_INCLUDE_DIR}" "${TRIG_INCLUDE_DIR}/*")
foreach(folder ${TRIG_FOLDERS})
    if(IS_DIRECTORY "${TRIG_INCLUDE_DIR}/${folder}")
        set(libfile "${TRIG_LIB_DIR}/lib${folder}.so")
        
        # Only add target if the library exists
        if(EXISTS "${libfile}")
            add_library(Trig::${folder} SHARED IMPORTED)
            set_target_properties(Trig::${folder} PROPERTIES
                IMPORTED_LOCATION "${libfile}"
                INTERFACE_INCLUDE_DIRECTORIES "${TRIG_INCLUDE_DIR}/${folder}"
            )
            message(STATUS "Registered Trig module: ${folder}")
        else()
            message(WARNING "Library for folder '${folder}' not found: ${libfile}")
        endif()
    endif()
endforeach()

#    /--------------------------
#
#                                            <zynomon@proton.me>
#                       add,
#  
#                                     include(/usr/lib/cmake/Trig/TrigConfig.cmake)
#  
#                                                                                                in your cmake
#
#                                                                                                            -----------------------/