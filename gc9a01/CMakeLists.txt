project(gc9a01)

set (GC9A01_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/gc9a01.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/font8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/font12.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/font16.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/font20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/font24.c
    )

add_library(${PROJECT_NAME} ${GC9A01_SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/fonts
        ${PROJECT_SOURCE_DIR}/threedimensional
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        sdk
    )