idf_component_register(
	SRCS "command.c"
	INCLUDE_DIRS "."
	REQUIRES "lownet"
	PRIV_REQUIRES "serial" "mbedtls"
)
