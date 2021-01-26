#define _ARGBADJUST(a,r,g,b) (((a&0xFF)<<24)|((r&0xff)<<16)|((g&0xff)<<8)|((b&0xff)))

#define COLOR_BLACK		_ARGBADJUST(255,0,0,0)
#define COLOR_RED		_ARGBADJUST(255,255,0,0)
#define COLOR_GREEN		_ARGBADJUST(255,0,255,0)
#define COLOR_BLUE		_ARGBADJUST(255,0,0,255)
#define COLOR_PURPLE	_ARGBADJUST(255,200,50,175)
#define COLOR_WHITE		_ARGBADJUST(255,255,255,255)
#define COLOR_UPD_TEXT  _ARGBADJUST(255,0,0,255)

#define UPD_BACK_GROUND_COLOR		COLOR_BLACK

#define UPD_MAX_LINE_WIDTH			48
//#define LCD_PANEL_WIDTH             800
//#define LCD_PANEL_HEIGHT            480
#define PROGRESS_BAR_HEIGHT         30
#define PROGRESS_BAR_WIDTH          600


// Resolution types
typedef enum{
    RESOL_UNKNOWN=0,
    RESOL_320_240,
    RESOL_480_272
} RESOL_TYPE_et;

// Text position
typedef enum{
    POS_UNKNOWN=0,
    POS_LEFT,
    POS_RIGHT,
    POS_TOP,
    POS_BOTTOM,
    POS_CENTER
} POS_TYPE_et;


extern void UPDPrintLineSize(char *pu8Str, unsigned int u32TextColor, unsigned short u16Size, unsigned char u8Posx, unsigned char u8LineNumber);



