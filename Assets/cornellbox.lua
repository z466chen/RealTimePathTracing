mat_red = gr.material({0.63, 0.065, 0.05}, {0.5, 0.7, 0.5}, 25)
mat_green = gr.material({0.14, 0.45, 0.091}, {0.14, 0.45, 0.091}, 25)
mat_white = gr.material({0.725, 0.71, 0.68}, {0.725, 0.71, 0.68}, 25)
mat_marble = gr.solid({3, 4, 5}, 1.0, 25.0)


scene_root = gr.node('root')

floor = gr.mesh( 'floor', 'cornellbox/floor.obj' )
floor:set_material(mat_marble)
scene_root:add_child(floor)

left = gr.mesh( 'left', 'cornellbox/left.obj' )
left:set_material(mat_marble)
scene_root:add_child(left)

right = gr.mesh( 'right', 'cornellbox/right.obj' )
right:set_material(mat_marble)
scene_root:add_child(right)

shortbox = gr.mesh( 'shortbox', 'cornellbox/shortbox.obj' )
shortbox:set_material(mat_white)
scene_root:add_child(shortbox)

tallbox = gr.mesh( 'tallbox', 'cornellbox/tallbox.obj' )
tallbox:set_material(mat_white)
scene_root:add_child(tallbox)

white_light = gr.light({278.0, 548.7, 279.5}, {0.5, 0.5, 0.5}, {1, 0, 0})

gr.render(scene_root, 'cornelbox.png', 512, 512,
	  {278, 273, -500}, {278, 273, -400}, {0, 1, 0}, 50,
	  {0.1, 0.1, 0.1}, {white_light, magenta_light})