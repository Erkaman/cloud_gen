#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>

#include <vector>
#include <string>

using std::vector;
using std::string;
using std::to_string;

#define PI 3.14

const int WIDTH = 1000;
const int HEIGHT = 1000;

//if creating a cloud takes more attempts than this, give up on that cloud.
const int MAX_ATTEMPTS = 10;

string cloudStr = "";
string defStr = "";

// in range [0,1]
float randFloat() {
    return rand() / (float)RAND_MAX;
}

// in range [min, max]
float randFloat(float min, float max) {
    return min + (max - min) * randFloat();
}

class vec2 {
public:
    float x;
    float y;

    vec2(float x, float y) {
        this->x = x;
        this->y = y;
    }

    vec2() {
        this->x = this->y = 0;
    }

    friend vec2 operator-(const vec2& a, const vec2& b) {
        return vec2(
            a.x - b.x,
            a.y - b.y
            );
    }

    friend vec2 operator+(const vec2& a, const vec2& b) {
        return vec2(

            a.x + b.x,
            a.y + b.y
            );
    }

    friend vec2 operator*(const float f, const vec2& a) {
        return vec2(
            f * a.x,
            f * a.y
            );
    }

    static float length(const vec2& a) {
        return sqrt(vec2::dot(a, a));
    }

    static vec2 normalize(const vec2& a) {
        return (1.0f / vec2::length(a)) * a;
    }

    static float dot(const vec2& a, const vec2& b) {
        return a.x*b.x + a.y*b.y;
    }

    static float distance(const vec2& a, const vec2& b) {
        return length(a - b);
    }
};

class AABB {
public:
    vec2 min;
    vec2 max;

    AABB() {
        min.x = FLT_MAX;
        min.y = FLT_MAX;

        max.x = -FLT_MAX;
        max.y = -FLT_MAX;
    }

    // check if AABBs collide.
    bool collides(AABB other) {
        float awidth = this->max.x - this->min.x;
        float aheight = this->max.y - this->min.y;

        float ax = this->min.x + (awidth) * 0.5f;
        float ay = this->min.y + (aheight) * 0.5f;

        float bwidth = other.max.x - other.min.x;
        float bheight = other.max.y - other.min.y;

        float bx = other.min.x + (bwidth) * 0.5f;
        float by = other.min.y + (bheight) * 0.5f;

        return (fabs(ax - bx) * 2 < (awidth + bwidth)) &&
            (fabs(ay - by) * 2 < (aheight + bheight));

    }
};

// aabbs of already generated clouds.
vector<AABB> aabbs;

void genCloud(
    const int N,
    const float RX, const float RY,
    const float MIN_HUMP_RAD,
    const float MAX_HUMP_RAD,
    const float HUMP_RAND
    ) {

    int attempts = 0;
    while(true) {

        if(attempts > MAX_ATTEMPTS) {
            return; // Too many attempts. GIVE UP!
        }

        // cloud position.
        float PX = randFloat(0, WIDTH);
        float PY = randFloat(0, HEIGHT);

        vector<vec2> pos(N);

        float ANGULAR_SHIFT = randFloat(0.0f, 2.0f);

        // generate points on an ellipse.
        for(int i = 0; i < N; i++) {
            float theta = (i / (float)N) * 2.0 * PI;
            pos[i].x = PX + RX * cos(theta + ANGULAR_SHIFT);
            pos[i].y = PY + RY * sin(theta + ANGULAR_SHIFT);
        }

        vec2 prev;

        AABB aabb;

        string str = "";

        for(int i = 0; i < (N+1); i++) {
            vec2 v = pos[(i) % N];

            if(i == 0) {
                // for first point, we just position the cursor.
                str += "<path d=\"M" + to_string(v.x) + ", " + to_string(v.y);
            } else {
                // every edge in the ellipse is used to create a cubic bezier curve.
                // we create a hump-shaped cubic bezier curve.

                // edge direction.
                vec2 dir = vec2::normalize(v - prev);
                // edge normal.
                vec2 n = vec2::normalize(vec2(dir.y, -dir.x));

                // hump radius.
                float RAD = randFloat(MIN_HUMP_RAD, MAX_HUMP_RAD);

                // control points. a cubic bezier curve has two control points.
                // if we place the control points along the edge normal, we
                // get a hump shape.
                vec2 cp0 = prev + RAD * n;
                vec2 cp1 = v +    RAD *n;

                float UA = -HUMP_RAND;
                float UB = +HUMP_RAND;

                // but for some variation, we also randomly displace the control points
                // some.
                cp0.x += randFloat(UA, UB);
                cp0.y += randFloat(UA, UB);
                cp1.x += randFloat(UA, UB);
                cp1.x += randFloat(UA, UB);

                vec2 p0 = prev;
                vec2 p1 = cp0;
                vec2 p2 = cp1;
                vec2 p3 = v;

                // we need the AABB of the cloud.
                // so traverse the bezier curve from 'p0' to 'p3' so that we can determine AABB.
                // if we traverse enough points on the curve, that should be enough.
                // this is only an approximate solution. An exact analytic solution
                // may exist, but I'm too lazy find one :D
                for(float t = 0; t < 1.0; t +=0.01f) {
                    vec2 f =
                        1.0f *  (1.0f - t) * (1.0f - t) * (1.0f - t) * p0 +
                        3.0f *   t         * (1.0f - t) * (1.0f - t) * p1 +
                        3.0f *   t         * t          * (1.0f - t) * p2 +
                        1.0f *   t         * t          * t          * p3;

                    if(f.x < aabb.min.x) {
                        aabb.min.x = f.x;
                    }
                    if(f.y < aabb.min.y) {
                        aabb.min.y = f.y;
                    }

                    if(f.x > aabb.max.x) {
                        aabb.max.x = f.x;
                    }
                    if(f.y > aabb.max.y) {
                        aabb.max.y = f.y;
                    }
                }

                // output cubic bezier curve.
                str += "C " + to_string(cp0.x) + " " + to_string(cp0.y) + ", "
                    +        to_string(cp1.x) + " " + to_string(cp1.y) + ", "
                    +        to_string(v.x) + " " + to_string(v.y);
            }
            prev = v;
        }

        str += "Z\n"; // close loop. path is now done.
        str += "\" />";

        if(aabb.min.x < 0 || aabb.min.y < 0 || aabb.max.x > WIDTH || aabb.max.y > HEIGHT) {
            continue; // doesn't look good if parts of the cloud is outside image. REJECT.
        }

        attempts++;

        bool collides = false;
        for(int i = 0; i < aabbs.size(); i++) {
            AABB other = aabbs[i];
            if(aabb.collides(other)) {
                collides = true;
            }
        }
        // if collides with already existing cloud, REJECT.
        if(collides)
            continue;

        // otherwise, ACCEPT the cloud.
        aabbs.push_back(aabb);
        cloudStr += str;
        break;
    }
}

int main(int argc, char** argv) {
    int SEED = 0;
    srand(SEED);

    // 0: blue_sky.svg
    // 1: dawn.svg
    // 2: storm.svg
    // 3: night.svg
    int TYPE = 0;

    if(TYPE == 0) {
        // blue_sky.svg

        defStr +=
            "<linearGradient id=\"cloudGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
            "<stop offset=\"50%\" stop-color=\"#ffffff\"/>"
            "<stop offset=\"100%\" stop-color=\"#8888bb\"/>"
            "</linearGradient>";
        defStr +=
            "<linearGradient id=\"backgroundGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#559dcc\"/>"
            "<stop offset=\"50%\" stop-color=\"#559dcc\"/>"

            "<stop offset=\"100%\" stop-color=\"#99dfee\"/>"
            "</linearGradient>";
        while(aabbs.size() < 8 ) {
            genCloud(int(randFloat(6,9)), // humps.
                     randFloat(75.0f,130.0f), // ellipse width
                     randFloat(50.0f,65.0f), // ellipse height
                     randFloat(29.0f,39.0f), randFloat(40.0f,48.0f), // hump radius
                     randFloat(17.0f, 27.0f) // hump rand.
                );
        }
        while(aabbs.size() < 21 ) {
            genCloud(int(randFloat(6,9)), // humps.
                     randFloat(40.0f,80.0f), // ellipse width
                     randFloat(20.0f,35.0f), // ellipse height,
                     randFloat(14.0f,16.0f), randFloat(22.0f,24.0f), // hump radius
                     randFloat(4.0f, 9.0f) // hump rand.
                );
        }
    } else if(TYPE == 1) {
        defStr +=
            "<linearGradient id=\"cloudGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#aaaaaa\"/>"
            "<stop offset=\"100%\" stop-color=\"#cc7777\"/>"
            "</linearGradient>";
        defStr +=
            "<linearGradient id=\"backgroundGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#8E728B\"/>"

            "<stop offset=\"65%\" stop-color=\"#8E728B\"/>"

            "<stop offset=\"100%\" stop-color=\"#FC8F5F\"/>"
            "</linearGradient>";

        while(aabbs.size() < 15 ) {
            genCloud(int(randFloat(7,11)), // humps.
                     randFloat(80.0f,120.0f), // ellipse width
                     randFloat(20.0f,35.0f), // ellipse height,
                     randFloat(19.0f,21.0f), randFloat(24.0f,25.0f), // hump radius
                     randFloat(4.0f, 9.0f) // hump rand.
                );
        }

        while(aabbs.size() < 25 ) {
            genCloud(int(randFloat(5,7)), // humps.
                     randFloat(30.0f,50.0f), // ellipse width
                     randFloat(20.0f,40.0f), // ellipse height,
                     randFloat(15.0f,17.0f), randFloat(27.0f,29.0f), // hump radius
                     randFloat(8.0f, 12.0f) // hump rand.
                );
        }
    } else if(TYPE == 2) {
        defStr +=
            "<linearGradient id=\"cloudGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#aaaaaa\"/>"
            "<stop offset=\"10%\" stop-color=\"#aaaaaa\"/>"

            "<stop offset=\"80%\" stop-color=\"#444444\"/>"

            "<stop offset=\"100%\" stop-color=\"#333333\"/>"
            "</linearGradient>";
        defStr +=
            "<linearGradient id=\"backgroundGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
            "<stop offset=\"50%\" stop-color=\"#777777\"/>"
            "<stop offset=\"100%\" stop-color=\"#aaaaaa\"/>"

            "</linearGradient>";
        while(aabbs.size() < 3 ) {
            genCloud(int(randFloat(8,13)), // humps.
                     randFloat(170.0f,260.0f), // ellipse width
                     randFloat(80.0f,120.0f), // ellipse height
                     randFloat(39.0f,40.0f), randFloat(60.0f,70.0f), // hump radius
                     randFloat(17.0f, 27.0f) // hump rand.
                );
        }

        while(aabbs.size() < 12 ) {
            genCloud(int(randFloat(6,9)), // humps.
                     randFloat(40.0f,80.0f), // ellipse width
                     randFloat(20.0f,35.0f), // ellipse height,
                     randFloat(14.0f,16.0f), randFloat(22.0f,24.0f), // hump radius
                     randFloat(4.0f, 9.0f) // hump rand.
                );
        }

        while(aabbs.size() < 15 ) {
            genCloud(int(randFloat(6,11)), // humps.
                     randFloat(110.0f,160.0f), // ellipse width
                     randFloat(20.0f, 50.0f), // ellipse height,
                     randFloat(19.0f,21.0f), randFloat(27.0f,29.0f), // hump radius
                     randFloat(10.0f, 19.0f) // hump rand.
                );
        }
    } else if( TYPE == 3) {
        defStr +=
            "<linearGradient id=\"cloudGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
              "<stop offset=\"10%\" stop-color=\"#666699\"/>"
            "<stop offset=\"50%\" stop-color=\"#444488\"/>"

            "</linearGradient>";
        defStr +=
            "<linearGradient id=\"backgroundGradient\"  x1=\"0\" x2=\"0\" y1=\"0\" y2=\"1\" >"
            "<stop offset=\"0%\" stop-color=\"#000055\"/>"
            "<stop offset=\"100%\" stop-color=\"#333355\"/>"

            "</linearGradient>";

        while(aabbs.size() < 12 ) {
            genCloud(int(randFloat(4,12)), // humps.
                     randFloat(40.0f,80.0f), // ellipse width
                     randFloat(20.0f,35.0f), // ellipse height,
                     randFloat(17.0f,20.0f), randFloat(23.0f,27.0f), // hump radius
                     randFloat(4.0f, 9.0f) // hump rand.
                );
        }
    }


    //
    // We have created all clouds and gradients. Now just output the SVG.
    //

    FILE* fp;
    fp = stdout;

    fprintf(fp, "<svg width=\"%f\" height=\"%f\" version=\"1.1\" baseProfile=\"full\" xmlns=\"http://www.w3.org/2000/svg\">\n", (float)WIDTH, (float)HEIGHT);

    // output gradient.
    fprintf(fp, "<defs>%s</defs>", defStr.c_str());

    // set background color.
    fprintf(fp,"<rect ");
    fprintf(fp,"width=\"%d\" ", WIDTH);
    fprintf(fp,"height=\"%d\" ", HEIGHT);
    fprintf(fp,"x=\"0\" ");
    fprintf(fp,"y=\"0\" ");
    fprintf(fp,"fill=\"url(#backgroundGradient)\"/>\n");

    // default appearance of all clouds.
    fprintf(fp,"<g fill=\"url(#cloudGradient)\" stroke=\"none\"  stroke-width=\"1\" fill-opacity=\"1.0\">");

    // output clouds.
    fprintf(fp, "%s", cloudStr.c_str());


    fprintf(fp,"</g>");
    fprintf(fp, "</svg>");
    fclose(fp);

    return 0;
}
