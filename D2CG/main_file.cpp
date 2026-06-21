# include <iostream>
# include <vector>
# include <memory>
# include <windows.h>
# include <CommCtrl.h>
# include <d2d1.h>
# include <d2d1helper.h>

# pragma comment (lib , "comctl32.lib")
# pragma comment (lib , "d2d1.lib")

# include "main_file.hpp"
# include "direct_2d_objects.hpp"
# include "direct_2d_function.hpp"

# define WM_GET_GEOMETRIES_INFO 1024



const RECT* const get_screens_coordinate() {
    HWND run_time_terminal_windows_handle{ GetConsoleWindow() };
    HMONITOR screens_handle{ MonitorFromWindow(run_time_terminal_windows_handle , MONITOR_DEFAULTTONEAREST) };
    MONITORINFO screens_info{ sizeof(MONITORINFO) };
    GetMonitorInfo(screens_handle, &screens_info);
    static RECT output_screens_rects_value{ screens_info.rcWork };

    return &output_screens_rects_value;
}

// #7 Define an object where a static class child windows store different types of custom Direct2D geometries 

class StaticWindowsCustomGeometries {
    HWND static_class_windows_handle{};
    inline static std::vector<std::vector<std::unique_ptr<Direct2DGeometriesBase>>> static_windows_custom_direct_2d_geometries{};
    inline static std::vector<ID2D1Factory*> static_windows_factory_ptr { nullptr };
    inline static std::vector<ID2D1HwndRenderTarget*> static_windows_target_ptr{ nullptr };

    // #14 Set ID2D1Factory* and ID2D1HwndRenderTarget* objects for each static window 

    static HRESULT init_direct_factory_and_render_target_windows(const HWND* const static_windows_handle_ptr, int windows_id) {
        HRESULT output_result{};

        static_windows_factory_ptr.resize(GetDlgCtrlID(*static_windows_handle_ptr));
        static_windows_target_ptr.resize(GetDlgCtrlID(*static_windows_handle_ptr));
        output_result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &static_windows_factory_ptr.back());

        if (SUCCEEDED(output_result)) {
            RECT current_static_windows_clients_rects_values{};
            GetClientRect(*static_windows_handle_ptr, &current_static_windows_clients_rects_values);

            D2D1_HWND_RENDER_TARGET_PROPERTIES current_windows_properties{
                .hwnd{ *static_windows_handle_ptr } ,
                .pixelSize{ D2D1::SizeU(current_static_windows_clients_rects_values.right , current_static_windows_clients_rects_values.bottom) } ,
                .presentOptions{D2D1_PRESENT_OPTIONS_IMMEDIATELY}
            };

            static_windows_factory_ptr.back()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), current_windows_properties, &static_windows_target_ptr.back());
        }


        return SUCCEEDED(output_result);
    }

    // #14 

public :

    // #10 Subclass input static class windows into the static_windows_proc procedure 

    void operator()(const HWND* const input_static_windows_handle_ptr) {


        if (GetDlgCtrlID(static_class_windows_handle) == 0) {
            static_class_windows_handle = *input_static_windows_handle_ptr;
            static int current_windows_subclass_id { 1 };
            LONG_PTR input_windows_style{ GetWindowLongPtr(*input_static_windows_handle_ptr , GWL_STYLE) };

            if (((WNDPROC)GetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_USERDATA) == nullptr) && ((input_windows_style & SS_NOTIFY) && (input_windows_style & SS_OWNERDRAW) && (input_windows_style & WS_CHILD))) {
                WNDPROC old_windows_proc{ (WNDPROC)SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_WNDPROC, (LONG_PTR)static_windows_proc) };
                SetWindowLongPtr(static_class_windows_handle, GWLP_USERDATA, (LONG_PTR)old_windows_proc);
                SetWindowLongPtr(static_class_windows_handle, GWLP_ID, current_windows_subclass_id);
                SetWindowSubclass(static_class_windows_handle, static_windows_proc, current_windows_subclass_id, 0);
                init_direct_factory_and_render_target_windows(&static_class_windows_handle , current_windows_subclass_id);
                ++current_windows_subclass_id;

                // #16 resize the vectors to avoid the segmentation fault in rum time while push backing the geometries infos 
                static_windows_custom_direct_2d_geometries.resize(current_windows_subclass_id);
                // #16
            }
        }
    }

    // #10

    // #12 Use SFINAE principle to get the right function with sutiable inputs that depends on the custom geometry being moveable or non-moveable 

    template<bool is_moving_geometry , GeometriesShape shape , typename FillsBrushType , typename StrokesBrushType>
    typename std::enable_if<!is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType , StrokesBrushType> style) {
        // # 17 Wrap the geometries info and store in base class 
        
        Direct2DGeometriesInfo<is_moving_geometry, shape, FillsBrushType, StrokesBrushType> concrete_type_geometries_info{};
        concrete_type_geometries_info.geometries_dimension_values = dimension_values;
        concrete_type_geometries_info.geometries_style = style;
        
        
        auto geometry_function{ set_direct_2d_geometry_function<typename ShapeToDimensionType<shape>::Geometry>(&static_windows_factory_ptr[GetDlgCtrlID(static_class_windows_handle) - 1]) };
        geometry_function(concrete_type_geometries_info.geometries_dimension_values , &concrete_type_geometries_info.shapes_geometry);
        set_brush<FillsBrushType>(&static_windows_target_ptr[GetDlgCtrlID(static_class_windows_handle) - 1] , &concrete_type_geometries_info.geometries_style.fills_brush_info , &concrete_type_geometries_info.geometries_style.fills_brush_ptr);
        
        set_brush<StrokesBrushType>(&static_windows_target_ptr[GetDlgCtrlID(static_class_windows_handle) - 1] , &concrete_type_geometries_info.geometries_style.strokes_brush_info , &concrete_type_geometries_info.geometries_style.strokes_brush_ptr);

        static_windows_custom_direct_2d_geometries[GetDlgCtrlID(static_class_windows_handle) - 1].push_back(std::make_unique<GeometriesWrapper<is_moving_geometry, shape, FillsBrushType, StrokesBrushType>>(concrete_type_geometries_info));

        // #17
    }

    template<bool is_moving_geometry , GeometriesShape shape , typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
    typename std::enable_if<is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType, StrokesBrushType> style ,
        MovingGeometriesProp<space_type> geometries_ctrl_moving_properties) {

        /*
        Direct2DGeometriesInfo<is_moving_geometry, shape, fills_brush_type, strokes_brush_type , ctrl_type , space_type> concrete_type_geometries_info{};
        concrete_type_geometries_info.geometries_dimension_values = dimension_values;
        concrete_type_geometries_info.geometries_style = style;
        static_windows_custom_direct_2d_geometries[GetDlgCtrlID(static_class_windows_handle) - 1].push_back(std::make_unique<GeometriesWrapper<is_moving_geometry, shape, fills_brush_type, strokes_brush_type , ctrl_type , space_type>>(concrete_type_geometries_info));
        */
    }

    // #12

private : 
    
    // #11 A static class windows procedure to paint custom direct 2d geometries based on the geometries infos container 

    static LRESULT CALLBACK static_windows_proc(HWND current_static_windows_handle , UINT message_type , WPARAM word_parameter , LPARAM long_parameter , UINT_PTR id , DWORD_PTR procs_data) {
        WNDPROC old_windows_procedure{ (WNDPROC)GetWindowLongPtr(current_static_windows_handle , GWLP_USERDATA) };

        switch (message_type) {

        case WM_PAINT: {
            // #20 Paint statics windows custom direct 2d geometries in a for loop

            PAINTSTRUCT windows_paint_info{};
            HDC static_windows_device_context_handle{ BeginPaint(current_static_windows_handle , &windows_paint_info) };
            static_windows_target_ptr[ id - 1 ]->BeginDraw();
            static_windows_target_ptr[id - 1]->Clear(D2D1::ColorF(D2D1::ColorF::Black));

            for (int static_windows_geometry{ 0 }; static_windows_geometry < static_windows_custom_direct_2d_geometries[id - 1].size(); ++static_windows_geometry) {
                DimensionVariantPtr current_dimension_ptr { static_windows_custom_direct_2d_geometries[id - 1][static_windows_geometry].get()->get_dimension_ptr() };
                std::pair<BrushVariantPtrPtr, BrushVariantPtrPtr> current_geometries_brushs_ptr_ptr { static_windows_custom_direct_2d_geometries[id - 1][static_windows_geometry].get()->get_geometries_brush() };
                ID2D1HwndRenderTarget** current_windows_render_target { &static_windows_target_ptr[id - 1] };
                FLOAT current_geometries_strokes_width{ static_windows_custom_direct_2d_geometries[id - 1][static_windows_geometry].get()->get_strokes_width() };
                unsigned int current_goemetries_fills_distance_from_stroke{ static_windows_custom_direct_2d_geometries[id - 1][static_windows_geometry].get()->get_fills_distance_from_stroke() };

                std::visit([current_windows_render_target , current_geometries_strokes_width , current_goemetries_fills_distance_from_stroke, id](auto&& dimension , auto&& fill_brush_ptr_ptr, auto&& stroke_brush_ptr_ptr) {
                    using CurrentDimensionType = std::decay_t<decltype(dimension)>;

                    if (fill_brush_ptr_ptr && stroke_brush_ptr_ptr) {

                        if constexpr (std::is_same_v<CurrentDimensionType, D2D1_ELLIPSE*>) {
                            D2D1_ELLIPSE fills_dimension_with_distance{ *dimension };
                            fills_dimension_with_distance.radiusX -= static_cast<FLOAT>(current_goemetries_fills_distance_from_stroke);
                            fills_dimension_with_distance.radiusY -= static_cast<FLOAT>(current_goemetries_fills_distance_from_stroke);
                            (*current_windows_render_target)->FillEllipse(fills_dimension_with_distance, *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawEllipse(dimension, *stroke_brush_ptr_ptr, current_geometries_strokes_width, 0);
                        }

                        else if constexpr (std::is_same_v<CurrentDimensionType, D2D1_RECT_F*>) {
                            D2D1_RECT_F fills_dimension_with_distance{ *dimension };
                            fills_dimension_with_distance.left += current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.top += current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.right -= current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.bottom -= current_goemetries_fills_distance_from_stroke;

                            (*current_windows_render_target)->FillRectangle(fills_dimension_with_distance , *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawRectangle(dimension, *stroke_brush_ptr_ptr, current_geometries_strokes_width, 0);
                        }

                        else {
                            D2D1_ROUNDED_RECT fills_dimension_with_distance{ *dimension };
                            FLOAT original_x_centers_corner{ dimension->rect.left + dimension->radiusX };
                            FLOAT original_y_centers_corner{ dimension->rect.top + dimension->radiusY };

                            fills_dimension_with_distance.rect.left += current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.top += current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.right -= current_goemetries_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.bottom -= current_goemetries_fills_distance_from_stroke;
                            
                            fills_dimension_with_distance.radiusX = original_x_centers_corner - fills_dimension_with_distance.rect.left;
                            fills_dimension_with_distance.radiusY = original_y_centers_corner - fills_dimension_with_distance.rect.top;


                            std::cout << '\n';
                            std::cout << "Old Width : " << dimension->rect.right - dimension->rect.left << '\n';
                            std::cout << "Old Height : " << dimension->rect.bottom - dimension->rect.top << '\n';
                            std::cout << "Old left : " << dimension->rect.left << '\n';
                            std::cout << "Old Top : " << dimension->rect.top << '\n';
                            std::cout << "original_x_centers_corner : " << original_x_centers_corner << '\n';
                            std::cout << "original_y_centers_corner : " << original_y_centers_corner << '\n';
                            std::cout << "Old radiusX : " << dimension->radiusX << '\n';
                            std::cout << "Old radiusY : " << dimension->radiusY << '\n';
                            std::cout << '\n';
 
                            std::cout << '\n';
                            std::cout << "New Width : " << fills_dimension_with_distance.rect.right - fills_dimension_with_distance.rect.left << '\n';
                            std::cout << "New Height : " << fills_dimension_with_distance.rect.bottom - fills_dimension_with_distance.rect.top << '\n';
                            std::cout << "New left : " << fills_dimension_with_distance.rect.left << '\n';
                            std::cout << "New Top : " << fills_dimension_with_distance.rect.top << '\n';
                            std::cout << "New radiusX : " << fills_dimension_with_distance.radiusX << '\n';
                            std::cout << "New radiusY : " << fills_dimension_with_distance.radiusY << '\n';
                            std::cout << '\n';
 



                            (*current_windows_render_target)->FillRoundedRectangle(fills_dimension_with_distance , *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawRoundedRectangle(dimension, *stroke_brush_ptr_ptr, current_geometries_strokes_width, 0);
                        }
                    }


                    ; }, current_dimension_ptr , current_geometries_brushs_ptr_ptr.first, current_geometries_brushs_ptr_ptr.second);
            }

            static_windows_target_ptr[ id - 1 ]->EndDraw();
            
            // #20
            break;
        }


        case WM_MOUSEMOVE: {
            break;
        }

        case WM_MOUSELEAVE: {
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

    static ID2D1Factory* main_windows_factory_ptr{ nullptr };
    static ID2D1HwndRenderTarget* main_windows_render_target_ptr{ nullptr };


	switch (message_type) {

	case WM_CREATE: {
        HWND run_time_terminals_windows_handle{ GetConsoleWindow() };
        auto run_time_terminals_windows_rects_values_ptr { get_screens_coordinate() };
        SetWindowPos(run_time_terminals_windows_handle, 0, run_time_terminals_windows_rects_values_ptr->right - 500, 0, 500, 400, 0);
        
        static_1 = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_OWNERDRAW, 10, 10, 900, 380, main_windows_handle, 0 , ((LPCREATESTRUCT)long_parameter)->hInstance, 0);
        static_2 = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_OWNERDRAW, 10, 400, 900, 340, main_windows_handle, 0 , ((LPCREATESTRUCT)long_parameter)->hInstance, 0);
        a(&static_1);
        b(&static_2);

        a.add_custom_direct_2d_geometry<false, GeometriesShape::ELLIPSE, ID2D1SolidColorBrush* , ID2D1SolidColorBrush*>(D2D1::Ellipse(D2D1::Point2F(130.0f, 100.0f), 50.0f, 50.0f),
            
            { .fills_brush_info{ D2D1::ColorF(D2D1::ColorF::BlueViolet) } ,
              .fills_distance_from_stroke{ 20 } ,
              .strokes_brush_info{ D2D1::ColorF(D2D1::ColorF::Coral) } ,
              .strokes_width{ 5.5f }
            }
        );


        a.add_custom_direct_2d_geometry<false, GeometriesShape::ELLIPSE, ID2D1RadialGradientBrush* , ID2D1SolidColorBrush*>(D2D1::Ellipse(D2D1::Point2F(330.0f, 60.0f), 50.0f, 50.0f),
            
            { .fills_brush_info{ D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_WRAP , { {0.3 , D2D1::ColorF(D2D1::ColorF::Coral) } , {0.5 , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.8 , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } } , 
                D2D1::Point2F(330.0f, 60.0f) , 30.0f , 30.0f } ,


              .fills_distance_from_stroke{ 20 } ,
              .strokes_brush_info{ D2D1::ColorF(D2D1::ColorF::HotPink) } ,
              .strokes_width{ 10.5f }
            }
        );



        a.add_custom_direct_2d_geometry<false, GeometriesShape::ROUNDED_RECTANGLE, ID2D1LinearGradientBrush*, ID2D1LinearGradientBrush*>({ D2D1::RectF(200.0f , 200.0f , 800.0f , 350.0f) , 60.0f , 60.0f},
            { .fills_brush_info{ D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_WRAP , { {0.3 , D2D1::ColorF(D2D1::ColorF::Coral) } , {0.5 , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.8 , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } } , { D2D1::Point2F(200.0f , 200.0f) } , { D2D1::Point2F(800.0f , 350.0f) }  } ,
              
            .fills_distance_from_stroke{ 30 } ,


              .strokes_brush_info{ D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_WRAP , { {0.3 , D2D1::ColorF(D2D1::ColorF::Coral) } , {0.5 , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.8 , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } } , { D2D1::Point2F(200.0f , 200.0f) } , { D2D1::Point2F(800.0f , 350.0f) } } ,
              .strokes_width{ 10.5f }
            }
        );

        
        b.add_custom_direct_2d_geometry<false, GeometriesShape::ELLIPSE, ID2D1SolidColorBrush* , ID2D1SolidColorBrush*>(D2D1::Ellipse(D2D1::Point2F(150.0f, 100.0f), 90.0f, 90.0f),
            { .fills_brush_info{ D2D1::ColorF(D2D1::ColorF::Aquamarine) } ,
              .fills_distance_from_stroke{ 10 } ,
              .strokes_brush_info{ D2D1::ColorF(D2D1::ColorF::HotPink) } ,
              .strokes_width{ 15.5f }
            }
        );


        b.add_custom_direct_2d_geometry<false, GeometriesShape::RECTANGLE, ID2D1LinearGradientBrush*, ID2D1SolidColorBrush*>(D2D1::RectF(400.0f, 100.0f, 700.0f, 200.0f),
            { .fills_brush_info{ D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_WRAP , { {0.3 , D2D1::ColorF(D2D1::ColorF::Coral) } , {0.5 , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.8 , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } } , { D2D1::Point2F(400.0f , 100.0f) } , { D2D1::Point2F(700.0f , 200.0f) } } ,
            
              .fills_distance_from_stroke{ 10 } ,
              .strokes_brush_info{ D2D1::ColorF(D2D1::ColorF::Violet) } ,
              .strokes_width{ 10.5f }
            }
        );

        
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

// last comment #22

// deleted comments = #15

/*
// run time crash when you dont init fills brush prop or strokes brush prop
*/