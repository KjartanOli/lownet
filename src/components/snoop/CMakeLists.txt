idf_component_register(
	SRCS "snoop.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "lownet" "ping"
)
