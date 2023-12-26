set(PYTHON_SCRIPT ${sqlpp11_SOURCE_DIR}/scripts/sqlite2cpp.py)
set(GENERATED_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Table)

add_custom_target(
  table_header
  COMMAND python ${PYTHON_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/db.ddl ${GENERATED_HEADER} persistence -identity-naming
  DEPENDS ${PYTHON_SCRIPT}
  COMMENT "Generating ${GENERATED_HEADER}.h from ${CMAKE_CURRENT_SOURCE_DIR}/db.ddl"
)

add_dependencies(HistoricalMap table_header)
include_directories(${CMAKE_CURRENT_BINARY_DIR})