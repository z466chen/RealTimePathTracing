mat_red = gr.material({0.63, 0.065, 0.05}, {0.5, 0.7, 0.5}, 25)
mat_green = gr.material({0.14, 0.45, 0.091}, {0.14, 0.45, 0.091}, 25)
mat_white = gr.material({0.725, 0.71, 0.68}, {0.725, 0.71, 0.68}, 25)
mat_copper = gr.microfacet({0.95, 0.64, 0.54}, 0.2)
mat_mirror = gr.microfacet({1.00, 0.71, 0.29}, 0.0)
mat_light = gr.lightmat({23.9174, 19.2832,15.5404})
-- mat_light = gr.lightmat({1.0, 0.0,0.0})
mat_marble = gr.solid({3, 4, 5}, 1.0, 25.0)


scene_root = gr.node('root')

floor = gr.mesh( 'floor', 'cornellbox/floor.obj' )
floor:set_material(mat_marble)
scene_root:add_child(floor)

left = gr.mesh( 'left', 'cornellbox/left.obj' )
left:set_material(mat_red)
scene_root:add_child(left)

right = gr.mesh( 'right', 'cornellbox/right.obj' )
right:set_material(mat_green)
scene_root:add_child(right)

bunny = gr.mesh( 'bunny', 'bunny/bunny.obj' )
bunny:scale(1200,1200,1200)
bunny:rotate('Y',180)
bunny:translate(186,-35,169)
bunny:set_material(mat_copper)
scene_root:add_child(bunny)

tallbox = gr.mesh( 'tallbox', 'cornellbox/tallbox.obj' )
tallbox:set_material(mat_mirror)
scene_root:add_child(tallbox)

white_light = gr.mesh( 'light', 'cornellbox/light.obj' )
white_light:set_material(mat_light)
scene_root:add_child(white_light)

white_light2 = gr.mesh( 'light2', 'cornellbox/light.obj' )
white_light2:rotate('Z', 90)
white_light2:translate(1200, 0, 0)
white_light2:set_material(mat_light)
scene_root:add_child(white_light2)

temp_light = gr.light({278.0, 548.7, 279.5}, {0.5, 0.5, 0.5}, {1, 0, 0})

gr.render(scene_root, 'cornelbox.png', 512, 512,
	  {278, 273, -500}, {278, 273, -400}, {0, 1, 0}, 50,
	  {0.1, 0.1, 0.1}, {temp_light})