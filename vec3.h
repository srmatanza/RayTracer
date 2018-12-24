#pragma once

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include "pcg_random.hpp"
#include "float.h"

using namespace std;

#if USE_DOUBLE
typedef double tfp;
tfp TRACER_MAXFLOAT = DBL_MAX;
#else
typedef float tfp;
tfp TRACER_MAXFLOAT = FLT_MAX;
#endif

tfp drand() {
	static pcg_extras::seed_seq_from<random_device> seed_source;
	static pcg32 rng(seed_source);

	uint32_t x = rng();
	return (static_cast<tfp>(x) / static_cast<tfp>(UINT32_MAX));
};

class vec3 {
private:
	tfp e[3];

public:
	vec3() {}

	vec3(const vec3& anInst) {
		e[0] = anInst[0];
		e[1] = anInst[1];
		e[2] = anInst[2];
	}

	vec3(tfp e0, tfp e1, tfp e2) {
		e[0] = e0;
		e[1] = e1;
		e[2] = e2;
	}

	tfp x() const { return e[0]; }
	tfp y() const { return e[1]; }
	tfp z() const { return e[2]; }
	tfp r() const { return e[0]; }
	tfp g() const { return e[1]; }
	tfp b() const { return e[2]; }

	const vec3& operator+() const { return *this; }
	vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	tfp operator[](int i) const { return e[i]; }
	tfp& operator[](int i) { return e[i]; }

	vec3& operator+=(const vec3 &v);
	vec3& operator-=(const vec3 &v);
	vec3& operator*=(const vec3 &v);
	vec3& operator/=(const vec3 &v);
	vec3& operator*=(const tfp t);
	vec3& operator/=(const tfp t);

	tfp length() const {
		return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	}

	tfp squared_length() const {
		return (e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	}

	void make_unit_vector();
};

istream& operator>>(istream &is, vec3 &t) {
	is >> t[0] >> t[1] >> t[2];
	return is;
}

ostream& operator<<(ostream& os, const vec3 &t) {
	os << t.x() << " " << t.y() << " " << t.z();
	return os;
}

void vec3::make_unit_vector() {
	tfp k = 1.0 / length();
	e[0] *= k; e[1] *= k; e[2] *= k;
}

vec3 operator+(const vec3& v1, const vec3& v2) {
	return vec3(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z());
}

vec3 operator-(const vec3& v1, const vec3& v2) {
	return vec3(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z());
}

vec3 operator*(const vec3& v1, const vec3& v2) {
	return vec3(v1.x() * v2.x(), v1.y() * v2.y(), v1.z() * v2.z());
}

vec3 operator/(const vec3& v1, const vec3& v2) {
	return vec3(v1.x() / v2.x(), v1.y() / v2.y(), v1.z() / v2.z());
}

vec3 operator*(tfp t, const vec3& v) {
	return vec3(t*v.x(), t*v.y(), t*v.z());
}

vec3 operator*(const vec3& v, tfp t) {
	return vec3(t*v.x(), t*v.y(), t*v.z());
}

vec3 operator/(const vec3& v, tfp t) {
	return vec3(v.x() / t, v.y() / t, v.z() / t);
}

tfp dot(const vec3& v1, const vec3& v2) {
	return v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
}

vec3 cross(const vec3& v1, const vec3& v2) {
	return vec3(
		(v1.y()*v2.z() - v1.z()*v2.y()),
		-(v1.x()*v2.z() - v1.z()*v2.x()),
		(v1.x()*v2.y() - v1.y()*v2.x()));
}

vec3& vec3::operator+=(const vec3 &v) {
	e[0] += v[0];
	e[1] += v[1];
	e[2] += v[2];
	return *this;
}

vec3& vec3::operator*=(const vec3 &v) {
	e[0] *= v[0];
	e[1] *= v[1];
	e[2] *= v[2];
	return *this;
}

vec3& vec3::operator/=(const vec3 &v) {
	e[0] /= v[0];
	e[1] /= v[1];
	e[2] /= v[2];
	return *this;
}

vec3& vec3::operator-=(const vec3 &v) {
	e[0] -= v[0];
	e[1] -= v[1];
	e[2] -= v[2];
	return *this;
}

vec3& vec3::operator*=(const tfp t) {
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}

vec3& vec3::operator/=(const tfp t) {
	tfp k = 1.0 / t;
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}

vec3 unit_vector(const vec3& v) {
	return v / v.length();
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n)*n;
}

tfp schlick(tfp cosine, tfp ref_idx) {
	tfp r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

bool refract(const vec3& v, const vec3& n, tfp ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);
	tfp dt = dot(uv, n);
	tfp discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0) {
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else {
		return false;
	}
}
