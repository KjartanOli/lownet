idf_component_register(
	SRCS "tictactoe.c" "tictac_node.c"
	INCLUDE_DIRS "."
	REQUIRES "lownet" "tictactoe"
	PRIV_REQUIRES "serial"
)
