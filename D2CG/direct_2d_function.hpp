# ifndef DIRECT_2D_FUNCTION_HPP
# define DIRECT_2D_FUNCTION_HPP

# include <functional>
# include <type_traits>
# include "direct_2d_objects.hpp"

// #19 Map geometries shape to predefien member function from ID2D1Factory* and ID2D1HwndRenderTarget* to call suitable function in run time 
template<typename GeometryTypePtr>
std::enable_if<std::is_same_v<GeometryTypePtr, ID2D1RectangleGeometry*>, std::function <HRESULT(const D2D1_RECT_F&, ID2D1RectangleGeometry**)>>::type set_direct_2d_geometry_function(ID2D1Factory** windows_factory_ptr_ptr) {
    return [windows_factory_ptr_ptr](const D2D1_RECT_F& dimension, ID2D1RectangleGeometry geometry_ptr_ptr)->HRESULT { return (*windows_factory_ptr_ptr)->CreateRectangleGeometry(dimension, geometry_ptr_ptr); };
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

# endif