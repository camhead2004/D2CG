# include <iostream>
# include <vector>
# include <memory>
# include <windows.h>
# include <CommCtrl.h>
# include <d2d1.h>
# include <d2d1helper.h>

# pragma comment (lib , "comctl32.lib")
# pragma comment (lib , "d2d1.lib")
	
const RECT* const get_screens_coordinate() {
    HWND run_time_terminal_windows_handle{ GetConsoleWindow() };
    HMONITOR screens_handle{ MonitorFromWindow(run_time_terminal_windows_handle , MONITOR_DEFAULTTONEAREST) };
    MONITORINFO screens_info{ sizeof(MONITORINFO) };
    GetMonitorInfo(screens_handle, &screens_info);
    static RECT output_screens_rects_value{ screens_info.rcWork };

    return &output_screens_rects_value;
}


// #5 Additinal data for Direct2D moving geometries type

enum class GeometriesMovingCtrl {
    NONE = 0 ,
    ARROW_KEY = (1 << 1) ,
    MOUSEHWEEL = (2 << 1) ,
    LEFT_CLICKED = (3 << 1) ,
    LEFT_CLICKED_HOLD = (4 << 1) ,
};

enum class GeometriesMovingSpace {
    NONE , 
    X_AXIS_ONLY , 
    Y_AXIS_ONLY , 
    FREE
};

enum class DirectionInX {
    NONE , 
    UP , 
    DOWN , 
};

enum class DirectionInY {
    NONE , 
    LEFT ,
    RIGHT , 
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



enum class GeometriesShape{
    NONE , 
    RECTANGLE , 
    ROUNDED_RECTANGLE , 
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


// #8 Partial specialized GeometriesInfo based on being static or moving geometries to store extra moving properties filed on the object 

template<bool is_moving_geometry , GeometriesShape geos_shape , typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE , GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class Direct2DGeometriesInfo {};

template<GeometriesShape geos_shape , typename FillsBrushType , typename StrokesBrushType>
class Direct2DGeometriesInfo<false, geos_shape, FillsBrushType, StrokesBrushType> {
    typename ShapeToDimensionType<geos_shape>::Geometry shapes_geometry{};

public :
    typename ShapeToDimensionType<geos_shape>::Dimension geometries_dimension_values {};

};


template<GeometriesShape geos_shape , typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
class Direct2DGeometriesInfo<true , geos_shape, FillsBrushType, StrokesBrushType , ctrl_type , space_type> : public Direct2DGeometriesInfo <false , geos_shape, FillsBrushType, StrokesBrushType> {

public : 
    MovingGeometriesProp<space_type> moving_properties;
};

// #8


// #9 Make a run time polymorphism to store a geometries properties in different template parameters and types in a vector container

class Direct2DGeometriesBase {
public :
    virtual ~Direct2DGeometriesBase() = default;
    virtual void* get_geometries_info() = 0;
};


template<bool is_moving_geometry , GeometriesShape geos_shape , typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type = GeometriesMovingCtrl::NONE , GeometriesMovingSpace space_type = GeometriesMovingSpace::NONE>
class GeometriesWrapper{};

template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType>
class GeometriesWrapper<false, geos_shape, FillsBrushType, StrokesBrushType> : public Direct2DGeometriesBase {
    
public :
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


template<GeometriesShape geos_shape, typename FillsBrushType, typename StrokesBrushType , GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
class GeometriesWrapper<true , geos_shape , FillsBrushType , StrokesBrushType , ctrl_type , space_type> : public Direct2DGeometriesBase {

public :
    Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType , ctrl_type, space_type> geos_info{};
    GeometriesWrapper() = default;
    GeometriesWrapper(Direct2DGeometriesInfo<true, geos_shape, FillsBrushType, StrokesBrushType , ctrl_type , space_type> input_geometries_info) : geos_info(input_geometries_info) {
        //std::cout << "Constructor GeometriesWrapper<true, geos_shape, FillsBrushType, StrokesBrushType , ctrl_type , sapce_type> " << '\n';
    };

    void* get_geometries_info() override {
        return &geos_info;
    }

};




// #7 Define an object where a static class child windows store different types of custom Direct2D geometries 

class StaticWindowsCustomGeometries {
    inline static std::vector<std::vector<std::unique_ptr<Direct2DGeometriesBase>>> static_windows_custom_direct_2d_geometries{};


public :

    // #10 Subclass input static class windows into the static_windows_proc procedure 

    void operator()(const HWND* const input_static_windows_handle_ptr) {
        static int current_windows_subclass_id { 1 };
        LONG_PTR input_windows_style{ GetWindowLongPtr(*input_static_windows_handle_ptr , GWL_STYLE) };

        if (((WNDPROC)GetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_USERDATA) == nullptr) && ((input_windows_style & SS_NOTIFY) && (input_windows_style & SS_OWNERDRAW) && (input_windows_style & WS_CHILD))) {
            WNDPROC old_windows_proc{ (WNDPROC)SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_WNDPROC, (LONG_PTR)static_windows_proc) };
            SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_USERDATA, (LONG_PTR)old_windows_proc);
            SetWindowSubclass(*input_static_windows_handle_ptr, static_windows_proc, current_windows_subclass_id, 0);
            ++current_windows_subclass_id;
        }
    }

    // #10


private : 
    
    // #11 A static class windows procedure to paint custom direct 2d geometries based on the geometries infos container 

    static LRESULT CALLBACK static_windows_proc(HWND current_static_windows_handle , UINT message_type , WPARAM word_parameter , LPARAM long_parameter , UINT_PTR id , DWORD_PTR procs_data) {
        WNDPROC old_windows_procedure{ (WNDPROC)GetWindowLongPtr(current_static_windows_handle , GWLP_USERDATA) };

        switch (message_type) {

        case WM_PAINT: {
            break;
        }

        case WM_MOUSEMOVE: {
            break;
        }

        case WM_KEYDOWN: {
            break;
        }

        case WM_LBUTTONDOWN: {
        case WM_LBUTTONDBLCLK : 
            SetFocus(current_static_windows_handle);
            break;
        }

        }


        return CallWindowProc(old_windows_procedure, current_static_windows_handle , message_type , word_parameter, long_parameter);
    }

    // #11

};

// #7


// #1 Set main window procedure 

LRESULT CALLBACK main_windows_proc(HWND main_windows_handle , UINT message_type , WPARAM word_parameter , LPARAM long_parameter) {

    // Sample datas
    static HWND static_1{};
    static HWND static_2{};
    StaticWindowsCustomGeometries a{};
    StaticWindowsCustomGeometries b{};

	switch (message_type) {

	case WM_CREATE: {
        HWND run_time_terminals_windows_handle{ GetConsoleWindow() };
        auto run_time_terminals_windows_rects_values_ptr { get_screens_coordinate() };
        SetWindowPos(run_time_terminals_windows_handle, 0, run_time_terminals_windows_rects_values_ptr->right - 500, 0, 500, 400, 0);
		
        
        static_1 = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_OWNERDRAW, 10, 10, 900, 380, main_windows_handle, 0, ((LPCREATESTRUCT)long_parameter)->hInstance, 0);
        static_2 = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_OWNERDRAW, 10, 400, 900, 340, main_windows_handle, 0, ((LPCREATESTRUCT)long_parameter)->hInstance, 0);
        a(&static_1);
        b(&static_2);


        break;
	}

	default: {
		return DefWindowProc(main_windows_handle, message_type, word_parameter, long_parameter);
	}

	}

	return 0;
}

// #1


// #2 Execute main windows in run time 

void execute_the_main_window(COLORREF background_color) {
    WNDCLASS windows_class{ .lpfnWndProc{ main_windows_proc } , .hInstance{ GetModuleHandle(0) } , .hCursor{ LoadCursor(NULL, IDC_ARROW) } , .hbrBackground{ CreateSolidBrush(background_color) } , .lpszClassName{ TEXT("Main Window") } };
    RegisterClass(&windows_class);
    HWND main_windows_handle{ CreateWindowEx(0 ,  windows_class.lpszClassName , TEXT("Windows.exe") , WS_OVERLAPPEDWINDOW | WS_VISIBLE , 0 , 0 , 1000 , 800 , 0 , 0 , windows_class.hInstance , 0) };
    MSG windows_message{};

	// #3 Get windows current message in run time and dispatch to main windows procedure

    while (true) {

        while (GetMessage(&windows_message, main_windows_handle, 0, 0)) {
            TranslateMessage(&windows_message);
            DispatchMessage(&windows_message);
        }
    }

	// #3
}

// #2


int main() {
    execute_the_main_window(RGB(0 , 0 , 0));


	return 0;
}