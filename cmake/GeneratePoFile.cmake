function(generatePoFile SOURCES)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/locale)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/locale/${LOCALE})

    # Construct the xgettext command
    add_custom_command(
        OUTPUT POT_DUMMY
        COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -k__ -o ${POT_FILE} --from-code=UTF-8 ${SOURCES} --no-location --c++
        COMMENT "Generating POT file"
    )
    # Create a custom target for generating the .pot file
    add_custom_target(translation_pot ALL DEPENDS POT_DUMMY)

    add_custom_command(
        OUTPUT PO_DUMMY
        COMMAND ${CMAKE_COMMAND} 
                -DFILE_TO_CHECK=${PO_FILE}
                -DPO_FILE=${PO_FILE}
                -DPOT_FILE=${POT_FILE}
                -DGETTEXT_MSGMERGE_EXECUTABLE=${GETTEXT_MSGMERGE_EXECUTABLE}
                -DGETTEXT_MSGINIT_EXECUTABLE=${GETTEXT_MSGINIT_EXECUTABLE}
                -P ${CMAKE_SOURCE_DIR}/cmake/PotFileToPoFile.cmake
        DEPENDS translation_pot
        COMMENT "Checking file existence and executing appropriate command"
    )
    add_custom_target(translation_po ALL DEPENDS PO_DUMMY)

    # remove data so that only content change will trigger git tracking
if (APPLE)
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND sed -i '' '/POT-Creation-Date:/d' ${PO_FILE}    # macos needs an extra ''
        COMMAND sed -i '' '/PO-Revision-Date:/d' ${PO_FILE}
        DEPENDS translation_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
elseif(LINUX)
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND sed -i '/POT-Creation-Date:/d' ${PO_FILE}
        COMMAND sed -i '/PO-Revision-Date:/d' ${PO_FILE}
        DEPENDS translation_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
else()
    # WINDOWS
    add_custom_command(
        OUTPUT REMOVE_DATE
        COMMAND pwsh -File ${CMAKE_SOURCE_DIR}/script/processPoFile.ps1 -poFile ${PO_FILE}
        DEPENDS translation_po
        COMMENT "Remove DATE from ${PO_FILE}"
    )
endif()
    add_custom_target(REMOVE_DATE_translation ALL DEPENDS REMOVE_DATE)

    # remove pot file
    add_custom_command(
        OUTPUT REMOVE_DUMMY # Use a dummy target
        COMMAND ${CMAKE_COMMAND} -E remove ${POT_FILE}
        DEPENDS translation_po
        COMMENT "Removing file: ${POT_FILE}"
    )
    add_custom_target(remove_translation_pot ALL DEPENDS REMOVE_DUMMY)
endfunction()
