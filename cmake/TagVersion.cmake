function(createTagVersion)

    find_package(Git)
    if (GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "[0-9]*\\.[0-9]*\\.[0-9]*" OUTPUT_VARIABLE FULL_TAG_VERSION)
        message(STATUS "Tag version ${FULL_TAG_VERSION}")
        string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" VERSION_MAJOR "${FULL_TAG_VERSION}")
        string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${FULL_TAG_VERSION}")
        string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${FULL_TAG_VERSION}")
        string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-(.*)\n" "\\1" VERSION_TWEAK "${FULL_TAG_VERSION}")

        if ("${VERSION_TWEAK}" STREQUAL "${FULL_TAG_VERSION}")
            # git describe output was X.Y.Z
            set(VERSION_TWEAK "0")
        elseif ("${VERSION_TWEAK}" STREQUAL "dirty")
            # git describe output was X.Y.Z-dirty
            set(VERSION_TWEAK "0+dirty")
        else ()
            # git describe output was X.Y.Z-1-gsha[-dirty]
            # PEP 440 local version requires the tweak string to look like 1+gsha[.dirty]
            string(REGEX REPLACE "^([0-9]+)-(.*)" "\\1+\\2" VERSION_TWEAK "${VERSION_TWEAK}")
            string(REPLACE "-dirty" ".dirty" VERSION_TWEAK "${VERSION_TWEAK}")
        endif ()

        if ("${VERSION_MAJOR}" STREQUAL "" OR "${VERSION_MINOR}" STREQUAL "" OR "${VERSION_PATCH}" STREQUAL ""
                OR "${VERSION_TWEAK}" STREQUAL "")
            set(VERSION_MAJOR "0" PARENT_SCOPE)
            set(VERSION_MINOR "0" PARENT_SCOPE)
            set(VERSION_PATCH "0" PARENT_SCOPE)
            set(VERSION_TWEAK "0" PARENT_SCOPE)
        else ()
            set(VERSION_MAJOR "${VERSION_MAJOR}" PARENT_SCOPE)
            set(VERSION_MINOR "${VERSION_MINOR}" PARENT_SCOPE)
            set(VERSION_PATCH "${VERSION_PATCH}" PARENT_SCOPE)
            set(VERSION_TWEAK "${VERSION_TWEAK}" PARENT_SCOPE)
        endif ()

    else ()
        set(VERSION_MAJOR "0" PARENT_SCOPE)
        set(VERSION_MINOR "0" PARENT_SCOPE)
        set(VERSION_PATCH "0" PARENT_SCOPE)
        set(VERSION_TWEAK "0" PARENT_SCOPE)
    endif ()

endfunction()
