#ifndef ONB_H
#define ONB_H
#include "vec3.hpp"
class onb {
	vec3 axis[3]; //onb坐标系三个坐标轴
public:
	onb(const vec3& n) { //构造函数传入的是法线
		axis[2] = unit_vector(n);
		vec3 a = (std::fabs(axis[2].x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0); //如果法线的x分量过大就选择y,避免选择的轴与法线平行
		axis[1] = unit_vector(cross(axis[2], a));
		axis[0] = cross(axis[2], axis[1]);
	} 
	const vec3& u() const { return axis[0]; }
	const vec3& v() const { return axis[1]; }
	const vec3& w() const { return axis[2]; }
	vec3 transform(const vec3& v) const { //切换回世界坐标
		
		return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
	}
};

#endif