#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H
#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

class constant_medium : public hittable { //恒定密度的体积介质，比如烟雾
	//散射概率prob = C * delta T 
	//C与optical density成正比
	shared_ptr<hittable> boundary;
	double neg_inv_density;
	//与体积的密度成反比，表示光线在稀疏体积内需要更长的距离才能发生散射
	shared_ptr<material> phase_function; //描述散射
public:
	constant_medium(shared_ptr<hittable> boundary, double density, shared_ptr<texture> tex)
		: boundary(boundary), neg_inv_density(-1 / density),
		phase_function(make_shared<isotropic>(tex))
	{}
	constant_medium(shared_ptr<hittable> boundary, double density, const color& albedo)
		: boundary(boundary), neg_inv_density(-1 / density),
		phase_function(make_shared<isotropic>(albedo))
	{}
	aabb bounding_box() const override { return boundary->bounding_box(); }
	
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		hit_record rec1, rec2;//光线与该constant medium进行相交判断得到的两个交点
		if (!boundary->hit(r, interval::universe, rec1))
			return false;
		if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
			return false;
		//先判断光线与介质有没有相交
		if (rec1.t < ray_t.min) rec1.t = ray_t.min;
		//如果rec1是负数，那么就是从物体内部打出来的。
		//那么rec1就改成0.001
		if (rec2.t > ray_t.max) rec2.t = ray_t.max;
		//如果rec2超过了interval范围，那么rec2改成interval的最大值
		if (rec1.t >= rec2.t)
			return false;
		if (rec1.t < 0)
			rec1.t = 0;
		auto ray_length = r.direction().length(); //光线的单位长度
		auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length; //光线在介质内部走的距离
		auto hit_distance = neg_inv_density * std::log(random_double()); //随机一个光线发生散射的距离
		if (hit_distance > distance_inside_boundary)//如果随机的散射距离大于光线在介质内部的距离那么就不散射；相当于不hit
			return false;
		rec.t = rec1.t + hit_distance / ray_length;//散射的时间点
		rec.p = r.at(rec.t); //散射点的位置
		rec.normal = vec3(1, 0, 0); // arbitrary，随机的一个法线值
		rec.front_face = true; // also arbitrary
		rec.mat = phase_function;//材质随便
		return true;
	}

};

#endif
