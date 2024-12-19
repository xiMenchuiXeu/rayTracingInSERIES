#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H
#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

class constant_medium : public hittable { //�㶨�ܶȵ�������ʣ���������
	//ɢ�����prob = C * delta T 
	//C��optical density������
	shared_ptr<hittable> boundary;
	double neg_inv_density;
	//��������ܶȳɷ��ȣ���ʾ������ϡ���������Ҫ�����ľ�����ܷ���ɢ��
	shared_ptr<material> phase_function; //����ɢ��
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
		hit_record rec1, rec2;//�������constant medium�����ཻ�жϵõ�����������
		if (!boundary->hit(r, interval::universe, rec1))
			return false;
		if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
			return false;
		//���жϹ����������û���ཻ
		if (rec1.t < ray_t.min) rec1.t = ray_t.min;
		//���rec1�Ǹ�������ô���Ǵ������ڲ�������ġ�
		//��ôrec1�͸ĳ�0.001
		if (rec2.t > ray_t.max) rec2.t = ray_t.max;
		//���rec2������interval��Χ����ôrec2�ĳ�interval�����ֵ
		if (rec1.t >= rec2.t)
			return false;
		if (rec1.t < 0)
			rec1.t = 0;
		auto ray_length = r.direction().length(); //���ߵĵ�λ����
		auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length; //�����ڽ����ڲ��ߵľ���
		auto hit_distance = neg_inv_density * std::log(random_double()); //���һ�����߷���ɢ��ľ���
		if (hit_distance > distance_inside_boundary)//��������ɢ�������ڹ����ڽ����ڲ��ľ�����ô�Ͳ�ɢ�䣻�൱�ڲ�hit
			return false;
		rec.t = rec1.t + hit_distance / ray_length;//ɢ���ʱ���
		rec.p = r.at(rec.t); //ɢ����λ��
		rec.normal = vec3(1, 0, 0); // arbitrary�������һ������ֵ
		rec.front_face = true; // also arbitrary
		rec.mat = phase_function;//�������
		return true;
	}

};

#endif
