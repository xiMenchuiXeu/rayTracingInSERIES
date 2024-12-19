#ifndef QUAD_H
#define QUAD_H

#include "hittable.hpp"
#include "hittable_list.hpp"

class quad : public hittable {
	point3 Q;
	vec3 u, v; //Q+v与Q+v得到Q点相邻的两个顶点
	shared_ptr<material> mat;
	aabb bbox;
	vec3 w; //计算交点用的中间值
	vec3 normal;
	double D; //Ax+By+Cz+D = 0
	double area;
public:
	virtual void set_bounding_box() {
		auto bbox_diagonal1 = aabb(Q, Q + u + v);
		auto bbox_diagonal2 = aabb(Q + u, Q + v);
		bbox = aabb(bbox_diagonal1, bbox_diagonal2);
	}
	quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat) :Q(Q), u(u), v(v), mat(mat)
	{
		auto n = cross(u, v);
		normal = unit_vector(n);
		D = dot(normal, Q);
		w = n / dot(n, n);
		set_bounding_box();
		area = n.length();

	}

	aabb bounding_box()const override { return bbox; }
	
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		//t = (D - n*P)/n*d
		//p是平面的点，d是光线的方向向量

		//P = Q + alpha * u + beta * v; P是四边形内任意一点
		//alpha = w * (p x v)
		//beta = w * (u x p)
		// p = P - Q
		//w = n / n * (u x v) = n / n*n
		auto denom = dot(normal, r.direction());
		// No hit if the ray is parallel to the plane.
		if (std::fabs(denom) < 1e-8)
			return false;

		auto t = (D - dot(normal, r.origin())) / denom;

		if (!ray_t.contains(t))
			return false;

		//光线击中所在平面的话，进行进一步判断看看是否击中平面中的四边形
		auto intersection = r.at(t);
		vec3 planar_hitpt_vector = intersection - Q; //计算p = P - Q
		auto alpha = dot(w, cross(planar_hitpt_vector, v));
		auto beta = dot(w, cross(u, planar_hitpt_vector));
		if (!is_interior(alpha, beta, rec))
			return false;
		rec.t = t;
		rec.p = intersection;
		rec.mat = mat;
		rec.set_face_normal(r, normal);
		return true;

	}
	virtual bool is_interior(double a, double b, hit_record& rec)const {
		interval unit_interval = interval(0, 1);
		if (!unit_interval.contains(a) || !unit_interval.contains(b))
			return false;

		rec.u = a;
		rec.v = b;
		return true;

	}

	//for quad light samling
	vec3 random(const point3& origin)const override {
		//从quad光源处随机产生一个点 sampling point
		auto p = Q + (random_double() * u) + (random_double() * v);
		return p - origin; //返回hit point到采样光源的方向
	}
	double pdf_value(const point3& origin, const vec3& direction)const override { //给定一个光线起点和光线方向，判断该光线对应的光源pdf
		hit_record rec;
		if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
			return 0;
		auto distance_squared = rec.t * rec.t * direction.length_squared();
		
		auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());
		return distance_squared / (cosine * area); 
		//从光源采样对应的dw的概率密度
	}
	
};


inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat) {
	// Returns the 3D box (six sides) that contains the two opposite vertices a & b.
	auto sides = make_shared<hittable_list>();

	auto min = point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
	auto max = point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

	auto dx = vec3(max.x() - min.x(), 0, 0);
	auto dy = vec3(0, max.y() - min.y(), 0);
	auto dz = vec3(0, 0, max.z() - min.z());
	
	sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()), dx, dy, mat)); // front
	sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz, dy, mat)); // right
	sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx, dy, mat)); // back
	sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dz, dy, mat)); // left
	sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()), dx, -dz, mat)); // top
	sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dx, dz, mat)); // bottom
	return sides;
}

#endif