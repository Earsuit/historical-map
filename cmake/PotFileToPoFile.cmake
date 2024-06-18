# Check if the file exists
if(EXISTS "${FILE_TO_CHECK}")
    message(STATUS "File ${FILE_TO_CHECK} exists, merging")
    execute_process(COMMAND msgmerge --update ${PO_FILE} ${POT_FILE} --backup=off)
else()
    message(STATUS "File ${FILE_TO_CHECK} does not exist, creating")
    execute_process(COMMAND msginit --input=${POT_FILE} --locale=zh_CN.UTF-8 --output=${PO_FILE} --no-translator)
endif()
