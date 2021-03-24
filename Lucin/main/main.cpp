#include <iostream>

#include <chrono>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "Renderer/sphere.h"
#include "Renderer/camera.h"
#include "Renderer/hittable_list.h"
#include "Renderer/material.h"

color ray_color(const ray& r, const hittable& world, int depth) {
	hit_record rec; 
	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void write_color(color& col, int samples_per_pixel) {
	auto r = col.x();
	auto g = col.y();
	auto b = col.z();

	// Divide the color by the number of samples and gamma-correct for gamma=2.0.
	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	col[0] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
    col[1] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
    col[2] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
}


int main() {

	//timer
	auto start = std::chrono::steady_clock::now();

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 400;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	unsigned char* data = new unsigned char[image_width * image_height * 3];
	const int samples_per_pixel = 100;
	const int maxDepth = 50;


	// World
	auto R = cos(pi / 4);
	hittable_list world;

	auto material_left = make_shared<lambertian>(color(0, 0, 1));
	auto material_right = make_shared<lambertian>(color(1, 0, 0));

	world.add(make_shared<sphere>(point3(-R, 0, -1), R, material_left));
	world.add(make_shared<sphere>(point3(R, 0, -1), R, material_right));

	// Camera

	camera cam(90.0, aspect_ratio);

	int index = 0;
	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);

			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (int)(image_width - 1);
				auto v = (j + random_double()) / (int)(image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, maxDepth);
			}

			write_color(pixel_color, samples_per_pixel);
			data[index++] = static_cast<char>(pixel_color.x());
			data[index++] = static_cast<char>(pixel_color.y());
			data[index++] = static_cast<char>(pixel_color.z());
		}
	}
	//stbi_write_png("../image.png", image_width, image_height, 3, data, image_width * sizeof(unsigned char)*3);
	stbi_write_tga("../image.tga", image_width, image_height, 3, data);
	//stbi_write_jpg("../image.jpg", image_width, image_height, 3, data, 100);
	std::cerr << "\nDone.\n";

	auto end = std::chrono::steady_clock::now();

	std::cerr << "Elapsed time in seconds : "
		<< std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
		<< " s" ;

}