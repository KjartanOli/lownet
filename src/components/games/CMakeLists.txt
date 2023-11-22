idf_component_register(
	SRCS "games.c"
	INCLUDE_DIRS "."
	REQUIRES "lownet" "tictactoe"
	PRIV_REQUIRES "serial" "utility"
)
