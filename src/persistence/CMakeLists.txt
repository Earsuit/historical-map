set(DDL_TO_TABLE_SCRIPT ${sqlpp11_SOURCE_DIR}/scripts/sqlite2cpp.py)
set(TABLE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Table)
set(SQL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Table.sql)
set(DDL_TO_SQL_COMMAND_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Ddl2SqlCommands.py)
set(SQL_COMMANDS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Commands.h)

add_custom_command(
  OUTPUT ${TABLE_HEADER}.h
  COMMAND python ${DDL_TO_TABLE_SCRIPT} ${SQL_FILE} ${TABLE_HEADER} table
  DEPENDS ${DDL_TO_TABLE_SCRIPT} ${SQL_FILE}
  COMMENT "Generating ${TABLE_HEADER}.h from ${SQL_FILE}"
)

add_custom_command(
  OUTPUT ${SQL_COMMANDS_HEADER}
  COMMAND python ${DDL_TO_SQL_COMMAND_SCRIPT} ${SQL_FILE} ${SQL_COMMANDS_HEADER} persistence
  DEPENDS ${DDL_TO_SQL_COMMAND_SCRIPT} ${SQL_FILE}
  COMMENT "Generating ${SQL_COMMANDS_HEADER}.h from ${SQL_FILE}"
)

add_custom_target(table_header DEPENDS ${TABLE_HEADER}.h)
add_custom_target(command_header DEPENDS ${SQL_COMMANDS_HEADER})

set(PERSISTENCE_SRC
  DatabaseManager.cpp
  exporterImporter/ExporterImporterFactory.h
  exporterImporter/ExporterImporterRegistrar.h
  exporterImporter/IExporterImporter.h
  exporterImporter/JsonExporterImporter.cpp
  exporterImporter/JsonExporterImporter.h
  exporterImporter/Util.h
)

add_library(libpersistence STATIC ${PERSISTENCE_SRC})
target_link_libraries(libpersistence PRIVATE sqlpp11::sqlite3 Threads::Threads)
add_dependencies(libpersistence command_header table_header)