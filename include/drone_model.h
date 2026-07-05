#ifndef DRONE_MODEL_H
#define DRONE_MODEL_H

struct Point3D { float x; float y; float z; };
struct Face { int v1; int v2; int v3; int v4; uint16_t baseColor; };

// --- PASTE YOUR TRUE PYTHON CONVERTER SCRIPT ARRAYS HERE ---
const int TOTAL_VERTICES = 521; // Replace with your real vertex output count
const int TOTAL_FACES = 521;    // Your true triangulated polygon total count

extern const Point3D vertices[TOTAL_VERTICES];
extern const Face faces[TOTAL_FACES];

#endif