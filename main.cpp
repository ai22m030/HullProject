#include <iostream>
#include <random>
#include <SDL.h>
#include <list>
#include <chrono>
#include <thread>
#include <semaphore>
#include "Hull.h"

using namespace std;

const int POINTS_COUNT = 11;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static list<SDL_Point> global_list;
static list<SDL_Point>* final_list;

std::binary_semaphore
        convexSignalMainToThread{0},
        convexSignalThreadToMain{0};

class Calculator {
public:
    const std::list<SDL_Point>* const point_list;

    explicit Calculator(list<SDL_Point>& point_list): point_list(&point_list) {
        point_list.sort(convex::Hull::PointComparator());
    };

    void calc() {
        final_list = this->divide(const_cast<list<SDL_Point> *>(this->point_list));
    }

    /*
    list<SDL_Point>* calc() {
        return this->divide(const_cast<list<SDL_Point> *>(this->point_list));
    }
     */

    list<SDL_Point>* divide(list<SDL_Point>* points) const {
        if (points->size() <= 3) {
            return this->brute(*points);
        }

        auto* left = new list<SDL_Point>;
        auto* right = new list<SDL_Point>;
        auto it_left = left->begin();
        auto it_right = right->begin();
        auto it_right_begin = points->begin();

        auto left_hull_size = ((int)points->size() + 2 - 1) / 2;
        auto right_hull_size = (int)points->size() / 2;
        std::advance(it_right_begin, left_hull_size);

        left->insert( it_left,
                      points->begin(),
                      next( points->begin(), left_hull_size));


        right->insert(it_right,
                      it_right_begin,
                      next(it_right_begin, right_hull_size ));

        if(this->point_list->size() != points->size()) {
            delete points;
        }

        return Calculator::merge(*this->divide(left),*this->divide(right));
    }

    static list<SDL_Point>* merge(list<SDL_Point>& left_hull, list<SDL_Point>& right_hull) {
        auto* list = new std::list<SDL_Point>;

        /**
         * TODO
         *
         * Implement merging here
         *
         * 1.Step search rightest point in left_hull
         * 2.Step search leftest point in right_hull
         * 3.Build upper and lower tangents
         * 4.Remove points between tangents
         * 5.Merge hulls to list / delete right and left hull
         */
        list->insert(list->end(), left_hull.begin(), left_hull.end());
        list->insert(list->end(), right_hull.begin(), right_hull.end());

        global_list.insert(global_list.end(), left_hull.begin(), left_hull.end());
        global_list.insert(global_list.end(), right_hull.begin(), right_hull.end());

        delete &left_hull;
        delete &right_hull;

        return list;
    }

    static int orientation(SDL_Point p_line_1, SDL_Point p_line_2, SDL_Point p_check) {
        return ((p_line_1.x - p_check.x) * (p_line_2.y - p_check.y)) -
            ((p_line_1.y - p_check.y) * (p_line_2.x - p_check.x));
    }

    list<SDL_Point>* brute(list<SDL_Point>& points) const {
        auto* list = new std::list<SDL_Point>;

        if(points.size() == 1 || points.size() == 2) {
            list->insert(list->end(), points.begin(), points.end());
            delete &points;

            return list;
        }

        SDL_Point point[3];
        int i = 0;

        for(auto &it : points) {
            point[i] = it;
            i++;
        }

        auto orient = Calculator::orientation(point[0],point[1],point[2]);

        if(orient < 0) {
            list->insert(list->end(), point[2]);
            list->insert(list->end(), point[0]);
            list->insert(list->end(), point[1]);
        } else
            list->insert(list->end(), points.begin(), points.end());

        if(this->point_list->size() != points.size())
            delete &points;

        return list;
    }
};

bool comparePoints(SDL_Point a, SDL_Point b) {
    if(a.x == b.x && a.y == b.y)
        return true;

    return false;
}

void calculator_t(list<SDL_Point> init_list) {
    convexSignalMainToThread.acquire();
    auto c = Calculator(init_list);

    c.calc();

    std::this_thread::sleep_for(3s);
    convexSignalThreadToMain.release();
}

int main() {
    list<SDL_Point> init_list;
    SDL_Point points [POINTS_COUNT];
    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<double> dist(10.0, 100.0);

    for(auto &point : points) {
        point = {(int)dist(mt), (int)dist(mt)};
        init_list.push_back(point);
    }

    /*
    init_list.push_back({110, 50});
    init_list.push_back({150, 70});
    init_list.push_back({20, 20});
    init_list.push_back({70, 30});
    init_list.push_back({50, 20});
    init_list.push_back({70, 50});
    init_list.push_back({50, 60});
    init_list.push_back(make_pair(20, 20));
    init_list.push_back(make_pair(70, 30));
    init_list.push_back(make_pair(50, 20));
    init_list.push_back(make_pair(70, 50));
    init_list.push_back(make_pair(50, 60));
    init_list.push_back(make_pair(110, 50));
    init_list.push_back(make_pair(150, 70));
    init_list.push_back(make_pair(120, 30));
    init_list.push_back(make_pair(150, 30));
    init_list.push_back(make_pair(160, 50));
    init_list.push_back(make_pair(120, 70));

    convex::Hull hull(init_list);

    list<SDL_Point>& ans = hull.divide_conquer();
    for (auto e: ans)
        cout << e.x << " "
             << e.y << endl;

    delete &ans;
     */

    thread t1(calculator_t, init_list);


    for(auto p : init_list) {
        std::cout << p.x << ", " << p.y << std::endl;
    }

    convexSignalMainToThread.release();

    convexSignalThreadToMain.acquire();

    //delete final_list;

    auto it_end = final_list->begin();
    it_end++;

    t1.join();

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
                                        SDL_WINDOW_SHOWN,
                                        &window,
                                        &renderer) == 0) {
            SDL_SetWindowTitle(window, "Convex hull");
            SDL_UpdateWindowSurface(window);
            SDL_Event event;
            SDL_bool done = SDL_FALSE;
            auto first_run = 10;

            while (!done) {
                auto it_begin = final_list->begin();
                if(it_end != final_list->end()) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                }

                if(first_run > 0) {
                    SDL_RenderPresent(renderer);
                    SDL_Delay(300);
                    first_run--;
                } else if(it_end != final_list->end()) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderSetScale(renderer, 6.0f, 4.0f);

                    while(it_begin != it_end) {
                        SDL_RenderDrawPoint(renderer, it_begin->x, it_begin->y);
                        it_begin++;
                    }

                    it_end++;

                    SDL_RenderPresent(renderer);
                    SDL_Delay(500);
                } else if(it_end == final_list->end() && first_run != -1) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderSetScale(renderer, 6.0f, 4.0f);
                    SDL_RenderDrawPoints(renderer, points, POINTS_COUNT);
                    SDL_RenderPresent(renderer);
                    first_run = -1;
                }

                if(!SDL_PollEvent(&event)) continue;

                switch (event.type) {
                    case SDL_QUIT:
                        done = SDL_TRUE;
                        break;
                }
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    } else
        std::cout << "Failed to init SDL : " << SDL_GetError();

    delete final_list;

    SDL_Quit();

    return 0;
}
