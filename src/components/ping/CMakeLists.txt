idf_component_register(
	SRCS "ping.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "mask" "snoop"
)
