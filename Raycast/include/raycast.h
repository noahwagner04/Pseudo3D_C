#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

/*
all the possible faces a ray can hit
top and bottom being the floor and ceiling
*/
typedef enum {
	raycast_north,
	raycast_south,
	raycast_east,
	raycast_west,
	raycast_top,
	raycast_bottom
} raycast_face_t;

// simple 32bit color struct
typedef struct {
	uint8_t r, g, b, a;
} raycast_color_t;

typedef struct {
	raycast_color_t color;
	uint32_t location;
} raycast_screen_pixel_t;

/*
this function runs for every pixel on the screen that coorelates to a surface, whether it be wall, floor, or ceiling
it provides information about that pixels location in the world through its arguments
the user can use this information to map textures to surfaces, implement depth lighting,
source lighting, glossy floors, sunlight shadows, and more
the first parameter is the pixel that the user is supposed to change
*/
typedef void (*surface_pixel_t)(raycast_screen_pixel_t*, int map_x, int map_y, double unit_x, double unit_y, raycast_face_t face, double depth);

/*
similar to the function above, but it instead runs for every pixel on the screen
that coorelates to a sprite in the raycast scene
*/
typedef void (*sprite_pixel_t)(raycast_screen_pixel_t*, int id, double unit_x, double unit_y, double depth);

// simple 2d point struct
typedef struct {
	double x, y;
} raycast_point_t;

// vector is the same as point, only used to distinguish the intent of a variable
typedef raycast_point_t raycast_vector_t;

/*
used by cast ray functions, holds relavent
information about the hit location of the ray
*/
typedef struct {
	raycast_point_t hit_point;
	double distance;
	raycast_face_t face;
	uint8_t wall_type;
} raycast_hit_info_t;

/*
objects represent basic information about sprites in the scene,
its up to the user to provide the texture of the object
*/
typedef struct {
	int id;
	raycast_point_t position;
	double height, size;
} raycast_object_t;

/*
the renderer is responsible for storing information about the screen,
certain render settings, and the two pixel functions provided by the user
*/
typedef struct {
	uint32_t *pixel_data;
	double *depth_buffer;
	uint32_t screen_width, screen_height;
	double aspect_ratio;
	char render_top_bottom, render_walls, render_sprites;
	surface_pixel_t surface_pixel;
	sprite_pixel_t sprite_pixel;
} raycast_renderer_t;

/*
the scene is responsible for storing information about
the geometry of the world, and the objects in the scene
*/
typedef struct {
	uint8_t *world_map;
	uint32_t world_width, world_height, object_count;
	raycast_object_t *objects;
	double wall_height, top_height, bottom_height;
} raycast_scene_t;

/*
the camera is responsible for storing information about where
the world will be rendered, and what fov it will be rendered at
*/
typedef struct {
	raycast_point_t position;
	raycast_vector_t direction;
	raycast_vector_t plane;
	double height, focal_length, plane_length;
	int pitch;
} raycast_camera_t;

// utility functions
void raycast_uint32_to_color(uint32_t, raycast_color_t*);
uint32_t raycast_color_to_uint32(raycast_color_t*);


// init functions
void raycast_object_init(raycast_object_t*, int id, double x, double y);
void raycast_scene_init(raycast_scene_t*, uint8_t *world_map, uint32_t world_width, uint32_t world_height, raycast_object_t *objects, uint32_t object_count);
void raycast_camera_init(raycast_camera_t*, raycast_renderer_t*, double x, double y);

// returns -1 on failure to initialize or 0 on success
int raycast_renderer_init(raycast_renderer_t*, uint32_t *pixel_data, uint32_t width, uint32_t height, surface_pixel_t surface_pixel, sprite_pixel_t sprite_pixel);

// de-init functions
void raycast_renderer_free(raycast_renderer_t*);


// camera movement functions

/*
user cannot directly manipulate the direction
attribute of the camera because camera plane must be perpendicular
to camera direction at all times, hence why these functions exist
*/
void raycast_camera_set_dir(raycast_camera_t*, double x, double y);
void raycast_camera_rotate(raycast_camera_t*, double angle);
void raycast_camera_set_rotation(raycast_camera_t*, double angle);

// cast ray functions

/*
only used internally by the render engine and other cast ray functions, faster but it requires specific setup
by the camera, and it doesn't calculate the true euclidean distance traveled by the ray
uses hit_info in first parameter to return information about where the ray hit
*/
void raycast_DDA(raycast_hit_info_t*, raycast_scene_t*, double pos_x, double pos_y, double dir_x, double dir_y, double initial_ray_length);

/*
cast ray function intended for the user to use, slower than the previous
function but it gives more practical information, information returned regarding
where the ray hit is stored in the hit_info struct
*/
void raycast_cast_ray(raycast_hit_info_t*, raycast_scene_t*, double pos_x, double pos_y, double dir_x, double dir_y);

/*
casts a ray from one point to another, returns 1 if there
is an obstruction, and 0 if there is no obstruction
*/
int raycast_check_obstruction(raycast_scene_t*, double start_x, double start_y, double end_x, double end_y);

// render functions
void raycast_render_walls(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);
void raycast_render_top_bottom(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);
void raycast_render_sprites(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);

// runs all three previous render function to draw a complete world
void raycast_render(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);

#endif