include ../../../htx.mk
TARGET = \
            hxefabricbus.readme \
            default.ab \
            default.xyz \
            rules.all_links \
            default.inter_chip \
            default.inter_node

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxefabricbus/
	${CP} ${TARGET} ${REGRULES}/hxefabricbus/ 

clean:
	${RM} -rf ${REGRULES}/hxefabricbus/* 
