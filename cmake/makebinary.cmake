function(makebinarys name)

    # Create target names
    set(ELF_FILE ${name}.elf)
    set(MAP_FILE ${name}.map)
    set(BIN_FILE ${name}.bin)
    set(HEX_FILE ${name}.hex)
    set(LSS_FILE ${name}.lss)
    set(DMP_FILE ${name}.dmp)

    add_custom_target(${HEX_FILE} 
        DEPENDS ${ELF_FILE} 
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${ELF_FILE} ${HEX_FILE}
        )
    add_custom_target(${BIN_FILE} 
        DEPENDS ${ELF_FILE} 
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
        )
    add_custom_target(${LSS_FILE} 
        DEPENDS ${ELF_FILE} 
        COMMAND ${CMAKE_OBJDUMP} -S ${ELF_FILE} > ${LSS_FILE}
        )
    add_custom_target(${DMP_FILE} 
        DEPENDS ${ELF_FILE} 
        COMMAND ${CMAKE_OBJDUMP} -x -D -marm -Mforce-thumb ${ELF_FILE} > ${DMP_FILE}
        )
    add_custom_target(${ELF_FILE}-size 
        DEPENDS ${HEX_FILE} ${BIN_FILE} ${LSS_FILE} ${DMP_FILE} ${ELF_FILE}
        COMMAND ${CMAKE_SIZE} -B ${ELF_FILE}
        )
    add_custom_target(${name} 
        ALL DEPENDS 
        ${ELF_FILE} ${ELF_FILE}-size ${HEX_FILE} ${BIN_FILE} ${LSS_FILE} ${DMP_FILE}
        )

    # Make it public
    set(MAP_FILE ${MAP_FILE} PARENT_SCOPE)
    set(ELF_FILE ${ELF_FILE} PARENT_SCOPE)

endfunction()
