#include "../include/renderer.h"

Recti Rectf_to_i(Rectf rectf)
{
    return (Recti){
        .top_left = {(int)rectf.top_left.x, (int)rectf.top_left.y},
        .bottom_right = {(int)rectf.bottom_right.x, (int)rectf.bottom_right.y}};
}