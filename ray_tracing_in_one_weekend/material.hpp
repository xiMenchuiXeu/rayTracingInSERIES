#ifndef MATERIAL_H
#define MATERIAL_H
#include "hittable.hpp"
#include "texture.hpp"
#include "pdf.hpp"

class scatter_record {
public:
	color attenuation;
	shared_ptr<pdf> pdf_ptr;
	bool skip_pdf; //镜面反射概率密度函数是一个冲激函数
	ray skip_pdf_ray;
};

class isotropic;
class material {
public:
	virtual ~material() = default;
	virtual bool scatter(
		const ray& r_in, const hit_record& rec, scatter_record& srec) const {
		return false;
	}
	virtual color emitted(
		const ray& r_in, const hit_record& rec, double u, double v, const point3& p
	) const {
		return color(0, 0, 0);
	}
	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const {
		return 0;
	}//importance sampling； scattering PDF
};

class lambertian : public material {
	color albedo; //对rgb三种光吸收的程度
	shared_ptr<texture> tex;
public:
	lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
	lambertian(shared_ptr<texture> tex) : tex(tex) {}
	//albedo代表材质对于不同波长的反射率，范围∈0,1
	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override{
		srec.attenuation = tex->value(rec.u, rec.v, rec.p);
		srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
		srec.skip_pdf = false;
		return true;
	}
	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const override {
		auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
		return cos_theta < 0 ? 0 : cos_theta / pi;
	}
};


class metal : public material {
	color albedo;
	double fuzz;
public:
	metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
		
		srec.attenuation = albedo;
		srec.pdf_ptr = nullptr;
		srec.skip_pdf = true;
		srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());
		return true;
	}
};

class dielectric : public material {
private:
	// Refractive index in vacuum or air, or the ratio of the material's refractive index over
	// the refractive index of the enclosing media
	double refraction_index;  //相对折射率，物体的折射率/包围介质的折射率
	
	static double reflectance(double cosine, double refraction_index) {
		//Schlick's 近似进行frenel的描绘
		auto r0 = (1 - refraction_index) / (1 + refraction_index); //1是空气折射率
		r0 = r0 * r0;
		return r0 + (1 - r0) * std::pow((1 - cosine), 5); //cosine是入射光与法线的夹角
	}
public:
	dielectric(double refraction_index) : refraction_index(refraction_index) {}
	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
		srec.attenuation = color(1.0, 1.0, 1.0);
		srec.pdf_ptr = nullptr;
		srec.skip_pdf = true;
		double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index; //相对折射率的计算
		
		vec3 unit_direction = unit_vector(r_in.direction());
		double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);//入射光线与接触点法线的cos
		double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = ri * sin_theta > 1.0; //是否发生全反射？
		vec3 direction;
		if(cannot_refract || reflectance(cos_theta, ri) > random_double()) 
			direction = reflect(unit_direction, rec.normal);//发生反射
		else//随机选择光线是折射还是反射
			direction = refract(unit_direction, rec.normal, ri);//发生折射
		
		srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());
		return true;
	}
};

class diffuse_light : public material {
public:
	diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
	diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}
	color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p)
		const override {
		if (!rec.front_face)
			return color(0, 0, 0); //如果不是从物体外面打入光源就返回0(也就是黑色)
		return tex->value(u, v, p);
	}
	//光源没重写scatter所以直接返回false
	//if (!rec.mat->scatter(r, rec, attenuation, scattered)) //遇到光源直接return color_from_emission;
private:
	shared_ptr<texture> tex;
};

//各向同性介质，随机产生一个反射方向
class isotropic : public material {
	shared_ptr<texture> tex;
public:
	isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
	isotropic(shared_ptr<texture> tex) : tex(tex) {}
	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
		srec.attenuation = tex->value(rec.u, rec.v, rec.p);
		srec.pdf_ptr = make_shared<sphere_pdf>();
		srec.skip_pdf = false;
		return true;
	}
	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const override {
		return 1 / (4 * pi);
	}
};

#endif