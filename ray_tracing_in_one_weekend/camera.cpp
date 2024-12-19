#include "camera.hpp"
#include "material.hpp"
const double PI = 3.14159265358979323846;
color camera::ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
	// If we've exceeded the ray bounce limit, no more light is gathered.
	
	if (depth <= 0)
		return color(0, 0, 0);
	hit_record rec;
	
	// If the ray hits nothing, return the background color.
	if (!world.hit(r, interval(0.001, infinity), rec))
		return background;
	
	scatter_record srec;

	color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p); //交点的材质发光程度
	if (!rec.mat->scatter(r, rec, srec)) //主要作用是遇到光源直接return
		return color_from_emission;
	
	if (srec.skip_pdf) {//如果是specular材质,pdf是冲激函数，我们不使用pdf进行采样
		return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);
	}
	//只有非镜面反射我们才使用pdf进行采样，pdf的计算，涉及从光源中采样以及根据物体自身的性质进行散射
	auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
	mixture_pdf p(light_ptr, srec.pdf_ptr);

	ray scattered = ray(rec.p, p.generate(), r.time()); //取样一条散射光线,从光源采样或者根据自身材质采样
	auto pdf_value = p.value(scattered.direction());//取样该散射光线的pdf

	double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered); //由物体表面材质决定
	
	color sample_color = ray_color(scattered, depth - 1, world, lights);
	color color_from_scatter =
		(srec.attenuation * scattering_pdf * sample_color) / pdf_value; //recursive
	
	return color_from_emission + color_from_scatter;
}

void camera::initialize() {
	image_height = int(image_width / aspect_ratio);
	image_height = (image_height < 1) ? 1 : image_height;

	sqrt_spp = int(std::sqrt(samples_per_pixel));
	pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp); //1/spp
	recip_sqrt_spp = 1.0 / sqrt_spp;//每个网格的长度
	
	center = lookfrom;
	// Determine viewport dimensions.
	//auto focal_length = (lookfrom - lookat).length();

	auto theta = degrees_to_radians(vfov);
	auto h = std::tan(theta / 2);
	auto viewport_height = 2 * h * focus_dist;
	auto viewport_width = viewport_height * (double(image_width) / image_height);

	//对相机相对正交基进行计算
	w = unit_vector(lookfrom - lookat);//u是unit vector指向右侧，v指向相机上侧， w指向正前方的后面(-z方向是朝向)
	u = unit_vector(cross(vup, w));
	v = cross(w, u);
	
	// Calculate the vectors across the horizontal and down the vertical viewport edges.
	vec3 viewport_u = viewport_width * u; // Vector across viewport horizontal edge
	vec3 viewport_v = viewport_height * -v; // Vector down viewport vertical edge
	
	// Calculate the horizontal and vertical delta vectors from pixel to pixel.
	pixel_delta_u = viewport_u / image_width;
	pixel_delta_v = viewport_v / image_height;
	
	// Calculate the location of the upper left pixel.
	auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
	pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

	// Calculate the camera defocus disk basis vectors.
	auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
	defocus_disk_u = u * defocus_radius;
	defocus_disk_v = v * defocus_radius;
}
void camera::render(const hittable& world, const hittable& lights) {
	initialize();
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = 0; j < image_height; j++) {
		std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
		for (int i = 0; i < image_width; i++) {
			color pixel_color(0, 0, 0);
			for (int s_j = 0; s_j < sqrt_spp; s_j++) {
				for (int s_i = 0; s_i < sqrt_spp; s_i++) {
					ray r = get_ray(i, j, s_i, s_j);
					pixel_color += ray_color(r, max_depth, world, lights);
				}
			}
			write_color(std::cout, pixel_samples_scale * pixel_color);
		}
	}
	std::clog << "\rDone. \n";
}

ray camera::get_ray(int i, int j, int s_i, int s_j) const { 
	// Construct a camera ray originating from the origin and directed at randomly sampled
	// point around the pixel location i, j.
	auto offset = sample_square_stratified(s_i, s_j);
	auto pixel_sample = pixel00_loc
		+ ((i + offset.x()) * pixel_delta_u)
		+ ((j + offset.y()) * pixel_delta_v);
	auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
	auto ray_direction = pixel_sample - ray_origin;
	auto ray_time = random_double(); //返回一个0,1的double value,ray的time是随机生成的
	return ray(ray_origin, ray_direction, ray_time);
}
vec3 camera::sample_square() const {
	// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
	return vec3(random_double() - 0.5, random_double() - 0.5, 0);
}

point3 camera::defocus_disk_sample() const {
	// Returns a random point in the camera defocus disk.
	auto p = random_in_unit_disk();//返回一个xy平面单位圆上的点
	return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}


vec3 camera::sample_square_stratified(int s_i, int s_j) const {
	// Returns the vector to a random point in the square sub-pixel specified by grid
	// indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].
	auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
	auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;
	return vec3(px, py, 0);
} 
//每个像素分成sqrt_spp * sqrt_spp的二维grid大小，spp个光线从每个grid单位中打出