if(NOT GETTEXT_FOUND)
    message(FATAL_ERROR "gettext is not found!")
endif()

function(generatePoFile MODULE_NAME SOURCE_DIR SOURCES)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/locale)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/locale/${LOCALE})

    # Construct the .pot file path
    set(POT_FILE "${LOCALE_DIR}/${MODULE_NAME}.pot")
    set(PO_FILE "${LOCALE_DIR}/${LOCALE}/${MODULE_NAME}.po")

    set(FULL_PATH)
    foreach(file IN LISTS SOURCES)
        list(APPEND FULL_PATH "${SOURCE_DIR}/${file}")
    endforeach()

    # Construct the xgettext command
    add_custom_command(
        OUTPUT POT_DUMMY
        COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -k__ -o ${POT_FILE} --from-code=UTF-8 ${FULL_PATH} --no-location
        DEPENDS ${FULL_PATH}
        COMMENT "Generating ${POT_FILE} from ${FULL_PATH}"
    )
    # Create a custom target for generating the .pot file
    add_custom_target(${MODULE_NAME}_pot ALL DEPENDS POT_DUMMY)

    add_custom_command(
        OUTPUT PO_DUMMY
        COMMAND ${CMAKE_COMMAND} 
                -DFILE_TO_CHECK=${PO_FILE}
                -DPO_FILE=${PO_FILE}
                -DPOT_FILE=${POT_FILE}
                -P ${CMAKE_SOURCE_DIR}/cmake/PotFileToPoFile.cmake
        DEPENDS ${MODULE_NAME}_pot
        COMMENT "Checking file existence and executing appropriate command"
    )
    add_custom_target(${MODULE_NAME}_po ALL DEPENDS PO_DUMMY)

    # remove data so that only content change will trigger git tracking
if (APPLE)
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND sed -i '' '/POT-Creation-Date:/d' ${PO_FILE}    # macos needs an extra ''
        COMMAND sed -i '' '/PO-Revision-Date:/d' ${PO_FILE}
        DEPENDS ${MODULE_NAME}_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
elseif(LINUX)
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND sed -i '/POT-Creation-Date:/d' ${PO_FILE}
        COMMAND sed -i '/PO-Revision-Date:/d' ${PO_FILE}
        DEPENDS ${MODULE_NAME}_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
else()
    # WINDOWS
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND pwsh -File ${CMAKE_SOURCE_DIR}/script/processPoFile.ps1 -poFile ${PO_FILE}
        DEPENDS ${MODULE_NAME}_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
endif()
    add_custom_target(REMOVE_DATE_${MODULE_NAME} ALL DEPENDS REMOVE_DATE)

    # remove pot file
    add_custom_command(
        OUTPUT REMOVE_DUMMY # Use a dummy target
        COMMAND ${CMAKE_COMMAND} -E remove ${POT_FILE}
        DEPENDS ${MODULE_NAME}_po
        COMMENT "Removing file: ${POT_FILE}"
    )
    add_custom_target(remove_${MODULE_NAME}_pot ALL DEPENDS REMOVE_DUMMY)
endfunction()
