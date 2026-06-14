# ifndef DIRECT_2D_OBJECTS_HPP
# define DIRECT_2D_OBJECTS_HPP

# include <d2d1.h>

// #5 Additinal data for Direct2D moving geometries type

enum class GeometriesMovingCtrl {
    NONE = 0,
    ARROW_KEY = (1 << 1),
    MOUSEHWEEL = (2 << 1),
    LEFT_CLICKED = (3 << 1),
    LEFT_CLICKED_HOLD = (4 << 1),
};

enum class GeometriesMovingSpace {
    NONE,
    X_AXIS_ONLY,
    Y_AXIS_ONLY,
    FREE
};

enum class DirectionInX {
    NONE,
    UP,
    DOWN,
};

enum class DirectionInY {
    NONE,
    LEFT,
    RIGHT,
};

// #6 Set limited directions based on input GeometriesMovingSpace

template<GeometriesMovingSpace>
struct ValidDirection {};

template<>
struct ValidDirection<GeometriesMovingSpace::X_AXIS_ONLY> {
    using Direction = DirectionInX;
};

template<>
struct ValidDirection <GeometriesMovingSpace::Y_AXIS_ONLY> {
    using Direction = DirectionInY;
};

template<>
struct ValidDirection <GeometriesMovingSpace::FREE> {
    using Direction = void;
};


template<GeometriesMovingSpace limited_space>
struct ArrowsDirection {
    typename ValidDirection<limited_space>::Direction arrow_left{};
    typename ValidDirection<limited_space>::Direction arrow_up{};
    typename ValidDirection<limited_space>::Direction arrow_right{};
    typename ValidDirection<limited_space>::Direction arrow_down{};
};

template<GeometriesMovingSpace limited_space>
struct MouseWheelsDirection {
    typename ValidDirection<limited_space>::Direction wheel_up{};
    typename ValidDirection<limited_space>::Direction wheel_down{};
};

// #6

template<GeometriesMovingSpace limited_space>
struct MovingGeometriesProp {
    ArrowsDirection<limited_space> arrow_keys_direction;
    MouseWheelsDirection<limited_space> mouse_wheel_direction;
};

// #5



enum class GeometriesShape {
    NONE,
    RECTANGLE,
    ROUNDED_RECTANGLE,
    ELLIPSE
};

// #4 Specialized direct 2d dimension type to set the objects dimension type 

template<GeometriesShape>
struct ShapeToDimensionType {};

template<>
struct ShapeToDimensionType<GeometriesShape::RECTANGLE> {
    using Dimension = D2D1_RECT_F;
    using Geometry = ID2D1RectangleGeometry*;
};

template<>
struct ShapeToDimensionType<GeometriesShape::ROUNDED_RECTANGLE> {
    using Dimension = D2D1_ROUNDED_RECT;
    using Geometry = ID2D1RoundedRectangleGeometry*;
};

template<>
struct ShapeToDimensionType<GeometriesShape::ELLIPSE> {
    using Dimension = D2D1_ELLIPSE;
    using Geometry = ID2D1EllipseGeometry*;
};

// #4 


// #13 Configure a user_define_type for capsule and store the custom geometries style 

struct GradientStopCollectionProp {
    D2D1_GAMMA gamma_value{};
    D2D1_EXTEND_MODE extention_value{};
    std::vector<D2D1_GRADIENT_STOP> gradients_stops{};
};

template<typename BrushType>
struct Direct2DBrushStyle {};


template<>
struct Direct2DBrushStyle<ID2D1SolidColorBrush*> {
    D2D1::ColorF::Enum brushes_solid_color;
};


template<>
struct Direct2DBrushStyle<ID2D1LinearGradientBrush*> : public GradientStopCollectionProp {
    D2D1_POINT_2F linear_gradients_start_point{};
    D2D1_POINT_2F linear_gradients_end_point{};
};

template<>
struct Direct2DBrushStyle<ID2D1RadialGradientBrush*> : public GradientStopCollectionProp {
    D2D1_POINT_2F center{};
    FLOAT x_radius{};
    FLOAT y_radius{};
};

template<typename FillsBrushType , typename StrokesBrushType>
struct GeometriesCustomStyle {
    Direct2DBrushStyle<FillsBrushType> fills_brush_info{};
    unsigned int fills_distance_from_stroke{};
    Direct2DBrushStyle<StrokesBrushType> strokes_brush_info{};
    FLOAT strokes_width{};
};

// #13


// #8 Partial specialized GeometriesInfo based on being static or moving geometries to store extra moving properties filed on the object 

template<bool is_moving_geometry, GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType, GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE, GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class Direct2DGeometriesInfo {};

template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType>
class Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> {
    typename ShapeToDimensionType<geos_shape>::Geometry shapes_geometry{};

public:
    typename ShapeToDimensionType<geos_shape>::Dimension geometries_dimension_values{};
    GeometriesCustomStyle<FillsBrushType, StrokesBrushType> geometries_style{};

};


template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType, GeometriesMovingCtrl ctrl_type, GeometriesMovingSpace space_type>
class Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> : public Direct2DGeometriesInfo <false, geos_shape, FillsBrushType, StrokesBrushType> {

public:
    MovingGeometriesProp<space_type> moving_properties;
};

// #8


// #9 Make a run time polymorphism to store a geometries properties in different template parameters and types in a vector container

class Direct2DGeometriesBase {
public:
    virtual ~Direct2DGeometriesBase() = default;
    virtual void* get_geometries_info() = 0;
};


template<bool is_moving_geometry, GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType, GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE, GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class GeometriesWrapper {};

template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType>
class GeometriesWrapper<false, geos_shape, FillsBrushType, StrokesBrushType> : public Direct2DGeometriesBase {

public:
    Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> geos_info{};

    GeometriesWrapper() = default;
    GeometriesWrapper(Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> input_geometries_info) : geos_info(input_geometries_info) {
        //std::cout << "Constructor GeometriesWrapper<false, geos_shape, FillsBrushType, StrokesBrushType> " << '\n';
    };

    void* get_geometries_info() override {
        return &geos_info;
    }

};

// #9


template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType, GeometriesMovingCtrl ctrl_type, GeometriesMovingSpace space_type>
class GeometriesWrapper<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> : public Direct2DGeometriesBase {

public:
    Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> geos_info{};
    GeometriesWrapper() = default;
    GeometriesWrapper(Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> input_geometries_info) : geos_info(input_geometries_info) {
        //std::cout << "Constructor GeometriesWrapper<true, geos_shape, FillsBrushType, StrokesBrushType , ctrl_type , sapce_type> " << '\n';
    };

    void* get_geometries_info() override {
        return &geos_info;
    }

};



# endif