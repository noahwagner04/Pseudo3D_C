#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "raycast.h"

// utility functions
void raycast_uint32_to_color(uint32_t num, raycast_color_t* color) {
	color->r = (num & 0xFF000000) >> 24;
	color->g = (num & 0x00FF0000) >> 16;
	color->b = (num & 0x0000FF00) >> 8;
	color->a = (num & 0x000000FF);
}

uint32_t raycast_color_to_uint32(raycast_color_t* color) {
	return (uint32_t)((color->r << 24) + (color->g << 16) + (color->b << 8) + color->a);
}

// init functions

void raycast_object_init(raycast_object_t* object, int id, double x, double y) {
	object->id = id;
	object->position.x = x;
	object->position.y = y;
	object->size = 1;
	object->height = 0;
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
	camera->height = 0;
	camera->focal_length = 1;
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

	camera->plane.x = -(camera->direction.y) * camera->plane_length;
	camera->plane.y = camera->direction.x * camera->plane_length;
}

// cast ray functions

void raycast_DDA(raycast_hit_info_t* hit_info, raycast_scene_t* scene, double pos_x, double pos_y, double dir_x, double dir_y, double initial_ray_length) {
	int map_x = (int)pos_x;
	int map_y = (int)pos_y;

	double side_dist_x;
	double side_dist_y;

	int step_x;
	int step_y;

	double delta_dist_x = dir_x == 0 ? 1e30 : fabs(initial_ray_length / dir_x);
	double delta_dist_y = dir_y == 0 ? 1e30 : fabs(initial_ray_length / dir_y);

	uint8_t hit = 0;
	uint8_t side = 0;

	// DDA setup step directions and initial side distances
	if (dir_x < 0) {
		step_x = -1;
		side_dist_x = (pos_x - map_x) * delta_dist_x;
	} else {
		step_x = 1;
		side_dist_x = (map_x + 1.0 - pos_x) * delta_dist_x;
	}
	if (dir_y < 0) {
		step_y = -1;
		side_dist_y = (pos_y - map_y) * delta_dist_y;
	} else {
		step_y = 1;
		side_dist_y = (map_y + 1.0 - pos_y) * delta_dist_y;
	}

	// perform DDA
	while (hit == 0) {
		// increment ray position
		if (side_dist_x > side_dist_y) {
			map_y += step_y;
			side_dist_y += delta_dist_y;
			side = 1;
		} else {
			map_x += step_x;
			side_dist_x += delta_dist_x;
			side = 0;
		}

		// check if ray is out of map bounds, manually stop casting if so
		if (map_x < 0 || map_y < 0 || map_x >= scene->world_width || map_y >= scene->world_height) {
			break;
		}
		hit = scene->world_map[map_x + scene->world_width * map_y];
	}

	// calculate distance the ray traveled
	if (side == 0) {
		hit_info->distance = side_dist_x - delta_dist_x;
	} else {
		hit_info->distance = side_dist_y - delta_dist_y;
	}

	// set other info in hit_info struct
	hit_info->hit_point.x = map_x;
	hit_info->hit_point.y = map_y;

	hit_info->wall_type = hit;

	// calculate face hit
	if (side == 0) {
		if (dir_x > 0) {
			hit_info->face = raycast_east;
		} else {
			hit_info->face = raycast_west;
		}
	} else {
		if (dir_y > 0) {
			hit_info->face = raycast_south;
		} else {
			hit_info->face = raycast_north;
		}
	}
}

void raycast_cast_ray(raycast_hit_info_t* hit_info, raycast_scene_t* scene, double pos_x, double pos_y, double dir_x, double dir_y) {
	double initial_ray_length = sqrt(dir_x * dir_x + dir_y * dir_y);

	// cast ray
	raycast_DDA(hit_info, scene, pos_x, pos_y, dir_x, dir_y, initial_ray_length);

	// calculate the floating point location of where the ray hit
	hit_info->hit_point.x = pos_x + dir_x / initial_ray_length * hit_info->distance;
	hit_info->hit_point.y = pos_y + dir_y / initial_ray_length * hit_info->distance;
}

int raycast_check_obstruction(raycast_scene_t* scene, double start_x, double start_y, double end_x, double end_y) {
	// set up variables for DDA function
	double dir_x = end_x - start_x;
	double dir_y = end_y - start_y;

	double between_length = sqrt(dir_x * dir_x + dir_y * dir_y);

	raycast_hit_info_t hit_info;

	//cast ray
	raycast_DDA(&hit_info, scene, start_x, start_y, dir_x, dir_y, between_length);

	// check if ray hit end point
	if (hit_info.distance < between_length) {
		return 1;
	} else {
		return 0;
	}
}

// render functions

void raycast_render_walls(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {
	int w = renderer->screen_width;
	int h = renderer->screen_height;

	for (int x = 0; x < w; x++) {
		// ranges from -0.5 to 0.5 depending on x
		double camera_x = (x / (double)w * 2 - 1) / 2;

		// ray directions
		double ray_dir_x = camera->direction.x * camera->focal_length + camera->plane.x * camera_x;
		double ray_dir_y = camera->direction.y * camera->focal_length + camera->plane.y * camera_x;

		/*
		hit information of ray will be located in this struct,
		some names will not match what the variable represents
		*/
		raycast_hit_info_t hit_info;

		// cast ray, assume ray length is 1 for faster performance and to calculate perpendicular distance instead of euclidean
		raycast_DDA(&hit_info, scene, camera->position.x, camera->position.y, ray_dir_x, ray_dir_y, 1);

		// if we hit out of the map, don't do anything
		if (hit_info.wall_type == 0) continue;

		int line_height = (int)(h / hit_info.distance);
		int column_center = (int)(h * camera->height / hit_info.distance);
		int draw_start = h / 2 - (int)(line_height * scene->wall_height - (double)line_height / 2) + camera->pitch + column_center;
		int draw_end = h / 2 + line_height / 2 + camera->pitch + column_center;

		//calculate value of wall_x
		double wall_x; //where exactly the wall was hit
		if (hit_info.face == raycast_east || hit_info.face == raycast_west) {
			wall_x = camera->position.y + hit_info.distance * ray_dir_y;
		} else {
			wall_x = camera->position.x + hit_info.distance * ray_dir_x;
		}
		wall_x -= floor(wall_x);

		// LATER: maybe come back to this, I don't know if I'm checking the right faces
		// if (hit_info.face == raycast_east || hit_info.face == raycast_north) wall_x = 1 - wall_x;

		// How much to increase the texture coordinate per screen pixel
		double step = 1.0 / (draw_end - draw_start);

		//Starting texture coordinate
		double wall_y = draw_start < 0 ? abs(draw_start) * step : 0;

		// constrain draw_start and draw_end to be within the range 0 - screen_height
		if (draw_start < 0) draw_start = 0;
		if (draw_end >= h) draw_end = h;

		raycast_screen_pixel_t pixel;

		for (int y = draw_start; y < draw_end; y++) {
			int index = x + w * y;

			if (renderer->depth_buffer[index] > hit_info.distance) {
				raycast_uint32_to_color(renderer->pixel_data[index], &(pixel.color));

				pixel.location = index;

				renderer->surface_pixel(&pixel, hit_info.hit_point.x, hit_info.hit_point.y, wall_x, wall_y, hit_info.face, hit_info.distance);

				renderer->pixel_data[index] = raycast_color_to_uint32(&(pixel.color));
				renderer->depth_buffer[index] = hit_info.distance;
			}

			wall_y += step;
		}
	}
}

/*
LATER: add different height floor / ceiling functionality
*/
void raycast_render_top_bottom(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {
	int w = renderer->screen_width;
	int h = renderer->screen_height;
	int half_h = h / 2;

	for (int y = 0; y < h; y++) {
		int is_floor = y > half_h + camera->pitch;

		// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
		double ray_dir_x0 = camera->direction.x * camera->focal_length - camera->plane.x / 2;
		double ray_dir_y0 = camera->direction.y * camera->focal_length - camera->plane.y / 2;
		double ray_dir_x1 = camera->direction.x * camera->focal_length + camera->plane.x / 2;
		double ray_dir_y1 = camera->direction.y * camera->focal_length + camera->plane.y / 2;

		// Current y position compared to the center of the screen (the horizon)
		int p = y - (half_h + camera->pitch);

		// Vertical position of the camera.
		double pos_z = is_floor ? (camera->height + 1.0 / 2) * h : (1 - (camera->height + 1.0 / 2)) * h;

		// Horizontal distance from the camera to the floor for the current row.
		// 0.5 is the z position exactly in the middle between floor and ceiling.
		double row_distance = p == 0 ? 1e9 : fabs(pos_z / p);

		// calculate the real world step vector we have to add for each x (parallel to camera plane)
		// adding step by step avoids multiplications with a weight in the inner loop
		double floor_step_x = row_distance * (ray_dir_x1 - ray_dir_x0) / w;
		double floor_step_y = row_distance * (ray_dir_y1 - ray_dir_y0) / w;

		// real world coordinates of the leftmost column. This will be updated as we step to the right.
		double floor_x = camera->position.x + row_distance * ray_dir_x0;
		double floor_y = camera->position.y + row_distance * ray_dir_y0;

		for (int x = 0; x < w; ++x) {
			// the cell coord is simply got from the integer parts of floor_x and floor_y
			int cell_x = (int)floor_x;
			int cell_y = (int)floor_y;

			// the floating point coordinates within the cell, ranges between 0 - 1
			double unit_x = floor_x - cell_x;
			double unit_y = floor_y - cell_y;

			raycast_screen_pixel_t pixel;

			int index = y * w + x;

			if (renderer->depth_buffer[index] > row_distance) {
				raycast_face_t face;

				raycast_uint32_to_color(renderer->pixel_data[index], &(pixel.color));

				pixel.location = index;

				// we are drawing the ceiling
				if (is_floor) {
					face = raycast_bottom;
				} else {
					// we are drawing the floor
					face = raycast_top;
				}

				renderer->surface_pixel(&pixel, cell_x, cell_y, fabs(unit_x), fabs(unit_y), face, row_distance);
				renderer->pixel_data[index] = raycast_color_to_uint32(&(pixel.color));
				renderer->depth_buffer[index] = row_distance;
			}

			floor_x += floor_step_x;
			floor_y += floor_step_y;
		}
	}
}

void raycast_render_sprites(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {
	int w = renderer->screen_width;
	int h = renderer->screen_height;

	double camera_i_x = camera->plane.x / 2;
	double camera_i_y = camera->plane.y / 2;

	double camera_j_x = camera->direction.x * camera->focal_length;
	double camera_j_y = camera->direction.y * camera->focal_length;

	for (int i = 0; i < scene->object_count; i++) {
		raycast_object_t *object = scene->objects + i;

		//translate sprite position to relative to camera
		double sprite_x = object->position.x - camera->position.x;
		double sprite_y = object->position.y - camera->position.y;

		//transform sprite with the inverse camera matrix
		double inv_det = 1.0 / (camera_i_x * camera_j_y - camera_j_x * camera_i_y);

		// sprites coordinates relative to the camera, y coordinate used as depth (z-axis)
		double transform_x = inv_det * (camera_j_y * sprite_x - camera_j_x * sprite_y);
		double transform_y = inv_det * (-camera_i_y * sprite_x + camera_i_x * sprite_y);

		// if the sprite is behind us, don't draw it
		if (transform_y < 0) continue;

		// screen coordinates of the sprite's center
		int sprite_screen_x = (int)((1 + transform_x / transform_y) / 2 * w);
		int sprite_screen_y = (int)((1 - object->height / transform_y) / 2 * h) + camera->pitch + (int)(h * camera->height / transform_y);

		//calculate height of the sprite on screen
		int sprite_height = abs((int)(h * (object->size / transform_y)));
		//calculate lowest and highest pixel to fill in current stripe
		int draw_start_y = sprite_screen_y - sprite_height / 2;
		int draw_end_y = sprite_screen_y + sprite_height / 2;

		//calculate width of the sprite
		int sprite_width = sprite_height; // same as height of sprite, given that it's square
		int draw_start_x = sprite_screen_x - sprite_width / 2;
		int draw_end_x = sprite_screen_x + sprite_width / 2;

		// how much to increase percent variables per pixel
		double step = 1.0 / sprite_height;

		// sprite percentage coordinates (ranges from 0 - 1) the current pixel is on
		double sprite_percent_x = draw_start_x < 0 ? (double)abs(draw_start_x) / sprite_height : 0;
		double sprite_percent_x_initial = sprite_percent_x;
		double sprite_percent_y = draw_start_y < 0 ? (double)abs(draw_start_y) / sprite_height : 0;

		// clamp draw start / end values to fit into the screen
		if (draw_start_y < 0) draw_start_y = 0;
		if (draw_end_y >= h) draw_end_y = h;

		if (draw_start_x < 0) draw_start_x = 0;
		if (draw_end_x > w) draw_end_x = w;

		raycast_screen_pixel_t pixel;

		for (int y = draw_start_y; y < draw_end_y; y++) {
			for (int x = draw_start_x; x < draw_end_x; x++) {
				int index = x + y * w;

				if (renderer->depth_buffer[index] > transform_y) {
					raycast_uint32_to_color(renderer->pixel_data[index], &(pixel.color));

					pixel.location = index;

					renderer->sprite_pixel(&pixel, object->id, sprite_percent_x, sprite_percent_y, transform_y);

					renderer->pixel_data[index] = raycast_color_to_uint32(&(pixel.color));
					// only account for the sprites depth if the pixel being drawn is fully opaque
					if (pixel.color.a == 255) {
						renderer->depth_buffer[index] = transform_y;
					}
				}
				sprite_percent_x += step;
			}
			sprite_percent_x = sprite_percent_x_initial;
			sprite_percent_y += step;
		}
	}
}

void raycast_render(raycast_renderer_t* renderer, raycast_scene_t* scene, raycast_camera_t* camera) {
	if (renderer->pixel_data == NULL) return;

	// clear the screen before next frame
	memset(renderer->pixel_data, 0, renderer->screen_width * renderer->screen_height * 4);

	// reset the depth buffer
	for (int x = 0; x < renderer->screen_width; ++x) {
		for (int y = 0; y < renderer->screen_height; ++y) {
			renderer->depth_buffer[x * renderer->screen_height + y] = INFINITY;
		}
	}

	// render surfaces
	if (renderer->surface_pixel != NULL) {

		if (scene->world_map != NULL && renderer->render_walls) raycast_render_walls(renderer, scene, camera);

		if (renderer->render_top_bottom) raycast_render_top_bottom(renderer, scene, camera);
	}

	// render objects
	if (scene->objects != NULL && renderer->sprite_pixel != NULL && renderer->render_sprites) {
		raycast_render_sprites(renderer, scene, camera);
	}
}