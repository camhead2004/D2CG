# ifndef DIRECT_2D_FUNCTION_HPP
# define DIRECT_2D_FUNCTION_HPP

# include <functional>
# include <type_traits>
# include "direct_2d_objects.hpp"

// #19 Map geometries shape to predefien member function from ID2D1Factory* and ID2D1HwndRenderTarget* to call suitable function in run time 
template<typename GeometryTypePtr>
std::enable_if<std::is_same_v<GeometryTypePtr, ID2D1RectangleGeometry*>, std::function <HRESULT(const D2D1_RECT_F&, ID2D1RectangleGeometry**)>>::type set_direct_2d_geometry_function(ID2D1Factory** windows_factory_ptr_ptr) {
    return [windows_factory_ptr_ptr](const D2D1_RECT_F& dimension, ID2D1RectangleGeometry** geometry_ptr_ptr)->HRESULT { return (*windows_factory_ptr_ptr)->CreateRectangleGeometry(dimension, geometry_ptr_ptr); };
}

template<typename GeometryTypePtr>
std::enable_if<std::is_same_v<GeometryTypePtr, ID2D1RoundedRectangleGeometry*>, std::function <HRESULT(const D2D1_ROUNDED_RECT&, ID2D1RoundedRectangleGeometry**)>>::type set_direct_2d_geometry_function(ID2D1Factory** windows_factory_ptr_ptr) {
    return [windows_factory_ptr_ptr](const D2D1_ROUNDED_RECT& dimension, ID2D1RoundedRectangleGeometry** geometry_ptr_ptr)->HRESULT { return (*windows_factory_ptr_ptr)->CreateRoundedRectangleGeometry(dimension, geometry_ptr_ptr); };
}

template<typename GeometryTypePtr>
std::enable_if<std::is_same_v<GeometryTypePtr, ID2D1EllipseGeometry*>, std::function<HRESULT(const D2D1_ELLIPSE&, ID2D1EllipseGeometry**)>>::type set_direct_2d_geometry_function(ID2D1Factory** windows_factory_ptr_ptr) {
    return [windows_factory_ptr_ptr](const D2D1_ELLIPSE& dimension, ID2D1EllipseGeometry** geometry_ptr_ptr)->HRESULT { return (*windows_factory_ptr_ptr)->CreateEllipseGeometry(dimension, geometry_ptr_ptr); };
}

// #19

//#21 Map create brush function based on Direct2D brush type

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1SolidColorBrush*>, std::function<HRESULT(const D2D1_COLOR_F&, ID2D1SolidColorBrush**)>>::type create_brush_function_type(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr) {
    return [windows_render_target_ptr_ptr](const D2D1_COLOR_F& solid_color, ID2D1SolidColorBrush** brush_ptr_ptr)->HRESULT { return (*windows_render_target_ptr_ptr)->CreateSolidColorBrush(solid_color, brush_ptr_ptr); };
}

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1LinearGradientBrush*>, std::function<HRESULT(const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES& , ID2D1GradientStopCollection* , ID2D1LinearGradientBrush**)>>::type create_brush_function_type(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr) {

    return [windows_render_target_ptr_ptr](const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES& brush_prop, ID2D1GradientStopCollection* stop_collection, ID2D1LinearGradientBrush** brush_ptr_ptr)->HRESULT
        { return (*windows_render_target_ptr_ptr)->CreateLinearGradientBrush(brush_prop , stop_collection, brush_ptr_ptr); };
}

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1RadialGradientBrush*>, std::function<HRESULT(const D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES&, ID2D1GradientStopCollection*, ID2D1RadialGradientBrush**)>>::type create_brush_function_type(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr) {

    return [windows_render_target_ptr_ptr](const D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES& brush_prop, ID2D1GradientStopCollection* stop_collection, ID2D1RadialGradientBrush** brush_ptr_ptr)->HRESULT
        { return (*windows_render_target_ptr_ptr)->CreateRadialGradientBrush(brush_prop, stop_collection, brush_ptr_ptr); };
}

//#21 

// #22 Map a void function to set the brush datas based on brush types in compile time 

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1SolidColorBrush*> , void>::type set_brush(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr , Direct2DBrushStyle<BrushType>* brush_info_ptr , BrushType* brush_ptr_ptr) {
    auto create_direct_2d_brush_function{ create_brush_function_type<BrushType>(windows_render_target_ptr_ptr) };
    create_direct_2d_brush_function(brush_info_ptr->brush_solid_color , brush_ptr_ptr);
}

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1LinearGradientBrush*> , void>::type set_brush(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr, Direct2DBrushStyle<BrushType>* brush_info_ptr, BrushType* brush_ptr_ptr) {
    auto create_direct_2d_brush_function{ create_brush_function_type<BrushType>(windows_render_target_ptr_ptr) };
    ID2D1GradientStopCollection* linear_gradients_stop_collection_ptr{ nullptr };
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES linear_gradient_prop{ .startPoint{ brush_info_ptr->linear_gradients_start_point } , .endPoint{ brush_info_ptr->linear_gradients_end_point} };

    (*windows_render_target_ptr_ptr)->CreateGradientStopCollection(brush_info_ptr->gradients_stops.data(), brush_info_ptr->gradients_stops.size(), brush_info_ptr->gamma_value, brush_info_ptr->extention_value, &linear_gradients_stop_collection_ptr);

    create_direct_2d_brush_function(linear_gradient_prop, linear_gradients_stop_collection_ptr, brush_ptr_ptr);
}

template<typename BrushType>
std::enable_if<std::is_same_v<BrushType, ID2D1RadialGradientBrush*> , void>::type set_brush(ID2D1HwndRenderTarget** windows_render_target_ptr_ptr, Direct2DBrushStyle<BrushType>* brush_info_ptr, BrushType* brush_ptr_ptr) {
    auto create_direct_2d_brush_function{ create_brush_function_type<BrushType>(windows_render_target_ptr_ptr) };
    ID2D1GradientStopCollection* radial_gradients_stop_collection_ptr{ nullptr };
    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radial_gradient_prop{ .center{ brush_info_ptr->center } , .gradientOriginOffset{D2D1::Point2F(0.0f , 0.0f)} , .radiusX{ brush_info_ptr->x_radius } , .radiusY{ brush_info_ptr->y_radius } };

    (*windows_render_target_ptr_ptr)->CreateGradientStopCollection(brush_info_ptr->gradients_stops.data(), brush_info_ptr->gradients_stops.size(), brush_info_ptr->gamma_value, brush_info_ptr->extention_value, &radial_gradients_stop_collection_ptr);

    create_direct_2d_brush_function(radial_gradient_prop, radial_gradients_stop_collection_ptr, brush_ptr_ptr);
}

// #22

# endif