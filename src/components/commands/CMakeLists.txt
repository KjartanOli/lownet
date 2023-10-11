idf_component_register(
	SRCS "commands.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "snoop" "mask"
)
