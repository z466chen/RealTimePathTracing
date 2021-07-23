-- A simple scene with some miscellaneous geometry.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)
mat5 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25, "specular")
mat6 = gr.material({0.3, 0.3, 0.3}, {0.5, 0.7, 0.5}, 25, "specular")
mat_marble = gr.solid({3, 4, 5},1.0, 25.0)
mat_marble_2 = gr.solid({2, 5, 0},1.0, 25.0)
scene_root = gr.node('root')

-- csg = gr.csg('csg', 'smooth')
-- csg:rotate('Z', 180)
-- csg:translate(0,0,-400)
-- scene_root:add_child(csg)

-- c = gr.nh_box('c', {0, 50, 0}, 100)
-- csg:add_child(c)
-- c:set_material(mat_marble)

-- s = gr.nh_sphere('s', {0,-50,0}, 100)
-- csg:add_child(s)
-- s:set_material(mat_marble)

csg = gr.csg('csg', 'smooth')
csg:rotate('X', 45)
csg:scale(2.0, 2.0, 2.0)
csg:translate(0,0,-400)
scene_root:add_child(csg)

main = gr.csg('main', 'difference')
csg:add_child(main)

left = gr.cylinder('left', 50, 50)
main:add_child(left)
left:set_material(mat_marble_2)

right = gr.cylinder('right', 40, 45)
right:translate(0,20,0)
main:add_child(right)
right:set_material(mat_marble_2)

handle = gr.torus('handle', 27, 3)
handle:translate(-50, -10, 0)
handle:rotate('X', 90)
csg:add_child(handle)
handle:set_material(mat_marble_2)

s2 = gr.nh_sphere('s2', {200, 50, -100}, 150)
scene_root:add_child(s2)
s2:set_material(mat_marble)

s3 = gr.nh_sphere('s3', {0, -1200, -500}, 1000)
scene_root:add_child(s3)
s3:set_material(mat5)

b1 = gr.nh_box('b1', {-200, -225, 100}, 100)
scene_root:add_child(b1)
b1:set_material(mat4)

-- s4 = gr.nh_sphere('s4', {-100, 25, -300}, 50)
s4 = gr.nh_sphere('s4', {-150, -085, 150}, 30)
scene_root:add_child(s4)
s4:set_material(mat3)

s5 = gr.nh_sphere('s5', {0, 100, -250}, 25)
scene_root:add_child(s5)
s5:set_material(mat1)

-- A small stellated dodecahedron.

steldodec = gr.mesh( 'dodec', 'smstdodeca.obj' )
steldodec:set_material(mat3)
scene_root:add_child(steldodec)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene_root, 'csg.png', 512, 512,
	  {100, 0, 600}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, magenta_light})
