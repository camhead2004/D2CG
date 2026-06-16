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
    inline static ID2D1Factory* current_static_windows_factory_ptr { nullptr };
    inline static ID2D1HwndRenderTarget* current_static_windows_target_ptr{ nullptr };

    // #15 Update the windows render target rects values if the procedures windows handle change in WM_MOUSEMOVE , WM_PAINT messages in run time

    static bool update_the_render_target(const HWND* const procudure_windows_handle_ptr) {
        RECT render_target_rects_values{ 0 , 0 , 0 , 0 };

        if (current_static_windows_factory_ptr && current_static_windows_target_ptr) {
            HWND render_target_windows_handle{ current_static_windows_target_ptr->GetHwnd() };
            GetWindowRect(render_target_windows_handle, &render_target_rects_values);
        }
        
        RECT procedure_windows_rects_values{};
        GetWindowRect(*procudure_windows_handle_ptr, &procedure_windows_rects_values);

        return (render_target_rects_values.left != procedure_windows_rects_values.left || render_target_rects_values.top != procedure_windows_rects_values.top || render_target_rects_values.right != procedure_windows_rects_values.right ||
            render_target_rects_values.bottom != procedure_windows_rects_values.bottom);
    }

    // #15 


    // #14 Update the factory and hwnd render target to draw direct 2d custom geometries based on this variables 

    static HRESULT init_direct_factory_and_render_target_windows(const HWND* const static_windows_handle_ptr , ID2D1Factory** factory_ptr_ptr , ID2D1HwndRenderTarget** current_windows_renders_target_ptr_ptr) {
        HRESULT output_result {};
        
        if (SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, factory_ptr_ptr))) {
            RECT static_windows_clients_rects_values{};

            if (!GetClientRect(*static_windows_handle_ptr , &static_windows_clients_rects_values)) {
                std::cout << "Error at getting the current rects values " << '\n';
                return 0;
            }

            D2D1_HWND_RENDER_TARGET_PROPERTIES static_windows_properties{ .hwnd{*static_windows_handle_ptr} , .pixelSize{ D2D1::SizeU(static_windows_clients_rects_values.right , static_windows_clients_rects_values.bottom) } ,
            .presentOptions{D2D1_PRESENT_OPTIONS_IMMEDIATELY} };
            output_result = (*factory_ptr_ptr)->CreateHwndRenderTarget(D2D1::RenderTargetProperties() , static_windows_properties ,current_windows_renders_target_ptr_ptr);
        }


        return SUCCEEDED(output_result);
    }

    // #14 

public :

    // #10 Subclass input static class windows into the static_windows_proc procedure 

    void operator()(const HWND* const input_static_windows_handle_ptr) {
        static_class_windows_handle = *input_static_windows_handle_ptr;
        static int current_windows_subclass_id { 1 };
        LONG_PTR input_windows_style{ GetWindowLongPtr(*input_static_windows_handle_ptr , GWL_STYLE) };

        if (((WNDPROC)GetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_USERDATA) == nullptr) && ((input_windows_style & SS_NOTIFY) && (input_windows_style & SS_OWNERDRAW) && (input_windows_style & WS_CHILD))) {
            WNDPROC old_windows_proc{ (WNDPROC)SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_WNDPROC, (LONG_PTR)static_windows_proc) };
            SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_USERDATA, (LONG_PTR)old_windows_proc);
            SetWindowLongPtr(*input_static_windows_handle_ptr, GWLP_ID , current_windows_subclass_id);
            SetWindowSubclass(*input_static_windows_handle_ptr, static_windows_proc, current_windows_subclass_id, 0);
            ++current_windows_subclass_id;

            // #16 resize the vectors to avoid the segmentation fault in rum time while push backing the geometries infos 
            static_windows_custom_direct_2d_geometries.resize(current_windows_subclass_id);
            // #16
        }
    }

    // #10

    // #12 Use SFINAE principle to get the right function with sutiable inputs that depends on the custom geometry being moveable or non-moveable 

    template<bool is_moving_geometry , GeometriesShape shape , Direct2DPredefineBrushType fills_brush_type , Direct2DPredefineBrushType strokes_brush_type>
    typename std::enable_if<!is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<fills_brush_type , strokes_brush_type> style) {
        // # 17 Wrap the geometries info and store in base class 
        
        Direct2DGeometriesInfo<is_moving_geometry, shape, fills_brush_type, strokes_brush_type> concrete_type_geometries_info{};
        concrete_type_geometries_info.geometries_dimension_values = dimension_values;
        concrete_type_geometries_info.geometries_style = style;
        static_windows_custom_direct_2d_geometries[GetDlgCtrlID(static_class_windows_handle) - 1].push_back(std::make_unique<GeometriesWrapper<is_moving_geometry, shape, fills_brush_type, strokes_brush_type>>(concrete_type_geometries_info));
        
        // #17
    }

    template<bool is_moving_geometry , GeometriesShape shape, Direct2DPredefineBrushType fills_brush_type, Direct2DPredefineBrushType strokes_brush_type, GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
    typename std::enable_if<is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<fills_brush_type, strokes_brush_type> style ,
        MovingGeometriesProp<space_type> geometries_ctrl_moving_properties) {

    }

    // #12

private : 
    
    // #11 A static class windows procedure to paint custom direct 2d geometries based on the geometries infos container 

    static LRESULT CALLBACK static_windows_proc(HWND current_static_windows_handle , UINT message_type , WPARAM word_parameter , LPARAM long_parameter , UINT_PTR id , DWORD_PTR procs_data) {
        WNDPROC old_windows_procedure{ (WNDPROC)GetWindowLongPtr(current_static_windows_handle , GWLP_USERDATA) };

        switch (message_type) {

        case WM_PAINT: {
            PAINTSTRUCT windows_paint_info{};
            HDC static_windows_device_context_handle{ BeginPaint(current_static_windows_handle , &windows_paint_info) };

            if (update_the_render_target(&current_static_windows_handle)) {

                if (!SUCCEEDED(init_direct_factory_and_render_target_windows(&current_static_windows_handle, &current_static_windows_factory_ptr, &current_static_windows_target_ptr))) {
                    std::cout << "Error at setting the windows render target " << '\n';
                }
            }

            EndPaint(current_static_windows_handle, &windows_paint_info);
            break;
        }

        case WM_MOUSEMOVE: {

            if (update_the_render_target(&current_static_windows_handle)) {

                if (!SUCCEEDED(init_direct_factory_and_render_target_windows(&current_static_windows_handle, &current_static_windows_factory_ptr, &current_static_windows_target_ptr))) {
                    std::cout << "Error at setting the windows render target " << '\n';
                }
            }

            break;
        }

        case WM_MOUSELEAVE: {
            std::cout << "WM_MOUSELEAVE " << '\n';
            break;
        }

        case WM_KEYDOWN: {
            //static_windows_custom_direct_2d_geometries[id][0].get()->get_dimension_ptr();

            if (LOWORD(word_parameter) == VK_SPACE) {
                
                std::visit([](auto&& dimension) {
                    using T = std::decay_t<decltype(dimension)>;

                    if constexpr (std::is_same<T , D2D1_ELLIPSE*>()) {
                        D2D1_ELLIPSE* ellipse_ptr{ dimension };

                        std::cout << '\n';
                        std::cout << "point x : " << ellipse_ptr->point.x << '\n';
                        std::cout << "point y : " << ellipse_ptr->point.y << '\n';
                        std::cout << "radiusX  : " << ellipse_ptr->radiusX << '\n';
                        std::cout << "radiusY  : " << ellipse_ptr->radiusY << '\n';
                        std::cout << '\n';
                    }
                    
                    
                ; } , static_windows_custom_direct_2d_geometries[id - 1][0].get()->get_dimension_ptr());
            
            }
            
            else if (LOWORD(word_parameter) == VK_RETURN) {

                std::visit([](auto&& dimension) {
                    using T = std::decay_t<decltype(dimension)>;

                    if constexpr (std::is_same<T, D2D1_ELLIPSE*>()) {
                        std::cout << '\n';
                        std::cout << "ENTER " << '\n';
                        D2D1_ELLIPSE* ellipse_ptr{ dimension };
                        ellipse_ptr->point.x *= 2;
                        ellipse_ptr->point.y *= 2;
                        ellipse_ptr->radiusX *= 2;
                        ellipse_ptr->radiusY *= 2;
                    }


                    ; }, static_windows_custom_direct_2d_geometries[id - 1][0].get()->get_dimension_ptr());

            }

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

        a.add_custom_direct_2d_geometry<false, GeometriesShape::ELLIPSE, Direct2DPredefineBrushType::SOLID, Direct2DPredefineBrushType::SOLID>(D2D1::Ellipse(D2D1::Point2F(30.0f, 30.0f), 50.0f, 50.0f), {});
        b.add_custom_direct_2d_geometry<false, GeometriesShape::ELLIPSE, Direct2DPredefineBrushType::SOLID, Direct2DPredefineBrushType::SOLID>(D2D1::Ellipse(D2D1::Point2F(50.0f, 50.0f), 90.0f, 90.0f), {});


        /*
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &main_windows_factory_ptr);
        RECT main_windows_clients_rects{};
        GetClientRect(main_windows_handle, &main_windows_clients_rects);
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_prop{ .hwnd{main_windows_handle} , .pixelSize{ D2D1::SizeU(main_windows_clients_rects.right , main_windows_clients_rects.bottom) } , .presentOptions{D2D1_PRESENT_OPTIONS_IMMEDIATELY} };
        HRESULT result_value{ main_windows_factory_ptr->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), hwnd_prop, &main_windows_render_target_ptr) };
        */
        
        break;
	}

        /*
    case WM_PAINT: {
        PAINTSTRUCT paint_info{};
        HDC device_context_handle{ BeginPaint(main_windows_handle , &paint_info) };

        
        main_windows_render_target_ptr->BeginDraw();
        ID2D1SolidColorBrush* solid_brush_ptr{ nullptr };
        ID2D1LinearGradientBrush* linear_gradient_brush_ptr{ nullptr };
        ID2D1RadialGradientBrush* radial_gradient_brush_ptr{ nullptr };
        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES linear_prop{ .startPoint{D2D1::Point2F(240.0f , 240.0f)} , .endPoint{D2D1::Point2F(300.0f , 300.0f)} };
        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radial_prop{ .center{D2D1::Point2F(300.0f , 300.0f)} , .radiusX{ 40.0f } , .radiusY{ 40.0f } };

        ID2D1GradientStopCollection* stop_collection_ptr { nullptr };
        D2D1_GRADIENT_STOP stops[4]{ {0.1f , D2D1::ColorF(D2D1::ColorF::Aqua) } , { 0.3f , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , { 0.5f , D2D1::ColorF(D2D1::ColorF::Coral) }, { 0.8f , D2D1::ColorF(D2D1::ColorF::HotPink) } };
        main_windows_render_target_ptr->CreateGradientStopCollection(stops, 4, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_WRAP, &stop_collection_ptr);

        main_windows_render_target_ptr->CreateLinearGradientBrush(linear_prop , stop_collection_ptr , &linear_gradient_brush_ptr);
        main_windows_render_target_ptr->CreateRadialGradientBrush(radial_prop , stop_collection_ptr , &radial_gradient_brush_ptr);



        main_windows_render_target_ptr->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlueViolet) , &solid_brush_ptr);
        //main_windows_render_target_ptr->CreateLinearGradientBrush( , );


        main_windows_render_target_ptr->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(300.0f, 300.0f), 100.0f, 100.0f) , radial_gradient_brush_ptr , 10.0f);
        main_windows_render_target_ptr->FillEllipse(D2D1::Ellipse(D2D1::Point2F(300.0f, 300.0f), 60.0f, 60.0f) , radial_gradient_brush_ptr);



        main_windows_render_target_ptr->EndDraw();


        EndPaint(main_windows_handle, &paint_info);
        return 0;
    }
        */


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