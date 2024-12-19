#ifndef CAMERA_H
#define CAMERA_H
#include "hittable.hpp"
#include "pdf.hpp"
/*
焦平面内的物体：

	位于焦平面上的物体，来自不同随机点的光线会聚焦到像素点，因此成像清晰。

焦平面外的物体：

	焦平面外的物体，来自不同随机点的光线不能在图像平面上聚焦到同一像素点，而是分散到多个像素上，产生模糊效果。
*/
class camera {
public:
	/* Public Camera Parameters Here */
	double aspect_ratio = 1.0; // Ratio of image width over height
	int image_width = 100; // Rendered image width in pixel count

	int samples_per_pixel = 10; // Count of random samples for each pixel
	int max_depth = 10; // Maximum number of ray bounces into scene
	color background; // Scene background color
	
	double vfov = 90; //vertical view angle(field of view)

	point3 lookfrom = point3(0, 0, 0);//相机位置坐标
	point3 lookat = point3(0, 0, -1);//相机看向的地方
	vec3 vup = vec3(0, 1, 0); //相机朝上方向

	double defocus_angle = 0; // 定义lens的大小，radious/focal_length
	double focus_dist = 10; // Distance from camera lookfrom point to plane of perfect focus

	void render(const hittable& world, const hittable& lights);
private:
	/* Private Camera Variables Here */
	int image_height; // Rendered image height

	double pixel_samples_scale; // Color scale factor for a sum of pixel samples
	int sqrt_spp; //spp开根号的结果，将每个像素分成 sqrt_spp * sqrt_spp 个单位
	double recip_sqrt_spp; // 1 / sqrt_spp


	point3 center; // Camera center
	point3 pixel00_loc; // Location of pixel 0, 0
	vec3 pixel_delta_u; // Offset to pixel to the right
	vec3 pixel_delta_v; // Offset to pixel below

	vec3 u, v, w; //相机的相对正交基，u是unit vector指向右侧，v指向相机上侧， w指向正前方的后面(-z方向是朝向)
	vec3 defocus_disk_u; // Defocus disk horizontal radius  
	vec3 defocus_disk_v; // Defocus disk vertical radius

	void initialize();
	color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const;
	ray get_ray(int i, int j, int s_i, int s_j) const;
	vec3 sample_square() const;
	point3 defocus_disk_sample() const;
	vec3 sample_square_stratified(int s_i, int s_j) const;

};


#endif