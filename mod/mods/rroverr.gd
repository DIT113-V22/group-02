extends Reference

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var mod_name: String = "example"

# Called when the node enters the scene tree for the first time.
func init(global) -> void:
	global.register_environment("parking/Lot", preload("res://mods/PocketParkingTerrain.tscn"))
	global.register_vehicle("PocketParkingVehicle", preload("res://vehicle/PocketParkingVehicle.tscn"))
	print("Hello World!")

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
