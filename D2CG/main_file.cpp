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

POINT get_cursor_position_in_screen_client(const HWND* const windows_handle_ptr) {
    POINT output_coordinate{};
    GetCursorPos(&output_coordinate);
    ScreenToClient(*windows_handle_ptr , &output_coordinate);
    
    return output_coordinate;
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

    template<bool is_moving_geometry, GeometriesShape shape, typename FillsBrushType, typename StrokesBrushType>
    void set_input_geometries_info(auto input_geometries_info_ptr , ID2D1Factory** static_windows_factory_ptr_ptr , ID2D1HwndRenderTarget** windows_render_target_ptr_ptr , typename ShapeToDimensionType<shape>::Dimension dimension_values ,  GeometriesCustomStyle<FillsBrushType , StrokesBrushType> style) {
        
        input_geometries_info_ptr->geometries_dimension_values = dimension_values;
        input_geometries_info_ptr->geometries_style = style;
        auto geometry_function{ set_direct_2d_geometry_function<typename ShapeToDimensionType<shape>::Geometry>(static_windows_factory_ptr_ptr) };
        geometry_function(input_geometries_info_ptr->geometries_dimension_values, &input_geometries_info_ptr->shapes_geometry);
        set_brush<FillsBrushType>(windows_render_target_ptr_ptr , &input_geometries_info_ptr->geometries_style.fills_brush_info, &input_geometries_info_ptr->geometries_style.fills_brush_ptr);
        set_brush<StrokesBrushType>(windows_render_target_ptr_ptr , &input_geometries_info_ptr->geometries_style.strokes_brush_info, &input_geometries_info_ptr->geometries_style.strokes_brush_ptr);
    }

    template<bool is_moving_geometry , GeometriesShape shape , typename FillsBrushType , typename StrokesBrushType>
    typename std::enable_if<!is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType , StrokesBrushType> style) {
        // # 17 Wrap static geometries info and store in base class 
        
        Direct2DGeometriesInfo<is_moving_geometry, shape, FillsBrushType, StrokesBrushType> concrete_type_geometries_info{};
        
        set_input_geometries_info<is_moving_geometry, shape, FillsBrushType, StrokesBrushType>(&concrete_type_geometries_info, &static_windows_factory_ptr[GetDlgCtrlID(static_class_windows_handle) - 1], &static_windows_target_ptr[GetDlgCtrlID(static_class_windows_handle) - 1], dimension_values, style);

        static_windows_custom_direct_2d_geometries[GetDlgCtrlID(static_class_windows_handle) - 1].push_back(std::make_unique<GeometriesWrapper<is_moving_geometry, shape, FillsBrushType, StrokesBrushType>>(concrete_type_geometries_info));
        // #17
    }

    template<bool is_moving_geometry , GeometriesShape shape , typename FillsBrushType , typename StrokesBrushType , GeometriesMovingCtrl ctrl_type , GeometriesMovingSpace space_type>
    typename std::enable_if<is_moving_geometry, void>::type add_custom_direct_2d_geometry(typename ShapeToDimensionType<shape>::Dimension dimension_values, GeometriesCustomStyle<FillsBrushType, StrokesBrushType> style ,
        MovingGeometriesProp geometries_ctrl_moving_properties) {

        // #23 Wrap moving geometries info and store in base class 

        Direct2DGeometriesInfo<is_moving_geometry, shape, FillsBrushType , StrokesBrushType , ctrl_type , space_type> concrete_type_geometries_info{};
        
        set_input_geometries_info<is_moving_geometry, shape, FillsBrushType, StrokesBrushType>(&concrete_type_geometries_info, &static_windows_factory_ptr[GetDlgCtrlID(static_class_windows_handle) - 1],
            &static_windows_target_ptr[GetDlgCtrlID(static_class_windows_handle) - 1], dimension_values, style);

        concrete_type_geometries_info.moving_properties = geometries_ctrl_moving_properties;
        static_windows_custom_direct_2d_geometries[GetDlgCtrlID(static_class_windows_handle) - 1].push_back(std::make_unique<GeometriesWrapper<is_moving_geometry, shape, FillsBrushType , StrokesBrushType , ctrl_type , space_type>>(concrete_type_geometries_info));

        // #23 
    }

    // #12

private : 
    
    static void change_geometries_position_on_arrow_key(D2D1_POINT_2F* geometries_position_ptr , GeometriesMovingSpace moving_space_type , const XYDirection* input_arrow_direction_ptr) {

        if (*input_arrow_direction_ptr == XYDirection::LEFT && ((static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::FREE)) || (static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::X_AXIS_ONLY))) ) {
            geometries_position_ptr->x -= 5;
        }

        else if (*input_arrow_direction_ptr == XYDirection::UP && ((static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::FREE)) || (static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::Y_AXIS_ONLY)))) {
            geometries_position_ptr->y -= 5;
        }

        else if (*input_arrow_direction_ptr == XYDirection::RIGHT && ((static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::FREE)) || (static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::X_AXIS_ONLY)))) {
            geometries_position_ptr->x += 5;

        }

        else if (*input_arrow_direction_ptr == XYDirection::DOWN && ((static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::FREE)) || (static_cast<unsigned>(moving_space_type) == static_cast<unsigned>(GeometriesMovingSpace::Y_AXIS_ONLY)))) {
            geometries_position_ptr->y += 5;
        }
    }

    static void change_the_geometries_position_on_mouse_wheel(D2D1_POINT_2F* geometries_position_ptr, GeometriesMovingSpace moving_space_type, const XYDirection* input_mouse_wheel_direction_ptr) {

        if (moving_space_type != GeometriesMovingSpace::FREE) {
            
            if (*input_mouse_wheel_direction_ptr == XYDirection::LEFT && (moving_space_type == GeometriesMovingSpace::X_AXIS_ONLY)) {
                geometries_position_ptr->x -= 5;
            }


            else if (*input_mouse_wheel_direction_ptr == XYDirection::RIGHT && (moving_space_type == GeometriesMovingSpace::X_AXIS_ONLY)) {
                geometries_position_ptr->x += 5;
            
            }

            else if (*input_mouse_wheel_direction_ptr == XYDirection::UP && (moving_space_type == GeometriesMovingSpace::Y_AXIS_ONLY)) {
                geometries_position_ptr->y -= 5;
            
            }

            else if (*input_mouse_wheel_direction_ptr == XYDirection::DOWN && (moving_space_type == GeometriesMovingSpace::Y_AXIS_ONLY)) {
                geometries_position_ptr->y += 5;
            }
        }

        else {
            std::cout << "The geometries moving space must not be GeometriesMovingSpace::FREE in mousewheel control " << '\n';
        }
    }


    static void set_moving_geometries_coordinate_in_left_click_events(const HWND* const static_windows_handle_ptr, UINT procedure_id, Direct2DGeometriesBase* moving_geometries_info, GeometriesEssentialInfoInProc* current_geometries_info_procs_data_ptr, const std::pair<unsigned int, unsigned int>* const focused_moving_geometry_ptr, bool left_clicked_hold) {

        moving_geometries_info->get_geometries_essential_info_in_run_time(current_geometries_info_procs_data_ptr);

        if (current_geometries_info_procs_data_ptr->current_moving_ctrl_type & ((left_clicked_hold) ? GeometriesMovingCtrl::LEFT_CLICKED_HOLD : GeometriesMovingCtrl::LEFT_CLICKED)) {
            POINT current_clicked_cursor_position{ get_cursor_position_in_screen_client(static_windows_handle_ptr) };
            D2D1_POINT_2F new_clicked_coordinate{ D2D1::Point2F(0.0f , 0.0f) };

            if ((current_geometries_info_procs_data_ptr->current_moving_space_type == GeometriesMovingSpace::X_AXIS_ONLY) || (current_geometries_info_procs_data_ptr->current_moving_space_type == GeometriesMovingSpace::FREE)) {
                new_clicked_coordinate.x = current_clicked_cursor_position.x - current_geometries_info_procs_data_ptr->current_geometries_center_coordinate.x;
            }

            if ((current_geometries_info_procs_data_ptr->current_moving_space_type == GeometriesMovingSpace::Y_AXIS_ONLY) || (current_geometries_info_procs_data_ptr->current_moving_space_type == GeometriesMovingSpace::FREE)) {
                new_clicked_coordinate.y = current_clicked_cursor_position.y - current_geometries_info_procs_data_ptr->current_geometries_center_coordinate.y;
            }

            static_windows_custom_direct_2d_geometries[focused_moving_geometry_ptr->first][focused_moving_geometry_ptr->second].get()->set_geometries_new_coordinate(&static_windows_factory_ptr[procedure_id - 1],
                current_geometries_info_procs_data_ptr->current_geos_dimension_variant, current_geometries_info_procs_data_ptr->current_shapes_geometry_variant, &new_clicked_coordinate);

            InvalidateRect(*static_windows_handle_ptr, 0, TRUE);
        }
    }

    // #11 A static class windows procedure to paint custom direct 2d geometries based on the geometries infos container 

    static LRESULT CALLBACK static_windows_proc(HWND current_static_windows_handle , UINT message_type , WPARAM word_parameter , LPARAM long_parameter , UINT_PTR id , DWORD_PTR procs_data) {
        WNDPROC old_windows_procedure{ (WNDPROC)GetWindowLongPtr(current_static_windows_handle , GWLP_USERDATA) };
        static std::pair<unsigned int, unsigned int> current_focused_moving_geometry{ -1 , -1 };
        GeometriesEssentialInfoInProc current_geometries_info_procs_data{};
        TRACKMOUSEEVENT mouse_tracker{ sizeof(TRACKMOUSEEVENT) , TME_LEAVE , current_static_windows_handle , 0 };
        std::vector<StaticWindowsStyle>* static_windows_style_ptr { (std::vector<StaticWindowsStyle>*)GetWindowLongPtr(current_static_windows_handle , GWLP_HINSTANCE) };

        switch (message_type) {

        case WM_PAINT: {
            // #20 Paint statics windows custom direct 2d geometries in a for loop

            PAINTSTRUCT windows_paint_info{};
            HDC static_windows_device_context_handle{ BeginPaint(current_static_windows_handle , &windows_paint_info) };
            static_windows_target_ptr[ id - 1 ]->BeginDraw();
            
            static_windows_target_ptr[id - 1]->Clear((*static_windows_style_ptr)[id - 1].static_windows_background_color);

            ID2D1SolidColorBrush* static_windows_rects_brush_ptr { nullptr };
            static_windows_target_ptr[id - 1]->CreateSolidColorBrush((*static_windows_style_ptr)[id - 1].static_windows_strokes_color, &static_windows_rects_brush_ptr);

            static_windows_target_ptr[id - 1]->DrawRectangle(D2D1::RectF(windows_paint_info.rcPaint.left , windows_paint_info.rcPaint.top , windows_paint_info.rcPaint.right , windows_paint_info.rcPaint.bottom) , static_windows_rects_brush_ptr , (*static_windows_style_ptr)[id - 1].static_windows_strokes_width , 0);

            
            for (int static_windows_geometry{ 0 }; static_windows_geometry < static_windows_custom_direct_2d_geometries[id - 1].size(); ++static_windows_geometry) {
                ID2D1HwndRenderTarget** current_windows_render_target { &static_windows_target_ptr[id - 1] };
                static_windows_custom_direct_2d_geometries[id - 1][static_windows_geometry].get()->get_geometries_essential_info_in_run_time(&current_geometries_info_procs_data);

                std::visit([current_windows_render_target , current_geometries_info_procs_data , id](auto&& dimension , auto&& fill_brush_ptr_ptr, auto&& stroke_brush_ptr_ptr) {
                    
                    using CurrentDimensionType = std::decay_t<decltype(dimension)>;

                    if (*fill_brush_ptr_ptr && *stroke_brush_ptr_ptr) {

                        if constexpr (std::is_same_v<CurrentDimensionType, D2D1_ELLIPSE*>) {
                            D2D1_ELLIPSE fills_dimension_with_distance{ *dimension };
                            fills_dimension_with_distance.radiusX -= static_cast<FLOAT>(current_geometries_info_procs_data.current_fills_distance_from_stroke);
                            fills_dimension_with_distance.radiusY -= static_cast<FLOAT>(current_geometries_info_procs_data.current_fills_distance_from_stroke);
                            (*current_windows_render_target)->FillEllipse(fills_dimension_with_distance, *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawEllipse(dimension, *stroke_brush_ptr_ptr, current_geometries_info_procs_data.current_strokes_width , 0);
                        }

                        else if constexpr (std::is_same_v<CurrentDimensionType, D2D1_RECT_F*>) {
                            D2D1_RECT_F fills_dimension_with_distance{ *dimension };
                            fills_dimension_with_distance.left += current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.top += current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.right -= current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.bottom -= current_geometries_info_procs_data.current_fills_distance_from_stroke;

                            (*current_windows_render_target)->FillRectangle(fills_dimension_with_distance , *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawRectangle(dimension, *stroke_brush_ptr_ptr, current_geometries_info_procs_data.current_strokes_width, 0);
                        }

                        else {
                            D2D1_ROUNDED_RECT fills_dimension_with_distance{ *dimension };
                            FLOAT original_x_centers_corner{ dimension->rect.left + dimension->radiusX };
                            FLOAT original_y_centers_corner{ dimension->rect.top + dimension->radiusY };

                            fills_dimension_with_distance.rect.left += current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.top += current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.right -= current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            fills_dimension_with_distance.rect.bottom -= current_geometries_info_procs_data.current_fills_distance_from_stroke;
                            
                            fills_dimension_with_distance.radiusX = original_x_centers_corner - fills_dimension_with_distance.rect.left;
                            fills_dimension_with_distance.radiusY = original_y_centers_corner - fills_dimension_with_distance.rect.top;


                            (*current_windows_render_target)->FillRoundedRectangle(fills_dimension_with_distance , *fill_brush_ptr_ptr);
                            (*current_windows_render_target)->DrawRoundedRectangle(dimension, *stroke_brush_ptr_ptr, current_geometries_info_procs_data.current_strokes_width, 0);
                        }
                    }


                    ; }, current_geometries_info_procs_data.current_geos_dimension_variant , current_geometries_info_procs_data.current_geometries_brush_variant.first , current_geometries_info_procs_data.current_geometries_brush_variant.second);
            }

            static_windows_target_ptr[ id - 1 ]->EndDraw();
            
            // #20
            break;
        }


        case WM_MOUSEMOVE: {
            TrackMouseEvent(&mouse_tracker);

            if (word_parameter & MK_LBUTTON && (current_focused_moving_geometry.first != -1 && current_focused_moving_geometry.second != -1)) {
                RECT current_static_windows_rects_values{};
                GetWindowRect(current_static_windows_handle , &current_static_windows_rects_values);
                current_static_windows_rects_values = { current_static_windows_rects_values.left + 2 , current_static_windows_rects_values.top + 2 , current_static_windows_rects_values.right - 2 , current_static_windows_rects_values.bottom - 2 };
                ClipCursor(&current_static_windows_rects_values);

                set_moving_geometries_coordinate_in_left_click_events(&current_static_windows_handle , id , static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get(),
                    &current_geometries_info_procs_data, &current_focused_moving_geometry, true);
            }
            
            break;
        }

        case WM_MOUSELEAVE: {
            //current_focused_moving_geometry = { -1 , -1 };
            break;
        }

        case WM_KEYDOWN: {
            
            // #25 UPdate moving geometries position in ( ctrl_type & GeometriesMovingCtrl::ARROW_KEY ) condition  


            if (current_focused_moving_geometry.first != -1 && current_focused_moving_geometry.second != -1) {
                GeometriesMovingCtrl ctrl_type{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_ctrl_type() };
                GeometriesMovingSpace moving_space{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_moving_space_type() };

                if (ctrl_type & GeometriesMovingCtrl::ARROW_KEY) {
                    const MovingGeometriesProp* current_geometries_moving_prop{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_moving_geometries_arrow_key_and_mouse_wheel_direction() };

                    DimensionVariantPtr current_geometries_dimension_ptr{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_dimension_ptr() };
                    GeometriesVariantPtrPtr current_geometries_ptr_ptr { static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_shapes_geometry_ptr_ptr() };
                    D2D1_POINT_2F new_moving_geometries_position{};

                    switch (LOWORD(word_parameter)) {

                    case VK_UP: {
                        change_geometries_position_on_arrow_key(&new_moving_geometries_position, moving_space, &current_geometries_moving_prop->arrow_keys_direction.arrow_up);
                        break;
                    }

                    case VK_LEFT: {
                        change_geometries_position_on_arrow_key(&new_moving_geometries_position, moving_space, &current_geometries_moving_prop->arrow_keys_direction.arrow_left);
                        break;
                    }

                    case VK_RIGHT: {
                        change_geometries_position_on_arrow_key(&new_moving_geometries_position, moving_space, &current_geometries_moving_prop->arrow_keys_direction.arrow_right);
                        break;
                    }
                    case VK_DOWN: {
                        change_geometries_position_on_arrow_key(&new_moving_geometries_position, moving_space, &current_geometries_moving_prop->arrow_keys_direction.arrow_down);
                        break;
                    }

                    }



                    static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->set_geometries_new_coordinate(&static_windows_factory_ptr[id - 1], current_geometries_dimension_ptr, current_geometries_ptr_ptr, &new_moving_geometries_position);

                    InvalidateRect(current_static_windows_handle, 0, TRUE);
                    
                }
            }

            // #25

            break;
        }
            
        case WM_MOUSEWHEEL: {

            // #26 Update moving geometries position im (ctrl_type & GeometriesMovingCtrl::MOUSEWHEEL)

            if (current_focused_moving_geometry.first != -1 && current_focused_moving_geometry.second != -1) {
                GeometriesMovingCtrl ctrl_type{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_ctrl_type() };
                GeometriesMovingSpace moving_space{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_moving_space_type() };
                
                const MovingGeometriesProp* current_geometries_moving_prop{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_moving_geometries_arrow_key_and_mouse_wheel_direction() };


                DimensionVariantPtr current_geometries_dimension_ptr{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_dimension_ptr() };
                GeometriesVariantPtrPtr current_geometries_ptr_ptr{ static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->get_shapes_geometry_ptr_ptr() };

                auto mouse_wheel_value{ GET_WHEEL_DELTA_WPARAM(word_parameter) };

                if (ctrl_type & GeometriesMovingCtrl::MOUSEWHEEL) {
                    D2D1_POINT_2F new_moving_geometries_position{};

                    if (mouse_wheel_value > 0) {
                        change_the_geometries_position_on_mouse_wheel(&new_moving_geometries_position, moving_space , &current_geometries_moving_prop->mouse_wheel_direction.wheel_up);
                    }

                    else {
                        change_the_geometries_position_on_mouse_wheel(&new_moving_geometries_position, moving_space , &current_geometries_moving_prop->mouse_wheel_direction.wheel_down);
                    }

                    static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get()->set_geometries_new_coordinate(&static_windows_factory_ptr[id - 1], current_geometries_dimension_ptr, current_geometries_ptr_ptr, &new_moving_geometries_position);
                }


                InvalidateRect(current_static_windows_handle, 0, TRUE);
            }

            // #26 
            break;
        }

        case WM_LBUTTONDOWN: {
        case WM_LBUTTONDBLCLK : 
            
            // #32 Make current_focused_moving_geometry to -1 , -1 to avoid moving ctrl execute when the previous  windows is not focused 

            if ((id - 1) != current_focused_moving_geometry.first) {
                current_focused_moving_geometry = { -1 , -1 };
            }

            // #32

            // #27 Set a focus value for moving geometry bases on the left clicked event on run time 

            for (unsigned int current_geometry{ 0 }; current_geometry < static_windows_custom_direct_2d_geometries[id - 1].size(); ++current_geometry) {
                
                if (!static_windows_custom_direct_2d_geometries[id - 1][current_geometry].get()->is_static_geometry()) {
                    SetFocus(current_static_windows_handle);
                    POINT current_cursor_position{ get_cursor_position_in_screen_client(&current_static_windows_handle) };
                    static_windows_custom_direct_2d_geometries[id - 1][current_geometry].get()->get_geometries_essential_info_in_run_time(&current_geometries_info_procs_data);
                    std::visit([current_cursor_position , current_geometries_info_procs_data , id , current_geometry](auto&& geometry_ptr_ptr) {
                        BOOL is_fills_clicked{};
                        BOOL is_strokes_clicked{};

                        (*geometry_ptr_ptr)->FillContainsPoint(D2D1::Point2F(current_cursor_position.x, current_cursor_position.y), D2D1::Matrix3x2F::Identity(), &is_fills_clicked);
                        (*geometry_ptr_ptr)->StrokeContainsPoint(D2D1::Point2F(current_cursor_position.x, current_cursor_position.y), current_geometries_info_procs_data.current_strokes_width , 0, D2D1::Matrix3x2F::Identity(), &is_strokes_clicked);

                        if (is_fills_clicked || is_strokes_clicked) {
                            current_focused_moving_geometry = { id - 1 , current_geometry };
                        }

                        ; }, current_geometries_info_procs_data.current_shapes_geometry_variant);
                }
            }
            
            // #27

            // #28 Upadate the moving geometries position in (ctrl_type & GeometriesMovingControl::LEFT_CLICKED)
            
            if (current_focused_moving_geometry.first != -1 && current_focused_moving_geometry.second != -1) {
                set_moving_geometries_coordinate_in_left_click_events(&current_static_windows_handle, id, static_windows_custom_direct_2d_geometries[current_focused_moving_geometry.first][current_focused_moving_geometry.second].get(),
                    &current_geometries_info_procs_data, &current_focused_moving_geometry, false);
            }

            // #28

            break;
        }

        case WM_RBUTTONDOWN : {
        case WM_RBUTTONDBLCLK :
            current_focused_moving_geometry = { -1 , -1 };
            break;
        }

        case WM_LBUTTONUP: {
            ClipCursor(nullptr);
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
    static std::pair<std::vector<HWND>, std::vector<StaticWindowsCustomGeometries>> static_windows_with_custom_direct_2d_geometries{};


    // Sample datas

    /*
    static HWND static_1{};
    static HWND static_2{};
    StaticWindowsCustomGeometries a{};
    StaticWindowsCustomGeometries b{};
    */
    

	switch (message_type) {

    case WM_CREATE: {
        HWND run_time_terminals_windows_handle{ GetConsoleWindow() };
        auto run_time_terminals_windows_rects_values_ptr{ get_screens_coordinate() };
        SetWindowPos(run_time_terminals_windows_handle, 0, run_time_terminals_windows_rects_values_ptr->right - 500, 0, 500, 400, 0);
        
        std::vector<StaticWindowsStyle>* static_windows_info_ptr{ (std::vector<StaticWindowsStyle>*)((LPCREATESTRUCT)long_parameter)->lpCreateParams };

        for (int static_windows_index{ 0 }; static_windows_index < static_windows_info_ptr->size(); ++static_windows_index) {
            
            static_windows_with_custom_direct_2d_geometries.first.push_back(
                CreateWindowEx(
                    0,
                    WC_STATIC,
                    0,
                    WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOTIFY | SS_OWNERDRAW,
                    (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.left,
                    (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.top,
                    (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.right - (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.left,
                    (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.bottom - (*static_windows_info_ptr)[static_windows_index].static_windows_rects_values.top,
                    main_windows_handle,
                    0,
                    ((LPCREATESTRUCT)long_parameter)->hInstance,
                    0
                )
            );
            
            static_windows_with_custom_direct_2d_geometries.second.resize(static_windows_index + 1);
            static_windows_with_custom_direct_2d_geometries.second[static_windows_index](&static_windows_with_custom_direct_2d_geometries.first.back());
            SetWindowLongPtr(static_windows_with_custom_direct_2d_geometries.first.back() , GWLP_HINSTANCE , (LONG_PTR)static_windows_info_ptr);
        }

        static_windows_with_custom_direct_2d_geometries.second[ 0 ].add_custom_direct_2d_geometry<true, GeometriesShape::RECTANGLE, ID2D1LinearGradientBrush*, ID2D1RadialGradientBrush*, GeometriesMovingCtrl::ARROW_KEY, GeometriesMovingSpace::FREE>(
            { 30.0f , 30.0f , 500.0f , 200.0f },
            
            { 
                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.1f , D2D1::ColorF(D2D1::ColorF::RoyalBlue) } , {0.3f , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } , {0.5f , D2D1::ColorF(D2D1::ColorF::DarkViolet) }, {0.6f , D2D1::ColorF(D2D1::ColorF::DeepPink)}} , {0.0f , 30.0f} , {0.0f , 150.0f}},
                20 , 
                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.1f , D2D1::ColorF(D2D1::ColorF::RoyalBlue) } , {0.3f , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } , {0.5f , D2D1::ColorF(D2D1::ColorF::DarkViolet) }, {0.6f , D2D1::ColorF(D2D1::ColorF::DeepPink)}} , D2D1::Point2F(100.0f , 100.0f) , 470.0f , 100.0f } ,
                7.5f ,
                { false , true }
            },

            { {XYDirection::LEFT , XYDirection::UP , XYDirection::RIGHT , XYDirection::DOWN} }
        );

        static_windows_with_custom_direct_2d_geometries.second[0].add_custom_direct_2d_geometry<true, GeometriesShape::ELLIPSE, ID2D1RadialGradientBrush*, ID2D1LinearGradientBrush*, GeometriesMovingCtrl::ARROW_KEY | GeometriesMovingCtrl::LEFT_CLICKED | GeometriesMovingCtrl::LEFT_CLICKED_HOLD
            | GeometriesMovingCtrl::MOUSEWHEEL , GeometriesMovingSpace::X_AXIS_ONLY>(
            
            { D2D1::Point2F(800 , 90.f) , 50.0f , 50.0f },

            {
                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.3f , D2D1::ColorF(D2D1::ColorF::RoyalBlue) } , {0.4f , D2D1::ColorF(D2D1::ColorF::MediumSpringGreen) } , {0.7f , D2D1::ColorF(D2D1::ColorF::BlueViolet) } } , D2D1::Point2F(800 , 90) , 30.0f , 30.0f },

                10 ,

                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.3f , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.6f , D2D1::ColorF(D2D1::ColorF::Orange)} } , {750.0f , 0.0f} , {950.0f , 0.0f}  } ,
                
                4.5f ,

                { true , false }
            },


            { {XYDirection::LEFT , XYDirection::RIGHT , XYDirection::RIGHT , XYDirection::LEFT} , { XYDirection::RIGHT , XYDirection::LEFT } }
        );
        


        static_windows_with_custom_direct_2d_geometries.second[1].add_custom_direct_2d_geometry<true, GeometriesShape::ROUNDED_RECTANGLE, ID2D1LinearGradientBrush*, ID2D1LinearGradientBrush*, GeometriesMovingCtrl::ARROW_KEY | GeometriesMovingCtrl::LEFT_CLICKED_HOLD , GeometriesMovingSpace::FREE>(
            { 30.0f , 30.0f , 600.0f , 200.0f , 100.0f , 130.0f },
            
            {
                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.3f , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.5f , D2D1::ColorF(D2D1::ColorF::Orange)} } , {30.0f, 30.0f} , {100.0f , 100.0f} } ,
                50 ,
                {D2D1_GAMMA_2_2 , D2D1_EXTEND_MODE_MIRROR , { {0.3f , D2D1::ColorF(D2D1::ColorF::BlueViolet) } , {0.5f , D2D1::ColorF(D2D1::ColorF::Orange)} } , {30.0f, 30.0f} , {100.0f , 100.0f} } ,
                6.5f ,
                { true , false }
            },

            { {XYDirection::LEFT , XYDirection::UP , XYDirection::RIGHT , XYDirection::DOWN} }
        );

        break;
	}

    // #29 Handle left clicked hold event outside of static windows in main windows procedure  

    case WM_MOUSEMOVE: {
        break;
    }

    // #29 

	default: {
		return DefWindowProc(main_windows_handle, message_type, word_parameter, long_parameter);
	}

	}

	return 0;
}

// #1


// #2 Execute main windows in run time 

void execute_the_main_window(RECT main_windows_rects_values , COLORREF background_color , std::vector<StaticWindowsStyle> static_windows_style_info) {
    WNDCLASS windows_class{ .lpfnWndProc{ main_windows_proc } , .hInstance{ GetModuleHandle(0) } , .hCursor{ LoadCursor(NULL, IDC_ARROW) } , .hbrBackground{ CreateSolidBrush(background_color) } , .lpszClassName{ TEXT("Main Window") } };
    RegisterClass(&windows_class);
    
    HWND main_windows_handle{ CreateWindowEx(0 ,  windows_class.lpszClassName , TEXT("Windows.exe") , WS_OVERLAPPEDWINDOW | WS_VISIBLE , main_windows_rects_values.left , main_windows_rects_values.top , main_windows_rects_values.right - main_windows_rects_values.left , main_windows_rects_values.bottom - main_windows_rects_values.top , 0 , 0 , windows_class.hInstance , &static_windows_style_info) };

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
    execute_the_main_window({ 0 , 0 , 1050 , 820 }, RGB(0 , 0 , 0), 
    { { {10 , 10 , 1000 , 400 } , D2D1::ColorF(D2D1::ColorF::BlueViolet) , D2D1::ColorF(D2D1::ColorF::DodgerBlue) , 13.5f } , { {10 , 430 , 1000 , 770 } , D2D1::ColorF(D2D1::ColorF::Black) , D2D1::ColorF(D2D1::ColorF::LightBlue) , 13.5f} });


    return 0;
}

// last comment #32

// deleted comments = #15


// https://github.com/camhead2004/D2CG/blob/master/D2CG/main_file.cpp
// https://github.com/camhead2004/D2CG/blob/master/D2CG/direct_2d_objects.hpp
// https://github.com/camhead2004/D2CG/blob/master/D2CG/direct_2d_function.hpp