#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

typedef enum {
	raycast_north, 
	raycast_south, 
	raycast_east, 
	raycast_west, 
	raycast_top, 
	raycast_bottom
} raycast_face_t;

typedef struct {
	uint8_t r, g, b, a;
} raycast_color_t;

typedef void (*surface_pixel_t)(raycast_color_t*, int map_x, int map_y, double unit_x, double unit_y, raycast_face_t face, double depth);
typedef void (*sprite_pixel_t)(raycast_color_t*, double unit_x, double unit_y, double depth);

typedef struct {
	double x, y;
} raycast_point_t;

typedef raycast_point_t raycast_vector_t;

typedef struct {
	int id;
	raycast_point_t position;
	double size;
} raycast_object_t;

typedef struct {
	uint32_t width, height;
	double aspect_ratio;
	uint32_t *pixel_data;
	double *depth_buffer;
	char fast_top_bottom, render_top_bottom, render_walls, render_sprites;
	surface_pixel_t surface_pixel;
	sprite_pixel_t sprite_pixel;
} raycast_renderer_t;

typedef struct {
	char *world_map;
	uint32_t world_width, world_height;
	raycast_object_t *objects;
	double wall_height, top_height, bottom_height;
} raycast_scene_t;

typedef struct {
	raycast_point_t position;
	raycast_vector_t direction;
	double pitch, height, focalLength;
} raycast_camera_t;

void raycast_object_init(raycast_object_t*, int id, double x, double y);
void raycast_scene_init(raycast_scene_t*, char *world_map, raycast_object_t *objects);
void raycast_camera_init(raycast_camera_t*, double x, double y, double aspect_ratio);
void raycast_renderer_init(raycast_renderer_t*, uint32_t *pixel_data, uint32_t width, uint32_t height, surface_pixel_t surface_pixel, sprite_pixel_t sprite_pixel);

void raycast_renderer_free(raycast_renderer_t*);

void raycast_camera_set_dir(raycast_camera_t*, double x, double y);
void raycast_camera_rotate(raycast_camera_t*, double angle);
void raycast_camera_set_rotation(raycast_camera_t*, double angle);

void raycast_render_walls(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);
void raycast_render_top_bottom(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);
void raycast_render_sprites(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);
void raycast_render(raycast_renderer_t*, raycast_scene_t*, raycast_camera_t*);

#endif