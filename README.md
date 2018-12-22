# RayTracer
This is my version of the code for the Ray Tracing in One Weekend book.

![ray tracing sample](https://raw.githubusercontent.com/srmatanza/RayTracer/master/sample_1.png "My first attempt at ray tracing! 400x800")

Shirley's advice on parallelizing this code in the book is to generate groups of images with fewer samples per pixel in parallel and then combine the images. Instead, my approach is to create a pool of threads that pulls ray coordinates off of a queue and completely colors each pixel in parallel. I'm a little disappointed that it doesn't seem to be scaling linearly, as you can see by my results below (the parameters to Tracer are all optional and are the number of samples, the width and height of the image, and the number of threads to use):

These results are from a Ryzen 5 1600X Six-core processor running at stock speeds (~3Ghz) on Windows 10:

One Thread
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 1
We cast 170163774 rays.

real    0m19.389s
```
Two Threads
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 2
We cast 170068362 rays.

real    0m16.837s
```
Four threads
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 4
We cast 170044347 rays.

real    0m15.947s
```
Eight threads
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 8
We cast 169823686 rays.

real    0m12.947s
```
Twelve threads
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 12
We cast 169719126 rays.

real    0m12.165s
```
Twenty Four threads
```
$> time ./build/x64-Release/Tracer.exe 200 800 400 24
We cast 169714345 rays.

real    0m11.778s
```
