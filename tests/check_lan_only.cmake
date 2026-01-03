file(GLOB_RECURSE PRODUCTION_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/*.h"
)

set(FORBIDDEN_TOKENS
    "QNetworkAccessManager"
    "QNetworkRequest"
    "http://"
    "https://"
)

foreach(SOURCE_FILE IN LISTS PRODUCTION_SOURCES)
    file(READ "${SOURCE_FILE}" SOURCE_CONTENT)
    foreach(TOKEN IN LISTS FORBIDDEN_TOKENS)
        string(FIND "${SOURCE_CONTENT}" "${TOKEN}" MATCH_POSITION)
        if(NOT MATCH_POSITION EQUAL -1)
            message(FATAL_ERROR
                "LAN-only policy violation in ${SOURCE_FILE}: ${TOKEN}"
            )
        endif()
    endforeach()
endforeach()

