# ifndef DIRECT_2D_OBJECTS_HPP
# define DIRECT_2D_OBJECTS_HPP

# include <d2d1.h>
# include <variant>

// #5 Additinal data for Direct2D moving geometries type



enum class GeometriesMovingCtrl : unsigned int {
    NONE = 0,
    ARROW_KEY = (1 << 0),
    MOUSEWHEEL = (1 << 1),
    LEFT_CLICKED = (1 << 2),
    LEFT_CLICKED_HOLD = (1 << 3),
};

constexpr GeometriesMovingCtrl operator|(GeometriesMovingCtrl left_operand, GeometriesMovingCtrl right_operand) {
    using EnumBaseType = std::underlying_type_t<GeometriesMovingCtrl >;
    return static_cast<GeometriesMovingCtrl>( (static_cast<EnumBaseType>(left_operand) | static_cast<EnumBaseType>(right_operand)) );
}

unsigned int operator&(GeometriesMovingCtrl left_operand, GeometriesMovingCtrl right_operand) {
    using EnumBaseType = std::underlying_type_t<GeometriesMovingCtrl>;
    return static_cast<unsigned int>(static_cast<unsigned>(left_operand) & static_cast<unsigned>(right_operand));
}


enum class GeometriesMovingSpace {
    NONE,
    X_AXIS_ONLY,
    Y_AXIS_ONLY,
    FREE
};

enum class XYDirection {
    NONE , 
    UP , 
    LEFT , 
    RIGHT ,
    DOWN
};

// #6 Set type definition for arrow and mousewheel based on the (GeometriesMovingCtrl & GeometriesMovingCtrl::ARRROW_KEY) and (GeometriesMovingCtrl & GeometriesMovingCtrl::MOUSEWHEEL) conditions

struct ArrowsDirection {
    XYDirection arrow_left{};
    XYDirection arrow_up{};
    XYDirection arrow_right{};
    XYDirection arrow_down{};
};

struct MouseWheelsDirection {
    XYDirection wheel_up{};
    XYDirection wheel_down{};
};

// #6

struct MovingGeometriesProp {
    ArrowsDirection arrow_keys_direction{};
    MouseWheelsDirection mouse_wheel_direction{};
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
    D2D1_COLOR_F brush_solid_color;
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
    FillsBrushType fills_brush_ptr{};
    StrokesBrushType strokes_brush_ptr{};
};

// #13


// #8 Partial specialized GeometriesInfo based on being static or moving geometries to store extra moving properties data member on the object 

// #18 use std::variant to store all types Direct2DGeometries data member in run time 

using DimensionVariantPtr = std::variant<D2D1_RECT_F*, D2D1_ROUNDED_RECT*, D2D1_ELLIPSE*>;
using GeometriesVariantPtrPtr = std::variant<ID2D1RectangleGeometry**, ID2D1RoundedRectangleGeometry**, ID2D1EllipseGeometry**>;
using BrushVariantPtrPtr = std::variant<ID2D1SolidColorBrush**, ID2D1LinearGradientBrush**, ID2D1RadialGradientBrush**>;

// #18

template<bool is_moving_geometry, GeometriesShape geos_shape, typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE, GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class Direct2DGeometriesInfo {};

template<GeometriesShape geos_shape, typename FillsBrushType , typename StrokesBrushType>
class Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> {
public:
    typename ShapeToDimensionType<geos_shape>::Geometry shapes_geometry{};
    typename ShapeToDimensionType<geos_shape>::Dimension geometries_dimension_values{};
    GeometriesCustomStyle<FillsBrushType , StrokesBrushType> geometries_style{};
};

template<GeometriesShape geos_shape , typename FillsBrushType, typename StrokesBrushType, GeometriesMovingCtrl ctrl_type, GeometriesMovingSpace space_type>
class Direct2DGeometriesInfo<true, geos_shape, FillsBrushType , StrokesBrushType , ctrl_type, space_type> : public Direct2DGeometriesInfo <false , geos_shape , FillsBrushType , StrokesBrushType> {
    
public:
    MovingGeometriesProp moving_properties;
};


// #30 Encapsulate geometries info to get all data needed to set geometries data on windows procedure

struct GeometriesEssentialInfoInProc{
    GeometriesVariantPtrPtr current_shapes_geometry_variant{};
    DimensionVariantPtr current_geos_dimension_variant{};
    std::pair<BrushVariantPtrPtr, BrushVariantPtrPtr> current_geometries_brush_variant{};
    FLOAT current_strokes_width{};
    unsigned int current_fills_distance_from_stroke{};
    const MovingGeometriesProp* current_moving_properties_ptr {};
    GeometriesMovingCtrl current_moving_ctrl_type{};
    GeometriesMovingSpace current_moving_space_type {};
    D2D1_POINT_2F current_geometries_center_coordinate{};
};

// #30


// #8


// #9 Make a run time polymorphism to store a geometries properties in different template parameters and types in a vector container

class Direct2DGeometriesBase {
public:
    virtual ~Direct2DGeometriesBase() = default;
    virtual bool is_static_geometry() = 0;
    virtual DimensionVariantPtr get_dimension_ptr() = 0;
    virtual D2D1_POINT_2F get_geometries_center_position(DimensionVariantPtr geometries_dimension_ptr) = 0;
    virtual void set_geometries_new_coordinate(ID2D1Factory** static_windows_factory_ptr_ptr, DimensionVariantPtr dimension_ptr, GeometriesVariantPtrPtr geometry_ptr_ptr, D2D1_POINT_2F* new_coordinate) = 0;
    virtual std::pair<BrushVariantPtrPtr, BrushVariantPtrPtr> get_geometries_brush() = 0;
    virtual GeometriesVariantPtrPtr get_shapes_geometry_ptr_ptr() = 0;
    virtual unsigned int get_fills_distance_from_stroke() = 0;
    virtual FLOAT get_strokes_width() = 0;

    // #24 Make default specialization for GeometriesWrapper false partial specialization and override it in true partial specialization  

    virtual const GeometriesMovingCtrl get_ctrl_type() { return GeometriesMovingCtrl::NONE; };
    virtual const GeometriesMovingSpace get_moving_space_type() { return GeometriesMovingSpace::NONE; };
    virtual const MovingGeometriesProp* get_moving_geometries_arrow_key_and_mouse_wheel_direction() { return {}; };

    // #24


    virtual void get_geometries_essential_info_in_run_time(GeometriesEssentialInfoInProc* geometries_info_ptr) = 0;
};


template<bool is_moving_geometry, GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType , GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE, GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class GeometriesWrapper {};


template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType>
class GeometriesWrapper<false, geos_shape, FillsBrushType , StrokesBrushType> : public Direct2DGeometriesBase {

public:
    Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> geos_info{};

    GeometriesWrapper() = default;
    GeometriesWrapper(Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> input_geometries_info) : geos_info(input_geometries_info) {
        //std::cout << "Constructor GeometriesWrapper<false, geos_shape, FillsBrushType, StrokesBrushType> " << '\n';
    };
    
    bool is_static_geometry() override { return true; };

    DimensionVariantPtr get_dimension_ptr() override {
        return &geos_info.geometries_dimension_values;
    }

    D2D1_POINT_2F get_geometries_center_position(DimensionVariantPtr geometries_dimension_ptr) override {

        return std::visit([](auto&& dimension)->D2D1_POINT_2F {
            using CurrentDimensionType = std::decay_t<decltype(dimension)>;

            if constexpr (std::is_same_v<CurrentDimensionType, D2D1_ELLIPSE*>) {
                return dimension->point;
            }

            else if constexpr (std::is_same_v<CurrentDimensionType, D2D1_RECT_F*>) {
                D2D1_POINT_2F rectangle_size{ D2D1::Point2F(dimension->right - dimension->left , dimension->bottom - dimension->top) };
                return D2D1::Point2F(dimension->left + (rectangle_size.x / 2.0f), dimension->top + (rectangle_size.y / 2.0f));
            }

            else if constexpr (std::is_same_v <CurrentDimensionType, D2D1_ROUNDED_RECT*>) {
                D2D1_POINT_2F rectangle_size{ D2D1::Point2F(dimension->rect.right - dimension->rect.left , dimension->rect.bottom - dimension->rect.top) };
                return D2D1::Point2F(dimension->rect.left + (rectangle_size.x / 2.0f), dimension->rect.top + (rectangle_size.y / 2.0f));
            }
            ;}, geometries_dimension_ptr);
    }

    void set_geometries_new_coordinate(ID2D1Factory** static_windows_factory_ptr_ptr , DimensionVariantPtr dimension_ptr , GeometriesVariantPtrPtr geometry_ptr_ptr , D2D1_POINT_2F* new_coordinate) override {

        std::visit([static_windows_factory_ptr_ptr , dimension_ptr, new_coordinate](auto&& dimension, auto&& geometry_ptr_ptr) {
            using CurrentDimensionType = std::decay_t<decltype(dimension)>;
            using CurrentGeometryType = std::decay_t<decltype(geometry_ptr_ptr)>;

            if constexpr (std::is_same_v<CurrentDimensionType, D2D1_ELLIPSE*> && std::is_same_v<CurrentGeometryType , ID2D1EllipseGeometry**>) {
                D2D1_ELLIPSE* new_ellipse_dimension_ptr{ dimension };
                new_ellipse_dimension_ptr->point.x += new_coordinate->x;
                new_ellipse_dimension_ptr->point.y += new_coordinate->y;

                (*static_windows_factory_ptr_ptr)->CreateEllipseGeometry(*dimension, geometry_ptr_ptr);
            }

            else if constexpr (std::is_same_v <CurrentDimensionType, D2D1_RECT_F*> && std::is_same_v<CurrentGeometryType, ID2D1RectangleGeometry**>) {
                D2D1_RECT_F* new_rectangle_dimension_ptr{ dimension };
                D2D1_POINT_2F old_rectangles_size{ D2D1::Point2F(new_rectangle_dimension_ptr->right - new_rectangle_dimension_ptr->left , new_rectangle_dimension_ptr->bottom - new_rectangle_dimension_ptr->top) };
                D2D1_POINT_2F rectangles_center_position{ dimension->left + (old_rectangles_size.x / 2.0f) , dimension->top + (old_rectangles_size.y / 2.0f) };
                rectangles_center_position.x += new_coordinate->x;
                rectangles_center_position.y += new_coordinate->y;

                new_rectangle_dimension_ptr->left = rectangles_center_position.x - (old_rectangles_size.x / 2.0f);
                new_rectangle_dimension_ptr->top = rectangles_center_position.y - (old_rectangles_size.y / 2.0f);
                new_rectangle_dimension_ptr->right = rectangles_center_position.x + (old_rectangles_size.x / 2.0f);
                new_rectangle_dimension_ptr->bottom = rectangles_center_position.y + (old_rectangles_size.y / 2.0f);
                (*static_windows_factory_ptr_ptr)->CreateRectangleGeometry(*dimension, geometry_ptr_ptr);
            }

            else if constexpr (std::is_same_v <CurrentDimensionType, D2D1_ROUNDED_RECT*> && std::is_same_v<CurrentGeometryType, ID2D1RoundedRectangleGeometry**>) {
                D2D1_ROUNDED_RECT* new_rounded_rectangle_dimension_ptr{ dimension };

                D2D1_POINT_2F old_rounded_rectangle_size{ D2D1::Point2F(new_rounded_rectangle_dimension_ptr->rect.right - new_rounded_rectangle_dimension_ptr->rect.left , new_rounded_rectangle_dimension_ptr->rect.bottom - new_rounded_rectangle_dimension_ptr->rect.top) };

                D2D1_POINT_2F rounded_rectangles_center_position{ dimension->rect.left + (old_rounded_rectangle_size.x / 2.0f) , dimension->rect.top + (old_rounded_rectangle_size.y / 2.0f) };
                rounded_rectangles_center_position.x += new_coordinate->x;
                rounded_rectangles_center_position.y += new_coordinate->y;

                new_rounded_rectangle_dimension_ptr->rect.left = rounded_rectangles_center_position.x - (old_rounded_rectangle_size.x / 2.0f);
                new_rounded_rectangle_dimension_ptr->rect.top = rounded_rectangles_center_position.y - (old_rounded_rectangle_size.y / 2.0f);
                new_rounded_rectangle_dimension_ptr->rect.right = rounded_rectangles_center_position.x + (old_rounded_rectangle_size.x / 2.0f);
                new_rounded_rectangle_dimension_ptr->rect.bottom = rounded_rectangles_center_position.y + (old_rounded_rectangle_size.y / 2.0f);
                (*static_windows_factory_ptr_ptr)->CreateRoundedRectangleGeometry(*dimension, geometry_ptr_ptr);
            }


            ; }, dimension_ptr, geometry_ptr_ptr);


    }

    std::pair<BrushVariantPtrPtr, BrushVariantPtrPtr> get_geometries_brush() override {
        return { &geos_info.geometries_style.fills_brush_ptr , &geos_info.geometries_style.strokes_brush_ptr };
    }

    GeometriesVariantPtrPtr get_shapes_geometry_ptr_ptr() override {
        return &geos_info.shapes_geometry;
    }
    
    unsigned int get_fills_distance_from_stroke() override {
        return geos_info.geometries_style.fills_distance_from_stroke;
    }

    FLOAT get_strokes_width() override {
        return geos_info.geometries_style.strokes_width;
    };

    void get_geometries_essential_info_in_run_time(GeometriesEssentialInfoInProc* geometries_info_ptr) override {
        geometries_info_ptr->current_shapes_geometry_variant = get_shapes_geometry_ptr_ptr();
        geometries_info_ptr->current_geos_dimension_variant = get_dimension_ptr();
        geometries_info_ptr->current_geometries_brush_variant = get_geometries_brush();
        geometries_info_ptr->current_strokes_width = get_strokes_width();
        geometries_info_ptr->current_fills_distance_from_stroke = get_fills_distance_from_stroke();
        geometries_info_ptr->current_moving_properties_ptr = get_moving_geometries_arrow_key_and_mouse_wheel_direction();
        geometries_info_ptr->current_moving_ctrl_type = get_ctrl_type();
        geometries_info_ptr->current_moving_space_type = get_moving_space_type();
        geometries_info_ptr->current_geometries_center_coordinate = get_geometries_center_position(geometries_info_ptr->current_geos_dimension_variant);
    }


};


template<GeometriesShape geos_shape, typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type, GeometriesMovingSpace space_type>
class GeometriesWrapper<true, geos_shape , FillsBrushType , StrokesBrushType , ctrl_type, space_type> : public GeometriesWrapper<false , geos_shape , FillsBrushType , StrokesBrushType>  {

public:
    Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> geos_info{};
    GeometriesWrapper() = default;
    GeometriesWrapper(Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType, ctrl_type, space_type> input_geometries_info) : GeometriesWrapper<false, geos_shape, FillsBrushType, StrokesBrushType>(input_geometries_info) , geos_info(input_geometries_info) {};
    
    bool is_static_geometry() override { return false; };
    
    const GeometriesMovingCtrl get_ctrl_type() override {
        return ctrl_type;
    };
    
    const GeometriesMovingSpace get_moving_space_type() override {
        return space_type;
    };

    const MovingGeometriesProp* get_moving_geometries_arrow_key_and_mouse_wheel_direction() override {
        return &geos_info.moving_properties;
    }

};

// #9

# endif