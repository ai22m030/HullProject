#include <iostream>
#include <SDL.h>
#include <list>
#include "Hull.h"

using namespace std;

const int POINTS_COUNT = 11;

int main() {
    list<SDL_Point> a;

    srand(time(nullptr));
    for(int i = 0; i < POINTS_COUNT; i++) {
        a.push_back({(10 + (rand() % 90)), (10 + (rand() % 90))});
        //a.push_back(make_pair(points[i].x, points[i].y));
    }

    /*
    a.push_back({110, 50});
    a.push_back({150, 70});
    a.push_back({20, 20});
    a.push_back({70, 30});
    a.push_back({50, 20});
    a.push_back({70, 50});
    a.push_back({50, 60});
    a.push_back(make_pair(20, 20));
    a.push_back(make_pair(70, 30));
    a.push_back(make_pair(50, 20));
    a.push_back(make_pair(70, 50));
    a.push_back(make_pair(50, 60));
    a.push_back(make_pair(110, 50));
    a.push_back(make_pair(150, 70));
    a.push_back(make_pair(120, 30));
    a.push_back(make_pair(150, 30));
    a.push_back(make_pair(160, 50));
    a.push_back(make_pair(120, 70));
     */

    convex::Hull hull(a);

    list<SDL_Point>& ans = hull.divide_conquer();
    for (auto e: ans)
        cout << e.x << " "
             << e.y << endl;

    delete &ans;

    std::cout << "Hello, World!" << std::endl;
    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
        std::cout << "Failed to init SDL : " << SDL_GetError();

    return 0;
}
