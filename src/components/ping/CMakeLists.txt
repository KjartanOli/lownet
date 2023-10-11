idf_component_register(
	SRCS "app_ping.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "mask" "snoop"
)
