idf_component_register(
	SRCS "app_chat.c"
	INCLUDE_DIRS "."
	REQUIRES "serial" "utility"
	PRIV_REQUIRES "snoop" "mask"
)
