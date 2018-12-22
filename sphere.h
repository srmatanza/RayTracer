#pragma once

#include "hitable.h"

class sphere : public hitable {
public:
	sphere() {}
	sphere(vec3 center, tfp r, material* mat) : center(center), radius(r), pMat(mat) {}
	virtual bool hit(const ray& r, tfp tmin, tfp tmax, hit_record& rec) const;
private:
	vec3 center;
	tfp radius;
	material* pMat;
};

bool sphere::hit(const ray& r, tfp tmin, tfp tmax, hit_record& rec) const {
	vec3 oc = r.origin() - center;
	tfp a = dot(r.direction(), r.direction());
	tfp b = dot(oc, r.direction());
	tfp c = dot(oc, oc) - radius * radius;
	tfp discriminant = b*b - a*c;
	rec.pMat = pMat;
	if (discriminant > 0) {
		tfp temp = (-b - sqrt(b*b - a*c)) / a;
		if (temp < tmax && temp > tmin) {
			rec.t = temp;
			rec.p = r.point_at(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
		temp = (-b + sqrt(b*b - a*c)) / a;
		if (temp < tmax && temp > tmin) {
			rec.t = temp;
			rec.p = r.point_at(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
	}
	return false;
}
