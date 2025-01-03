set(GAME_DIR "${CMAKE_BINARY_DIR}/game")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/base")

file(MAKE_DIRECTORY ${GAME_DIR})
add_custom_target(Soldat_Assets
  COMMAND ${ASSET_DIR}/create_smod.sh
  COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_DIR}/soldat.smod ${GAME_DIR}/soldat.smod
  COMMAND ${CMAKE_COMMAND} -E rename ${ASSET_DIR}/soldat.smod ${GAME_DIR}/soldat_server.smod
  WORKING_DIRECTORY ${ASSET_DIR}
  COMMENT "Creating assets files"
)

add_custom_target(SoldatGame_full ALL
  DEPENDS SoldatGame Soldat_Assets
  COMMENT "Building all assets"
)