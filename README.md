D2CG (Direct 2D Custom Geometries)

****Attention*****
This procject is preface for our future projects and its not a practical projects on its own. 

D2CG Explanation :
This is a side project that lets have geometries like  Rectangle , Rounded Rectangle and Ellipse with the custom styling that you can choose , and draw those geometries in a WS_OVERLAPPEDWINDOW style , in its window procedure in WM_PAINT event at run time by using Windows SDK and using headers like <windows.h> for making a window procedure in run time and use Direct2D libraries like <d2d1.h> and <d2d1helper.h> to use its coloring and styling features and drawing those geometries.

You can have multiple static class windows (WC_STATIC) in a main window . furthermore , you can set static windows top-left position , width , height , backgrounds color , strokes color and also strokes width for every single window . 
Each static class windows can have their own geometries , and each geometries can have their own dimensions , shapes , coloring and be in a static position or change its position by Left Click , Mouse Wheel , Arrow Key controls in WM_MOUSEMOVE , WM_MOUSEWHEEL and WM_KEYDOWN event in run time .

For geometries styling , the project try its best to make an diverse customization from setting dimension to setting geometries color style (solid , linear-gradient , radial-gradient). 


These are some the customization option for every geometries that project provide : 

- Ellipse :
  Able to set ellipse center and ellipse x radius and y radius.

- Rectangle : Able to set rectangles top-left and right-bottom position.

- Rounded Rectangle : Like Rectangle plus you can set how much the rectangle can be rounded in both x and y axis.

Similar customization : 

-Colring : 
Each geometries can have their own strokes color and fills color and you set strokes width for each geometry and also you can set how filling color distance can be from the geometries stroke and the space between the geometries fills and strokes will be colored by static windows background color. 

-Coloring Style :
The two part of the Direct2D geometry (fill and stroke) , each of them can be colored in a solid , linear gradient and radial gradient way 

For linear gradinet you set StarPoint and EndPoint , to express lienr gradinets coloring (e.g .StartPoint{ 100 , 100 } , .EndPoint{ 200 , 200 } = diagonal linear gradinet , 
.StartPoint{750.0f , 0.0f} , .EndPoint{950.0f , 0.0f} = vetical gradient , .StartPoint{0.0f , 40.0f} , .EndPoint{0.0f , 140.0f} = horizontal gradient)




  
