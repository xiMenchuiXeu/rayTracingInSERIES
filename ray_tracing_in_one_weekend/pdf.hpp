#ifndef PDF_H
#define PDF_H
#include "onb.hpp"
#include "hittable_list.hpp"
extern const double PI;
class pdf {
public:
	virtual ~pdf() {}
	virtual double value(const vec3& direction) const = 0;
	virtual vec3 generate() const = 0;
};

class sphere_pdf : public pdf {
public:
	sphere_pdf() {}
	double value(const vec3& direction) const override {
		return 1 / (4 * pi);
	}
	vec3 generate() const override {
		return random_unit_vector();
	}
};

class cosine_pdf : public pdf {
public:
	cosine_pdf(const vec3& w) : uvw(w) {}
	double value(const vec3& direction) const override {
		auto cosine_theta = dot(unit_vector(direction), uvw.w());
		return std::fmax(0, cosine_theta / pi);
	}
	vec3 generate() const override {
		return uvw.transform(random_cosine_direction());
	}
private:
	onb uvw;
};

class hittable_pdf : public pdf {
	const hittable& objects; //光源
	point3 origin; //hit point
public:
	hittable_pdf(const hittable& objects, const point3& origin)
		: objects(objects), origin(origin) //
	{}
	double value(const vec3& direction) const override { 
		return objects.pdf_value(origin, direction);
	}//得到以光源为采样面积时，direction的采样密度
	vec3 generate() const override {
		return objects.random(origin);
	} //随机得到一条指向光源的采样光线


};


class mixture_pdf : public pdf { //这里采样直接采样，按照0.5直接光照与0.5间接光照进行pdf求解
	shared_ptr<pdf> p[2];
public:
	mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
		p[0] = p0;
		p[1] = p1;
	}

	double value(const vec3& direction)const override {
		return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
	}

	vec3 generate()const override {
		if (random_double() < 0.5)
			return p[0]->generate();
		else
			return p[1]->generate(); //随机生成一个方向
	}
};

#endif