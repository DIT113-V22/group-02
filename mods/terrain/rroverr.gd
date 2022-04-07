extends Reference


# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var mod_name: String = "example"

# Called when the node enters the scene tree for the first time.
func init(global) -> void:
	global.register_environment("example/Example", preload("res://RestaurantSpicySalmon.tscn"))
	global.register_vehicle("TemplateCar", preload("res://TemplateCar.tscn"))
	print("Hello World!")

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
