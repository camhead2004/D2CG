# ifndef MAIN_FILE_HPP
# define MAIN_FILE_HPP

# include <windows.h>
# include <d2d1.h>

struct StaticWindowsStyle {
    RECT static_windows_rects_values{};
    D2D1_COLOR_F static_windows_background_color{};
    D2D1_COLOR_F static_windows_strokes_color{};
    FLOAT static_windows_strokes_width{ 0.0f };
};


# endif