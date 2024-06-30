# Check if the file exists
if(EXISTS "${FILE_TO_CHECK}")
    execute_process(COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update ${PO_FILE} ${POT_FILE} --backup=off)
    message(STATUS "File ${FILE_TO_CHECK} exists, merging")
else()
    execute_process(COMMAND ${GETTEXT_MSGINIT_EXECUTABLE} --input=${POT_FILE} --locale=zh_CN.UTF-8 --output=${PO_FILE} --no-translator)
    message(STATUS "File ${FILE_TO_CHECK} does not exist, creating")
endif()
