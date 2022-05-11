extends Node

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var mod_name: String = "example"

# Called when the node enters the scene tree for the first time.
func init(global) -> void:
	global.register_environment("parking/Lot", preload("res://mods/PocketParkingTerrain.tscn"))
	global.register_vehicle("PocketParkingVehicle", preload("res://vehicle/PocketParkingVehicle.tscn"))
	print("Hello World!")

func init_cam_pos() -> Transform:
	return $CamPosition.global_transform

func get_spawn_position(hint: String) -> Transform:
	match hint:
		"debug_vehicle": return $CamPosition.global_transform
		_: return $VehicleSpawn.global_transform

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
