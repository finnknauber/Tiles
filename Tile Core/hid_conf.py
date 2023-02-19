Import("env")

print(env.Dump())

board_config = env.BoardConfig()
# should be array of VID:PID pairs
board_config.update("build.hwids", [
  ["0x2321", "0x8027"]
])

board_config.update("build.usb_product", "Tile Core")