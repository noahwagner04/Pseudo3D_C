#include <stdlib.h>
#include <math.h>

#include "raycast.h"

// init functions

void raycast_object_init(raycast_object_t* object, int id, double x, double y) {
	object->id = id;
	object->position.x = x;
	object->position.y = y;
	object->size = 1;
}

void raycast_scene_init(raycast_scene_t* scene, uint8_t *world_map, uint32_t world_width, uint32_t world_height, raycast_object_t *objects, uint32_t object_count) {
	scene->world_map = world_map;
	scene->world_width = world_width;
	scene->world_height = world_height;

	scene->objects = objects;
	scene->object_count = object_count;

	scene->wall_height = 1;
	scene->top_height = 1;
	scene->bottom_height = 0;
}

void raycast_camera_init(raycast_camera_t* camera, raycast_renderer_t *renderer, double x, double y) {
	camera->position.x = x;
	camera->position.y = y;

	camera->direction.x = 1;
	camera->direction.y = 0;

	camera->plane.x = 0;
	camera->plane.y = renderer->aspect_ratio;
	camera->plane_length = renderer->aspect_ratio;

	camera->pitch = 0;
	camera->height = 0.5;
	camera->focal_Length = 1;
}

int raycast_renderer_init(raycast_renderer_t* renderer, uint32_t *pixel_data, uint32_t screen_width, uint32_t screen_height, surface_pixel_t surface_pixel, sprite_pixel_t sprite_pixel) {
	renderer->pixel_data = pixel_data;
	renderer->screen_width = screen_width;
	renderer->screen_height = screen_height;
	renderer->aspect_ratio = (double) screen_width / screen_height;

	renderer->depth_buffer = (double *) malloc(screen_width * screen_height * sizeof(double));

	if (renderer->depth_buffer == NULL) {
		return -1;
	}

	renderer->fast_top_bottom = 0;
	renderer->render_top_bottom = 1;
	renderer->render_walls = 1;
	renderer->render_sprites = 1;

	renderer->surface_pixel = surface_pixel;
	renderer->sprite_pixel = sprite_pixel;
	return 0;
}

// de-init functions

void raycast_renderer_free(raycast_renderer_t* renderer) {
	free(renderer->depth_buffer);
}

// camera movement functions

void raycast_camera_set_dir(raycast_camera_t* camera, double x, double y) {
	double length = sqrt(x * x + y * y);
	if (length == 0) return;
	camera->direction.x = x / length;
	camera->direction.y = y / length;

	camera->plane.x = -(camera->direction.y) * camera->plane_length;
	camera->plane.y = camera->direction.x * camera->plane_length;
}

void raycast_camera_rotate(raycast_camera_t* camera, double angle) {
	double i_x = cos(angle);
	double i_y = sin(angle);
	double j_x = -i_y;
	double j_y = i_x;

	double x = camera->direction.x;
	double y = camera->direction.y;

	camera->direction.x = x * i_x + y * j_x;
	camera->direction.y = x * i_y + y * j_y;

	camera->plane.x = -(camera->direction.y) * camera->plane_length;
	camera->plane.y = camera->direction.x * camera->plane_length;
}

void raycast_camera_set_rotation(raycast_camera_t* camera, double angle) {
	camera->direction.x = cos(angle);
	camera->direction.y = sin(angle);

	camera->plane.x = -sin(angle) * camera->plane_length;
	camera->plane.y = cos(angle) * camera->plane_length;
}

// cast ray functions

void raycast_camera_cast_ray(raycast_hit_info_t* hit_info, raycast_camera_t* camera, raycast_scene_t* scene) {

}

void raycast_cast_ray(raycast_hit_info_t* hit_info, raycast_scene_t* scene, double pos_x, double pos_y, double dir_x, double dir_y) {

}

int raycast_check_obstruction(raycast_scene_t* scene, double start_x, double start_y, double end_x, double end_y) {

}

// render functions

void raycast_render_walls(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {

}

void raycast_render_top_bottom(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {

}

void raycast_render_sprites(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {

}

// NOTE: check if scene.world_map or scene.objects is NULL, if so dont run the render function for those, if pixel data is NULL, dont run the function at all
void raycast_render(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {

}