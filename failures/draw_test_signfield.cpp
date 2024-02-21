bool renderer::draw_test_signfield(float4 p_color)
{
    // (center anchored)
    GLfloat circle_x = 1920.f / 2.f;
    GLfloat circle_y = 1080.f / 2.f;
    GLfloat circle_radius = 200.f;
    GLfloat circle_radius_squared = circle_radius * circle_radius;
    GLfloat circle_feather = 0.02f;          // percentage
    GLfloat circle_feather_thickness = circle_radius * circle_feather;

    GLfloat distance0 = 0;
    GLfloat distance1 = 0;
    GLfloat distance2 = 0;
    GLfloat distance3 = 0;
    GLfloat color_mult = 0.f;

    // Find the maximum reasonable distance a shape can be from a point on the
    // screen and normalize it to the scale of the gradient_approximation Lookup Table
    GLfloat gradient_normal = 7000.f / ( (1920.f * 1920.f) + (1080.f * 1080.f) );
    GLfloat gradient_size_adjustment = 1; // magic number

    for (int y = 0; y < 1080; ++y)
    {
        for (int x = 0; x < 1920; x += 3)
        {
            distance0 = std::abs( (0.f +x-circle_x ) * ( 0.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));
            distance1 = std::abs( (2.f +x-circle_x ) * ( 1.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));
            distance2 = std::abs( (2.f +x-circle_x ) * ( 2.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));
            distance3 = std::abs( (3.f +x-circle_x ) * ( 3.f +x-circle_x)) + std::abs( (y - circle_y) * (y - circle_y));

            // Fast Approximation
            // (it's not actually faster, lol, vertorization is the only option)
            int approx_point_0 =  static_cast<int>( float( roundf(gradient_normal * distance0 * gradient_size_adjustment)) );
            int approx_point_1 =  static_cast<int>( roundf(gradient_normal * distance1 * gradient_size_adjustment) );
            int approx_point_2 =  static_cast<int>( roundf(gradient_normal * distance2 * gradient_size_adjustment) );
            int approx_point_3 =  static_cast<int>( roundf(gradient_normal * distance3 * gradient_size_adjustment) );
            mbuffer[x + (1920*y)] = gradient_approximation[ approx_point_0 < 10000 ? approx_point_0 :  9999 ];
            mbuffer[1 + x + (1920*y)] = gradient_approximation[ approx_point_1 < 10000 ? approx_point_1 :  9999 ];
            mbuffer[2 + x + (1920*y)] = gradient_approximation[ approx_point_2 < 10000 ? approx_point_2 :  9999 ];
            mbuffer[3 + (1920*y)] = gradient_approximation[ approx_point_3 < 10000 ? approx_point_3 :  9999 ];
            // color_mult = ((1 / circle_radius) * sqrtf(distance0) ) * 0.8;
            // mbuffer[x + (1920*y)] = p_color * color_mult;
        }
    }
    return true;

}
