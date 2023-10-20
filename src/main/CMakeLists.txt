idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
	REQUIRES "crypt" "network" "ping" "chat" "mask" "snoop" "commands" "serial"
)
