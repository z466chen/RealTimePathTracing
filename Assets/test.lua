-- A simple scene with some miscellaneous geometry.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)
mat5 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25, "specular")
mat6 = gr.material({0.3, 0.3, 0.3}, {0.5, 0.7, 0.5}, 25, "specular")
mat_marble = gr.solid({2, 5, 0},1.0, 25.0)

scene_root = gr.node('root')

csg = gr.csg('csg', 'smooth')
csg:scale(2.0, 2.0, 2.0)
csg:translate(0,0,-400)
scene_root:add_child(csg)

main = gr.csg('main', 'difference')
csg:add_child(main)

left = gr.cylinder('left', 50, 50)
main:add_child(left)
left:set_material(mat_marble)

right = gr.cylinder('right', 40, 45)
right:translate(0,20,0)
main:add_child(right)
right:set_material(mat_marble)

handle = gr.torus('handle', 27, 3)
handle:translate(-50, -10, 0)
handle:rotate('X', 90)
csg:add_child(handle)
handle:set_material(mat_marble)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene_root, 'test.png', 512, 512,
	  {0, 150, 100}, {0, 0, -400}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, magenta_light})
