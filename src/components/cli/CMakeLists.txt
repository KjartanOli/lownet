idf_component_register(
	SRCS "cli.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "snoop" "mask"
)
