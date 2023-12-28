set(DDL_TO_TABLE_SCRIPT ${sqlpp11_SOURCE_DIR}/scripts/sqlite2cpp.py)
set(TABLE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Table)
set(SQL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/InitializeDB.sql)

add_custom_target(
  table_header
  COMMAND python ${DDL_TO_TABLE_SCRIPT} ${SQL_FILE} ${TABLE_HEADER} persistence -identity-naming
  DEPENDS ${PYTHON_SCRIPT}
  COMMENT "Generating ${TABLE_HEADER}.h from ${CMAKE_CURRENT_SOURCE_DIR}/db.ddl"
)

set(DDL_TO_SQL_COMMAND_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/ddl2SqlCommands.py)
set(SQL_COMMANDS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Commands.h)

add_custom_target(
  command_header
  COMMAND python ${DDL_TO_SQL_COMMAND_SCRIPT}
                 ${SQL_FILE} 
                 ${SQL_COMMANDS_HEADER} 
                 persistence
  COMMENT "Generating ${SQL_COMMANDS_HEADER}.h from ${CMAKE_CURRENT_SOURCE_DIR}/db.ddl"
)

add_dependencies(HistoricalMap table_header command_header)
include_directories(${CMAKE_CURRENT_BINARY_DIR})