// Tracer.cpp : Defines the entry point for the application.
//

#include "Tracer.h"

#include <stdlib.h>
#include <thread>
#include <mutex>

#include "vec3.h"
#include "ray.h"

#include "material.h"

#include "sphere.h"
#include "hitableList.h"

#include "camera.h"

using namespace std;

uint64_t gTotalRays = 0;
double dtLockTotalms = 0;

struct pixel {
	int u;
	int v;
	uint32_t color;

	pixel(int ux, int vx, uint32_t c) {
		u = ux; v = vx; color = c;
	}
};

struct sceneState {
	int nx;
	int ny;
	camera *cam;
	hitable *world;
	pixel* pixelBuffer;
};

mutex g_pixelLock;
mutex g_uvCoords_mutex;
mutex g_ioLock;

#define MAX_DEPTH 75

vec3 color(const ray& r, hitable *world, int &raysCast, int depth=0) {

	raysCast++;
	
	hit_record rec;
	if (world->hit(r, 0.001, TRACER_MAXFLOAT, rec)) {

		ray scattered;
		vec3 attenuation;
		if (depth < MAX_DEPTH && rec.pMat->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, raysCast, depth+1);
		}
		else {
			return vec3(0.0, 0.0, 0.0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		tfp t = 0.5*(unit_direction.y() + 1.0);
		vec3 oneVector(1.0, 1.0, 1.0);
		vec3 skyColor(0.5, 0.7, 1.0);
		return (1.0 - t)*oneVector + t * skyColor;
	}
}

uint32_t colorPixel(int u, int v, int nx, int ny, int ns, camera* cam, hitable* world, int& totalRays) {
	// This is what should be scheduled
	vec3 col(0, 0, 0);
	for (int s = 0; s < ns; s++) {
		tfp ux = tfp(u + drand()) / tfp(nx);
		tfp vx = tfp(v + drand()) / tfp(ny);
		ray r = cam->get_ray(ux, vx);

		int raysCast = 0;
		col += color(r, world, raysCast);
		totalRays += raysCast;
	}

	col /= tfp(ns);
	col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

	int ir = int(255.99*col[0]);
	int ig = int(255.99*col[1]);
	int ib = int(255.99*col[2]);

	uint32_t pixelColor = 0;
	pixelColor |= 0xff000000 & (ir << 24);
	pixelColor |= 0x00ff0000 & (ig << 16);
	pixelColor |= 0x0000ff00 & (ib << 8);

	return pixelColor;
}

hitable* aWholeNewWorld() {

	int iNumSpheres = 30;
	hitable** list = (hitable**)malloc(iNumSpheres*sizeof(hitable*));

	vec3 earthLocation = vec3(0, -100.5, -1);
	// The 'earth'
	list[0] = new sphere(earthLocation, 100, new lambertian(vec3(0.8, 0.75, 0.2)));
	//list[0] = new sphere(earthLocation, 100, new metal(vec3(0.8, 0.6, 0.2), 0.2));

	list[1] = new sphere(vec3(0, 3.0, -6.0), 1.5, new dielectric(1.5));
	list[1] = new sphere(vec3(0, 3.0, -6.0), -1.45, new dielectric(1.5));
	
	// The spheres
	for (int i = 2; i < iNumSpheres; i++) {
		tfp sX, sZ, sR;
		
		tfp matRand = drand();
		material *pMaterial;
		if (matRand > 0.66) {
			pMaterial = new metal(vec3(drand(), drand(), drand()), 0.075);
		}
		else if (matRand > 0.33) {
			pMaterial = new lambertian(vec3(drand(), drand(), drand()));
		}
		else {
			pMaterial = new lambertian(vec3(drand(), drand(), drand()));
		}

		tfp box = 4.0;
		tfp maxSize = 0.3;

		sX = box * drand()* 2.0 - box;
		sZ = box * drand()* 2.0 - box;
		sR = maxSize * drand() + 0.05;
		vec3 newSphere = vec3(sX, 0, sZ) - earthLocation;
		newSphere.make_unit_vector();
		newSphere *= (100.0 + sR);
		newSphere = earthLocation + newSphere;

		list[i] = new sphere(newSphere, sR, pMaterial);
	}

	return new hitable_list(list, 30);
}

int main(int argc, char** argv)
{
	auto start = chrono::steady_clock::now();
	int nx = 400;
	int ny = 200;
	int ns = 200;
	int maxThreads = thread::hardware_concurrency();

	if (argc > 1) {
		long newSamples = atol(argv[1]);
		ns = static_cast<int>(newSamples);
	}

	if (argc > 3) {
		long newX = atol(argv[2]);
		long newY = atol(argv[3]);
		nx = static_cast<int>(newX);
		ny = static_cast<int>(newY);
	}

	if (argc > 4) {
		long tc = atol(argv[4]);
		maxThreads = tc;
	}

	//for (int i = 0; i < ns; i++) {
	//	cout << drand() << endl;
	//}

	uint32_t* imgBuffer = (uint32_t*)malloc(nx * ny * sizeof(uint32_t));

	/*
	hitable *list[5];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.2));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	hitable *world = new hitable_list(list, sizeof(list) / sizeof(*list));
	*/
	hitable *world = aWholeNewWorld();
	camera cam;

	vector<pair<int, int>> uvCoords;
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			uvCoords.push_back(make_pair(i, j));
		}
	}

	vector<thread*> threads;
	auto processPixels = [&uvCoords, world, &cam, nx, ny, ns, imgBuffer]() {
		int batchSize = 2500;
		int totalPixels = 0;
		int threadTotalRays = 0;
		double dT = 0;

		vector<pixel> localPixels;
		
		auto beforeLoop = chrono::steady_clock::now();
		while (uvCoords.size() > 0) {
			vector<pair<int, int>> pixelBatch;
			
			auto beforeLock = chrono::steady_clock::now();
			{
				lock_guard<mutex> guard(g_uvCoords_mutex);
				while (uvCoords.size() > 0 && pixelBatch.size() < batchSize) {
					pixelBatch.push_back(uvCoords.back());
					uvCoords.pop_back();
				}
			}
			auto afterLock = chrono::steady_clock::now();
			dT += chrono::duration <double, milli>((afterLock - beforeLock)).count();

			for (auto it = pixelBatch.begin(); it < pixelBatch.end(); it++) {
				int totalRays = 0;
				uint32_t pxColor = colorPixel(it->first, it->second, nx, ny, ns, &cam, world, totalRays);
				threadTotalRays += totalRays;
				localPixels.push_back(pixel(it->first, it->second, pxColor));
				totalPixels++;
			}

			for (auto it = localPixels.begin(); it < localPixels.end(); it++) {
				int iPixelIndex = (ny - it->v - 1)*nx + it->u;
				imgBuffer[iPixelIndex] = it->color;
			}
		}
		auto afterLoop = chrono::steady_clock::now();
		auto dtL = chrono::duration <double, milli>((afterLoop - beforeLoop)).count();

		{
			lock_guard<mutex> guard(g_ioLock);
			cout << "Processed " << totalPixels << "px in " << dtL << "ms (which is " << ((totalPixels*1000.0) / dtL) << " pixels per sec)\n";
		}

		gTotalRays += threadTotalRays;
		dtLockTotalms += dT;
	};

	for (int i = 1; i < maxThreads; i++) {
		thread* t = new thread(processPixels);
#if WIN32
#else
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i, &cpuset);
		int rc = pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
#endif
		threads.push_back(t);
	}

	auto beginColoring = chrono::steady_clock::now();

	// Make sure the main thread has something to do
	processPixels();

	for (auto it = threads.begin(); it < threads.end(); it++) {
		thread* t = *it;
		t->join();
	}

	auto beginImage = chrono::steady_clock::now();
	/*
	for (auto it = pixels.begin(); it < pixels.end(); it++) {
		pixel px = *it;
		imgBuffer[((ny - px.v - 1)*nx) + px.u] = px.color;
	}
	*/
	
	ofstream ppm;
	ppm.open("image.ppm", ios::out | ios::binary);
	ppm << "P3" << endl << nx << " " << ny << "\n255\n";

	for (int i = 0; i < nx*ny; i++) {
		uint32_t p = imgBuffer[i];
		int ir = (p >> 24) & 0xff;
		int ig = (p >> 16) & 0xff;
		int ib = (p >> 8) & 0xff;
		ppm << ir << " " << ig << " " << ib << endl;
	}

	ppm.close();
	auto doneWriting = chrono::steady_clock::now();

	auto dtSetup = beginColoring - start;
	auto dtMT = beginImage - beginColoring;
	auto dtImg = doneWriting - beginImage;
	cout << "We cast " << gTotalRays << " rays." << endl;
	cout << "Setup time: " << chrono::duration <double, milli>(dtSetup).count() << " ms" << endl;
	cout << "The multithreaded part: " << chrono::duration <double, milli>(dtMT).count() << " ms" << endl;
	cout << "Time spent locking and unlocking the mutex: " << dtLockTotalms << " ms" << endl;
	cout << "Writing out the image: " << chrono::duration <double, milli>(dtImg).count() << " ms" << endl;
	return 0;
}

