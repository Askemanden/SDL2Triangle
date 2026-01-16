#pragma once
#include "drawjob.h"

/// @brief A 2 by 2 matrix used for transforming jobs.
/// @param M two by two double list representing the matrix
typedef struct
{
    double M[2][2]
} TransformationMatrix;

/// @brief Modifies a draw job to have the origin placed at `position`
/// @param job The job to modify
/// @param position The position to place the origin at
/// @return The modified draw job
DrawJob drawjob_shift(DrawJob job, Pointi position);

/// @brief Returns a rotated draw job rotated around (0,0) by the given angle in radians
/// @param job Job to rotate
/// @param angle_rad Angle by which to rotate
/// @return Returns a new job rotated by the angle
DrawJob drawjob_rotate(DrawJob job, double angle_rad);

/// @brief Returns a rotated draw job rotated by the given angle around center_of_rotation
/// @param job Job to rotate
/// @param angle_rad Angle by which to rotate
/// @param center_of_rotation Point to rotate around
/// @return Returns a new job rotated by the angle around the given center
DrawJob drawjob_rotate_around_point(DrawJob job, double angle_rad, Pointf center_of_rotation);

/// @brief Returns a drawjob with a sheared coordinate system.
/// @param job Draw job to shear
/// @param shear_x Amount to shear along x
/// @param shear_y Amount to shear along y
/// @return A sheared draw job
DrawJob drawjob_shear(DrawJob job, double shear_x, double shear_y);

/// @brief Returns a job with the coordinate system transformed with the matrix
/// @param job Job to return with the transformed coordinate system
/// @param matrix Matrix to transform the coordinate system with
/// @return A job with a transformed coordinate system
DrawJob drawjob_transform(DrawJob job, TransformationMatrix matrix);