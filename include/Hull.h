//
// Created by AVA on 01.10.22.
//
#pragma once

#include <list>
#include <SDL.h>

#define CALC_ORIENTATION(a, b, c) (((b.y - a.y) * (c.x - b.x) - (c.y - b.y) * (b.x - a.x)) >= 0 ? CLOCK_WISE : COUNTER_CLOCKWISE)
#define CALC_ORIENTATION_PTR(a, b, c) (((b->y - a->y) * (c.x - b->x) - (c.y - b->y) * (b->x - a->x)) >= 0 ? MOST_RIGHT : MOST_LEFT)

namespace convex {
    class Hull {
    public:
        enum Orientation {
            CLOCK_WISE,
            COUNTER_CLOCKWISE,
        };
        enum PointPosition {
            MOST_RIGHT,
            MOST_LEFT
        };
        struct ComparePoints
        {
            template <typename SDL_Point>
            bool operator() (SDL_Point const &a, SDL_Point const &b) const {
                return a.x == b.x && a.y == b.y;
            }
        };
        struct PointComparator
        {
            bool operator ()(const SDL_Point& a, const SDL_Point& b) const {
                return a.x < b.x;
            }
        };
        struct PointsOrientation
        {
            SDL_Point* a;
            SDL_Point* b;
            PointPosition position;
            PointsOrientation(SDL_Point& a, SDL_Point& b, PointPosition position1){
                this->a = &a;
                this->b = &b;
                this->position = position1;
            }
            bool operator()(SDL_Point point) const {
                if((point.x == a->x && point.y == a->y) ||
                    (point.x == b->x && point.y == b->y))
                    return false;

                return CALC_ORIENTATION_PTR(a, b, point) != position;
            }
        };
        explicit Hull(std::list<SDL_Point>& point_list);
        std::list<SDL_Point>& divide_conquer();

    private:
        std::list<SDL_Point>* point_list;
        std::list<SDL_Point>& divide(std::list<SDL_Point> &points);
        std::list<SDL_Point>& merge(std::list<SDL_Point> &left_hull, std::list<SDL_Point> &right_hull);
        static std::list<SDL_Point>& conquer(std::list<SDL_Point> &points);
        static int hull_start(std::list<SDL_Point>& point_list, PointPosition pos);
    };
}
