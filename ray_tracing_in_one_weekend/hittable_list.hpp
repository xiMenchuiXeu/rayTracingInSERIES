#pragma once
#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H
#include "hittable.hpp"
#include "aabb.hpp"
#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;

class hittable_list : public hittable {
	aabb bbox;
public:
	std::vector<shared_ptr<hittable>> objects; //世界所有的物体
	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }
	aabb bounding_box()const override { return bbox; }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) {
		objects.push_back(object);
		bbox = aabb(bbox, object->bounding_box());
	}
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		hit_record temp_rec;
		bool hit_anything = false;
		auto closest_so_far = ray_t.max; //每次更新更新最大值，由interval的max决定是否hit成功
		for (const auto& object : objects) {
			if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
				hit_anything = true;//if (!world.hit(r, interval(0.001, infinity), rec))
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}
		return hit_anything;
	}
	double pdf_value(const point3& origin, const vec3& direction)const override {
		auto weight = 1.0 / objects.size();
		auto sum = .0;
		for (const auto& object : objects)
			sum += weight * object->pdf_value(origin, direction);
		return sum;
	}
	vec3 random(const point3& origin)const override {
		auto int_size = int(objects.size());
		return objects[random_int(0, int_size - 1)]->random(origin);//从作为光源数组中随机采样一个光源发出的光线
	 }
};

#endif