#pragma once
#ifndef HITTABLE_H
#define HITTABLE_H
#include "ray.hpp"
#include "interval.hpp"
#include "aabb.hpp"
class material;
class hit_record {
public:
	point3 p; //hit point
	vec3 normal; //hit point normal
	double t; //o+td�е�t,��Ӧ��������
	double u;
	double v;//��Ӧ����������
	bool front_face;
	//�������ǹ涨������Զָ������
	shared_ptr<material>mat;

	void set_face_normal(const ray& r, const vec3& outward_normal) {
		// Sets the hit record normal vector.
		// NOTE: the parameter `outward_normal` is assumed to have unit length.
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};
class hittable {
public:
	virtual ~hittable() = default;
	virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
	virtual aabb bounding_box()const = 0;

	virtual double pdf_value(const point3& origin, const vec3& direction) const {
		return 0.0;
	}
	virtual vec3 random(const point3& origin) const {
		return vec3(1, 0, 0);
	}
};

class translate :public hittable {
	shared_ptr<hittable>object; //Ҫ�ƶ�������
	vec3 offset; //�ƶ���ƫ����
	aabb bbox;//Ҫ�ƶ�������İ�Χ��
public:
	//constructor
	translate(shared_ptr<hittable> object, const vec3& offset)
		: object(object), offset(offset)
	{
		bbox = object->bounding_box() + offset;
	}
	aabb bounding_box()const override {
		return bbox;
	}
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		// Move the ray backwards by the offset
		ray offset_r(r.origin() - offset, r.direction(), r.time());
		// Determine whether an intersection exists along the offset ray (and if so, where)
		if (!object->hit(offset_r, ray_t, rec))
			return false;
		// Move the intersection point forwards by the offset
		rec.p += offset;
		return true;
	}
};

class rotate_y :public hittable {
	shared_ptr<hittable> object;
	double sin_theta; //��ʱ����ת�ĽǶ�
	double cos_theta;
	aabb bbox;
public:
	aabb bounding_box() const override { return bbox; }
	rotate_y(shared_ptr<hittable> object, double angle) : object(object) {
		auto radians = degrees_to_radians(angle); //������ת�Ƕ�
		sin_theta = std::sin(radians);
		cos_theta = std::cos(radians); 
		bbox = object->bounding_box(); //δ��ת�����bounding box
		point3 min(infinity, infinity, infinity);
		point3 max(-infinity, -infinity, -infinity);
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
					auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
					auto z = k * bbox.z.max + (1 - k) * bbox.z.min;
					//�õ�bounding_box������
					auto newx = cos_theta * x + sin_theta * z;
					auto newz = -sin_theta * x + cos_theta * z;
					vec3 tester(newx, y, newz);
					for (int c = 0; c < 3; c++) {
						min[c] = std::fmin(min[c], tester[c]);
						max[c] = std::fmax(max[c], tester[c]);
					}
				}
			}
		}
		bbox = aabb(min, max);//�õ���ת�������boundingbox
	}
	//��y����ʱ����תtheta�Ĺ�ʽ
	//x' = cos theta * x + sin theta * z;
	//z' = - sin theta * x + cos theta * z;
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		// Transform the ray from world space to object space.
		auto origin = point3(
			(cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
			r.origin().y(),
			(sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
		);
		auto direction = vec3(
			(cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
			r.direction().y(),
			(sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
		);
		ray rotated_r(origin, direction, r.time()); //��ת����object����
		// Determine whether an intersection exists in object space (and if so, where).
		if (!object->hit(rotated_r, ray_t, rec))
			return false;
		// Transform the intersection from object space back to world space.
		rec.p = point3(
			(cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
			rec.p.y(),
			(-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
		);//ת������������
		rec.normal = vec3(
			(cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
			rec.normal.y(),
			(-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
		);//����Ҳ��Ҫת����������
		return true;
	}
	
};
#endif