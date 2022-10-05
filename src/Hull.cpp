//
// Created by AVA on 01.10.22.
//

#include "Hull.h"
#include <iterator>
#include <list>
#include <set>
#include <iostream>

using namespace std;

convex::Hull::Hull(list<SDL_Point>& point_list) {
    this->point_list = &point_list;
    this->point_list->sort(convex::Hull::PointComparator());
}

int convex::Hull::hull_start(std::list<SDL_Point>& point_list, convex::Hull::PointPosition pos) {
    int i = 0, index = 0, position = -1;
    bool save;

    for(auto & it : point_list) {
        save = false;

        if(position == -1) {
            position = it.x;
        }

        switch (pos) {
            case MOST_LEFT:
                if(position > it.x)
                    save = true;
                break;
            case MOST_RIGHT:
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

list<SDL_Point>& convex::Hull::merge(list<SDL_Point>& left_hull, list<SDL_Point>& right_hull) {
    list<SDL_Point> tmp_list;
    // Set iterators
    auto left_hull_start = left_hull.begin();
    auto right_hull_start = right_hull.begin();

    std::advance(left_hull_start, convex::Hull::hull_start(left_hull, MOST_RIGHT));
    std::advance(right_hull_start, convex::Hull::hull_start(right_hull, MOST_LEFT));

    while(std::any_of(left_hull.begin(),
                      left_hull.end(),
                      PointsOrientation((SDL_Point&)(*left_hull_start),
                                        (SDL_Point&)(*right_hull_start),
                                        MOST_RIGHT)) ||
                      std::any_of(left_hull.begin(),
                        left_hull.end(),
                        PointsOrientation((SDL_Point&)(*right_hull_start),
                                          (SDL_Point&)(*left_hull_start),
                                          MOST_LEFT))) {
        while(std::any_of(left_hull.begin(),
                          left_hull.end(),
                          PointsOrientation((SDL_Point&)(*left_hull_start),
                                            (SDL_Point&)(*right_hull_start),
                                            MOST_RIGHT))) {
            /*
            this->point_list->remove_if([=](SDL_Point point){
                return (point.x == (*left_hull_start).x) && (point.y == (*left_hull_start).y);
            });*/
            tmp_list.insert(tmp_list.end(), *left_hull_start);
            --left_hull_start;
        }

        while(std::any_of(left_hull.begin(),
                          left_hull.end(),
                          PointsOrientation((SDL_Point&)(*right_hull_start),
                                            (SDL_Point&)(*left_hull_start),
                                            MOST_LEFT))) {/*
            this->point_list->remove_if([=](SDL_Point point){
                return point.x == (*right_hull_start).x && point.y == (*right_hull_start).y;
            });*/
            tmp_list.insert(tmp_list.end(), *left_hull_start);
            ++right_hull_start;
        }
    }

    tmp_list.unique(convex::Hull::ComparePoints());

    for(auto l : tmp_list){
        std::cout << "X:" << l.x << " Y:" << l.y << std::endl;
    }

    left_hull_start = left_hull.begin();
    right_hull_start = right_hull.begin();

    while(std::any_of(right_hull.begin(),
                      right_hull.end(),
                      PointsOrientation((SDL_Point&)(*left_hull_start),
                                        (SDL_Point&)(*right_hull_start),
                                        MOST_RIGHT)) ||
          std::any_of(right_hull.begin(),
                      right_hull.end(),
                      PointsOrientation((SDL_Point&)(*right_hull_start),
                                        (SDL_Point&)(*left_hull_start),
                                        MOST_LEFT))) {
        while(std::any_of(right_hull.begin(),
                          right_hull.end(),
                          PointsOrientation((SDL_Point&)(*left_hull_start),
                                            (SDL_Point&)(*right_hull_start),
                                            MOST_RIGHT))) {/*
            this->point_list->remove_if([=](SDL_Point point){
                return point.x == (*left_hull_start).x && point.y == (*left_hull_start).y;
            });*/
            ++left_hull_start;
        }

        while(std::any_of(right_hull.begin(),
                          right_hull.end(),
                          PointsOrientation((SDL_Point&)(*right_hull_start),
                                            (SDL_Point&)(*left_hull_start),
                                            MOST_LEFT))) {/*
            this->point_list->remove_if([=](SDL_Point point){
                return point.x == (*right_hull_start).x && point.y == (*right_hull_start).y;
            });*/
            --right_hull_start;
        }
    }

    delete &left_hull;
    delete &right_hull;

    return *this->point_list;
}

list<SDL_Point>& convex::Hull::conquer(list<SDL_Point>& points) {
    if(points.size() == 1 || points.size() == 2)
        return points;

    SDL_Point point[3];
    int i = 0;

    for(auto & it : points) {
        point[i] = it;
        i++;
    }

    if(CALC_ORIENTATION(point[0],point[1],point[2]) == CLOCK_WISE)
        return points;

    i = 0;
    for(auto & it : points) {
        if(i == 0) {
            it = point[2];
        } else if(i == 1) {
            it = point[0];
        } else {
            it = point[1];
        }
        i++;
    }

    return points;
}

list<SDL_Point>& convex::Hull::divide(list<SDL_Point>& points) {
    // If the number of points is less than 6 then the
    // function uses the brute algorithm to find the
    // convex hull
    if (points.size() <= 3) {
        return this->conquer(points);
    }

    auto left = new list<SDL_Point>;
    auto right = new list<SDL_Point>;

    auto it_left = left->begin();
    auto it_right = right->begin();

    left->insert( it_left,
                  points.begin(),
                  next( points.begin(), ((int)points.size() + 2 - 1) / 2) );

    points.reverse();

    right->insert( it_right,
                   points.begin(),
                   next( points.begin(), (int)points.size() / 2 ) );

    right->reverse();

    // merging the convex hulls
    return this->merge(this->divide(*left), this->divide(*right));
}

list<SDL_Point>& convex::Hull::divide_conquer() {
    return this->divide(*this->point_list);
}


