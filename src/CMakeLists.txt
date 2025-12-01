INCLUDE ("${CMAKE_SOURCE_DIR}/cmake-common/sac2c-variables.cmake")
INCLUDE ("${CMAKE_SOURCE_DIR}/cmake-common/generate-sac2c-dependency-targets.cmake")

ADD_CUSTOM_TARGET (${TARGET}-all-modules ALL)

# C files
SET (C_DEPS_SRC
    src/JPEG/jpeg.c
    src/PGM/array2pgm.c
    src/PGM/pgm2array.c
    src/PNG/array2png.c
    src/PNG/imshow.c
    src/PNG/png2array.c
    src/PPM/array2ppm.c
    src/PPM/ppm2array.c
)

# SaC files
SET (SAC_SRC
    JPEG.sac
    PGM.sac
    PNG.sac
    PPM.sac
)

# For every C source, compile an object file maintaining the right
# location in the binary directory so that SaC can pick it up
FOREACH (name ${C_DEPS_SRC})
    SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")

    GET_FILENAME_COMPONENT (dir ${name} DIRECTORY)
    GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
    SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${dst}${OBJEXT}")

    # Put the object file in the same location as the source file
    FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")

    ADD_CUSTOM_COMMAND (
        OUTPUT "${dst}"
        MAIN_DEPENDENCY "${src}"
        IMPLICIT_DEPENDS C "${src}"
        COMMAND
            ${SAC2C} -Xp -I${CMAKE_CURRENT_SOURCE_DIR}/${dir}
                    -Xp -I${CMAKE_CURRENT_BINARY_DIR}/${dir}
                    ${SAC2C_CC_FLAGS}
                    -v0 -noprelude -cc ccrmod -o "${dst}" "${src}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
        COMMENT "Generating ${dst} from ${src} for target ${TARGET}")
ENDFOREACH ()

# Make a directory for sac2c output
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")

# This functions tries to match a module name with its stored path information
# in a list. It returns on the first result, or returns empty.
FUNCTION (MATCH_NAME_IN_LIST list name ret)
    MESSAGE (DEBUG "Searching for ${name} in list")
    FOREACH (item ${list})
        IF ("${item}" MATCHES "${name}\.(sac|xsac)")
            MESSAGE (DEBUG "Found a match: ${item}")
            SET (${ret} ${item} PARENT_SCOPE)
            BREAK ()
        ENDIF ()
    ENDFOREACH ()
ENDFUNCTION ()

# For every SaC file, compile Tree and Mod files.
FOREACH (name ${SAC_SRC})
    SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")

    # sac2c requires object files relative to the working directory of the call to sac2c
    GET_FILENAME_COMPONENT (dir "${CMAKE_CURRENT_BINARY_DIR}/${name}" DIRECTORY)
    GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
    SET (mod "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/lib${dst}Mod${VARIANT}${MODEXT}")
    SET (tree "${DLL_BUILD_DIR}/tree/${TARGET_ENV}/${SBI}/lib${dst}Tree${VARIANT}${TREE_DLLEXT}")

    RESOLVE_SAC_DEPS_AS_TARGETS ("${name}" "<TARGET>-module-<NAME>" target_list obj_list src_list)
    MESSAGE (DEBUG "Targets ${name}: ${target_list}")
    MESSAGE (DEBUG "Objects ${name}: ${obj_list}")
    MESSAGE (DEBUG "Sources ${name}: ${src_list}")
    MESSAGE (STATUS "Computing dependencies of ${name}")

    SET (src_deps)
    FOREACH (src_name ${src_list})
        MESSAGE (DEBUG "Searching for ${src_name} in source path list")
        MATCH_NAME_IN_LIST ("${SAC_SRC}" "${src_name}" src_path)
        IF (src_path)
            LIST (APPEND src_deps "${CMAKE_CURRENT_SOURCE_DIR}/${src_path}")
        ELSE ()
            MESSAGE (STATUS "Unable to find path to ${src_name} in sources")
        ENDIF ()
    ENDFOREACH ()
    LIST (APPEND deps_list ${src_deps} ${obj_list} ${target_list})
    UNSET (src_deps)
    UNSET (src_list)
    UNSET (obj_list)
    UNSET (target_list)
    MESSAGE (STATUS "Dependencies of ${name} are: ${deps_list}")
    # The current system cannot find Stdlib files. This is a really ugly solution:
    # we manually remove known Stdlib dependencies, and assume they exist.
    # I don't know how to fix this...
    LIST(FILTER deps_list EXCLUDE REGEX ".*(Array|Color8|File|FileSystem|Process|RuntimeError|ScalarArith|String|SysErr|TermFile|Terminal|World)")

    # Make sure that we have the directory we are changing to.
    FILE (MAKE_DIRECTORY "${dir}")

    ADD_CUSTOM_TARGET (${TARGET}-module-${dst} DEPENDS "${mod}" "${tree}")
    ADD_DEPENDENCIES (${TARGET}-all-modules ${TARGET}-module-${dst})

    ADD_CUSTOM_COMMAND (
        OUTPUT ${mod} ${tree}
        COMMAND ${SAC2C} -v0 ${SAC2C_CC_FLAGS} -o ${DLL_BUILD_DIR} "${src}"
        WORKING_DIRECTORY "${dir}"
        MAIN_DEPENDENCY "${src}"
        DEPENDS ${deps_list}
        COMMENT "Building module ${dst} for target ${TARGET}")

    # Install compiled Tree/Mod parts to the corresponding locations
    INSTALL (
        FILES "${mod}"
        DESTINATION ${_install_mod_dir}/${TARGET_ENV}/${SBI}
        COMPONENT modules)
    INSTALL (
        FILES "${tree}"
        DESTINATION ${_install_tree_dir}/tree/${TARGET_ENV}/${SBI}
        COMPONENT trees)

    UNSET (deps_list)
ENDFOREACH ()
