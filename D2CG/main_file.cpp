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

    // #12 Use SFINAE principle to get the right function with sutiable inputs that depends on the custom geometry being moveable or non-moveable 

    template<bool is_moving_geometry , GeometriesShape shape , typename FillsBrushType , typename StrokesBrushType>
    typename std::enable_if<!is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType, StrokesBrushType> style) {

    }

    template<bool is_moving_geometry , GeometriesShape shape, typename FillsBrushType, typename StrokesBrushType , GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
    typename std::enable_if<is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType, StrokesBrushType> style , 
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

            EndPaint(current_static_windows_handle, &windows_paint_info);
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