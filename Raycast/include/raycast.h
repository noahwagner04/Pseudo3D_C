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
} raycast_face;

typedef struct {
	uint8_t r, g, b, a;
} raycast_color_t;

typedef struct {
	double x, y;
} raycast_point_t;

typedef raycast_point_t raycast_vector_t;

typedef struct {
	uint32_t width, height;
	char fast_top_bottom;
	uint32_t *pixel_data;
	void (*surface_pixel)(raycast_color_t*, int, int, double, double, raycast_face, double);
	void (*sprite_pixel)(raycast_color_t*, double, double, double);
} raycast_renderer_t;

typedef struct {
	int id;
	raycast_point_t position;
	double size;
} raycast_object_t;

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

#endif