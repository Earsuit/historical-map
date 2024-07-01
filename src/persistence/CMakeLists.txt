if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(DDL_TO_TABLE_SCRIPT ${sqlpp11_BIN_DIRS_RELEASE}/sqlite2cpp.py)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DDL_TO_TABLE_SCRIPT ${sqlpp11_BIN_DIRS_DEBUG}/sqlite2cpp.py)
endif()
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
  exporterImporter/ExporterImporterFactory.h
  exporterImporter/ExporterImporterRegistrar.h
  exporterImporter/IExporterImporter.h
  exporterImporter/JsonExporterImporter.cpp
  exporterImporter/JsonExporterImporter.h
  exporterImporter/ExporterImporterRegistration.cpp
  exporterImporter/BsonExporterImporter.cpp
  exporterImporter/BsonExporterImporter.h
  exporterImporter/IExporterImporter.h
  exporterImporter/ImportManager.cpp
  exporterImporter/ImportManager.h
  HistoricalCache.h
  HistoricalCache.cpp
  Data.h
  Database.h
)

add_library(libpersistence STATIC ${PERSISTENCE_SRC})
# we will not use sql in this libray but libraries linked to it
target_link_libraries(libpersistence PRIVATE Threads::Threads INTERFACE SQLite::SQLite3 sqlpp11::sqlpp11)
target_include_directories(libpersistence INTERFACE ${SQLite3_INCLUDE_DIRS})
add_dependencies(libpersistence command_header table_header)