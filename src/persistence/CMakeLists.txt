set(DDL_TO_TABLE_SCRIPT ${sqlpp11_SOURCE_DIR}/scripts/sqlite2cpp.py)
set(TABLE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Table)
set(SQL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Table.sql)
set(DDL_TO_SQL_COMMAND_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Ddl2SqlCommands.py)
set(SQL_COMMANDS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Commands.h)

add_custom_target(
    table_header
    COMMAND python ${DDL_TO_TABLE_SCRIPT} ${SQL_FILE} ${TABLE_HEADER} table
    DEPENDS ${PYTHON_SCRIPT}
    COMMENT "Generating ${TABLE_HEADER}.h from ${SQL_FILE}"
)

add_custom_target(
    command_header
    COMMAND python ${DDL_TO_SQL_COMMAND_SCRIPT}
                    ${SQL_FILE} 
                    ${SQL_COMMANDS_HEADER} 
                    persistence
    COMMENT "Generating ${SQL_COMMANDS_HEADER}.h from ${SQL_FILE}"
)

add_dependencies(HistoricalMap table_header command_header)

set(PERSISTENCE_SRC
    PersistenceManager.cpp
    DataCache.cpp
)

target_sources(HistoricalMap PRIVATE ${PERSISTENCE_SRC})