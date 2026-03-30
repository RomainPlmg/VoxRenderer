// ===========================
// Ray structure
// ===========================
struct Ray {
    vec3 origin;
    vec3 dir;
};


// ===========================
// Material structure
// ===========================
#define Diffuse 0
#define Metal 1
#define Glass 2
#define Emit 3

struct Material {
    uint type;
    float weight;
    float rough;
    float spec;
    float ior;
    float att;
    float flux;
    float _pad;
};


// ===========================
// HitInfo structure
// ===========================
struct HitInfo {
    bool didHit;
    float hitDistance;
    vec3 hitPoint;
    vec3 hitNormal;
    uint colorIdx;
};


// ===========================
// Constants
// ===========================
const float EPSILON = 0.0001;
const vec3 SUN_DIR = normalize(vec3(-0.5, 1.0, 0.3));
const vec3 SUN_COLOR = vec3(1.0, 0.9, 0.8);