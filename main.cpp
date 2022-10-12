#include <iostream>
#include <random>
#include <SDL.h>
#include <list>

#define ORIENT(a, b, c) ((b.y - a.y) * (c.x - b.x) - (c.y - b.y) * (b.x - a.x))

using namespace std;

const int POINTS_COUNT = 11;
const int DRAW_DELAY = 200;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static list<SDL_Point> global_list;
static list<SDL_Point>* final_list;

inline bool operator==(SDL_Point const &a, SDL_Point const &b)
{
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(SDL_Point const &a, SDL_Point const &b)
{
    return !(a == b);
}

class Calculator {
public:
    enum PointPosition {
        RIGHT,
        LEFT
    };
    struct PointsOrientation {
        SDL_Point *a;
        SDL_Point *b;
        PointPosition position;

        PointsOrientation(SDL_Point& a, SDL_Point& b, PointPosition position1) {
            this->a = &a;
            this->b = &b;
            this->position = position1;
        }

        bool operator()(SDL_Point point) const {
            if ((point.x == a->x && point.y == a->y) ||
                (point.x == b->x && point.y == b->y))
                return false;

            int orient = ORIENT((*a), (*b), point);

            if(orient > 0)
                return this->position != PointPosition::LEFT;
            else if(orient < 0)
                return this->position != PointPosition::RIGHT;
            else
                return false;
        }
    };
    struct PointComparator
    {
        bool operator ()(const SDL_Point& a, const SDL_Point& b) const {
            return a.x < b.x;
        }
    };

    const list<SDL_Point>* const point_list;

    explicit Calculator(list<SDL_Point>& point_list): point_list(&point_list) {
        point_list.sort(PointComparator());
    };

    list<SDL_Point>* calc() {
        return this->divide(const_cast<list<SDL_Point> *>(this->point_list));
    }

    static int hull_start(std::list<SDL_Point>& point_list, PointPosition pos) {
        int i = 0, index = 0, position = -1;
        bool save;

        for(auto &it : point_list) {
            save = false;

            if(position == -1) {
                position = it.x;
            }

            switch (pos) {
                case LEFT:
                    if(position > it.x)
                        save = true;
                    break;
                case RIGHT:
                    if(position < it.x)
                        save = true;
                    break;
            }

            if(save) {
                position = it.x;
                index = i;
            }

            i++;
        }

        return index;
    }

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

        auto left_hull_start = left_hull.begin();
        auto right_hull_start = right_hull.begin();

        std::list<SDL_Point> rm_left_hull;
        std::list<SDL_Point> rm_right_hull;

        std::advance(left_hull_start, hull_start(left_hull, RIGHT));
        std::advance(right_hull_start, hull_start(right_hull, LEFT));

        auto cp_left_hull_start = left_hull_start;
        auto cp_right_hull_start = right_hull_start;

        while(!(std::any_of(left_hull.begin(),
                            left_hull.end(),
                            PointsOrientation((SDL_Point&)(*left_hull_start),
                                              (SDL_Point&)(*right_hull_start),
                                              PointPosition::RIGHT)) &&
                std::any_of(right_hull.begin(),
                            right_hull.end(),
                            PointsOrientation((SDL_Point&)(*left_hull_start),
                                              (SDL_Point&)(*right_hull_start),
                                              PointPosition::RIGHT)))) {
            while(!std::any_of(left_hull.begin(),
                               left_hull.end(),
                               PointsOrientation((SDL_Point&)(*left_hull_start),
                                                 (SDL_Point&)(*right_hull_start),
                                                 PointPosition::RIGHT))) {
                rm_left_hull.insert(rm_left_hull.end(), *left_hull_start);
                if(left_hull_start == left_hull.begin())
                    left_hull_start = --(left_hull.end());
                else
                    left_hull_start--;
            }

            while(!std::any_of(right_hull.begin(),
                               right_hull.end(),
                               PointsOrientation((SDL_Point&)(*left_hull_start),
                                                 (SDL_Point&)(*right_hull_start),
                                                 PointPosition::RIGHT))) {
                rm_right_hull.insert(rm_right_hull.end(), *right_hull_start);
                if(right_hull_start == --(right_hull.end()))
                    right_hull_start = right_hull.begin();
                else
                    right_hull_start++;
            }
        }

        auto upper_tangent_left = left_hull_start;
        auto upper_tangent_right = right_hull_start;

        left_hull_start = cp_left_hull_start;
        right_hull_start = cp_right_hull_start;

        while(!(std::any_of(left_hull.begin(),
                            left_hull.end(),
                            PointsOrientation((SDL_Point&)(*left_hull_start),
                                              (SDL_Point&)(*right_hull_start),
                                              PointPosition::LEFT)) &&
                std::any_of(right_hull.begin(),
                            right_hull.end(),
                            PointsOrientation((SDL_Point&)(*left_hull_start),
                                              (SDL_Point&)(*right_hull_start),
                                              PointPosition::LEFT)))) {
            while(!std::any_of(left_hull.begin(),
                               left_hull.end(),
                               PointsOrientation((SDL_Point&)(*left_hull_start),
                                                 (SDL_Point&)(*right_hull_start),
                                                 PointPosition::LEFT))) {
                rm_left_hull.insert(rm_left_hull.end(), *left_hull_start);
                if(left_hull_start == --(left_hull.end()))
                    left_hull_start = left_hull.begin();
                else
                    left_hull_start++;
            }

            while(!std::any_of(right_hull.begin(),
                               right_hull.end(),
                               PointsOrientation((SDL_Point&)(*left_hull_start),
                                                 (SDL_Point&)(*right_hull_start),
                                                 PointPosition::LEFT))) {
                rm_right_hull.insert(rm_right_hull.end(), *right_hull_start);
                if(right_hull_start == right_hull.begin())
                    right_hull_start = --(right_hull.end());
                else
                    right_hull_start--;
            }
        }

        auto lower_tangent_left = left_hull_start;
        auto lower_tangent_right = right_hull_start;

        for(auto rm_p : rm_left_hull)
            if(*upper_tangent_left != rm_p && *lower_tangent_left != rm_p)
                left_hull.remove(rm_p);

        for(auto rm_p : rm_right_hull)
            if(*upper_tangent_right != rm_p && *lower_tangent_right != rm_p)
                right_hull.remove(rm_p);

        list->insert(list->end(), left_hull.begin(), left_hull.end());
        list->insert(list->end(), right_hull.begin(), right_hull.end());
        /*
        left_hull_start = left_hull.begin();

        while(*left_hull_start != *lower_tangent_left)
            left_hull_start++;

        list->insert(list->end(), *left_hull_start);
        if(left_hull_start == --left_hull.end())
            left_hull_start = left_hull.begin();
        else
            left_hull_start++;

        while (*left_hull_start != *lower_tangent_left){
            list->insert(list->end(), *left_hull_start);
            if(left_hull_start == --left_hull.end())
                left_hull_start = left_hull.begin();
            else
                left_hull_start++;
        }

        right_hull_start = right_hull.begin();

        while (*right_hull_start != *upper_tangent_right)
            right_hull_start++;

        list->insert(list->end(), *right_hull_start);
        if(right_hull_start == --right_hull.end())
            right_hull_start = right_hull.begin();
        else
            right_hull_start++;

        while(*right_hull_start != *upper_tangent_right) {
            list->insert(list->end(), *right_hull_start);
            if(right_hull_start == --right_hull.end())
                right_hull_start = right_hull.begin();
            else
                right_hull_start++;
        }

        while(*left_hull_start != *upper_tangent_left) {
            list->insert(list->end(), *left_hull_start);
            left_hull_start++;
        }

        list->insert(list->end(), *upper_tangent_left);
        list->insert(list->end(), *upper_tangent_right);

        right_hull_start = right_hull.begin();

        while(*right_hull_start != *upper_tangent_right)
            right_hull_start++;

        if(right_hull_start == --right_hull.end())
            right_hull_start = right_hull.begin();
        else
            right_hull_start++;

        while(*right_hull_start != *upper_tangent_right) {
            list->insert(list->begin(), *right_hull_start);
            if(right_hull_start == --right_hull.end())
                right_hull_start = right_hull.begin();
            else
                right_hull_start++;
        }

        while (left_hull_start != --left_hull.end()) {
            left_hull_start++;
            list->insert(list->end(), *left_hull_start);
        }

        //list->insert(list->end(), left_hull.begin(), left_hull.end());
        //list->insert(list->end(), right_hull.begin(), right_hull.end());

        while(lower_tangent_left != upper_tangent_left) {
            list->insert(list->end(), *lower_tangent_left);
            if(lower_tangent_left == --left_hull.end())
                lower_tangent_left = left_hull.begin();
            else
                lower_tangent_left++;
        }

        list->insert(list->end(), *lower_tangent_left);

        while (upper_tangent_right != lower_tangent_right) {
            list->insert(list->end(), *upper_tangent_right);
            if(upper_tangent_right == --right_hull.end())
                upper_tangent_right = right_hull.begin();
            else
                upper_tangent_right++;
        }

        list->insert(list->end(), *upper_tangent_right);

        std::advance(left_hull_start, convex::Hull::hull_start(left_hull, convex::Hull::MOST_LEFT));

        while(left_hull_start != upper_tangent_left) {
            list->insert(list->end(), *left_hull_start);
            if(left_hull_start == --left_hull.end())
                left_hull_start = left_hull.begin();
            else
                left_hull_start++;
        }

        list->insert(list->end(), *upper_tangent_left);

        while(upper_tangent_right != lower_tangent_right) {
            list->insert(list->end(), *upper_tangent_right);
            if(upper_tangent_right == --right_hull.end())
                upper_tangent_right = right_hull.begin();
            else
                upper_tangent_right++;
        }

        list->insert(list->end(), *upper_tangent_right);

        std::advance(left_hull_start, convex::Hull::hull_start(left_hull, convex::Hull::MOST_LEFT));

        while(lower_tangent_left != left_hull_start) {
            list->insert(list->end(), *lower_tangent_left);
            if(lower_tangent_left == --left_hull.end())
                lower_tangent_left = left_hull.begin();
            else
                lower_tangent_left++;
        }

        //

        auto cp_lower_tangent_left = lower_tangent_left;
        auto cp_lower_tangent_right = lower_tangent_right;
        list->insert(list->end(), *upper_tangent_left);
        list->insert(list->end(), *upper_tangent_right);

        if(upper_tangent_right == --right_hull.end())
            upper_tangent_right = right_hull.begin();
        else
            upper_tangent_right++;

        while (lower_tangent_right != upper_tangent_right) {
            list->insert(list->end(), *upper_tangent_right);
            if(upper_tangent_right == --right_hull.end())
                upper_tangent_right = right_hull.begin();
            else
                upper_tangent_right++;
        }

        list->insert(list->end(), *cp_lower_tangent_right);
        list->insert(list->end(), *cp_lower_tangent_left);

        if(lower_tangent_left == --left_hull.end())
            lower_tangent_left = left_hull.begin();
        else
            lower_tangent_left++;

        while (lower_tangent_left != upper_tangent_left) {
            list->insert(list->end(), *lower_tangent_left);
            if(lower_tangent_left == --left_hull.end())
                lower_tangent_left = left_hull.begin();
            else
                lower_tangent_left++;
        }

        //

        if(lower_tangent_left == upper_tangent_left)
            list->insert(list->end(), *lower_tangent_left);
        else {
            auto left_start = lower_tangent_left;
            while (left_start != upper_tangent_left) {
                list->insert(list->end(), *left_start);
                if(left_start == --left_hull.end())
                    left_start = left_hull.begin();
                else
                    left_start++;
            }
            list->insert(list->end(), *upper_tangent_left);
        }

        if(lower_tangent_right == upper_tangent_right)
            list->insert(list->end(), *lower_tangent_right);
        else {
            auto right_start = lower_tangent_right;
            while (right_start != upper_tangent_right) {
                list->insert(list->end(), *right_start);
                if(right_start == right_hull.begin())
                    right_start = --right_hull.end();
                else
                    right_start--;
            }
            list->insert(list->end(), *upper_tangent_right);
        }

        //

        auto left_start = left_hull.begin();
        while (left_start != lower_tangent_left) {
            if(left_start == --left_hull.end())
                left_start = left_hull.begin();
            else
                left_start++;
        }

        while (left_start != upper_tangent_left) {
            list->insert(list->end(), *left_start);
            if(left_start == --left_hull.end())
                left_start = left_hull.begin();
            else
                left_start++;
        }

        auto right_start = --right_hull.end();
        while (right_start != upper_tangent_right) {
            if(right_start == right_hull.begin())
                right_start = --right_hull.end();
            else
                right_start--;
        }

        while (right_start != lower_tangent_right) {
            list->insert(list->end(), *right_start);
            if(right_start == right_hull.begin())
                right_start = --right_hull.end();
            else
                right_start--;
        }
         */

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
        //list->insert(list->end(), left_hull.begin(), left_hull.end());
        //list->insert(list->end(), right_hull.begin(), right_hull.end());

        //global_list.clear();
        //global_list.insert(global_list.end(), left_hull.begin(), left_hull.end());
        //global_list.insert(global_list.end(), right_hull.begin(), right_hull.end());

        delete &left_hull;
        delete &right_hull;

        return list;
    }

    list<SDL_Point>* brute(list<SDL_Point>& points) const {
        auto* list = new std::list<SDL_Point>;

        if(points.size() == 1 || points.size() == 2) {
            list->insert(list->end(), points.begin(), points.end());

            if(this->point_list->size() != points.size())
                delete &points;

            return list;
        }

        SDL_Point point[3];
        int i = 0;

        for(auto &it : points) {
            point[i] = it;
            i++;
        }

        int orient = ORIENT(point[0],point[1],point[2]);

        if(orient < 0) {
            list->insert(list->end(), point[0]);
            list->insert(list->end(), point[2]);
            list->insert(list->end(), point[1]);
        } else if(orient > 0) {
            list->insert(list->end(), point[0]);
            list->insert(list->end(), point[1]);
            list->insert(list->end(), point[2]);
        } else {
            list->insert(list->end(), point[0]);
            list->insert(list->end(), point[2]);
        }

        if(this->point_list->size() != points.size())
            delete &points;

        return list;
    }
};

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

    auto c = Calculator(init_list);

    final_list = c.calc();

    for(auto p : *final_list) {
        std::cout << p.x << ", " << p.y << std::endl;
    }

    auto it_end = final_list->begin();
    it_end++;

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
                    SDL_Delay(DRAW_DELAY);
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
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderSetScale(renderer, 6.0f, 4.0f);
                    SDL_RenderDrawPoints(renderer, points, POINTS_COUNT);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    for(auto p : *final_list)
                        SDL_RenderDrawPoint(renderer, p.x, p.y);
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
